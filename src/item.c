#include "item.h"

#include <glib.h>
#include "flecs.h"
#include "map.h"
#include "component.h"

ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y) {
    Map *map = ecs_singleton_get_mut(world, Map);

    if (!map || !map_contains(map, x, y))
        return 0;

    GArray *items = map->items[y][x];
    if (!items)
        items = map->items[y][x] = g_array_sized_new(FALSE, TRUE, sizeof(ecs_entity_t), 8);

    int i;
    for (i = 0; i < items->len; i++)
        if (g_array_index(items, ecs_entity_t, i) == 0) break;

    if (i == items->len)
        g_array_append_val(items, e);
    else
        g_array_index(items, ecs_entity_t, i) = e;

    ecs_set(world, e, Position, { x, y });

    return e;
}

ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y) {
    Map *map = ecs_singleton_get_mut(world, Map);

    if (!map || !map_contains(map, x, y) || !map->items[y][x])
        return 0;

    GArray *items = map->items[y][x];

    int i;
    ecs_entity_t *item;
    for (i = 0; i < items->len; i++) {
        item = &g_array_index(items, ecs_entity_t, i);
        if (*item == e && *item != 0) break;
        if (e == 0 && *item != 0) break;
    }

    if (i == items->len) return 0;

    e = *item;
    *item = 0;
    ecs_remove(world, e, Position);

    return e;
}

ecs_entity_t create_item(ecs_world_t *world, wchar_t glyph, const void *data, size_t size) {
    ecs_entity_t item = ecs_new(world, 0);
    ecs_set(world, item, Glyph, { glyph });
    ecs_set(world, item, Renderable, { true });

    ecs_set_id(world, item, ecs_id(Item), size, data);

    return item;
}
