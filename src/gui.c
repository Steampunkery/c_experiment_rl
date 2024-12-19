#include "gui.h"

#include "component.h"

#include "log.h"
#include "rlsmenu.h"
#include "sockui.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>

static enum rlsmenu_cb_res drop_item_cb(rlsmenu_frame *frame, void *e);
static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *e);
static void cleanup_cb1(rlsmenu_frame *frame);
static void cleanup_cb2(rlsmenu_frame *frame);
static void on_complete(rlsmenu_frame *frame);
static bool prep_player_inv_frame(FrameData *data, ecs_world_t *world);
static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world);
static uint32_t get_player_inv_data_id(FrameData *data);

rlsmenu_slist drop_frame = {
    .s = {
        .frame = {
            .type = RLSMENU_SLIST,
            .flags = RLSMENU_BORDER,
            .title = L"Drop",
            .state = &gui_state[alpha_to_idx('d')],
        },
        .cbs = &(rlsmenu_cbs) { drop_item_cb, on_complete, cleanup_cb1 },

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
        .cbs = &(rlsmenu_cbs) { NULL, on_complete, cleanup_cb1 },

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
        },
        .cbs = &(rlsmenu_cbs) { socket_menu_cb, on_complete, cleanup_cb2 },

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
        .get_data_id = get_player_inv_data_id,
    },
    [alpha_to_idx('i')] = {
        .frame = (rlsmenu_frame *) &inv_frame,
        .consumes_turn = false,
        .prep_frame = prep_player_inv_frame,
        .get_data_id = get_player_inv_data_id,
    },
    [alpha_to_idx('m')] = {
        .frame = (rlsmenu_frame *) &sock_frame,
        .consumes_turn = false,
        .prep_frame = prep_menu_select_frame,
    },
};

// Array that holds struct for poll events on socket menus
// Only thing stopping me from turning PollData into a set of components
// and storing them in the ECS is the struct pollfds need to be contiguous.
PollData poll_data = {
    .pollers = { { -1, 0, 0 } },
    .menus = { 0 },
    .n_menus = 0,
};

static void mnw_redraw(MenuNetWrapper *mnw)
{
    FrameData *data = mnw->frame_data;

    // If the underlying data has been invalidated, throw out the menu and start again
    // TODO: Find a less expensive way of updating changed menus?
    uint32_t data_id = data->get_data_id ? data->get_data_id(data) : 0;
    if (mnw->last_data_id != data_id) {
        mnw->last_data_id = data_id;
        rlsmenu_gui_deinit(&mnw->gui);
        rlsmenu_gui_init(&mnw->gui);
        data->prep_frame(data, data->world);
        rlsmenu_gui_push(&mnw->gui, data->frame);
    }

    // TODO: get sockui input here
    rlsmenu_update(&mnw->gui, RLSMENU_INVALID_KEY);
    rlsmenu_str s = rlsmenu_get_menu_str(&mnw->gui);
    if (s.has_changed)
        sockui_draw_menu(&mnw->sui, s.str, (int[2]) { s.h, s.w });
}

// TODO: Poll on client_fds?
// TODO: Explicit way to close socket menu by user
void handle_socket_menus(ecs_world_t *world)
{
    (void) world;
    if (!poll_data.n_menus) return;

    int n_events, i, ret;
    do {
        n_events = poll(poll_data.pollers, poll_data.n_menus, 0);
        if (n_events == -1 && errno == ENOMEM) {
            perror("poll");
            exit(1); // FIXME?
        }
    } while (n_events == -1 && errno == EINTR);

    struct pollfd *pfd = poll_data.pollers;
    MenuNetWrapper **mnw = poll_data.menus;
    for (i = 0; i < NPOLLS; i++, pfd++, mnw++) {
        // TODO: Make a better way of check if a MenuNetWrapper
        // is active and maybe do this drawing somewhere else
        if (*mnw && (*mnw)->sui.client_fd >= 0)
            mnw_redraw(*mnw);

        if (pfd->fd < 0 || !pfd->revents) continue;
        if (pfd->revents & POLLIN) {
            ret = sockui_attach_client(&(*mnw)->sui);

            if (ret == SOCKUI_ESYS) // FIXME: better error handling
                _log_msg(&g_game_log, strerror(errno));
            else
                log_msg(&g_game_log, L"Client attached...");

            pfd->events = 0; // Stop receiving POLLIN
        } else { // POLLERR, POLLNVAL, POLLHUP
            pfd->fd = -1;
            pfd->events = 0;
            sockui_close(&(*mnw)->sui);
            rlsmenu_gui_deinit(&(*mnw)->gui);
            free(*mnw);
            *mnw = NULL; // May use this as an invariant
            poll_data.n_menus--;
        }
    }

    assert(poll_data.n_menus >= 0);
}

static void cleanup_cb1(rlsmenu_frame *frame)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) frame;
    free(s->item_names);
}

static void cleanup_cb2(rlsmenu_frame *frame)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) frame;
    free(s->item_names);
    free(s->items);
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

// TODO: This function is purely a prototype. Fix ASAP
static enum rlsmenu_cb_res socket_menu_cb(rlsmenu_frame *frame, void *f)
{
    FrameData *sel_data = *(void **) f;
    FrameData *data = frame->state;

    // TODO: Associate this with an entity in a way that makes sense.
    // You need to be able to destroy the entity when done, so having
    // this pointer is not (necessarily) enough
    MenuNetWrapper *mnw = malloc(sizeof(*mnw));
    mnw->sui.port = 0;
    mnw->frame_data = sel_data;
    mnw->last_data_id = -1; // This forces an update the first time around

    sel_data->world = data->world;

    int err = sockui_init(&mnw->sui);
    if (err < 0) {
        fprintf(stderr, "sockui_init: %s\n", sockui_strerror(err));
        exit(1);
    }
    setsockopt(mnw->sui.serv_fd, SOL_SOCKET, SOCK_NONBLOCK, &(int) { 1 }, sizeof(int));

    int i;
    for (i = 0; i < NPOLLS; i++)
        if (poll_data.pollers[i].fd == -1) break;

    if (i == NPOLLS) { // TOOD: Cleanup and return callback failed? Log message?
        fprintf(stderr, __FILE__ ":%d:NOT ENOUGH POLLFDS!!\n", __LINE__);
        exit(1);
    }

    poll_data.pollers[i].fd = mnw->sui.serv_fd;
    poll_data.pollers[i].events = POLLIN;
    poll_data.menus[i] = mnw;
    poll_data.n_menus++;

    // Only initialize, will be setup later
    rlsmenu_gui_init(&mnw->gui);

    struct sockaddr_in sock_addr = { 0 };
    socklen_t len = sizeof(sock_addr);
    err = getsockname(mnw->sui.serv_fd, (void *) &sock_addr, &len);
    assert(!err);

    log_msg(&g_game_log, L"Socket available on port %d.", ntohs(sock_addr.sin_port));
    log_msg(&g_debug_log, L"Opened socket on port %d", ntohs(sock_addr.sin_port));

    return RLSMENU_CB_SUCCESS;
}

static bool prep_menu_select_frame(FrameData *data, ecs_world_t *world)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) data->frame;
    data->world = world;

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

static bool prep_player_inv_frame(FrameData *data, ecs_world_t *world)
{
    rlsmenu_list_shared *s = (rlsmenu_list_shared *) data->frame;
    Inventory *inv = ecs_get_mut(world, g_player_id, Inventory);
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

    // If inventory is empty, the menu can still be displayed asynchronously,
    // but return false to it is not displayed synchronously
    return inv->end != 0;
}

static uint32_t get_player_inv_data_id(FrameData *data)
{
    Inventory *inv = ecs_get_mut(data->world, g_player_id, Inventory);
    return inv->data_id;
}
