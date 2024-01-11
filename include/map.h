#pragma once
#include "flecs.h"
#include "dijkstra.h"

#define NUM_DIJKSTRA_MAPS 1

typedef struct _GArray GArray;
typedef enum { Floor = 0, Wall } TileType;
typedef struct Position MovementAction;

extern char tiletype_to_wchar[];

extern ECS_COMPONENT_DECLARE(Map);

enum dm_type { DM_TYPE_Item };
enum dm_order { DM_ORDER_GOLD };

typedef struct DMWrapper {
    DijkstraMap dm;
    enum dm_type type; // Item, Mob, Player...
    int subtype; // Item type, Mob type, Player type(?)
} DMWrapper;

typedef struct {
    int rows, cols;
    TileType **grid;
    GArray ***items;

    // This gets allocated to rows * cols -- the max possible number of
    // sources. This way it can be reused by every dijkstra map.
    size_t *dijkstra_sources;
    DMWrapper dijkstra_maps[NUM_DIJKSTRA_MAPS];
} Map;

Map *new_arena(Map *map, int rows, int cols);
void destroy_map(Map *map);

void *new_grid(int rows, int cols, size_t size, void *val);
void destroy_grid(void *grid);

bool is_passable(const Map *map, int x, int y);
int map_contains(const Map *map, int x, int y);
bool entity_can_traverse(ecs_world_t *world, ecs_entity_t e, MovementAction *mov);
