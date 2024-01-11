#pragma once
#include "flecs.h"

typedef struct AIController {
    void (*ai_func)(ecs_world_t *world, ecs_entity_t e, void *arg);
    void *state;
} AIController;

void left_walker(ecs_world_t *world, ecs_entity_t e, void *arg);
void do_nothing(ecs_world_t *world, ecs_entity_t e, void *arg);
void greedy_ai(ecs_world_t *world, ecs_entity_t e, void *arg);
