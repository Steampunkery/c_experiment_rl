#include "gui.h"

#include "component.h"

#include "rlsmenu.h"
#include <stdlib.h>

static enum rlsmenu_cb_res drop_item_cb(rlsmenu_frame *frame, void *e);
static void on_complete(rlsmenu_frame *frame);
static bool prep_player_inv_frame(FrameData *data, ecs_world_t *world);

rlsmenu_slist drop_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .title = L"Drop",
            .state = &gui_state[alpha_to_idx('d')],
        },
        .cbs = &(rlsmenu_cbs) { drop_item_cb, on_complete, NULL },

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
        },
        .cbs = &(rlsmenu_cbs) { NULL, on_complete, NULL },

        .items = NULL,
        .item_size = 0,
        .n_items = 0,
        .item_names = NULL,
    },
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
        .prep_frame = prep_player_inv_frame,
    },
    [alpha_to_idx('i')] = {
        .frame = (rlsmenu_frame *) &inv_frame,
        .consumes_turn = false,
        .prep_frame = prep_player_inv_frame,
    },
};

static void on_complete(rlsmenu_frame *frame)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) frame;
    free(s->item_names);

    rlsmenu_push_return(frame->parent, frame->state);
}

static enum rlsmenu_cb_res drop_item_cb(rlsmenu_frame *frame, void *e)
{
    FrameData *state = frame->state;
    ecs_set(state->world, g_player_id, DropAction, { *(ecs_entity_t *) e });
    return RLSMENU_CB_SUCCESS;
}

static bool prep_player_inv_frame(FrameData *data, ecs_world_t *world)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) data->frame;
    Inventory *inv = ecs_get_mut(world, g_player_id, Inventory);
    if (inv->end == 0) return false;

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
