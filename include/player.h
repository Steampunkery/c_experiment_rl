#pragma once
#include "input.h"
#include "rogue.h"

ecs_entity_t init_player(ecs_world_t *world);
GameState process_player_input(ecs_world_t *world, KeyInfo *key);
