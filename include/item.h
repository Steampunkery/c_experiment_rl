#pragma once
#include <stdint.h>
#include <stddef.h>

typedef int wchar_t;
typedef uint64_t ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;
typedef struct Map Map;

enum item_type { ITEM_TYPE_FOOD, ITEM_TYPE_GOLD, ITEM_TYPE_WEAPON };

typedef struct Item {
    enum item_type type;
    char *name;
} Item;

extern char item_type_to_glyph[];
extern Item mjolnir;

ecs_entity_t create_item(ecs_world_t *world, wchar_t glyph, const Item *data, size_t size);
ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
char get_item_type_glyph(enum item_type item);
ecs_entity_t get_item_type_at_pos(ecs_world_t *world, Map *map, enum item_type type, int x, int y);

typedef struct GoldItem {
    Item super;
    uint32_t amount;
} GoldItem;

typedef struct FoodItem {
    Item super;
    uint32_t satiation;
} FoodItem;
