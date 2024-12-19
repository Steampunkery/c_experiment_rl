#pragma once
#include <stdbool.h>
#include <stdint.h>

#define DATA_ID_ARG_SIZE 8

typedef struct ecs_world_t ecs_world_t;
typedef struct KeyInfo KeyInfo;
typedef struct rlsmenu_frame rlsmenu_frame;
typedef enum { DATA_ID_ENTITY_TARGET, DATA_ID_STATIC } data_id_t;

typedef struct FrameData FrameData;
struct FrameData {
    rlsmenu_frame *frame;
    ecs_world_t *world;

    bool consumes_turn;
    bool (*prep_frame)(FrameData *, ecs_world_t *);
    uint32_t (*get_data_id)(FrameData *);
    // Must be the same size as DATA_ID_ARG_SIZE
    uint64_t data_id_arg;
    data_id_t data_id_type;
};

extern FrameData gui_state[52];
