#pragma once
#include <stdbool.h>

typedef struct ecs_world_t ecs_world_t;
typedef struct KeyInfo KeyInfo;
typedef struct rlsmenu_frame rlsmenu_frame;

typedef struct FrameData FrameData;
struct FrameData {
    rlsmenu_frame *frame;
    ecs_world_t *world;

    bool consumes_turn;
    bool (*prep_frame)(FrameData *, ecs_world_t *);
};

extern FrameData gui_state[52];
