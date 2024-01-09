#pragma once
#include "flecs.h"
#include "religion.h"
#include "rogue.h"

typedef int wchar_t;

extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Inventory);
extern ECS_COMPONENT_DECLARE(MovementAction);
extern ECS_COMPONENT_DECLARE(PickupAction);
extern ECS_COMPONENT_DECLARE(DropAction);
extern ECS_COMPONENT_DECLARE(AIController);
extern ECS_COMPONENT_DECLARE(Glyph);
extern ECS_COMPONENT_DECLARE(Item);
extern ECS_COMPONENT_DECLARE(Religious);
extern ECS_COMPONENT_DECLARE(PrayerAction);
extern ECS_COMPONENT_DECLARE(SeeInvisible);
extern ECS_COMPONENT_DECLARE(Renderable);

extern ECS_TAG_DECLARE(Invisible);
extern ecs_entity_t MyTurn;

typedef struct Item {
    char *name;
} Item;

typedef struct Position {
    int x, y;
} Position, MovementAction;

// TODO: Don't allow entity == 0
typedef struct PickupAction {
    // Entity to pick up or 0 for first item in the stack 
    ecs_entity_t entity;
} PickupAction, DropAction;

typedef struct Inventory {
    int capacity;
    ecs_entity_t items[INVENTORY_MAX];
} Inventory;

typedef struct AIController {
    void (*ai_func)(ecs_world_t *world, ecs_entity_t e, void *arg);
} AIController;

typedef struct Glyph {
    wchar_t c;
} Glyph;

typedef struct Actor {
    int (*act)(ecs_world_t *world, ecs_entity_t *e);
} Actor;

typedef struct Religious {
    Religion *religion;
    int favors_left;
} Religious;

typedef struct PrayerAction {
    char dummy;
} PrayerAction;

typedef struct SeeInvisible {
    char dummy;
} SeeInvisible;

typedef struct Renderable {
    bool should_render;
} Renderable;

void register_components(ecs_world_t *world);
ecs_entity_t *inv_get_free_slot(Inventory *inv);
ecs_entity_t *inv_get_slot_of_item(Inventory *inv, ecs_entity_t e);
