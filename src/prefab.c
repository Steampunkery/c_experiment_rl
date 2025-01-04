#include "prefab.h"

#include "component.h"
#include "ai.h"

#include "flecs.h"

ECS_PREFAB_DECLARE(Item);
ECS_PREFAB_DECLARE(GoldItem);
ECS_PREFAB_DECLARE(FoodItem);
ECS_PREFAB_DECLARE(WeaponItem);

ECS_PREFAB_DECLARE(Monster);
ECS_PREFAB_DECLARE(Goblin);

void register_prefabs(ecs_world_t *world)
{
    /* Items */
    ECS_PREFAB_DEFINE(world, Item);
    ecs_set(world, Item, Name, { L"PLACEHOLDER" });
    ecs_set(world, Item, Weight, { 1 });
    ecs_set(world, Item, Value, { 1 });
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

    /* Monsters */
    ECS_PREFAB_DEFINE(world, Monster);
    ecs_set(world, Monster, Name, { L"PLACEHOLDER" });
    ecs_set(world, Monster, AIController, { do_nothing, NULL });
    ecs_set(world, Monster, InitiativeData, { 0, 10 });
    ecs_set(world, Monster, Renderable, { true });

    ECS_PREFAB_DEFINE(world, Goblin, (IsA, Monster));
    ecs_set(world, Goblin, Name, { L"Goblin" });
    ecs_set(world, Monster, Inventory, INV_NEW(10));
    ecs_set(world, Goblin, Glyph, { L'g' });
}
