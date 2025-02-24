#pragma once
#include "flecs.h"
#include "rogue.h"
#include "arena.h"

#include "rlsmenu.h"
#include "sockui.h"

#define INV_NEW(capacity) { 0, capacity, 0, { 0 } }

typedef int wchar_t;
typedef struct Religion Religion;
typedef struct FrameData FrameData;

extern ECS_COMPONENT_DECLARE(Name);
extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Weight);
extern ECS_COMPONENT_DECLARE(Value);
extern ECS_COMPONENT_DECLARE(Satiation);
extern ECS_COMPONENT_DECLARE(Stack);
extern ECS_COMPONENT_DECLARE(Health);
extern ECS_COMPONENT_DECLARE(Inventory);
extern ECS_COMPONENT_DECLARE(MovementAction);
extern ECS_COMPONENT_DECLARE(PickupAction);
extern ECS_COMPONENT_DECLARE(DropAction);
extern ECS_COMPONENT_DECLARE(QuaffAction);
extern ECS_COMPONENT_DECLARE(AIController);
extern ECS_COMPONENT_DECLARE(Glyph);
extern ECS_COMPONENT_DECLARE(Religious);
extern ECS_COMPONENT_DECLARE(PrayerAction);
extern ECS_COMPONENT_DECLARE(SeeInvisible);
extern ECS_COMPONENT_DECLARE(Renderable);
extern ECS_COMPONENT_DECLARE(Map);
extern ECS_COMPONENT_DECLARE(InitiativeData);
extern ECS_COMPONENT_DECLARE(MenuNetWrapper);
extern ECS_COMPONENT_DECLARE(TimedStatusEffect);

extern ECS_TAG_DECLARE(Invisible);
extern ECS_TAG_DECLARE(MyTurn);
extern ECS_TAG_DECLARE(Targets);
extern ECS_TAG_DECLARE(HasQuaffEffect);
extern ECS_TAG_DECLARE(InInventory);
extern ECS_TAG_DECLARE(Poison);
extern ECS_TAG_DECLARE(HasAction);
extern ECS_TAG_DECLARE(ActionFromSocket);

typedef struct Name {
    wchar_t const *s;
} Name;

typedef struct Position {
    int x, y;
} Position;

typedef struct Weight {
    float val;
} Weight, Value, Satiation;

typedef struct Stack {
    int val;
} Stack, Health;

typedef struct MovementAction {
    int x, y;
    int cost;
} MovementAction;

// TODO: Don't allow entity == 0
typedef struct InvItemAction {
    // Entity to pick up or 0 for first item in the stack
    ecs_entity_t entity;
} InvItemAction, PickupAction, DropAction, QuaffAction;

typedef struct PrayerAction {
    char dummy;
} PrayerAction;

typedef struct Inventory {
    MenuChangeCounter data_id;
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

typedef struct MenuNetWrapper {
    MenuChangeCounter last_data_id;
    uint16_t client_port;

    sockui_t sui;
    rlsmenu_gui gui;
    FrameData *frame_data;
    arena a;
} MenuNetWrapper;

typedef struct TimedStatusEffect {
    // How many turns the effect is active for
    int turns;
    // The status effect component to add (must have a default constructor)
    ecs_id_t effect_comp;
    // The target of the effect. Set when timer entity is created
    ecs_entity_t target;
} TimedStatusEffect;

void register_components(ecs_world_t *world);
bool inv_full(const Inventory *inv);
bool inv_insert(Inventory *inv, ecs_entity_t e);
bool inv_delete(Inventory *inv, ecs_entity_t e);
Inventory inv_new(int capacity);
