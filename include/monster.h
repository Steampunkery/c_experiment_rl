#pragma once
#include <stdbool.h>

typedef struct Position MovementAction;
typedef struct ecs_world_t ecs_world_t;
typedef long unsigned int ecs_entity_t;

ecs_entity_t init_goblin(ecs_world_t *world, int x, int y);
void make_invisible(ecs_world_t *world, ecs_entity_t e);
bool try_move_entity(ecs_world_t *, ecs_entity_t, MovementAction *);
