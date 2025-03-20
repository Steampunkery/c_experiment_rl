#pragma once
#include "action.h"

typedef long unsigned int ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;
typedef struct DijkstraMap DijkstraMap;
typedef struct Map Map;
typedef struct Position Position;

void left_walker(ecs_world_t *world, ecs_entity_t e);
void do_nothing(ecs_world_t *world, ecs_entity_t e);
void greedy_ai(ecs_world_t *world, ecs_entity_t e);
void pet_ai(ecs_world_t *world, ecs_entity_t e);
void enemy_ai(ecs_world_t *world, ecs_entity_t e);

MovementAction dm_flow_downhill(DijkstraMap const *dm, Map const *map, Position const *pos);
MovementAction dm_flow_uphill(DijkstraMap const *dm, Map const *map, Position const *pos);

typedef void (*ai_cb)(ecs_world_t *world, ecs_entity_t e);
extern ai_cb aic_to_function[];
