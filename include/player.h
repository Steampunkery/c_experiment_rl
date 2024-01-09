#pragma once
#include "input.h"

typedef struct ecs_world_t ecs_world_t;
typedef uint64_t ecs_entity_t;

ecs_entity_t init_player(ecs_world_t *world);
bool process_player_input(ecs_world_t *world, KeyInfo key);
