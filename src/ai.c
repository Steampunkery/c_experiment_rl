#include "ai.h"

#include "input.h"
#include "monster.h"
#include "map.h"
#include "item.h"
#include "prefab.h"
#include "random.h"

#include "flecs.h"
#include "rogue.h"

#define do_wait_action(e) Move(world, e, &(MovementAction) { 0, 0, 100 });

void left_walker(ecs_world_t *world, ecs_entity_t e, void *)
{
    if (!try_move_entity(world, e, &(MovementAction) { -1, 0, 100 }))
        do_wait_action(e);
}

void do_nothing(ecs_world_t *world, ecs_entity_t e, void *)
{
    do_wait_action(e);
}

void greedy_ai(ecs_world_t *world, ecs_entity_t e, void *)
{
    Map const *map = ecs_singleton_get(world, Map);
    Position const *pos = ecs_get(world, e, Position);
    DijkstraMap const *dm = &map->dijkstra_maps[DM_ORDER_GOLD].dm;

    if (dm->map[XY_TO_IDX(pos->x, pos->y, map->cols)] > 0) {
        MovementAction ma = dm_flow_downhill(dm, map, pos);
        try_move_entity(world, e, &ma);
    } else {
        ecs_entity_t gold = first_prefab_at_pos(world, map, GoldItem, pos->x, pos->y, &(int){0});
        if (gold)
            Pickup(world, e, &(PickupAction) { gold });
    }
}

void pet_ai(ecs_world_t *world, ecs_entity_t e, void *)
{
    Map const *map = ecs_singleton_get(world, Map);
    Position const *pos = ecs_get(world, e, Position);
    DijkstraMap const *dm = &map->dijkstra_maps[DM_ORDER_PLAYER].dm;

    MovementAction ma;
    if (dm->map[XY_TO_IDX(pos->x, pos->y, map->cols)] > 2) {
        ma = dm_flow_downhill(dm, map, pos);
    } else {
        Position *pos = &direction8[randint(0, 7)];
        ma = (MovementAction) { pos->x, pos->y, get_cost_for_movement(pos->x, pos->y)};
    }

    if (!try_move_entity(world, e, &ma))
        do_wait_action(e);
}

void enemy_ai(ecs_world_t *world, ecs_entity_t e, void *arg)
{
    EnemyAIParams *params = arg;

    Map const *map = ecs_singleton_get(world, Map);
    DijkstraMap const *dm = &map->dijkstra_maps[DM_ORDER_PLAYER].dm;

    Health const *health = ecs_get(world, e, Health);
    Position const *pos = ecs_get(world, e, Position);

    bool did_action = false;
    MovementAction ma;
    if ((float) health->val/health->total < params->health_flee_p) {
        ma = dm_flow_uphill(dm, map, pos);
        did_action = try_move_entity(world, e, &ma);
    } else if (dm->map[XY_TO_IDX(pos->x, pos->y, map->cols)] > 1.5) {
        ma = dm_flow_downhill(dm, map, pos);
        did_action = try_move_entity(world, e, &ma);
    } else {
        Attack(world, e, &(AttackAction) { g_player_id });
        did_action = true;
    }

    if (!did_action)
        do_wait_action(e);
}

MovementAction dm_flow_downhill(DijkstraMap const *dm, Map const *map, Position const *pos)
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

MovementAction dm_flow_uphill(DijkstraMap const *dm, Map const *map, Position const *pos)
{
    MovementAction ma = { 0, 0, 0 };
    float cost = dm->map[XY_TO_IDX(pos->x, pos->y, map->cols)];
    for (int i = 0; i < 8; i++) {
        int x = pos->x + X_DIRS[i];
        int y = pos->y + Y_DIRS[i];

        if (!map_contains(map, x, y)) continue;
        if (!is_passable(map, x, y)) continue;

        float new_cost = dm->map[XY_TO_IDX(x, y, map->cols)];
        if (new_cost > cost) {
            cost = new_cost;
            ma = (MovementAction){ X_DIRS[i], Y_DIRS[i], 0 };
        }
    }

    ma.cost = get_cost_for_movement(ma.x, ma.y);

    return ma;
}
