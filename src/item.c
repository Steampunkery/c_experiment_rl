#include "item.h"

#include <glib.h>
#include "flecs.h"
#include "map.h"
#include "component.h"

char item_type_to_glyph[] = {
    [ITEM_TYPE_FOOD] = '%',
    [ITEM_TYPE_GOLD] = '$',
    [ITEM_TYPE_WEAPON] = '/',
};

// TODO: Make this a true weapon subtype
Item mjolnir = {
    .type = ITEM_TYPE_WEAPON,
    .name = "Mjolnir",
};

ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y) {
    Map *map = ecs_singleton_get_mut(world, Map);

    if (!map || !map_contains(map, x, y))
        return 0;

    GArray *items = map->items[y][x];
    if (!items)
        items = map->items[y][x] = g_array_sized_new(FALSE, TRUE, sizeof(ecs_entity_t), 8);

    guint i;
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

    guint i;
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

ecs_entity_t create_item(ecs_world_t *world, wchar_t glyph, const Item *data, size_t size) {
    ecs_entity_t item = ecs_new(world, 0);
    ecs_set(world, item, Glyph, { glyph });
    ecs_set(world, item, Renderable, { true });

    ecs_set_id(world, item, ecs_id(Item), size, data);

    return item;
}

char get_item_type_glyph(enum item_type item) {
    return item_type_to_glyph[item];
}

ecs_entity_t get_item_type_at_pos(ecs_world_t *world, Map *map, enum item_type type, int x, int y) {
    if (!map || !map_contains(map, x, y))
        return 0;

    GArray *items = map->items[y][x];
    if (!items)
        return 0;

    ecs_entity_t e;
    for (guint i = 0; i < items->len; i++) {
        e = g_array_index(items, ecs_entity_t, i);
        const Item *item = ecs_get(world, e, Item);
        if (item->type == type)
            return e;
    }

    return 0;
}
