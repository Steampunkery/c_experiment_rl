#pragma once
#include "component.h"

typedef long unsigned int ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;
typedef struct DijkstraMap DijkstraMap;
typedef struct Map Map;

void left_walker(ecs_world_t *world, ecs_entity_t e, void *arg);
void do_nothing(ecs_world_t *world, ecs_entity_t e, void *arg);
void greedy_ai(ecs_world_t *world, ecs_entity_t e, void *arg);

MovementAction dm_flow_downhill(DijkstraMap *dm, Map *map, const Position *pos);
