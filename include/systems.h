#pragma once
#include "flecs.h"

void register_systems(ecs_world_t *);

extern ecs_entity_t render;
extern ecs_entity_t initiative;

#define SYSTEMS                                                                                 \
    SYSTEM(AI, EcsOnUpdate, AIController, MyTurn)                                               \
    SYSTEM(ApplyPoison, EcsOnUpdate, (Targets, $t), Health($t), Poison, MyTurn)                 \
    SYSTEM(StatusEffectTimer, EcsOnUpdate, TimedStatusEffect, InitiativeData, MyTurn)

#define SYSTEM(s, ...) extern ECS_SYSTEM_DECLARE(s);
SYSTEMS
#undef SYSTEM
