#include "gui.h"

#include "action.h"
#include "arena.h"
#include "component.h"
#include "log.h"
#include "map.h"
#include "rogue.h"
#include "socket_menu.h"
#include "prefab.h"
#include "ds.h"

#include "rlsmenu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define alloc_rval(type) *(type *) alloc(&a, sizeof(type), 1)

typedef struct {
    ecs_query_desc_t query_desc;
    ActionFunc action;
} InvSlistCtx;

static enum rlsmenu_cb_res inv_slist_cb(rlsmenu_frame *frame, void *e);
static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *e);
static enum rlsmenu_cb_res pickup_cb(rlsmenu_frame *frame, void *e);
static enum rlsmenu_cb_res wield_cb(rlsmenu_frame *frame, void *e);
static void on_complete(rlsmenu_frame *frame);
static bool prep_inv_frame(FrameData *data, ecs_world_t *world, arena a);
static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world, arena a);
static bool prep_logger_frame(FrameData *data, ecs_world_t *world, arena a);
static bool prep_pickup_frame(FrameData *data, ecs_world_t *world, arena a);
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

rlsmenu_slist pickup_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .cbs = &(rlsmenu_cbs) { pickup_cb, on_complete, NULL },
        },
        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    }
};

rlsmenu_slist wield_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .cbs = &(rlsmenu_cbs) { wield_cb, on_complete, NULL },
        },
        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    }
};

#define INV_ITEM_ACTION_FRAMEDATA(name)             \
    &(FrameData) {                                  \
        .frame = (rlsmenu_frame *) &inv_frame_tmpl, \
        .consumes_turn = true,                      \
        .ctx = &(InvSlistCtx) {                     \
            .query_desc = { 0 },                    \
            .action = (ActionFunc) name             \
        },                                          \
        .title = L###name,                          \
        .prep_frame = prep_inv_frame,               \
        .get_data_id = get_inv_data_id,             \
        .data_id_type = DATA_ID_PLAYER_TARGET,      \
    }

#define LOGGER_FRAMEDATA(name, Title)               \
    &(FrameData) {                                  \
        .frame = (rlsmenu_frame *) &log_frame_tmpl, \
        .consumes_turn = false,                     \
        .prep_frame = prep_logger_frame,            \
        .title = Title,                             \
        .get_data_id = get_logger_data_id,          \
        .data_id_arg.ptr = &g_##name##_log,         \
        .data_id_type = DATA_ID_PTR,                \
    }

// Array of data to pass along with frame.
FrameData *gui_state[8] = {
    &(FrameData) {
        .frame = (rlsmenu_frame *) &inv_frame,
        .consumes_turn = false,
        .prep_frame = prep_inv_frame,
        .ctx = NULL,
        .title = L"Inventory",
        .get_data_id = get_inv_data_id,
        .data_id_type = DATA_ID_PLAYER_TARGET,
    },

    INV_ITEM_ACTION_FRAMEDATA(Drop),
    INV_ITEM_ACTION_FRAMEDATA(Quaff),

    LOGGER_FRAMEDATA(game, L"Game log"),
    LOGGER_FRAMEDATA(debug, L"Debug log"),

    &(FrameData) {
        .frame = (rlsmenu_frame *) &sock_frame,
        .consumes_turn = false,
        .prep_frame = prep_menu_select_frame,
        .title = L"Socketize Menu",
        .get_data_id = NULL,
        .data_id_arg.c = 0,
        .data_id_type = DATA_ID_CONST,
    },

    &(FrameData) {
        .frame = (rlsmenu_frame *) &pickup_frame,
        .consumes_turn = true,
        .prep_frame = prep_pickup_frame,
        .title = L"Pickup",
        .get_data_id = NULL,
        .data_id_arg.c = 0,
        .data_id_type = DATA_ID_CONST,
    },

    &(FrameData) {
        .frame = (rlsmenu_frame *) &wield_frame,
        .consumes_turn = false,
        .ctx = &(InvSlistCtx) { { 0 }, 0 },
        .prep_frame = prep_inv_frame,
        .title = L"Wield",
        .get_data_id = get_inv_data_id,
        .data_id_type = DATA_ID_PLAYER_TARGET,
    },
};

FrameData *gui_state_for(char c)
{
    switch (c) {
        case 'i': return gui_state[0];
        case 'd': return gui_state[1];
        case 'q': return gui_state[2];
        case 'G': return gui_state[3];
        case 'D': return gui_state[4];
        case 'm': return gui_state[5];
        case ',': return gui_state[6];
        case 'w': return gui_state[7];
        default: assert(!"Invalid GUI state!");
    }
}

#define FILL_FRAMEDATA_CTX_INV_SLIST(key, ...)                          \
    ((InvSlistCtx *) gui_state_for(key)->ctx)->query_desc =             \
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
    FILL_FRAMEDATA_CTX_INV_SLIST('w', { .id = ecs_pair(EcsIsA, WeaponItem) });
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

    if (ctx->action)
        ctx->action(data->world, g_player_id, &(InvItemAction) { *(ecs_entity_t *) e });
    return RLSMENU_CB_SUCCESS;
}

static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *f)
{
    FrameData *data = frame->state;
    FrameData *sel_data = *(void **) f;
    sel_data->world = data->world;

    if (poll_data.n_menus == NPOLLS) {
        log_msg(&g_game_log, L"Maximum supported menus already open");
        return RLSMENU_CB_FAILURE;
    }
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

static enum rlsmenu_cb_res pickup_cb(rlsmenu_frame *frame, void *e)
{
    FrameData *data = frame->state;

    Pickup(data->world, g_player_id, &(PickupAction) { *(ecs_entity_t *) e });
    return RLSMENU_CB_SUCCESS;
}

static enum rlsmenu_cb_res wield_cb(rlsmenu_frame *frame, void *_e)
{
    FrameData *data = frame->state;
    ecs_entity_t e = *(ecs_entity_t *) _e;

    Inventory *inv = ecs_get_mut(data->world, g_player_id, Inventory);
    WieldDescriptor *wd = ecs_get_mut(data->world, g_player_id, WieldDescriptor);

    inv_delete(data->world, inv, g_player_id, e);
    if (wd && wd->main)
        inv_insert(data->world, inv, g_player_id, wd->main);

    wd->main = e;
    log_msg(&g_game_log, L"You wield your %S", GET_NAME_COMP(data->world, e));

    return RLSMENU_CB_SUCCESS;
}

static wchar_t const **entity_names(ecs_world_t *world, ecs_entity_t *entities, int n, arena *a)
{
    wchar_t const **item_names = alloc(a, sizeof(*item_names), n);
    for (int i = 0; i < n; i++)
        item_names[i] = GET_NAME_COMP(world, entities[i]);

    return item_names;
}

static void *prep_frame_common(FrameData *data, ecs_world_t *world, arena *a)
{
    data->frame->state = data;
    data->world = world;
    data->a = *a;

    assert(data->title);
    data->frame->title = data->title;
    return data->frame;
}

// TODO: Most of this is unnecessary and can be done at compile time...
static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world, arena a)
{
    rlsmenu_list_shared *s = prep_frame_common(data, world, &a);

    s->items = gui_state;
    s->item_size = sizeof(void *);
    s->n_items = sizeof(gui_state) / sizeof(*gui_state);

    s->item_names = alloc(&a, sizeof(void *), s->n_items);
    for (int i = 0; i < s->n_items; i++)
        s->item_names[i] = ((FrameData **)s->items)[i]->title;

    return true;
}

static bool prep_inv_frame(FrameData *data, ecs_world_t *world, arena a)
{
    rlsmenu_list_shared *s = prep_frame_common(data, world, &a);

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

    while (ecs_query_next(&it)) {
        s->n_items += it.count;
        for (int i = 0; i < it.count; i++)
            alloc_rval(ecs_entity_t) = it.entities[i];
    }

    ecs_query_fini(q);

    s->item_names = entity_names(world, s->items, s->n_items, &a);

    return true;
}

static bool prep_logger_frame(FrameData *data, ecs_world_t *world, arena a)
{
    rlsmenu_msgbox *m = prep_frame_common(data, world, &a);

    Logger *l = data->data_id_arg.ptr;

    m->n_lines = l->data_id > MAX_LOG_MSGS ? MAX_LOG_MSGS : l->data_id;
    if (m->n_lines == 0) return true;

    m->lines = alloc(&a, sizeof(*m->lines), m->n_lines);
    for (int i = 1; i <= m->n_lines; i++)
        m->lines[m->n_lines - i] =  l->msgs[(l->head - i) & (MAX_LOG_MSGS - 1)];

    return true;
}

static bool prep_pickup_frame(FrameData *data, ecs_world_t *world, arena a)
{
    rlsmenu_list_shared *s = prep_frame_common(data, world, &a);
    Position const *pos = ecs_get(world, g_player_id, Position);
    entity_vec *entities = &ecs_singleton_get(world, Map)->entities[pos->y][pos->x];

    s->items = peeka(&a);
    s->n_items = 0;
    s->item_size = sizeof(ecs_entity_t);

    for (int i = 0; i < entities->size; i++)
        if (ecs_has_pair(world, entities->data[i], EcsIsA, Item)) {
            alloc_rval(ecs_entity_t) = entities->data[i];
            s->n_items++;
        }

    s->item_names = entity_names(world, s->items, s->n_items, &a);

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
