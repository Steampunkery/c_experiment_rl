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
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <uncursed/uncursed.h>
#include <glib.h>

CommandType get_command(KeyInfo *key, int msec_timeout);
void temp_map_init(ecs_world_t *world, Map *map);
void render_and_sock_menus(WindowHolder *wh);

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
    initialize_uncursed(&argc, argv);
    initscr();
    signal(SIGABRT, try_endwin);

    if (LINES < MIN_TERM_LINES || COLS < MIN_TERM_COLS) {
        printf("Terminal must be at least %dx%d\n", MIN_TERM_COLS, MIN_TERM_LINES);
        goto uncursed_done;
    }

    refresh();
    curs_set(0);
    WINDOW *basewin = newwin(LINES - 2, COLS, 1, 0);
    WINDOW *logwin = newwin(1, COLS, 0, 0);

    WindowHolder game_windows = {
        .base = basewin,
        .log = logwin,
    };

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
    new_map(map, LINES - 2, COLS);

    item_init(world);
    gui_init();

    rlsmenu_gui *gui = ecs_singleton_ensure(world, rlsmenu_gui);
    rlsmenu_gui_init(gui);
    arena frame_arena = new_arena(1 << 12);
    if (!frame_arena.beg)
        goto done;

    temp_map_init(world, map);

    // Put state variables here.
    // TODO!: Make these into a struct
    GameState state = PreTurn;
    KeyInfo key = { 0 };
    while (true) {
        switch (state) {
        case PreTurn:
            ecs_run(world, initiative, 0.0, NULL);
            state = ecs_is_enabled(world, g_player_id, MyTurn) ? PlayerTurn : RunSystems;
            break;
        case PlayerTurn:
            ecs_run(world, render, 0.0, &game_windows);

            // TODO: Find a better way to handle the player's turn elegantly
            do {
                handle_socket_menus();
                if (ecs_is_enabled(world, g_player_id, ActionFromSocket)) {
                    ecs_enable_component(world, g_player_id, ActionFromSocket, false);
                    state = RunSystems;
                    break;
                }

                CommandType cmd = get_command(&key, 7);
                if (cmd == GUICommand || cmd == PlayerGUICommand) {
                    state = NewGUIFrame;
                    break;
                } else if (cmd == QuitCommand) {
                    goto done;
                }

                state = RunSystems;
                // process_player_input returns whether the players turn is done
            } while (!process_player_input(world, key));

            break;
        case RunSystems:
            ecs_run(world, render, 0.0, &game_windows);
            update_dijkstra_maps(world, map);
            ecs_run(world, ecs_id(AI), 0.0, NULL);
            ecs_run(world, ecs_id(Move), 0.0, NULL);
            ecs_run(world, ecs_id(Pickup), 0.0, NULL);
            ecs_run(world, ecs_id(Drop), 0.0, NULL);
            ecs_run(world, ecs_id(Prayer), 0.0, NULL);
            ecs_run(world, ecs_id(Quaff), 0.0, NULL);

            ecs_run(world, ecs_id(ApplyPoison), 0.0, NULL);
            ecs_run(world, ecs_id(StatusEffectTimer), 0.0, NULL);

            state = PreTurn;
            break;
        case NewGUIFrame:
            int idx = alpha_to_idx(key.key);
            assert(idx >= 0);

            if (gui_state[idx].data_id_type == DATA_ID_PLAYER_TARGET)
                gui_state[idx].data_id_arg.pl = g_player_id;

            if (!gui_state[idx].prep_frame(&gui_state[idx], world, frame_arena)) {
                state = PlayerTurn;
                continue;
            }
            rlsmenu_gui_push(gui, gui_state[idx].frame);
            render_and_sock_menus(&game_windows);

            state = GUI;
            // FALLTHROUGH
        case GUI:
            get_command(&key, -1);
            enum rlsmenu_result res = rlsmenu_update(gui, translate_key(&key));
            render_and_sock_menus(&game_windows);

            switch (res) {
            case RLSMENU_DONE:
                FrameData *fd = rlsmenu_pop_return(gui);
                assert(fd);
                state = fd->consumes_turn ? RunSystems : PlayerTurn;
                break;
            case RLSMENU_CANCELED:
                state = PlayerTurn;
                break;
            case RLSMENU_CONT:;
            }
            break;
        }
    }

done:
    close_all_socket_menus();
    destroy_map(map);
    rlsmenu_gui_deinit(gui);
    ecs_fini(world);
uncursed_done:
    delwin(basewin);
    delwin(logwin);
    endwin();

    return 0;
}

// TODO: Enumerating menus like this is SHIT
CommandType get_command(KeyInfo *key, int msec_timeout)
{
    wint_t c = 0;
    int ret = timeout_get_wch(msec_timeout, &c);

    key->status = ret;
    key->key = c;

    if (c == KEY_HANGUP)
        assert(!"KEY_HANGUP Received! Very bad!");

    switch (ret) {
        case OK:
        case KEY_CODE_YES:
            if (c == KEY_ESCAPE)
                return QuitCommand;
            else if (c == KEY_INVALID)
                return InvalidCommand;

            switch (c) {
                case 'd':
                    return PlayerGUICommand;
                case 'q':
                    return PlayerGUICommand;
                case 'i':
                    return GUICommand;
                case 'm':
                    return GUICommand;
                case 'D':
                    return GUICommand;
                case 'G':
                    return GUICommand;
                default:
                    return HeroCommand;
            }
        case ERR:
            return InvalidCommand;
    }

    return InvalidCommand;
}

void temp_map_init(ecs_world_t *world, Map *map)
{
    g_player_id = init_player(world);

    ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 20 }));
    ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 21 }),
            ecs_value(AIController, { left_walker, NULL }));
    ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 23 }),
            ecs_value(AIController, { left_walker, NULL }));
    ecs_insert(world, { ecs_isa(Goblin), NULL }, ecs_value(Position, { 40, 22 }),
            ecs_value(AIController, { greedy_ai, map }), { Invisible, NULL });

    ecs_insert(world, { ecs_isa(Dog), NULL }, ecs_value(Position, { 10, 20 }),
            ecs_value(AIController, { pet_ai, map }));

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
}

void render_and_sock_menus(WindowHolder *wh)
{
    ecs_run(world, render, 0.0, wh);
    handle_socket_menus();
}
