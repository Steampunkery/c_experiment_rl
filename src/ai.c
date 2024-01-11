#include "ai.h"

#include "flecs.h"
#include "component.h"
#include "monster.h"
#include "input.h"
#include "map.h"
#include "rogue.h"

#include <float.h>

void left_walker(ecs_world_t *world, ecs_entity_t e, void *arg) {
    try_move_entity(world, e, &input_to_movement['4']);
}

void do_nothing(ecs_world_t *world, ecs_entity_t e, void *arg) {
    try_move_entity(world, e, &input_to_movement['5']);
}

void greedy_ai(ecs_world_t *world, ecs_entity_t e, void *arg) {
    Map *map = arg;
    const Position *pos = ecs_get(world, e, Position);
    DijkstraMap *dm = &map->dijkstra_maps[DM_ORDER_GOLD].dm;

    MovementAction ma = { 0, 0 };
    float cost = dm->map[pos->x + pos->y*map->cols];
    for (int i = 0; i < 8; i++) {
        int x = pos->x + X_DIRS[i];
        int y = pos->y + Y_DIRS[i];

        if (!map_contains(map, x, y)) continue;
        if (!is_passable(map, x, y)) continue;

        float new_cost = dm->map[x + y*map->cols];
        if (new_cost < cost) {
            cost = new_cost;
            ma = (MovementAction) { X_DIRS[i], Y_DIRS[i] };
        }
    }

    ecs_set_id(world, e, ecs_id(MovementAction), sizeof(MovementAction), &ma);
}
