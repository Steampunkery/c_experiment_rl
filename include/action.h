#pragma once

typedef struct ecs_world_t ecs_world_t;
typedef unsigned long ecs_entity_t;

// TODO: Don't allow entity == 0
typedef struct InvItemAction {
    // Entity to pick up or 0 for first item in the stack
    ecs_entity_t entity;
} InvItemAction, PickupAction, DropAction, QuaffAction;

typedef struct MovementAction {
    int x, y;
    int cost;
} MovementAction;

typedef struct AttackAction {
    ecs_entity_t target;
} AttackAction;

typedef void (*ActionFunc)(ecs_world_t *world, ecs_entity_t e, InvItemAction *arg);

void Move(ecs_world_t *world, ecs_entity_t e, MovementAction *ma);
void Pickup(ecs_world_t *world, ecs_entity_t e, PickupAction *pa);
void Drop(ecs_world_t *world, ecs_entity_t e, DropAction *da);
void Quaff(ecs_world_t *world, ecs_entity_t e, QuaffAction *qa);
void Attack(ecs_world_t *world, ecs_entity_t e, AttackAction *aa);
void Prayer(ecs_world_t *world, ecs_entity_t e, void *);
