#pragma once
#include "flecs.h"
#include "rogue.h"
#include "item.h"

typedef int wchar_t;
typedef struct Religion Religion;

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
extern ECS_COMPONENT_DECLARE(rlsmenu_gui);
extern ECS_COMPONENT_DECLARE(Logger);
extern ECS_COMPONENT_DECLARE(Map);
extern ECS_COMPONENT_DECLARE(InitiativeData);
extern ECS_COMPONENT_DECLARE(StatBlock);
extern ECS_COMPONENT_DECLARE(Equippable);
extern ECS_COMPONENT_DECLARE(Weight);

extern ECS_TAG_DECLARE(Invisible);
extern ECS_TAG_DECLARE(MyTurn);

typedef struct Equippable {
    /*ArmorSlot as;*/
    StatBlockMod stats;
} Equippable;

typedef struct Weight {
    int w;
} Weight;

typedef struct Value {
    int v;
} Value;

typedef struct Edible {
    int e;
} Edible;

typedef struct Position {
    int x, y;
} Position;

typedef struct MovementAction {
    int x, y;
    int cost;
} MovementAction;

// TODO: Don't allow entity == 0
typedef struct PickupAction {
    // Entity to pick up or 0 for first item in the stack
    ecs_entity_t entity;
} PickupAction, DropAction;

typedef struct PrayerAction {
    char dummy;
} PrayerAction;

typedef struct Inventory {
    int capacity;
    int end;
    ecs_entity_t items[INVENTORY_MAX];
} Inventory;

typedef struct Glyph {
    wchar_t c;
} Glyph;

typedef struct Actor {
    int (*act)(ecs_world_t *world, ecs_entity_t *e);
} Actor;

typedef struct AIController {
    void (*ai_func)(ecs_world_t *world, ecs_entity_t e, void *arg);
    void *state;
} AIController;

typedef struct Religious {
    Religion *religion;
    int favors_left;
} Religious;

typedef struct SeeInvisible {
    char dummy;
} SeeInvisible;

typedef struct Renderable {
    bool should_render;
} Renderable;

typedef struct InitiativeData {
    int points;
    int increment;
} InitiativeData;

void register_components(ecs_world_t *world);
bool inv_full(const Inventory *inv);
bool inv_insert(Inventory *inv, ecs_entity_t e);
bool inv_delete(Inventory *inv, ecs_entity_t e);
