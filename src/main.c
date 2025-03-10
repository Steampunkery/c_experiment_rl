#include "component.h"
#include "rogue.h"

#include "observer.h"
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

#include "rlsmenu.h"
#include "flecs.h"

#include <stdlib.h>
#include <stdbool.h>
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

    register_components(world);
    register_systems(world);
    register_observers(world);
    register_prefabs(world);

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
        switch (vars.state) {
        case PreTurn:
            ecs_run(world, initiative, 0.0, NULL);
            vars.state = ecs_is_enabled(world, g_player_id, MyTurn) ? PlayerTurn : RunSystems;
            break;
        case PlayerTurn:
            ecs_run(world, render, 0.0, &vars);

            // TODO: Find a better way to handle the player's turn elegantly
            do {
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
            ecs_run(world, ecs_id(StatusEffectTimer), 0.0, NULL);

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
            ecs_value(AIController, { left_walker, NULL }));
    map_place_entity(world, map, e, 40, 21);

    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 23 }),
            ecs_value(AIController, { left_walker, NULL }));
    map_place_entity(world, map, e, 40, 23);

    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 22 }),
            ecs_value(AIController, { greedy_ai, NULL }), { Invisible, NULL });
    map_place_entity(world, map, e, 40, 22);

    // Big hack to get around lifetime rules and allocation
    static EnemyAIParams melee_flee = { .health_flee_p = 0.5 };
    e = ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 40 }),
            ecs_value(AIController, { enemy_ai, &melee_flee}));
    map_place_entity(world, map, e, 40, 40);

    e = ecs_insert(world, { ecs_isa(Dog), NULL }, ecs_value(Position, { 10, 20 }),
            ecs_value(AIController, { pet_ai, NULL }));
    map_place_entity(world, map, e, 10, 20);

    ecs_entity_t gold1 = ecs_insert(world, { ecs_isa(GoldItem), NULL }, ecs_value(Stack, { 300 }));
    place_item(world, gold1, 1, 1);

    ecs_entity_t gold2 = ecs_insert(world, { ecs_isa(GoldItem), NULL }, ecs_value(Stack, { 300 }));
    place_item(world, gold2, map->cols - 2, 1);

    ecs_entity_t gold3 = ecs_insert(world, { ecs_isa(GoldItem), NULL }, ecs_value(Stack, { 300 }));
    place_item(world, gold3, map->cols - 2, map->rows - 2);

    ecs_entity_t gold4 = ecs_insert(world, { ecs_isa(GoldItem), NULL }, ecs_value(Stack, { 300 }));
    place_item(world, gold4, 1, map->rows - 2);

    ecs_entity_t item1 = ecs_insert(world,
            { ecs_isa(FoodItem), NULL },
            ecs_value(Satiation, { 42 }),
            ecs_value(Name, { L"Apple" })
    );
    place_item(world, item1, 18, 18);

    ecs_entity_t item2 = ecs_insert(world,
            { ecs_isa(FoodItem), NULL },
            ecs_value(Satiation, { 42 }),
            ecs_value(Name, { L"Orange" })
    );
    place_item(world, item2, 18, 19);

    ecs_entity_t item3 = ecs_insert(world,
            { ecs_isa(FoodItem), NULL },
            ecs_value(Satiation, { 42 }),
            ecs_value(Name, { L"Banana" })
    );
    place_item(world, item3, 18, 20);

    ecs_entity_t item4 = ecs_insert(world,
            { ecs_isa(FoodItem), NULL },
            ecs_value(Satiation, { 42 }),
            ecs_value(Name, { L"Kiwi" })
    );
    place_item(world, item4, 18, 21);

    ecs_entity_t poison_potion1 = ecs_insert(world,
            { ecs_isa(QuaffableItem), NULL },
            ecs_value_pair_2nd(HasQuaffEffect, TimedStatusEffect, {
                .turns = 10,
                .effect_comp = ecs_id(Poison)
            }),
            ecs_value(Name, { L"Potion of Poison" })
    );
    place_item(world, poison_potion1, 18, 23);

    ecs_entity_t poison_potion2 = ecs_insert(world,
            { ecs_isa(QuaffableItem), NULL },
            ecs_value_pair_2nd(HasQuaffEffect, TimedStatusEffect, {
                .turns = 10,
                .effect_comp = ecs_id(Poison)
            }),
            ecs_value(Name, { L"Potion of Poison" })
    );
    place_item(world, poison_potion2, 18, 24);

    ecs_entity_t health_potion = ecs_insert(world,
            { ecs_isa(QuaffableItem), NULL },
            ecs_value_pair_2nd(HasQuaffEffect, EntityCallbackEffect, {
                .f = health_potion_cb,
                .arg.c = 20
            }),
            ecs_value(Name, { L"Potion of Health" })
    );
    place_item(world, health_potion, 18, 25);
}

void render_and_sock_menus(GameVars *vars)
{
    ecs_run(world, render, 0.0, vars);
    handle_socket_menus();
}
