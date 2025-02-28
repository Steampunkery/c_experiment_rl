#include "map.h"

#include "component.h"
#include "prefab.h"
#include "rogue.h"
#include "ds.h"

#include "flecs.h"

#include <stdlib.h>

static ecs_query_t *item_q;

static void update_dijkstra_prefab_map(ecs_world_t *world, Map *map, DMWrapper *dm_wrap);
static void successors8(size_t idx, const void *state, successor_t **exits, uint8_t *n_exits);

char tiletype_to_wchar[] = {
    [Floor] = '.',
    [Wall] = '#',
};

Map *new_map(Map *map, int rows, int cols)
{
    map->rows = rows;
    map->cols = cols;
    TileType val = Floor;
    map->grid = (TileType **) new_grid(rows, cols, sizeof(**map->grid), &val);
    for (int i = 0; i < rows; i++)
        if (i == 0 || i == rows - 1)
            for (int j = 0; j < cols; j++)
                map->grid[i][j] = Wall;
        else
            map->grid[i][0] = map->grid[i][cols - 1] = Wall;

    entity_vec init = { 0 };
    map->entities = (entity_vec **) new_grid(rows, cols, sizeof(**map->entities), &init);

    map->dijkstra_sources = malloc(sizeof(*map->dijkstra_sources) * rows * cols);
    map->dijkstra_maps[DM_ORDER_GOLD] = (DMWrapper) {
        .dm = { 0 },
        .type = DM_TYPE_PREFAB,
        .subtype = GoldItem,
        .dirty = true,
    };

    map->dijkstra_maps[DM_ORDER_PLAYER] = (DMWrapper) {
        .dm = { 0 },
        .type = DM_TYPE_PREFAB,
        .subtype = Player,
        .dirty = true,
    };

    // TODO: Make a heuristic for arena size
    int cap = 1 << 28;
    map->dm_arena.beg = malloc(cap);
    map->dm_arena.end = map->dm_arena.beg + cap;

    // TODO: Investigate whether different successor functions would be useful
    for (int i = 0; i < NUM_DIJKSTRA_MAPS; i++)
        init_dijkstra_map((DijkstraMap *) &map->dijkstra_maps[i], rows, cols, successors8, map,
                          &map->dm_arena);

    map->should_rebuild_str = true;
    map->str = NULL;

    return map;
}

void destroy_map(Map *map)
{
    if (!map) return;

    for (int i = 0; i < map->rows; i++)
        for (int j = 0; j < map->cols; j++)
            entity_vec_clear(&map->entities[i][j]);

    destroy_grid(map->entities);
    destroy_grid(map->grid);

    free(map->dijkstra_sources);
    free(map->str);
}

char *get_map_str(Map *map)
{
    if (!map->should_rebuild_str && map->str) return map->str;

    if (!map->str) map->str = malloc(map->rows * map->cols * sizeof(*map->str));

    for (int i = 0; i < map->rows; i++)
        for (int j = 0; j < map->cols; j++)
            map->str[XY_TO_IDX(j, i, map->cols)] = tiletype_to_wchar[map->grid[i][j]];
    map->should_rebuild_str = false;

    return map->str;
}

/* Returns a two dimensional array of size rows * cols initialized to val */
void *new_grid(int rows, int cols, size_t size, void *val)
{
    uint8_t **grid = malloc(rows * sizeof(*grid));
    grid[0] = malloc(rows * cols * size + 1);

    for (int i = 1; i < rows; i++)
        grid[i] = grid[0] + i * cols * size;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            memcpy(grid[i] + j * size, val, size);

    grid[0][rows * cols * size] = 0;

    return (void *) grid;
}

/*  Frees a grid created by new_grid */
void destroy_grid(void *grid)
{
    free(((void **) grid)[0]);
    free(grid);
}

int map_contains(const Map *map, int x, int y)
{
    return x >= 0 && y >= 0 && x < map->cols && y < map->rows;
}

// TODO: Replace this with checking bit flags
bool is_passable(const Map *map, int x, int y)
{
    return map->grid[y][x] != Wall;
}

bool entity_can_traverse(ecs_world_t *world, ecs_entity_t e, Position *off)
{
    if (!off->x && !off->y) return true;
    Map const *map = ecs_singleton_get(world, Map);
    Position const *pos = ecs_get(world, e, Position);
    int new_x = pos->x + off->x;
    int new_y = pos->y + off->y;

    if (!map_contains(map, new_x, new_y)) return false;
    if (!is_passable(map, new_x, new_y)) return false;

    // TODO: If this becomes costly, separate items and monsters or keep a count?
    entity_vec *entities = &map->entities[new_y][new_x];
    for (int i = 0; i < entities->size; i++)
        if (ecs_has_pair(world, entities->data[i], EcsIsA, Monster))
            return false;

    return true;
}

// Assumes in-bounds
void map_place_entity(ecs_world_t *world, Map *map, ecs_entity_t e, int x, int y)
{
    entity_vec_push(&map->entities[y][x], e);
    mark_prefab_dms_dirty(world, map, e);
}

void map_remove_entity(ecs_world_t *world, Map *map, ecs_entity_t e, int x, int y)
{
    entity_vec *entities = &map->entities[y][x];
    assert(e != 0);

    int i;
    for (i = 0; i < entities->size; i++)
        if (entities->data[i] == e)
            break;

    assert(i != entities->size);

    entity_vec_erase_n(entities, i, 1);
    mark_prefab_dms_dirty(world, map, e);
}

void dijkstra_init(ecs_world_t *world)
{
    // clang-format off
    item_q = ecs_query(world, {
        .terms = {
            { .id = ecs_pair(EcsIsA, EcsWildcard) },
            { .id = ecs_id(Position) },
        },
        .cache_kind = EcsQueryCacheAuto
    });
    // clang-format on
}

/*
 * TODO: Consider separating dijkstra maps by type (mob, item, etc) then
 * having a system to handle each type of map. Using filters in a switch
 * this way feels like a hack.
 */
void update_dijkstra_maps(ecs_world_t *world, Map *map)
{
    for (int i = 0; i < NUM_DIJKSTRA_MAPS; i++) {
        switch (map->dijkstra_maps[i].type) {
        case DM_TYPE_PREFAB:
            update_dijkstra_prefab_map(world, map, map->dijkstra_maps + i);
            break;
        default:
            __builtin_unreachable();
        }
    }
}

static void update_dijkstra_prefab_map(ecs_world_t *world, Map *map, DMWrapper *dm_wrap)
{
    if (!dm_wrap->dirty) return;

    uint32_t n_sources = 0;
    ecs_iter_t it = ecs_query_iter(world, item_q);
    while (ecs_query_next(&it)) {
        Position *pos = ecs_field(&it, Position, 1);

        for (int i = 0; i < it.count; i++)
            if (ecs_has_pair(world, it.entities[i], EcsIsA, dm_wrap->subtype))
                map->dijkstra_sources[n_sources++] = XY_TO_IDX(pos[i].x, pos[i].y, map->cols);
    }

    build_dijkstra_map((DijkstraMap *) dm_wrap, map->dijkstra_sources, n_sources, &map->dm_arena);
    dm_wrap->dirty = false;
}

static void successors8(size_t idx, const void *state, successor_t **exits, uint8_t *n_exits)
{
    static successor_t all[8];
    const Map *map = state;

    *n_exits = 0;
    for (int i = 0; i < 8; i++) {
        int x = idx % map->cols + X_DIRS[i];
        int y = idx / map->cols + Y_DIRS[i];

        if (!map_contains(map, x, y)) continue;
        if (!is_passable(map, x, y)) continue;

        all[*n_exits].idx = (int) idx + XY_TO_IDX(X_DIRS[i], Y_DIRS[i], map->cols);
        all[*n_exits].cost = i > 3 ? 1.4f : 1.0f;
        (*n_exits)++;
    }

    *exits = all;
}

void mark_prefab_dms_dirty(ecs_world_t *world, Map *map, ecs_entity_t e)
{
    for (int i = 0; i < NUM_DIJKSTRA_MAPS; i++) {
        if (ecs_has_pair(world, e, EcsIsA, map->dijkstra_maps[i].subtype))
            map->dijkstra_maps[i].dirty = true;
    }
}
