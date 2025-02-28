#pragma once
#include "flecs.h"

void register_systems(ecs_world_t *);

extern ecs_entity_t render;
extern ecs_entity_t initiative;

#define SYSTEMS                                                                                 \
    SYSTEM(AI, EcsOnUpdate, AIController, MyTurn)                                               \
    SYSTEM(ApplyPoison, EcsOnUpdate, (Targets, $t), Health($t), Poison, MyTurn)                 \
    SYSTEM(StatusEffectTimer, EcsOnUpdate, TimedStatusEffect, InitiativeData, MyTurn)           \
    SYSTEM(DeathCleanup, EcsOnUpdate, Dead, Position, ?Inventory)

#define SYSTEM(s, ...) extern ECS_SYSTEM_DECLARE(s);
SYSTEMS
#undef SYSTEM
