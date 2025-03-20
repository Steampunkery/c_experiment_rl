#pragma once
#include "flecs.h"
#include "rogue.h"
#include "arena.h"

#include "rlsmenu.h"
#include "sockui.h"

#define GET_NAME_COMP(world, e) ecs_get(world, e, Name)->s

#define COMPONENTS            \
    COMPONENT(Name)           \
    COMPONENT(Religious)      \
    COMPONENT(Map)            \
    COMPONENT(MenuNetWrapper) \
    COMPONENT(WieldDescriptor)

#define COMPONENT(c) extern ECS_COMPONENT_DECLARE(c);
COMPONENTS
#undef COMPONENT

#define META_COMPS                                             \
    /* NOTE: Cannot register hooks on tags, so Invisible
     * and SeeInvisible must be *non-empty* structs */         \
    META_COMP(Invisible, STRUCT, { char dummy; });             \
    META_COMP(SeeInvisible, STRUCT, { char dummy; });          \
    META_COMP(Position, STRUCT, {                              \
        int32_t x;                                             \
        int32_t y;                                             \
    });                                                        \
    META_COMP(Stack, STRUCT, { int32_t val; });                \
    META_COMP(Weight, STRUCT, { float val; });                 \
    META_COMP(Value, STRUCT, { float val; });                  \
    META_COMP(Satiation, STRUCT, { float val; });              \
    META_COMP(Glyph, STRUCT, { uint32_t c; });                 \
    META_COMP(WeaponStats, STRUCT, {                           \
        uint8_t n;                                             \
        uint8_t sides;                                         \
        uint8_t offset;                                        \
    });                                                        \
    META_COMP(StatusType, ENUM, { SE_Probability, SE_Timed }); \
    META_COMP(StatusParam, STRUCT, {                           \
        StatusType type;                                       \
        uint64_t arg;                                          \
    });                                                        \
    META_COMP(GenStatusEffect, STRUCT, {                       \
        /* Generic parameter: num turns, prob to stop, etc */  \
        StatusParam param;                                     \
        /* The status effect component to add
         * (must have a default constructor) */                \
        ecs_id_t effect_comp;                                  \
        /* The target of the effect.
         * Set when timer entity is created */                 \
        ecs_entity_t target;                                   \
    });                                                        \
    META_COMP(EffectCallback, ENUM, { EC_HEALTH_POTION,        \
            EC_MAX });                                         \
    META_COMP(EntityCallbackEffect, STRUCT, {                  \
        EffectCallback ec;                                     \
        uint64_t arg;                                          \
    });                                                        \
    META_COMP(Health, STRUCT, {                                \
        int32_t total;                                         \
        int32_t val;                                           \
    });                                                        \
    META_COMP(Inventory, STRUCT, {                             \
        uint32_t data_id;                                      \
        uint32_t capacity;                                     \
        uint32_t count;                                        \
    });                                                        \
    META_COMP(InitiativeData, STRUCT, {                        \
        int32_t points;                                        \
        int32_t increment;                                     \
    });                                                        \
    META_COMP(AICallback, ENUM, { AIC_LEFT_WALKER,             \
            AIC_NOTHING, AIC_GREEDY, AIC_PET, AIC_ENEMY,       \
            AIC_MAX });                                        \
    META_COMP(AIController, STRUCT, {                          \
        AICallback aic;                                        \
    });                                                        \
    META_COMP(EnemyAIParams, STRUCT, {                         \
        float health_flee_p;                                   \
    });                                                        \
    META_COMP(WieldDescriptor, STRUCT, {                       \
        ecs_entity_t main;                                     \
    });

#define ECS_META_IMPL EXTERN
#define META_COMP(c, t, ...) ECS_##t(c, __VA_ARGS__)
META_COMPS
#undef META_COMP
#undef ECS_META_IMPL

// Typedefs for alternate names of meta components here
typedef struct WeaponStats DamageRoll;

#define TAGS              \
    TAG(Renderable)       \
    TAG(MyTurn)           \
    TAG(Targets)          \
    TAG(HasQuaffEffect)   \
    TAG(InInventory)      \
    TAG(Poison)           \
    TAG(Dead)             \
    TAG(ActionFromSocket) \
    TAG(Fiery)            \
    TAG(OnFire)

typedef struct Religion Religion;
typedef struct FrameData FrameData;

#define TAG(t) extern ECS_TAG_DECLARE(t);
TAGS
#undef TAG

/* Include count for easy (de)serialization. TODO: Consider replacing this with
 * a real wide string type */
typedef struct Name {
    wchar_t *s;
    // Does not include null terminator
    int32_t count;
} Name;

typedef struct Religious {
    Religion *religion;
    int favors_left;
} Religious;

typedef struct MenuNetWrapper {
    uint32_t last_data_id;
    uint16_t client_port;

    sockui_t sui;
    rlsmenu_gui gui;
    FrameData *frame_data;
    arena a;
} MenuNetWrapper;

void register_components(ecs_world_t *world);
bool inv_full(Inventory const *inv);
void inv_insert(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e);
void inv_delete(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e);
