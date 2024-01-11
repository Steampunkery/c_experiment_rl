#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <uncursed/uncursed.h>
#include <signal.h>
#include <glib.h>

#include "component.h"
#include "observer.h"
#include "input.h"
#include "flecs.h"
#include "map.h"
#include "player.h"
#include "monster.h"
#include "render.h"
#include "rogue.h"
#include "systems.h"
#include "ai.h"
#include "gui.h"
#include "log.h"
#include "item.h"

CommandType get_command(KeyInfo *key);
void temp_arena_init(ecs_world_t *world);

static ecs_world_t *world;
ecs_entity_t g_player_id;
char *idx_to_alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int X_DIRS[] = { 1, 0, -1, 0, 1, -1, -1, 1 };
int Y_DIRS[] = { 0, 1, 0, -1, 1, 1, -1, -1 };

int main(int argc, char **argv) {
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
    log_msg(logger, "Test Message");

    GuiStack *gui_stack = ecs_singleton_get_mut(world, GuiStack);
    gui_stack->frames = NULL;

    temp_arena_init(world);

    // Put state variables here.
    // TODO!: Make these into a struct
    GameState state = TakeInput;
    GuiFrame *curr_gui_frame = NULL;
    KeyInfo key;
    while (true) {
        ecs_run(world, render, 0.0, &game_windows);

        switch (state) {
            case TakeInput:
                CommandType cmd = get_command(&key);
                bool is_player_turn = ecs_has_id(world, g_player_id, MyTurn);
                if (cmd == GUICommand ||
                   (cmd == PlayerGUICommand && is_player_turn)) {
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

                // TODO: Copy structs so more than one can
                // exist in the stack at a time
                gui_stack->frames = g_slist_append(gui_stack->frames, &gui_frames[idx]);
                curr_gui_frame = &gui_frames[idx];

                state = GUI;
                // FALLTHROUGH
            case GUI:
                get_command(&key);
                CommandType resp = curr_gui_frame->cb(world, curr_gui_frame, &key);

                if (resp != CancelCommand && resp != SuccessCommand)
                    break;

                GSList *last = g_slist_last(gui_stack->frames);
                assert(last);

                GuiFrame *frame = last->data;
                g_free(frame->content);
                frame->content = NULL;
                bool should_consume_turn = frame->consumes_turn && resp == SuccessCommand;

                gui_stack->frames = g_slist_remove_link(gui_stack->frames, last);
                g_slist_free_1(last);

                if (!gui_stack->frames) {
                    state = should_consume_turn ? RunSystems : TakeInput;
                    curr_gui_frame = NULL;
                    break;
                }

        }
    }

done:
    destroy_map(map);
    ecs_fini(world);
uncursed_done:
    delwin(basewin);
    delwin(logwin);
    endwin();

    return 0;
}

CommandType get_command(KeyInfo *key) {
    int ret;
    wint_t c;
    ret = timeout_get_wch(10, &c);
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
}

void temp_arena_init(ecs_world_t *world) {
    g_player_id = init_player(world);

    ecs_entity_t goblin1 = init_goblin(world, 40, 20);

    ecs_entity_t goblin2 = init_goblin(world, 40, 21);
    ecs_set(world, goblin2, AIController, { left_walker });

    ecs_entity_t goblin3 = init_goblin(world, 40, 22);
    make_invisible(world, goblin3);

    ecs_entity_t item1 = create_item(world, '$', &(GoldItem) {
            .super = { ITEM_TYPE_GOLD, "Gold" },
            .amount = 300,
    }, sizeof(GoldItem));
    place_item(world, item1, 18, 18);

    ecs_entity_t item1a = create_item(world, 'a', &(FoodItem) {
            .super = { ITEM_TYPE_FOOD, "Apple" },
            .satiation = 42,
    }, sizeof(FoodItem));
    place_item(world, item1a, 18, 18);

    ecs_entity_t item2 = create_item(world, 'o', &(FoodItem) {
            .super = { ITEM_TYPE_FOOD, "Orange" },
            .satiation = 42,
    }, sizeof(FoodItem));
    place_item(world, item2, 18, 19);

    ecs_entity_t item3 = create_item(world, 'b', &(FoodItem) {
            .super = { ITEM_TYPE_FOOD, "Banana" },
            .satiation = 42,
    }, sizeof(FoodItem));
    place_item(world, item3, 18, 20);

    ecs_entity_t item4 = create_item(world, 'k', &(FoodItem) {
            .super = { ITEM_TYPE_FOOD, "Kiwi" },
            .satiation = 42,
    }, sizeof(FoodItem));
    place_item(world, item4, 18, 21);
}
