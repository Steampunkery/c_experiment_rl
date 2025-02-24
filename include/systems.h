#pragma once
#include "flecs.h"

void register_systems(ecs_world_t *);

extern ecs_entity_t render;
extern ecs_entity_t initiative;;

extern ECS_SYSTEM_DECLARE(AI);
extern ECS_SYSTEM_DECLARE(Move);
extern ECS_SYSTEM_DECLARE(Pickup);
extern ECS_SYSTEM_DECLARE(Drop);
extern ECS_SYSTEM_DECLARE(Quaff);
extern ECS_SYSTEM_DECLARE(Attack);
extern ECS_SYSTEM_DECLARE(Prayer);
extern ECS_SYSTEM_DECLARE(ApplyPoison);
extern ECS_SYSTEM_DECLARE(StatusEffectTimer);
