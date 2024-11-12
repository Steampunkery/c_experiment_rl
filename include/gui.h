#pragma once
#include <stdbool.h>
#include <poll.h>
#include <stdint.h>

#define NPOLLS 1

typedef struct ecs_world_t ecs_world_t;
typedef struct KeyInfo KeyInfo;
typedef struct rlsmenu_frame rlsmenu_frame;

typedef struct FrameData FrameData;
struct FrameData {
    rlsmenu_frame *frame;
    ecs_world_t *world;

    bool consumes_turn;
    bool (*prep_frame)(FrameData *, ecs_world_t *);
    uint32_t (*get_data_id)(FrameData *);
};

typedef struct MenuNetWrapper MenuNetWrapper;
typedef struct {
    struct pollfd pollers[NPOLLS];
    MenuNetWrapper *menus[NPOLLS];
    int n_menus;
} PollData;

extern FrameData gui_state[52];
extern PollData poll_data;

void handle_socket_menus(ecs_world_t *);
