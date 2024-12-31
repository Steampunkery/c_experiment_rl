#include "prefab.h"

#include "component.h"

#include "flecs.h"

ECS_PREFAB_DECLARE(Item);
ECS_PREFAB_DECLARE(GoldItem);
ECS_PREFAB_DECLARE(FoodItem);
ECS_PREFAB_DECLARE(WeaponItem);

void register_prefabs(ecs_world_t *world)
{
    ECS_PREFAB_DEFINE(world, Item);
    ecs_set(world, Item, Name, { L"PLACEHOLDER" });
    ecs_set(world, Item, Weight, { 1 });
    ecs_set(world, Item, Value, { 1 });
    ecs_set(world, Item, Glyph, { L'x' });
    ecs_set(world, Item, Renderable, { true });

    ECS_PREFAB_DEFINE(world, GoldItem, (IsA, Item));
    ecs_set(world, GoldItem, Name, { L"Gold" });
    ecs_set(world, GoldItem, Weight, { 0.1 });
    ecs_set(world, GoldItem, Stack, { 1 });
    ecs_set(world, GoldItem, Glyph, { L'$' });

    ECS_PREFAB_DEFINE(world, FoodItem, (IsA, Item));
    ecs_set(world, FoodItem, Weight, { 0.5 });
    ecs_set(world, FoodItem, Stack, { 1 });
    ecs_set(world, FoodItem, Satiation, { 10 });
    ecs_set(world, FoodItem, Glyph, { L'%' });

    ECS_PREFAB_DEFINE(world, WeaponItem, (IsA, Item));
    ecs_set(world, WeaponItem, Weight, { 5 });
    ecs_set(world, WeaponItem, Glyph, { L'/' });
}
