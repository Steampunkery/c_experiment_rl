#pragma once
#include "flecs.h"

#define PREFABS                        \
    PREFAB(Item)                       \
    PREFAB(GoldItem, (IsA, Item))      \
    PREFAB(FoodItem, (IsA, Item))      \
    PREFAB(WeaponItem, (IsA, Item))    \
    PREFAB(QuaffableItem, (IsA, Item)) \
    PREFAB(Monster)                    \
    PREFAB(Goblin, (IsA, Monster))     \
    PREFAB(Player, (IsA, Monster))     \
    PREFAB(Dog, (IsA, Monster))

#define PREFAB(p, ...) extern ECS_PREFAB_DECLARE(p);
PREFABS
#undef PREFAB

void register_prefabs(ecs_world_t *world);
