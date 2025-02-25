#include "item.h"

#include "map.h"
#include "component.h"
#include "prefab.h"
#include "ds.h"

#include "flecs.h"

// TODO: Consider making this a prefab
ecs_entity_t mjolnir;

void item_init(ecs_world_t *world)
{
    mjolnir = ecs_insert(world, { ecs_isa(WeaponItem), NULL }, ecs_value(Name, { L"Mjolnir" }));
}

ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y)
{
    Map *map = ecs_singleton_get_mut(world, Map);
    if (!map_contains(map, x, y)) return 0;

    map_place_entity(world, map, e, x, y);
    ecs_set(world, e, Position, { x, y });

    return e;
}

// Expects the existence of a Map. e == 0 indicates the first item on the tile.
// Note: Condider removing aforementioned feature
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
