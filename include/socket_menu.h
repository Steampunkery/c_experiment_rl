#pragma once
#include <stdint.h>
#include <poll.h>

#define NPOLLS 2

typedef struct ecs_world_t ecs_world_t;
typedef struct FrameData FrameData;

typedef struct MenuNetWrapper MenuNetWrapper;
typedef struct {
    struct pollfd pollers[NPOLLS];
    MenuNetWrapper *menus[NPOLLS];
    int n_menus;
} PollData;

extern PollData poll_data;

void handle_socket_menus();
void close_all_socket_menus();
MenuNetWrapper *mnw_new(FrameData *frame_data);
void mnw_free(MenuNetWrapper *mnw);
void alloc_menu(MenuNetWrapper *mnw);
void dealloc_menu(MenuNetWrapper **mnw, struct pollfd *pfd);
