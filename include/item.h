#pragma once
#include <stdint.h>
#include <stddef.h>

typedef int wchar_t;
typedef uint64_t ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;

enum item_type { ITEM_TYPE_FOOD, ITEM_TYPE_GOLD, ITEM_TYPE_WEAPON };

typedef struct Item {
    enum item_type type;
    char *name;
} Item;

ecs_entity_t create_item(ecs_world_t *world, wchar_t glyph, const Item *data, size_t size);
ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y);

typedef struct GoldItem {
    Item super;
    uint32_t amount;
} GoldItem;

typedef struct FoodItem {
    Item super;
    uint32_t satiation;
} FoodItem;
