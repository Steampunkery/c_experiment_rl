#include "item.h"

#include "map.h"
#include "component.h"

#include "flecs.h"
#include <glib.h>

wchar_t item_type_to_glyph[] = {
    [ITEM_TYPE_FOOD] = '%',
    [ITEM_TYPE_GOLD] = '$',
    [ITEM_TYPE_WEAPON] = '/',
    [ITEM_TYPE_ARMOR] = ')',
};

// TODO: Make this a true weapon subtype
Item mjolnir = {
    .type = ITEM_TYPE_WEAPON,
    .name = L"Mjolnir",
};

// TODO: Make this a true armor subtype
Item speed_boots = {
    .type = ITEM_TYPE_ARMOR,
    .name = L"Boots of Speed",
};

/*
Armor speed_boots = {
    .super = {
        .type = ITEM_TYPE_ARMOR,
        .name = L"Boots of Speed",
    },
    .slot = ARMOR_SLOT_BOOTS,
    .ac = 2,
    .effect = {
        .f = 
    }
}
*/

ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y)
{
    Map *map = ecs_singleton_get_mut(world, Map);
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
    Map *map = ecs_singleton_get_mut(world, Map);
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

ecs_entity_t create_item(ecs_world_t *world, wchar_t glyph, const Item *data, size_t size)
{
    ecs_entity_t item = ecs_new(world, 0);
    ecs_set(world, item, Glyph, { glyph });
    ecs_set(world, item, Renderable, { true });

    ecs_set_id(world, item, ecs_id(Item), size, data);

    return item;
}

wchar_t get_item_type_glyph(enum item_type item)
{
    return item_type_to_glyph[item];
}

ecs_entity_t get_item_type_at_pos(ecs_world_t *world, Map *map, enum item_type type, int x, int y)
{
    if (!map_contains(map, x, y)) return 0;

    GArray *items = map->items[y][x];
    if (!items) return 0;

    ecs_entity_t e;
    for (guint i = 0; i < items->len; i++) {
        e = g_array_index(items, ecs_entity_t, i);
        const Item *item = ecs_get(world, e, Item);
        if (item->type == type) return e;
    }

    return 0;
}
