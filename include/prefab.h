#pragma once
#include "flecs.h"

extern ECS_PREFAB_DECLARE(Item);
extern ECS_PREFAB_DECLARE(GoldItem);
extern ECS_PREFAB_DECLARE(FoodItem);
extern ECS_PREFAB_DECLARE(WeaponItem);

extern ECS_PREFAB_DECLARE(Monster);
extern ECS_PREFAB_DECLARE(Goblin);

void register_prefabs(ecs_world_t *world);
