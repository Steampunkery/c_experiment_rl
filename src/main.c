#include "component.h"
#include "rogue.h"

#include "map.h"
#include "player.h"
#include "monster.h"
#include "render.h"
#include "socket_menu.h"
#include "systems.h"
#include "ai.h"
#include "gui.h"
#include "log.h"
#include "prefab.h"
#include "item.h"
#include "arena.h"
#include "serialize.h"

#include "rlsmenu.h"
#include "flecs.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "uncursed.h"
#include <signal.h>

CommandType get_command(KeyInfo *key, int msec_timeout);
void temp_map_init(ecs_world_t *world, Map *map);
void render_and_sock_menus(GameVars *vars);

static ecs_world_t *world;
ecs_entity_t g_player_id;

int X_DIRS[] = { 1, 0, -1, 0, 1, -1, -1, 1 };
int Y_DIRS[] = { 0, 1, 0, -1, 1, 1, -1, -1 };

void try_endwin(int)
{
    endwin();
}

// TODO: Refactor this while damn file
int main(int argc, char **argv)
{
    /*for (volatile int i = 0; i == 0;);*/

    srand(time(NULL));

    initialize_uncursed(&argc, argv);
    initscr();
    signal(SIGABRT, try_endwin);

    if (LINES < MIN_TERM_LINES || COLS < MIN_TERM_COLS) {
        printf("Terminal must be at least %dx%d\n", MIN_TERM_COLS, MIN_TERM_LINES);
        goto uncursed_done;
    }

    refresh();
    curs_set(0);
    WINDOW *basewin = newwin(LINES - 4, COLS, 3, 0);
    WINDOW *logwin = newwin(3, COLS, 0, 0);
    WINDOW *statuswin = newwin(1, COLS, LINES - 1, 0);

    init_logger(&g_game_log);
    init_logger(&g_debug_log);
    log_msg(&g_game_log, L"Test Message");
    log_msg(&g_debug_log, L"Debug Test Message");

    world = ecs_init();
    ecs_log_enable_colors(false);
    ecs_os_api_t os_api = ecs_os_get_api();
    os_api.log_out_ = fopen("ecs.log", "w");
    ecs_os_set_api(&os_api);

    /*ECS_IMPORT(world, FlecsStats);*/
    /*ecs_singleton_set(world, EcsRest, {0});*/

    register_components(world);
    register_systems(world);
    register_prefabs(world);
    register_serialize(world);

    dijkstra_init(world);
    Map *map = ecs_singleton_ensure(world, Map);
    new_map(map, LINES - 4, COLS);

    item_init(world);
    gui_init();

    rlsmenu_gui gui;
    rlsmenu_gui_init(&gui);
    arena frame_arena = new_arena(1 << 12);
    if (!frame_arena.beg)
        goto done;

    temp_map_init(world, map);

    // Call this twice. Once to clean, twice to delete
    ecs_delete_empty_tables(world, 0, 1, 1, 0, 0);
    int del = ecs_delete_empty_tables(world, 0, 1, 1, 0, 0);
    log_msg(&g_debug_log, L"Deleted %d empty tables", del);

    GameVars vars = {
        .state = PreTurn,
        .basewin = basewin,
        .logwin = logwin,
        .statuswin = statuswin,
        .gui = &gui
    };

    // Put state variables here.
    // TODO!: Make these into a struct
    KeyInfo key = { 0 };
    while (true) {
        /*ecs_progress(world, 0.0);*/
        switch (vars.state) {
        case PreTurn:
            ecs_run(world, initiative, 0.0, NULL);
            vars.state = ecs_is_enabled(world, g_player_id, MyTurn) ? PlayerTurn : RunSystems;
            break;
        case PlayerTurn:
            ecs_run(world, render, 0.0, &vars);

            // TODO: Find a better way to handle the player's turn elegantly
            do {
                /*ecs_progress(world, 0.0);*/
                handle_socket_menus();
                if (ecs_is_enabled(world, g_player_id, ActionFromSocket)) {
                    ecs_enable_component(world, g_player_id, ActionFromSocket, false);
                    vars.state = RunSystems;
                    break;
                }
            } while ((vars.state = process_player_input(world, &key)) == PlayerTurn);

            break;
        case RunSystems:
            ecs_run(world, render, 0.0, &vars);
            update_dijkstra_maps(world, map);
            ecs_run(world, ecs_id(AI), 0.0, NULL);

            ecs_run(world, ecs_id(ApplyPoison), 0.0, NULL);
            ecs_run(world, ecs_id(ApplyOnFire), 0.0, NULL);
            ecs_run(world, ecs_id(ProcessStatusEffects), 0.0, NULL);

            ecs_run(world, ecs_id(DeathCleanup), 0.0, NULL);

            vars.state = PreTurn;
            break;
        case NewGUIFrame:
            FrameData *data = gui_state_for(key.key);
            if (data->data_id_type == DATA_ID_PLAYER_TARGET)
                data->data_id_arg.pl = g_player_id;

            if (!data->prep_frame(data, world, frame_arena)) {
                vars.state = PlayerTurn;
                break;
            }

            rlsmenu_gui_push(&gui, data->frame);
            render_and_sock_menus(&vars);

            vars.state = GUI;
            // FALLTHROUGH
        case GUI:
            get_command(&key, -1);
            enum rlsmenu_result res = rlsmenu_update(&gui, translate_key(&key));
            render_and_sock_menus(&vars);

            // Only process results and exit GUI state when last frame completes
            if (gui.frame_stack)
                break;

            switch (res) {
            case RLSMENU_DONE:
                FrameData *fd = rlsmenu_pop_return(&gui);
                assert(fd);
                vars.state = fd->consumes_turn ? RunSystems : PlayerTurn;
                break;
            case RLSMENU_CANCELED:
                vars.state = PlayerTurn;
                break;
            case RLSMENU_CONT:
            }
            break;
        case Quit:
            goto done;
        }
    }

done:
    close_all_socket_menus();
    destroy_map(map);
    rlsmenu_gui_deinit(&gui);
    ecs_fini(world);
uncursed_done:
    delwin(basewin);
    delwin(logwin);
    endwin();

    return 0;
}

void temp_map_init(ecs_world_t *world, Map *map)
{
    g_player_id = init_player(world);

    ecs_entity_t e;
    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 20 }));
    map_place_entity(world, map, e, 40, 20);

    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 21 }),
            ecs_value(AIController, { AIC_LEFT_WALKER }));
    map_place_entity(world, map, e, 40, 21);

    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 23 }),
            ecs_value(AIController, { AIC_LEFT_WALKER }));
    map_place_entity(world, map, e, 40, 23);

    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 22 }),
            ecs_value(AIController, { AIC_GREEDY }), { ecs_id(Invisible), NULL });
    map_place_entity(world, map, e, 40, 22);

    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 40 }),
            ecs_value(AIController, { AIC_ENEMY }),
            ecs_value(EnemyAIParams, { .health_flee_p = 0.5}));
    map_place_entity(world, map, e, 40, 40);

    e = ecs_insert(world, { ecs_isa(Dog), NULL }, ecs_value(Position, { 10, 20 }),
            ecs_value(AIController, { AIC_PET }));
    map_place_entity(world, map, e, 10, 20);

    ecs_world_from_json_file(world, "items.json", NULL);
    ecs_query_t *q = ecs_query(world, {
            .terms = {
                { .id = ecs_isa(Item) },
                { .id = ecs_id(Position) }
            }
    });

    ecs_iter_t it = ecs_query_iter(world, q);
    while (ecs_query_next(&it)) {
        Position const *pos = ecs_field(&it, Position, 1);
        for (int i = 0; i < it.count; i++)
            place_item(world, it.entities[i], pos[i].x, pos[i].y);
    }

    ecs_query_fini(q);
}

void render_and_sock_menus(GameVars *vars)
{
    ecs_run(world, render, 0.0, vars);
    handle_socket_menus();
}
