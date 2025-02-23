#include "socket_menu.h"

#include "arena.h"
#include "component.h"
#include "gui.h"
#include "log.h"
#include "input.h"

#include "rlsmenu.h"
#include "rogue.h"
#include "sockui.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Array that holds struct for poll events on socket menus
// Only thing stopping me from turning PollData into a set of components
// and storing them in the ECS is the struct pollfds need to be contiguous.
PollData poll_data = {
    .pollers = { [0 ... NPOLLS-1] = { -1, 0, 0 } },
    .menus = { 0 },
    .n_menus = 0,
};

void alloc_menu(MenuNetWrapper *mnw)
{
    int i;
    for (i = 0; i < NPOLLS; i++)
        if (poll_data.pollers[i].fd == -1) break;

    assert(i != NPOLLS && "Too many menus open (programmer error)");

    poll_data.pollers[i].fd = mnw->sui.serv_fd;
    poll_data.pollers[i].events = POLLIN;
    poll_data.menus[i] = mnw;
    poll_data.n_menus++;
}

void dealloc_menu(MenuNetWrapper **mnw, struct pollfd *pfd)
{
    *pfd = (struct pollfd) { -1, 0, 0 };
    mnw_free(*mnw);
    *mnw = NULL; // May use this as an invariant
    poll_data.n_menus--;
}

MenuNetWrapper *mnw_new(FrameData *frame_data)
{
    MenuNetWrapper *mnw = malloc(sizeof(*mnw));
    if (!mnw) goto mem_err;
    mnw->a = new_arena(1 << 12);
    if (!mnw->a.beg) goto mnw_err;

    mnw->sui.port = 0;
    mnw->frame_data = frame_data;
    mnw->last_data_id = -1; // This forces an update the first time around
    mnw->client_port = 0;

    int err = sockui_init(&mnw->sui, SOCKUI_NONBLOCK);
    if (err < 0) {
        log_msg(&g_game_log, L"Error opening menu socket");
        _log_msg(&g_debug_log, "sockui_init: %s", sockui_strerror(err));
        goto mnw_err;
    }

    struct sockaddr_in sock_addr = { 0 };
    socklen_t len = sizeof(sock_addr);
    err = getsockname(mnw->sui.serv_fd, (void *) &sock_addr, &len);
    if (err) {
        log_msg(&g_game_log, L"Error getting socket name");
        _log_msg(&g_debug_log, "getsockname: %s", strerror(errno));
        goto sockui_err;
    }

    mnw->client_port = ntohs(sock_addr.sin_port);

    // Only initialize, will be setup later
    rlsmenu_gui_init(&mnw->gui);

    return mnw;

sockui_err:
    sockui_close(&mnw->sui);
mnw_err:
    free(mnw);
mem_err:
    return NULL;
}

void mnw_free(MenuNetWrapper *mnw)
{
    sockui_close(&mnw->sui);
    rlsmenu_gui_deinit(&mnw->gui);
    free(mnw->a.beg);
    free(mnw);
}

// returns whether the update succeeded
static bool mnw_update(MenuNetWrapper *mnw)
{
    FrameData *data = mnw->frame_data;

    if (data->data_id_type == DATA_ID_PLAYER_TARGET)
        data->data_id_arg.pl = g_player_id;

    // If the underlying data has been invalidated, throw out the menu and start again
    // TODO: Find a less expensive way of updating changed menus?
    uint32_t data_id = data->get_data_id ? data->get_data_id(data) : 0;
    if (mnw->last_data_id != data_id) {
        mnw->last_data_id = data_id;
        rlsmenu_gui_deinit(&mnw->gui);
        rlsmenu_gui_init(&mnw->gui);

        if (!data->prep_frame(data, data->world, mnw->a))
            return false;
        rlsmenu_gui_push(&mnw->gui, data->frame);
    }

    int res = sockui_recv(&mnw->sui);
    if (res < 0)
        return false;

    if (res > -1 && res < 256)
        log_msg(&g_debug_log, L"Received %d on port %d", res, mnw->client_port);
    else if (res == SOCKUI_ESYS)
        return false;
    else if (res != 256)
        assert(!"Programmer Error!");

    // This squashes errors, but they should bubble up elsewhere
    res = rlsmenu_update(&mnw->gui, translate_sockui(res, &mnw->sui));
    switch (res) {
        case RLSMENU_CONT:
            rlsmenu_str s = rlsmenu_get_menu_str(&mnw->gui);
            return !s.has_changed || (sockui_draw_menu(&mnw->sui, s.str, (int[2]) { s.h, s.w }) == 0);
        case RLSMENU_DONE:
            if (mnw->gui.frame_stack)
                return true;

            FrameData *fd = rlsmenu_pop_return(&mnw->gui);
            assert(fd);
            if (fd->consumes_turn)
                ecs_enable_component(data->world, g_player_id, ActionFromSocket, true);

            // Ensures menu is automatically recreated on next update
            mnw->last_data_id = -1;
            return true;
        case RLSMENU_CANCELED:
            return false;
        default:
            assert(!"Programmer Error!");
    }
}

// TODO: Do better error handling here?
int do_poll()
{
    int n_events;
    do {
        n_events = poll(poll_data.pollers, NPOLLS, 0);
        if (n_events == -1 && errno != EINTR) {
            perror("poll");
            exit(1);
        }
    } while (n_events == -1 && errno == EINTR);
    assert(n_events >= 0);

    return n_events;
}

bool server_pollin(MenuNetWrapper *mnw, struct pollfd *pfd)
{
    int ret = sockui_attach_client(&mnw->sui);

    if (ret == SOCKUI_ESYS) { // FIXME: better error handling
        log_msg(&g_game_log, L"Failed to attach menu");
        _log_msg(&g_debug_log, "sockui_attach_client: %s", strerror(errno));
        return false;
    }

    log_msg(&g_game_log, L"Menu attached successfully");
    pfd->fd = mnw->sui.client_fd; // Start polling on the client
    return true;
}

// TODO: Poll on client_fds?
void handle_socket_menus()
{
    if (!poll_data.n_menus) return;

    do_poll();

    struct pollfd *pfd = poll_data.pollers;
    MenuNetWrapper **mnw = poll_data.menus;
    for (int i = 0; i < NPOLLS; i++, pfd++, mnw++) {
        // Check that MNW and client socket are initialized
        if (*mnw && (*mnw)->sui.client_fd >= 0)
            if (!mnw_update(*mnw))
                goto dealloc;

        if (pfd->fd < 0 || !pfd->revents)
            continue;

        if (pfd->revents & (POLLERR | POLLNVAL | POLLHUP)) {
            log_msg(&g_debug_log, L"Menu on port %d polled %hX and was deallocated");
            goto dealloc;
        }

        // We don't care about client POLLIN
        if (pfd->revents & POLLIN && pfd->fd == (*mnw)->sui.serv_fd) {
            if (!server_pollin(*mnw, pfd) || !mnw_update(*mnw))
                goto dealloc;
        }

        continue;
dealloc:
        dealloc_menu(mnw, pfd);
    }

    assert(poll_data.n_menus >= 0);
}

void close_all_socket_menus()
{
    struct pollfd *pfd = poll_data.pollers;
    MenuNetWrapper **mnw = poll_data.menus;
    for (int i = 0; i < NPOLLS; i++, pfd++,  mnw++) {
        if (*mnw)
            dealloc_menu(mnw, pfd);
    }

    assert(poll_data.n_menus == 0);
}
