#include "rogue.h"

#include "observer.h"
#include "map.h"
#include "player.h"
#include "monster.h"
#include "render.h"
#include "systems.h"
#include "ai.h"
#include "gui.h"
#include "log.h"

#include "rlsmenu.h"
#include "flecs.h"
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <uncursed/uncursed.h>
#include <signal.h>
#include <glib.h>

CommandType get_command(KeyInfo *key);
void temp_arena_init(ecs_world_t *world, Map *map);

static ecs_world_t *world;
ecs_entity_t g_player_id;

int X_DIRS[] = { 1, 0, -1, 0, 1, -1, -1, 1 };
int Y_DIRS[] = { 0, 1, 0, -1, 1, 1, -1, -1 };

int main(int argc, char **argv)
{
    // raise(SIGSTOP);
    initialize_uncursed(&argc, argv);
    initscr();

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

    world = ecs_init();

    register_components(world);
    register_systems(world);
    register_observers(world);

    Map *map = ecs_singleton_get_mut(world, Map);
    new_arena(map, LINES - 2, COLS);

    Logger *logger = ecs_singleton_get_mut(world, Logger);
    init_logger(logger);
    log_msg(logger, L"Test Message");

    rlsmenu_gui *gui = ecs_singleton_get_mut(world, rlsmenu_gui);
    rlsmenu_gui_init(gui);

    temp_arena_init(world, map);

    // Put state variables here.
    // TODO!: Make these into a struct
    GameState state = TakeInput;
    KeyInfo key;
    while (true) {
        ecs_run(world, render, 0.0, &game_windows);

        switch (state) {
        case TakeInput:
            CommandType cmd = get_command(&key);
            bool is_player_turn = ecs_has_id(world, g_player_id, MyTurn);
            if (cmd == GUICommand || (cmd == PlayerGUICommand && is_player_turn)) {
                state = NewGUIFrame;
                break;
            } else if (cmd == QuitCommand)
                goto done;

            // process_player_input returns whether the players turn is done
            if (process_player_input(world, key)) {
                state = RunSystems;
                break;
            }

            break;
        case RunSystems:
            update_dijkstra_maps(world, map);
            ecs_run(world, ecs_id(AI), 0.0, NULL);
            ecs_run(world, ecs_id(Move), 0.0, NULL);
            ecs_run(world, ecs_id(Pickup), 0.0, NULL);
            ecs_run(world, ecs_id(Drop), 0.0, NULL);
            ecs_run(world, ecs_id(Prayer), 0.0, NULL);

            state = TakeInput;
            break;
        case NewGUIFrame:
            int idx = alpha_to_idx(key.key);
            assert(idx >= 0);

            if (!gui_state[idx].prep_frame(&gui_state[idx], world)) {
                state = TakeInput;
                continue;
            }
            rlsmenu_gui_push(gui, gui_state[idx].frame);

            state = GUI;
            // FALLTHROUGH
        case GUI:
            get_command(&key);
            enum rlsmenu_result res = rlsmenu_update(gui, translate_key(&key));
            if (res == RLSMENU_DONE || res == RLSMENU_CANCELED) {
                FrameData *fd = rlsmenu_pop_return(gui);
                state = fd ? (fd->consumes_turn ? RunSystems : TakeInput) : TakeInput;
            }
            break;
        }
    }

done:
    destroy_map(map);
    rlsmenu_gui_deinit(gui);
    ecs_fini(world);
uncursed_done:
    delwin(basewin);
    delwin(logwin);
    endwin();

    return 0;
}

CommandType get_command(KeyInfo *key)
{
    wint_t c;
    int ret = timeout_get_wch(10, &c);

    key->status = ret;
    key->key = c;
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
        case 'i':
            return GUICommand;
        default:
            return HeroCommand;
        }
    case ERR:
        return InvalidCommand;
    }

    return InvalidCommand;
}

void temp_arena_init(ecs_world_t *world, Map *map)
{
    g_player_id = init_player(world);

    ecs_entity_t goblin1 = init_goblin(world, 40, 20);
    (void) goblin1;

    ecs_entity_t goblin2 = init_goblin(world, 40, 21);
    ecs_set(world, goblin2, AIController, { left_walker, NULL });

    ecs_entity_t goblin3 = init_goblin(world, 40, 22);
    ecs_set(world, goblin3, AIController, { greedy_ai, map });
    make_invisible(world, goblin3);

    ecs_entity_t gold1 = create_item(world, '$',
                                     (Item *) &(GoldItem){
                                             .super = { ITEM_TYPE_GOLD, L"Gold" },
                                             .amount = 300,
                                     },
                                     sizeof(GoldItem));
    place_item(world, gold1, 1, 1);

    ecs_entity_t gold2 = create_item(world, '$',
                                     (Item *) &(GoldItem){
                                             .super = { ITEM_TYPE_GOLD, L"Gold" },
                                             .amount = 300,
                                     },
                                     sizeof(GoldItem));
    place_item(world, gold2, map->cols - 2, 1);

    ecs_entity_t gold3 = create_item(world, '$',
                                     (Item *) &(GoldItem){
                                             .super = { ITEM_TYPE_GOLD, L"Gold" },
                                             .amount = 300,
                                     },
                                     sizeof(GoldItem));
    place_item(world, gold3, map->cols - 2, map->rows - 2);

    ecs_entity_t gold4 = create_item(world, '$',
                                     (Item *) &(GoldItem){
                                             .super = { ITEM_TYPE_GOLD, L"Gold" },
                                             .amount = 300,
                                     },
                                     sizeof(GoldItem));
    place_item(world, gold4, 1, map->rows - 2);

    ecs_entity_t item1 = create_item(world, 'a',
                                     (Item *) &(FoodItem){
                                             .super = { ITEM_TYPE_FOOD, L"Apple" },
                                             .satiation = 42,
                                     },
                                     sizeof(FoodItem));
    place_item(world, item1, 18, 18);

    ecs_entity_t item2 = create_item(world, 'o',
                                     (Item *) &(FoodItem){
                                             .super = { ITEM_TYPE_FOOD, L"Orange" },
                                             .satiation = 42,
                                     },
                                     sizeof(FoodItem));
    place_item(world, item2, 18, 19);

    ecs_entity_t item3 = create_item(world, 'b',
                                     (Item *) &(FoodItem){
                                             .super = { ITEM_TYPE_FOOD, L"Banana" },
                                             .satiation = 42,
                                     },
                                     sizeof(FoodItem));
    place_item(world, item3, 18, 20);

    ecs_entity_t item4 = create_item(world, 'k',
                                     (Item *) &(FoodItem){
                                             .super = { ITEM_TYPE_FOOD, L"Kiwi" },
                                             .satiation = 42,
                                     },
                                     sizeof(FoodItem));
    place_item(world, item4, 18, 21);
}
