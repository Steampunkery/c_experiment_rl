#include "gui.h"

#include "arena.h"
#include "component.h"
#include "log.h"
#include "rogue.h"
#include "socket_menu.h"
#include "prefab.h"

#include "rlsmenu.h"
#include "sockui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    ecs_query_desc_t query_desc;
    ecs_id_t *action;
} InvSlistCtx;

static enum rlsmenu_cb_res inv_slist_cb(rlsmenu_frame *frame, void *e);
static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *e);
static void on_complete(rlsmenu_frame *frame);
static bool prep_inv_frame(FrameData *data, ecs_world_t *world, arena a);
static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world, arena a);
static bool prep_logger_frame(FrameData *data, ecs_world_t *world, arena a);
static uint32_t get_inv_data_id(FrameData *data);
static uint32_t get_logger_data_id(FrameData *data);

rlsmenu_list inv_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_LIST,
            .flags = RLSMENU_BORDER,
            .state = &gui_state[alpha_to_idx('i')],
            .cbs = &(rlsmenu_cbs) { NULL, on_complete, NULL },
        },

        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    },
};

rlsmenu_slist inv_frame_tmpl = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .title = NULL,
            .state = NULL,
            .cbs = &(rlsmenu_cbs) { inv_slist_cb, on_complete, NULL },
        },
        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    },
};

rlsmenu_msgbox log_frame_tmpl = {
    .frame = {
        .type = RLSMENU_MSGBOX,
        .flags = RLSMENU_BORDER,
        .title = NULL,
        .state = NULL,
        .cbs = &(rlsmenu_cbs) { NULL, on_complete, NULL },
    },
    .lines = NULL,
    .n_lines = 0,
};

rlsmenu_slist sock_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .state = &gui_state[alpha_to_idx('m')],
            .cbs = &(rlsmenu_cbs) { socket_menu_cb, on_complete, NULL },
        },

        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    },
};

#define INV_ITEM_ACTION_FRAMEDATA(name, key)        \
    [alpha_to_idx(key)] = (FrameData) {             \
        .frame = (rlsmenu_frame *) &inv_frame_tmpl, \
        .consumes_turn = true,                      \
        .ctx = &(InvSlistCtx) {                     \
            .query_desc = { 0 },                    \
            .action = &ecs_id(name##Action)         \
        },                                          \
        .title = L###name,                          \
        .prep_frame = prep_inv_frame,               \
        .get_data_id = get_inv_data_id,             \
        .data_id_type = DATA_ID_PLAYER_TARGET,      \
    }

#define LOGGER_FRAMEDATA(name, Title, key)          \
    [alpha_to_idx(key)] = (FrameData) {             \
        .frame = (rlsmenu_frame *) &log_frame_tmpl, \
        .consumes_turn = false,                     \
        .prep_frame = prep_logger_frame,            \
        .title = Title,                             \
        .get_data_id = get_logger_data_id,          \
        .data_id_arg.ptr = &g_##name##_log,         \
        .data_id_type = DATA_ID_PTR,                \
    }

/* Array of data to pass along with frame. For now, this bool indicates if
 * the completion of the frame should pass the player's turn. Suject to
 * change.
 */
// FIXME: Make a struct that holds the filter and action and shit
FrameData gui_state[52] = {
    [alpha_to_idx('i')] = (FrameData) {
        .frame = (rlsmenu_frame *) &inv_frame,
        .consumes_turn = false,
        .prep_frame = prep_inv_frame,
        .ctx = NULL,
        .title = L"Inventory",
        .get_data_id = get_inv_data_id,
        .data_id_type = DATA_ID_PLAYER_TARGET,
    },

    INV_ITEM_ACTION_FRAMEDATA(Drop, 'd'),
    INV_ITEM_ACTION_FRAMEDATA(Quaff, 'q'),

    LOGGER_FRAMEDATA(game, L"Game log", 'G'),
    LOGGER_FRAMEDATA(debug, L"Debug log", 'D'),

    [alpha_to_idx('m')] = (FrameData) {
        .frame = (rlsmenu_frame *) &sock_frame,
        .consumes_turn = false,
        .prep_frame = prep_menu_select_frame,
        .title = L"Socketize Menu",
        .get_data_id = NULL,
        .data_id_arg.c = 0,
        .data_id_type = DATA_ID_CONST,
    },
};

#define FILL_FRAMEDATA_CTX_INV_SLIST(key, ...)                          \
    ((InvSlistCtx *) gui_state[alpha_to_idx(key)].ctx)->query_desc =    \
        (ecs_query_desc_t) {                                            \
            .terms = {                                                  \
                { .id = 0 /* gets filled in later */ },                 \
                __VA_ARGS__                                             \
            },                                                          \
            .cache_kind = EcsQueryCacheNone                             \
        };

void gui_init()
{
    FILL_FRAMEDATA_CTX_INV_SLIST('d');
    FILL_FRAMEDATA_CTX_INV_SLIST('q', { .id = ecs_pair(EcsIsA, QuaffableItem) });
}

static void on_complete(rlsmenu_frame *frame)
{
    rlsmenu_push_return(frame->parent, frame->state);
}

/**
 * This callback can only be used with actions that take the form InvItemAction.
 * TODO: Revisit choice of hardcoding g_player_id
 */
static enum rlsmenu_cb_res inv_slist_cb(rlsmenu_frame *frame, void *e)
{
    FrameData *data = frame->state;
    InvSlistCtx *ctx = data->ctx;

    ecs_set_id(data->world, g_player_id, ecs_pair(HasAction, *ctx->action), sizeof(InvItemAction), &(InvItemAction) { *(ecs_entity_t *) e });
    return RLSMENU_CB_SUCCESS;
}

static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *f)
{
    FrameData *data = frame->state;
    FrameData *sel_data = *(void **) f;
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

static void prep_frame_common(FrameData *data, ecs_world_t *world, arena *a)
{
    data->frame->state = data;
    data->world = world;
    data->a = *a;

    assert(data->title);
    data->frame->title = data->title;
}

// TODO: Most of this is unnecessary and can be done at compile time...
static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world, arena a)
{
    prep_frame_common(data, world, &a);
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) data->frame;

    if (poll_data.n_menus == NPOLLS) {
        log_msg(&g_game_log, L"Maximum supported menus already open");
        return false;
    }

    s->items = peeka(&a);
    s->item_size = sizeof(void *);
    s->n_items = 0;

    FrameData **f;
    for (size_t i = 0; i < sizeof(gui_state) / sizeof(*gui_state); i++)
        if (gui_state[i].frame) {
            f = alloc(&a, sizeof(void *), 1);
            *f = &gui_state[i];
            s->n_items++;
        }

    f = s->items;
    s->item_names = alloc(&a, sizeof(void *), s->n_items);
    for (int i = 0; i < s->n_items; i++) {
        s->item_names[i] = f[i]->title;
    }

    return true;
}

static bool prep_inv_frame(FrameData *data, ecs_world_t *world, arena a)
{
    prep_frame_common(data, world, &a);
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) data->frame;

    InvSlistCtx *ctx = data->ctx;

    // If there is no ctx, assume an inventory query with no filter
    ecs_query_desc_t *query_desc = ctx ? &ctx->query_desc : &(ecs_query_desc_t) {};
    // The entity target of InInventory is not known until now
    query_desc->terms[0].id = ecs_pair(InInventory, data->data_id_arg.pl);
    ecs_query_t *q = ecs_query_init(world, query_desc);
    ecs_iter_t it = ecs_query_iter(world, q);

    s->items = peeka(&a);
    s->n_items = 0;
    s->item_size = sizeof(ecs_entity_t);

    ecs_entity_t *e;
    while (ecs_query_next(&it)) {
        s->n_items += it.count;
        for (int i = 0; i < it.count; i++) {
            e = alloc(&a, sizeof(*e), 1);
            *e = it.entities[i];
        }
    }

    ecs_query_fini(q);

    s->item_names = peeka(&a);

    Name *name;
    ecs_entity_t *items = s->items;
    for (int i = 0; i < s->n_items; i++) {
        name = ecs_get_mut(world, items[i], Name);
        alloc(&a, sizeof(*s->item_names), 1);
        s->item_names[i] = name->s;
    }

    return true;
}

static bool prep_logger_frame(FrameData *data, ecs_world_t *world, arena a)
{
    prep_frame_common(data, world, &a);
    rlsmenu_msgbox *m = (rlsmenu_msgbox *) data->frame;

    Logger *l = data->data_id_arg.ptr;

    m->n_lines = l->data_id > MAX_LOG_MSGS ? MAX_LOG_MSGS : l->data_id;
    if (m->n_lines == 0) return true;

    m->lines = alloc(&a, sizeof(*m->lines), m->n_lines);
    for (int i = 1; i <= m->n_lines; i++)
        m->lines[m->n_lines - i] =  l->msgs[(l->head - i) & (MAX_LOG_MSGS - 1)];

    return true;
}

static uint32_t get_inv_data_id(FrameData *data)
{
    Inventory *inv = ecs_get_mut(data->world, data->data_id_arg.pl, Inventory);
    return inv->data_id;
}

static uint32_t get_logger_data_id(FrameData *data)
{
    return ((Logger *) data->data_id_arg.ptr)->data_id;
}
