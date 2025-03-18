#pragma once
#include "flecs.h"
#include "rogue.h"
#include "arena.h"

#include "rlsmenu.h"
#include "sockui.h"

#define INV_NEW(capacity) { 0, capacity, 0, { 0 } }
#define GET_NAME_COMP(world, e) ecs_get(world, e, Name)->s

#define COMPONENTS                  \
    COMPONENT(Name)                 \
    COMPONENT(Weight)               \
    COMPONENT(Value)                \
    COMPONENT(Satiation)            \
    COMPONENT(Health)               \
    COMPONENT(Inventory)            \
    COMPONENT(AIController)         \
    COMPONENT(Glyph)                \
    COMPONENT(Religious)            \
    COMPONENT(SeeInvisible)         \
    COMPONENT(Renderable)           \
    COMPONENT(Map)                  \
    COMPONENT(InitiativeData)       \
    COMPONENT(MenuNetWrapper)       \
    COMPONENT(GenStatusEffect)      \
    COMPONENT(EntityCallbackEffect) \
    COMPONENT(WeaponStats)          \
    COMPONENT(WieldDescriptor)

#define COMPONENT(c) extern ECS_COMPONENT_DECLARE(c);
COMPONENTS
#undef COMPONENT

#define META_COMPS          \
    META_COMP(Position, {   \
        int32_t x;          \
        int32_t y;          \
    });                     \
    META_COMP(Stack, {      \
        int32_t val;        \
    });                     \
    META_COMP(Weight, {     \
        float val;          \
    });                     \
    META_COMP(Value, {      \
        float val;          \
    });                     \
    META_COMP(Satiation, {  \
        float val;          \
    });                     \
    META_COMP(Glyph, {      \
        uint32_t c;         \
    });                     \
    META_COMP(Renderable, { \
        bool should_render; \
    });

static_assert(sizeof(uint32_t) == sizeof(wchar_t));

#define ECS_META_IMPL EXTERN
#define META_COMP ECS_STRUCT
META_COMPS
#undef META_COMP
#undef ECS_META_IMPL

#define TAGS              \
    TAG(Invisible)        \
    TAG(MyTurn)           \
    TAG(Targets)          \
    TAG(HasQuaffEffect)   \
    TAG(InInventory)      \
    TAG(Poison)           \
    TAG(Dead)             \
    TAG(ActionFromSocket) \
    TAG(Fiery)            \
    TAG(OnFire)

typedef int wchar_t;
typedef struct Religion Religion;
typedef struct FrameData FrameData;

#define TAG(t) extern ECS_TAG_DECLARE(t);
TAGS
#undef TAG

typedef struct Name {
    wchar_t const *s;
} Name;

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

typedef enum { SE_Probability, SE_Timed } StatusType;
typedef struct StatusParam {
    StatusType type;
    union {
        int turns;
        int stop_perc;
    } p;
} StatusParam;

typedef struct GenStatusEffect {
    // Generic parameter: num turns, prob to stop, etc
    StatusParam param;
    // The status effect component to add (must have a default constructor)
    ecs_id_t effect_comp;
    // The target of the effect. Set when timer entity is created
    ecs_entity_t target;
} GenStatusEffect;

typedef struct EntityCallbackEffect {
    void (*f)(ecs_world_t *world, ecs_entity_t e, union cb_arg arg);
    union cb_arg arg;
} EntityCallbackEffect;

typedef struct DamageRoll {
    uint8_t n;
    uint8_t sides;
    uint8_t offset;
} WeaponStats, DamageRoll;

typedef struct WieldDescriptor {
    ecs_entity_t main;
} WieldDescriptor;

void register_components(ecs_world_t *world);
bool inv_full(const Inventory *inv);
void inv_insert(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e);
void inv_delete(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e);
Inventory inv_new(int capacity);
