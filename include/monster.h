#pragma once
#include <stdbool.h>

typedef struct MovementAction MovementAction;
typedef struct ecs_world_t ecs_world_t;
typedef long unsigned int ecs_entity_t;

int get_cost_for_movement(int x, int y);
void make_invisible(ecs_world_t *world, ecs_entity_t e);
bool try_move_entity(ecs_world_t *, ecs_entity_t, MovementAction *);
