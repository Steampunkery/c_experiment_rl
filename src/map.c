#include "map.h"
#include "component.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>
#include <glib.h>

ECS_COMPONENT_DECLARE(Map);

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

    return map;
}

void destroy_map(Map *map) {
    if (!map) return;

    for (int i = 0; i < map->rows; i++)
        for (int j = 0; j < map->cols; j++)
            g_array_unref(map->items[i][j]);

    destroy_grid(map->items);
    destroy_grid(map->grid);
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
