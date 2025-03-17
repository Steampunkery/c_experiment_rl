#pragma once
#include "flecs.h"

void register_systems(ecs_world_t *);

extern ecs_entity_t render;
extern ecs_entity_t initiative;

#define SYSTEMS                                                              \
    SYSTEM(AI, 0, AIController, MyTurn)                                      \
    SYSTEM(ApplyPoison, 0, (Targets, $t), Health($t), Poison, MyTurn)        \
    SYSTEM(ApplyOnFire, 0, (Targets, $t), Health($t), OnFire, MyTurn)        \
    SYSTEM(ProcessStatusEffects, 0, GenStatusEffect, InitiativeData, MyTurn) \
    SYSTEM(DeathCleanup, 0, Health, Position, ?Inventory)

#define SYSTEM(s, ...) extern ECS_SYSTEM_DECLARE(s);
SYSTEMS
#undef SYSTEM
