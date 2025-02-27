#pragma once
#include "action.h"

typedef long unsigned int ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;
typedef struct DijkstraMap DijkstraMap;
typedef struct Map Map;
typedef struct Position Position;

typedef struct {
    float health_flee_p;
} EnemyAIParams;

void left_walker(ecs_world_t *world, ecs_entity_t e, void *arg);
void do_nothing(ecs_world_t *world, ecs_entity_t e, void *arg);
void greedy_ai(ecs_world_t *world, ecs_entity_t e, void *arg);
void pet_ai(ecs_world_t *world, ecs_entity_t e, void *arg);
void enemy_ai(ecs_world_t *world, ecs_entity_t e, void *arg);

MovementAction dm_flow_downhill(DijkstraMap const *dm, Map const *map, Position const *pos);
MovementAction dm_flow_uphill(DijkstraMap const *dm, Map const *map, Position const *pos);
