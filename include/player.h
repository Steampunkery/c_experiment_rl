#pragma once
#include "input.h"

ecs_entity_t init_player(ecs_world_t *world);
bool process_player_input(ecs_world_t *world, KeyInfo key);
