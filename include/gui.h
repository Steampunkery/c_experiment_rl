#pragma once
#include "flecs.h"
#include "rogue.h"

typedef struct KeyInfo KeyInfo;
typedef struct rlsmenu_frame rlsmenu_frame;

typedef struct FrameData FrameData;
struct FrameData {
    rlsmenu_frame *frame;
    ecs_world_t *world;

    bool consumes_turn;
    bool (*prep_frame)(FrameData *, ecs_world_t *);
};

extern ECS_COMPONENT_DECLARE(rlsmenu_gui);
extern FrameData gui_state[52];
