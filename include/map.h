#pragma once
#include "flecs.h"

typedef int wchar_t;
typedef struct _GArray GArray;
typedef enum { Floor = 0, Wall } TileType;
typedef struct Position MovementAction;

extern char tiletype_to_wchar[];

extern ECS_COMPONENT_DECLARE(Map);

typedef struct {
    int rows, cols;
    TileType **grid;
    GArray ***items;
} Map;

Map *new_arena(Map *map, int rows, int cols);
void destroy_map(Map *map);

void *new_grid(int rows, int cols, size_t size, void *val);
void destroy_grid(void *grid);

ecs_entity_t create_item(ecs_world_t *world, wchar_t glyph, char *name);
ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
bool entity_can_traverse(ecs_world_t *world, ecs_entity_t e, MovementAction *mov);
