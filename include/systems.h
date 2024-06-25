#pragma once
#include "flecs.h"

void register_systems(ecs_world_t *);

void Initiative(ecs_iter_t *it);
void Move(ecs_iter_t *it);
void AI(ecs_iter_t *it);
void Pickup(ecs_iter_t *it);
void Drop(ecs_iter_t *it);
void Prayer(ecs_iter_t *it);

extern ecs_entity_t render;

extern ECS_SYSTEM_DECLARE(Initiative);
extern ECS_SYSTEM_DECLARE(Move);
extern ECS_SYSTEM_DECLARE(Pickup);
extern ECS_SYSTEM_DECLARE(Drop);
extern ECS_SYSTEM_DECLARE(AI);
extern ECS_SYSTEM_DECLARE(Prayer);
