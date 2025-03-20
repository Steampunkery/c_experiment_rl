#include "prefab.h"

#include "component.h"
#include "ai.h"

#include "flecs.h"

#define PREFAB(p, ...) ECS_PREFAB_DECLARE(p);
PREFABS
#undef PREFAB

void register_prefabs(ecs_world_t *world)
{
#define PREFAB(p, ...) ECS_PREFAB_DEFINE(world, p, __VA_ARGS__);
PREFABS
#undef PREFAB

    /* Items */
    ecs_set(world, Item, Name, { L"PLACEHOLDER", 11 });
    ecs_set(world, Item, Weight, { 1 });
    ecs_set(world, Item, Value, { 1 });
    ecs_add(world, Item, Renderable);

    ecs_set(world, GoldItem, Name, { L"Gold", 4 });
    ecs_set(world, GoldItem, Weight, { 0.1 });
    ecs_set(world, GoldItem, Stack, { 1 });
    ecs_set(world, GoldItem, Glyph, { L'$' });

    ecs_set(world, FoodItem, Weight, { 0.5 });
    ecs_set(world, FoodItem, Stack, { 1 });
    ecs_set(world, FoodItem, Satiation, { 10 });
    ecs_set(world, FoodItem, Glyph, { L'%' });

    ecs_set(world, WeaponItem, Weight, { 5 });
    ecs_set(world, WeaponItem, Glyph, { L'/' });
    ecs_set(world, WeaponItem, WeaponStats, { 1, 4, 0 });

    ecs_set(world, QuaffableItem, Glyph, { L'!' });

    /* Monsters */
    ecs_set(world, Monster, Name, { L"PLACEHOLDER", 11 });
    ecs_set(world, Monster, AIController, { AIC_NOTHING });
    ecs_set(world, Monster, InitiativeData, { 0, 10 });
    ecs_set(world, Monster, Health, { 100, 100 });
    ecs_add(world, Monster, Renderable);

    ecs_set(world, Humanoid, WieldDescriptor, { 0 });

    ecs_set(world, Goblin, Name, { L"Goblin", 6 });
    ecs_set(world, Goblin, Inventory, { 0, 10, 0 });
    ecs_set(world, Goblin, Glyph, { L'g' });

    ecs_set(world, Player, Name, { L"Player", 6 });

    ecs_set(world, Dog, Name, { L"Dog", 3 });
    ecs_set(world, Dog, Health, { 20, 20 });
    ecs_set(world, Dog, Glyph, { L'd' });
}
