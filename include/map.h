#pragma once
#include "dijkstra.h"
#include <stdbool.h>

#define NUM_DIJKSTRA_MAPS 1

typedef struct _GArray GArray;
typedef enum { Floor = 0, Wall } TileType;
typedef struct Position Position;
typedef struct ecs_world_t ecs_world_t;
typedef long unsigned int ecs_entity_t;

extern char tiletype_to_wchar[];

enum dm_type { DM_TYPE_Item };
enum dm_order { DM_ORDER_GOLD };

typedef struct DMWrapper {
    DijkstraMap dm;
    enum dm_type type; // Item, Mob, Player...
    int subtype; // Item prefab, Mob type, Player type(?)
    bool dirty; // Consider making this "int flags"
} DMWrapper;

typedef struct Map {
    int rows, cols;
    TileType **grid;
    GArray ***items;

    // This gets allocated to rows * cols -- the max possible number of
    // sources. This way it can be reused by every dijkstra map. NOTE: This
    // will need to be changed for multithreading! (use arena allocator for
    // sources array?)
    size_t *dijkstra_sources;
    DMWrapper dijkstra_maps[NUM_DIJKSTRA_MAPS];
    arena dm_arena;

    char *str;
    bool should_rebuild_str;
} Map;

Map *new_arena(Map *map, int rows, int cols);
void destroy_map(Map *map);
char *get_map_str(Map *map);

void *new_grid(int rows, int cols, size_t size, void *val);
void destroy_grid(void *grid);

bool is_passable(const Map *map, int x, int y);
int map_contains(const Map *map, int x, int y);
bool entity_can_traverse(ecs_world_t *world, ecs_entity_t e, Position *off);
GArray *get_map_items_xy(const Map *map, int x, int y);
void map_place_item(ecs_world_t *world, Map *map, ecs_entity_t e, int x, int y);
ecs_entity_t map_pickup_item(ecs_world_t *world, Map *map, ecs_entity_t e, int x, int y);

void dijkstra_init(ecs_world_t *world);
void update_dijkstra_maps(ecs_world_t *world, Map *map);
void mark_item_dms_dirty(ecs_world_t *world, Map *map, ecs_entity_t e);
