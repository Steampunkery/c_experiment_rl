#include "gui.h"

#include "component.h"
#include "log.h"
#include "socket_menu.h"

#include "rlsmenu.h"
#include "sockui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static enum rlsmenu_cb_res drop_item_cb(rlsmenu_frame *frame, void *e);
static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *e);
static void slist_cleanup_cb1(rlsmenu_frame *frame);
static void slist_cleanup_cb2(rlsmenu_frame *frame);
static void msgbox_cleanup_cb1(rlsmenu_frame *frame);
static void on_complete(rlsmenu_frame *frame);
static bool prep_inv_frame(FrameData *data, ecs_world_t *world);
static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world);
static bool prep_logger_frame(FrameData *data, ecs_world_t *world);
static uint32_t get_inv_data_id(FrameData *data);
static uint32_t get_logger_data_id(FrameData *data);

rlsmenu_slist drop_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .title = L"Drop",
            .state = &gui_state[alpha_to_idx('d')],
            .cbs = &(rlsmenu_cbs) { drop_item_cb, on_complete, slist_cleanup_cb1 },
        },

        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    },
};

rlsmenu_list inv_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_LIST,
            .flags = RLSMENU_BORDER,
            .title = L"Inventory",
            .state = &gui_state[alpha_to_idx('i')],
            .cbs = &(rlsmenu_cbs) { NULL, on_complete, slist_cleanup_cb1 },
        },

        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    },
};

rlsmenu_slist sock_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .title = L"Socketize Menu",
            .state = &gui_state[alpha_to_idx('m')],
            .cbs = &(rlsmenu_cbs) { socket_menu_cb, on_complete, slist_cleanup_cb2 },
        },

        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    },
};

rlsmenu_msgbox debug_log_frame = {
    .frame = {
        .type = RLSMENU_MSGBOX,
        .flags = 0,
        .title = L"Debug Log",
        .state = &gui_state[alpha_to_idx('D')],
        .cbs = &(rlsmenu_cbs) { NULL, on_complete, msgbox_cleanup_cb1 },
    },

    .lines = NULL,
    .n_lines = 0,
};

/* Array of data to pass along with frame. For now, this bool indicates if
 * the completion of the frame should pass the player's turn. Suject to
 * change.
 */
// FIXME: Make a struct that holds the filter and action and shit
FrameData gui_state[52] = {
    [alpha_to_idx('d')] = {
        .frame = (rlsmenu_frame *) &drop_frame,
        .consumes_turn = true,
        .prep_frame = prep_inv_frame,
        .get_data_id = get_inv_data_id,
        .data_id_type = DATA_ID_ENTITY_TARGET,
    },
    [alpha_to_idx('i')] = {
        .frame = (rlsmenu_frame *) &inv_frame,
        .consumes_turn = false,
        .prep_frame = prep_inv_frame,
        .get_data_id = get_inv_data_id,
        .data_id_type = DATA_ID_ENTITY_TARGET,
    },
    [alpha_to_idx('m')] = {
        .frame = (rlsmenu_frame *) &sock_frame,
        .consumes_turn = false,
        .prep_frame = prep_menu_select_frame,
        .get_data_id = 0,
        .data_id_type = DATA_ID_STATIC,
    },
    [alpha_to_idx('D')] = {
        .frame = (rlsmenu_frame *) &debug_log_frame,
        .consumes_turn = false,
        .prep_frame = prep_logger_frame,
        .get_data_id = get_logger_data_id,
        .data_id_arg = (uint64_t) &g_debug_log,
        .data_id_type = DATA_ID_STATIC,
    },
};

static void slist_cleanup_cb1(rlsmenu_frame *frame)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) frame;
    free(s->item_names);
}

static void slist_cleanup_cb2(rlsmenu_frame *frame)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) frame;
    free(s->item_names);
    free(s->items);
}

static void msgbox_cleanup_cb1(rlsmenu_frame *frame)
{
    rlsmenu_msgbox *m = (rlsmenu_msgbox *) frame;
    free(m->lines);
}

static void on_complete(rlsmenu_frame *frame)
{
    rlsmenu_push_return(frame->parent, frame->state);
}

static enum rlsmenu_cb_res drop_item_cb(rlsmenu_frame *frame, void *e)
{
    FrameData *state = frame->state;
    ecs_set(state->world, g_player_id, DropAction, { *(ecs_entity_t *) e });
    return RLSMENU_CB_SUCCESS;
}

static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *f)
{
    FrameData *sel_data = *(void **) f;
    FrameData *data = frame->state;
    sel_data->world = data->world;

    // TODO: Associate this with an entity in a way that makes sense. You need
    // to be able to destroy the entity when done, so having this pointer is
    // not (necessarily) enough. We want these associated with entities so they
    // can be queried normally.
    MenuNetWrapper *mnw = mnw_new(sel_data);
    if (!mnw) return RLSMENU_CB_FAILURE;
    alloc_menu(mnw);

    log_msg(&g_game_log, L"Socket available on port %d.", mnw->client_port);
    log_msg(&g_debug_log, L"Opened socket on port %d", mnw->client_port);

    return RLSMENU_CB_SUCCESS;
}

static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) data->frame;
    data->world = world;

    if (poll_data.n_menus == NPOLLS) {
        log_msg(&g_game_log, L"Maximum supported menus already open");
        return false;
    }

    int total = 0;
    for (size_t i = 0; i < sizeof(gui_state) / sizeof(*gui_state); i++)
        if (gui_state[i].frame) total++;

    s->items = malloc(sizeof(void *) * total);
    s->item_size = sizeof(void *);
    s->n_items = total;
    s->item_names = malloc(sizeof(*s->item_names) * s->n_items);

    int j = 0;
    FrameData **f = s->items;
    for (size_t i = 0; i < sizeof(gui_state) / sizeof(*gui_state); i++)
        if (gui_state[i].frame) {
            *f = &gui_state[i];
            s->item_names[j++] = (*f)->frame->title;
            f++;
        }

    return true;
}

static bool prep_inv_frame(FrameData *data, ecs_world_t *world)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) data->frame;
    Inventory *inv = ecs_get_mut(world, (ecs_entity_t) data->data_id_arg, Inventory);
    data->world = world;

    s->items = inv->items;
    s->item_size = sizeof(*inv->items);
    s->n_items = inv->end;
    s->item_names = malloc(sizeof(*s->item_names) * s->n_items);

    Item *item;
    for (int i = 0; i < inv->end; i++) {
        item = ecs_get_mut(data->world, inv->items[i], Item);
        s->item_names[i] = item->name;
    }

    return true;
}

static bool prep_logger_frame(FrameData *data, ecs_world_t *world)
{
    rlsmenu_msgbox *m = (rlsmenu_msgbox *) data->frame;
    Logger *l = (Logger *) data->data_id_arg;
    data->world = world;

    // Note: m->lines must be freed when done
    if (!(m->n_lines = log_to_strs(l, &m->lines)))
        return false;

    return true;
}

static uint32_t get_inv_data_id(FrameData *data)
{
    Inventory *inv = ecs_get_mut(data->world, (ecs_entity_t) data->data_id_arg, Inventory);
    return inv->data_id;
}

static uint32_t get_logger_data_id(FrameData *data)
{
    return ((Logger *) data->data_id_arg)->data_id;
}
