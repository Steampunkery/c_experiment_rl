#pragma once
#include "arena.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct ecs_world_t ecs_world_t;
typedef unsigned long ecs_entity_t;
typedef struct KeyInfo KeyInfo;
typedef struct rlsmenu_frame rlsmenu_frame;
typedef enum { DATA_ID_PLAYER_TARGET, DATA_ID_PTR, DATA_ID_CONST } data_id_t;

typedef struct FrameData FrameData;
struct FrameData {
    rlsmenu_frame *frame;
    ecs_world_t *world;

    bool consumes_turn;

    bool (*prep_frame)(FrameData *, ecs_world_t *, arena);
    void *ctx;
    arena a;
    wchar_t *title;

    uint32_t (*get_data_id)(FrameData *);

    union {
        ecs_entity_t pl;
        void *ptr;
        uint64_t c;
    } data_id_arg;
    data_id_t data_id_type;
};

extern FrameData gui_state[52];

void gui_init();
