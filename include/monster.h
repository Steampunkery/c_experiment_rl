#pragma once

#include "flecs.h"

typedef struct Position MovementAction;

ecs_entity_t init_goblin(ecs_world_t *world, int x, int y);
void make_invisible(ecs_world_t *world, ecs_entity_t e);
bool try_move_entity(ecs_world_t *, ecs_entity_t, MovementAction *);
