#pragma once
#include "flecs.h"
#include "rogue.h"

typedef struct KeyInfo KeyInfo;
typedef struct GuiFrame GuiFrame;

typedef struct _GSList GSList;

typedef bool (*FilterFunc)(ecs_world_t *, const void *);
typedef bool (*ComponentAction)(ecs_world_t *, const void *);
typedef CommandType (*GuiCB)(ecs_world_t *, GuiFrame *, KeyInfo *);

typedef struct GuiStack {
    GSList *frames;
} GuiStack;
extern ECS_COMPONENT_DECLARE(GuiStack);

typedef struct GuiFrame {
    int x, y, boxed;
    char *content;
    GuiCB cb;
    FilterFunc filter;
    ComponentAction action;
    bool consumes_turn;
} GuiFrame;

CommandType inventory_cb(ecs_world_t *world, GuiFrame *gf, KeyInfo *key);

extern GuiFrame gui_frames[52];
