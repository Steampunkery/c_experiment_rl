#pragma once
#include "flecs.h"
#include "rogue.h"
#include "arena.h"

#include "rlsmenu.h"
#include "sockui.h"

#define INV_NEW(capacity) { 0, capacity, 0, { 0 } }
#define GET_NAME_COMP(world, e) ecs_get(world, e, Name)->s

#define COMPONENTS            \
    COMPONENT(Name)           \
    COMPONENT(Position)       \
    COMPONENT(Weight)         \
    COMPONENT(Value)          \
    COMPONENT(Satiation)      \
    COMPONENT(Stack)          \
    COMPONENT(Health)         \
    COMPONENT(Inventory)      \
    COMPONENT(AIController)   \
    COMPONENT(Glyph)          \
    COMPONENT(Religious)      \
    COMPONENT(SeeInvisible)   \
    COMPONENT(Renderable)     \
    COMPONENT(Map)            \
    COMPONENT(InitiativeData) \
    COMPONENT(MenuNetWrapper) \
    COMPONENT(TimedStatusEffect) \
    COMPONENT(EntityCallbackEffect) \
    COMPONENT(WeaponStats)

#define COMPONENT(c) extern ECS_COMPONENT_DECLARE(c);
COMPONENTS
#undef COMPONENT

#define TAGS            \
    TAG(Invisible)      \
    TAG(MyTurn)         \
    TAG(Targets)        \
    TAG(HasQuaffEffect) \
    TAG(InInventory)    \
    TAG(Poison)         \
    TAG(IsWielding)     \
    TAG(ActionFromSocket)

typedef int wchar_t;
typedef struct Religion Religion;
typedef struct FrameData FrameData;

#define TAG(t) extern ECS_TAG_DECLARE(t);
TAGS
#undef TAG

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
} Stack;

typedef struct Health {
    int total;
    int val;
} Health;

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

typedef struct EntityCallbackEffect {
    void (*f)(ecs_world_t *world, ecs_entity_t e, union cb_arg arg);
    union cb_arg arg;
} EntityCallbackEffect;

typedef struct WeaponStats {
    uint8_t n;
    uint8_t sides;
    uint8_t offset;
} WeaponStats;

void register_components(ecs_world_t *world);
bool inv_full(const Inventory *inv);
bool inv_insert(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e);
bool inv_delete(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e);
Inventory inv_new(int capacity);
