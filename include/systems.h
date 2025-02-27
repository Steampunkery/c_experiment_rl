#pragma once
#include "flecs.h"

void register_systems(ecs_world_t *);

extern ecs_entity_t render;
extern ecs_entity_t initiative;

#define SYSTEMS                                                                                 \
    SYSTEM(AI, EcsOnUpdate, AIController, MyTurn)                                               \
    SYSTEM(Move, EcsOnUpdate, Position, (HasAction, MovementAction), InitiativeData, MyTurn)    \
    SYSTEM(Pickup, EcsOnUpdate, Inventory, (HasAction, PickupAction), Position, InitiativeData, \
           MyTurn)                                                                              \
    SYSTEM(Drop, EcsOnUpdate, Inventory, (HasAction, DropAction), Position, InitiativeData,     \
           MyTurn)                                                                              \
    SYSTEM(Quaff, EcsOnUpdate, Inventory, (HasAction, QuaffAction), InitiativeData, MyTurn)     \
    SYSTEM(Attack, EcsOnUpdate, (HasAction, AttackAction), Position, InitiativeData,            \
            ?(IsWielding, $t), WeaponStats($t), MyTurn)                                         \
    SYSTEM(Prayer, EcsOnUpdate, (HasAction, PrayerAction), InitiativeData, MyTurn)              \
    SYSTEM(ApplyPoison, EcsOnUpdate, (Targets, $t), Health($t), Poison, MyTurn)                 \
    SYSTEM(StatusEffectTimer, EcsOnUpdate, TimedStatusEffect, InitiativeData, MyTurn)

#define SYSTEM(s, ...) extern ECS_SYSTEM_DECLARE(s);
SYSTEMS
#undef SYSTEM
