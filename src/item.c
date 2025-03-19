#include "item.h"

#include "map.h"
#include "component.h"
#include "ds.h"
#include "log.h"

#include "flecs.h"

entity_effect_cb ec_to_function[EC_MAX] = { health_potion_cb };

void item_init(ecs_world_t *world)
{
    (void) world;
}

// TODO: Revisit API of this and pickup_item w.r.t passing map as an argument
ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y)
{
    Map *map = ecs_singleton_get_mut(world, Map);
    if (!map_contains(map, x, y)) return 0;

    map_place_entity(world, map, e, x, y);
    ecs_set(world, e, Position, { x, y });

    return e;
}

// Expects the existence of a Map.
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y)
{
    Map *map = ecs_singleton_get_mut(world, Map);
    if (!map_contains(map, x, y)) return 0;

    map_remove_entity(world, map, e, x, y);
    ecs_remove(world, e, Position);

    return e;
}

ecs_entity_t first_prefab_at_pos(ecs_world_t *world, Map const *map, ecs_entity_t type, int x, int y, int *n)
{
    if (!map_contains(map, x, y)) return 0;

    entity_vec *entities = &map->entities[y][x];
    if (entities->size == 0) return 0;

    *n = 0;
    ecs_entity_t first = 0;
    for (int i = 0; i < entities->size; i++)
        if (ecs_has_pair(world, entities->data[i], EcsIsA, type)) {
            *n += 1;
            first = entities->data[i];
        }

    return first;
}

// Arg is the amount of healing provided
void health_potion_cb(ecs_world_t *world, ecs_entity_t e, uint64_t arg)
{
    Health *health = ecs_get_mut(world, e, Health);
    health->val = health->val + (int) arg > health->total ? health->total : health->val + (int) arg;
}

void apply_weapon_effects(ecs_world_t *world, ecs_entity_t w, ecs_entity_t, ecs_entity_t t, DamageRoll *)
{
    if (ecs_has(world, w, Fiery) && ecs_has(world, t, Health)) {
        ecs_entity(world, {
                .parent = t,
                .set = ecs_values(
                        // TOOD: Don't hardcode 33% probability
                        ecs_value(GenStatusEffect, { { SE_Probability, .arg = 33 }, OnFire, t }),
                        ecs_value(InitiativeData, { 0, 10 }),
                        { ecs_pair(Targets, t), NULL }),
                .add = ecs_ids(OnFire) // Use add to invoke constructor
        });
        log_msg(&g_game_log, L"%S is on fire!", GET_NAME_COMP(world, t));
    }
}
