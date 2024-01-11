#include "map.h"
#include "component.h"
#include "rogue.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>
#include <glib.h>

ECS_COMPONENT_DECLARE(Map);

static void successors8(size_t idx, const void *state, successor_t **exits, uint8_t *n_exits);

char tiletype_to_wchar[] = {
    [Floor] = '.',
    [Wall] = '#',
};

Map *new_arena(Map *map, int rows, int cols) {
    map->rows = rows;
    map->cols = cols;
    TileType val = Floor;
    map->grid = (TileType **) new_grid(rows, cols, sizeof(**map->grid), &val);
    for (int i = 0; i < rows; i++)
        if (i == 0 || i == rows - 1)
            for (int j = 0; j < cols; j++) map->grid[i][j] = Wall;
        else
            map->grid[i][0] = map->grid[i][cols-1] = Wall;

    GArray *init = NULL;
    map->items = (GArray ***) new_grid(rows, cols, sizeof(**map->items), &init);

    map->dijkstra_sources = malloc(sizeof(*map->dijkstra_sources) * rows * cols);
    map->dijkstra_maps[0] = (DMWrapper) {
        .dm = { 0 },
        .type = DM_TYPE_Item,
        .subtype = ITEM_TYPE_GOLD,
    };

    // TODO: Investigate whether different successor functions would be useful
    for (int i = 0; i < NUM_DIJKSTRA_MAPS; i++)
        set_successor_fn((DijkstraMap *) &map->dijkstra_maps[i], successors8, map);

    return map;
}

void destroy_map(Map *map) {
    if (!map) return;

    for (int i = 0; i < map->rows; i++)
        for (int j = 0; j < map->cols; j++)
            g_array_unref(map->items[i][j]);

    destroy_grid(map->items);
    destroy_grid(map->grid);

    free(map->dijkstra_sources);
    for (int i = 0; i < NUM_DIJKSTRA_MAPS; i++)
        destroy_dijkstra_map((DijkstraMap *) &map->dijkstra_maps[i]);
}

/* Returns a two dimensional array of size rows * cols initialized to val */
void *new_grid(int rows, int cols, size_t size, void *val) {
    uint8_t **grid = malloc(rows * sizeof(*grid));
    grid[0] = malloc(rows * cols * size + 1);

    for (int i = 1; i < rows; i++) grid[i] = grid[0] + i * cols * size;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) memcpy(grid[i] + j * size, val, size);

    grid[0][rows * cols * size] = 0;

    return (void *) grid;
}

/*  Frees a grid created by new_grid */
void destroy_grid(void *grid) {
    free(((void **)grid)[0]);
    free(grid);
}

int map_contains(const Map *map, int x, int y) {
    return x >= 0 && y >= 0 && x < map->cols && y < map->rows;
}

// TODO: Replace this with checking bit flags
bool is_passable(const Map *map, int x, int y) {
    return map->grid[y][x] != Wall;
}

bool entity_can_traverse(ecs_world_t *world, ecs_entity_t e, MovementAction *mov) {
    const Map *map = ecs_singleton_get(world, Map);
    const Position *pos = ecs_get(world, e, Position);
    int new_x = pos->x + mov->x;
    int new_y = pos->y + mov->y;

    if (!map_contains(map, new_x, new_y)) return 0;
    if (!is_passable(map, new_x, new_y)) return 0;

    return 1;
}

static void successors8(size_t idx, const void *state, successor_t **exits, uint8_t *n_exits) {
    static successor_t all[8];
    const Map *map = state;

    *n_exits = 0;
    for (int i = 0; i < 8; i++) {
        int x = idx % map->cols + X_DIRS[i];
        int y = idx / map->cols + Y_DIRS[i];

        if (!map_contains(map, x, y)) continue;
        if (!is_passable(map, x, y)) continue;

        all[*n_exits].idx = (int) idx + X_DIRS[i] + Y_DIRS[i] * map->cols;
        all[*n_exits].cost = i > 3 ? 1.4f : 1.0f;
        (*n_exits)++;
    }

    *exits = all;
}
