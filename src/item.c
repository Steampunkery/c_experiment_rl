#include "item.h"

#include "map.h"
#include "component.h"
#include "prefab.h"

#include "flecs.h"
#include <glib.h>

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

    map_place_item(world, map, e, x, y);
    ecs_set(world, e, Position, { x, y });

    return e;
}

// Expects the existence of a Map. e == 0 indicates the first item on the tile.
// Note: Condider removing aforementioned feature
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y)
{
    Map *map = ecs_singleton_get_mut(world, Map);
    if (!map_contains(map, x, y)) return 0;

    e = map_pickup_item(world, map, e, x, y);
    if (!e) return 0;
    ecs_remove(world, e, Position);

    return e;
}

ecs_entity_t get_typed_item_at_pos(ecs_world_t *world, Map *map, ecs_entity_t type, int x, int y)
{
    if (!map_contains(map, x, y)) return 0;

    GArray *items = map->items[y][x];
    if (!items) return 0;

    ecs_entity_t e;
    for (guint i = 0; i < items->len; i++) {
        e = g_array_index(items, ecs_entity_t, i);
        if (ecs_has_pair(world, e, EcsIsA, type)) return e;
    }

    return 0;
}
