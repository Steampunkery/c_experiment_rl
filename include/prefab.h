#pragma once
#include "flecs.h"

extern ECS_PREFAB_DECLARE(Item);
extern ECS_PREFAB_DECLARE(GoldItem);
extern ECS_PREFAB_DECLARE(FoodItem);
extern ECS_PREFAB_DECLARE(WeaponItem);

void register_prefabs(ecs_world_t *world);
