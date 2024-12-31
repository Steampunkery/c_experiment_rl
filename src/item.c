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
    Map *map = ecs_singleton_ensure(world, Map);
    if (!map_contains(map, x, y)) return 0;

    GArray *items = map->items[y][x];
    if (!items) items = map->items[y][x] = g_array_sized_new(FALSE, TRUE, sizeof(ecs_entity_t), 8);

    g_array_append_val(items, e);
    ecs_set(world, e, Position, { x, y });

    return e;
}

// Expects the existence of a Map
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y)
{
    Map *map = ecs_singleton_ensure(world, Map);
    if (!map_contains(map, x, y) || !map->items[y][x]) return 0;

    GArray *items = map->items[y][x];
    if (items->len == 0) return 0;

    guint i;
    ecs_entity_t item;
    if (e == 0) {
        i = 0;
        item = g_array_index(items, ecs_entity_t, 0);
    } else {
        for (i = 0; i < items->len; i++) {
            item = g_array_index(items, ecs_entity_t, i);
            if (item == e) break;
        }
        if (i == items->len) return 0;
    }

    e = item;
    g_array_remove_index_fast(items, i);
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
