#include "ai.h"

#include "input.h"
#include "monster.h"
#include "map.h"
#include "item.h"
#include "prefab.h"

#include "flecs.h"
#include <stdlib.h>

void left_walker(ecs_world_t *world, ecs_entity_t e, void *arg)
{
    (void) arg;
    MovementAction mov = { -1, 0, 100 };
    try_move_entity(world, e, &mov);
}

void do_nothing(ecs_world_t *world, ecs_entity_t e, void *arg)
{
    (void) arg;
    MovementAction mov = { 0, 0, 100 };
    try_move_entity(world, e, &mov);
}

void greedy_ai(ecs_world_t *world, ecs_entity_t e, void *arg)
{
    Map *map = arg;
    const Position *pos = ecs_get(world, e, Position);
    DijkstraMap *dm = &map->dijkstra_maps[DM_ORDER_GOLD].dm;

    if (dm->map[XY_TO_IDX(pos->x, pos->y, map->cols)] > 0) {
        MovementAction ma = dm_flow_downhill(dm, map, pos);
        try_move_entity(world, e, &ma);
    } else {
        ecs_entity_t gold = get_typed_item_at_pos(world, map, GoldItem, pos->x, pos->y);
        if (gold) ecs_set(world, e, PickupAction, { gold });
    }
}

void pet_ai(ecs_world_t *world, ecs_entity_t e, void *arg)
{
    Map *map = arg;
    const Position *pos = ecs_get(world, e, Position);
    DijkstraMap *dm = &map->dijkstra_maps[DM_ORDER_PLAYER].dm;

    MovementAction ma;
    if (dm->map[XY_TO_IDX(pos->x, pos->y, map->cols)] > 2) {
        ma = dm_flow_downhill(dm, map, pos);
    } else {
        Position *pos = &input_to_movement[(random() % 9) + '1'];
        ma = (MovementAction) { pos->x, pos->y, get_cost_for_movement(pos->x, pos->y)};
    }

    try_move_entity(world, e, &ma);
}

MovementAction dm_flow_downhill(DijkstraMap *dm, Map *map, const Position *pos)
{
    MovementAction ma = { 0, 0, 0 };
    float cost = dm->map[XY_TO_IDX(pos->x, pos->y, map->cols)];
    for (int i = 0; i < 8; i++) {
        int x = pos->x + X_DIRS[i];
        int y = pos->y + Y_DIRS[i];

        if (!map_contains(map, x, y)) continue;
        if (!is_passable(map, x, y)) continue;

        float new_cost = dm->map[XY_TO_IDX(x, y, map->cols)];
        if (new_cost < cost) {
            cost = new_cost;
            ma = (MovementAction){ X_DIRS[i], Y_DIRS[i], 0 };
        }
    }

    ma.cost = get_cost_for_movement(ma.x, ma.y);

    return ma;
}
