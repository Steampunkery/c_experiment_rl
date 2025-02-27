#include "systems.h"

#include "component.h"
#include "map.h"
#include "render.h"
#include "log.h"
#include "religion.h"
#include "item.h"

#include "flecs.h"

#define SYSTEM(s, ...) void s(ecs_iter_t *it);
SYSTEMS
#undef SYSTEM

#define SYSTEM(s, ...) ECS_SYSTEM_DECLARE(s);
SYSTEMS
#undef SYSTEM

void Initiative(ecs_iter_t *it);

ecs_entity_t render;
ecs_entity_t initiative;

void register_systems(ecs_world_t *world)
{
#define SYSTEM(s, ...) ECS_SYSTEM_DEFINE(world, s, __VA_ARGS__);
    SYSTEMS
#undef SYSTEM

    render = ecs_system(world, {
        .query.terms = {
            { .id = ecs_id(Position) },
            { .id = ecs_id(Glyph) },
            { .id = ecs_id(Renderable) },
        },
        .run = Render
    });

    initiative = ecs_system(world, {
        .query.terms = {
            { .id = ecs_id(InitiativeData) }
        },
        .run = Initiative
    });
}

void Initiative(ecs_iter_t *it)
{
    bool would_grant_turn = false;

    // Give initiatiive points until someone has a turn
    do {
        while (ecs_query_next(it)) {
            InitiativeData *init = ecs_field(it, InitiativeData, 0);

            for (int i = 0; i < it->count; i++) {
                init[i].points += init[i].increment;
                if (init[i].points >= 0) would_grant_turn = true;
            }
        }

        *it = ecs_query_iter(it->world, it->query);
    } while (!would_grant_turn);

    // Grant turns to all eligible entities
    while(ecs_query_next(it)) {
        InitiativeData *init = ecs_field(it, InitiativeData, 0);

        for (int i = 0; i < it->count; i++)
            ecs_enable_component(it->world, it->entities[i], MyTurn, init[i].points >= 0);
    }
}

void AI(ecs_iter_t *it)
{
    AIController *aic = ecs_field(it, AIController, 0);

    for (int i = 0; i < it->count; i++) {
        aic[i].ai_func(it->world, it->entities[i], aic[i].state);
    }
}

void ApplyPoison(ecs_iter_t *it)
{
    Health *health = ecs_field(it, Health, 1);

    for (int i = 0; i < it->count; i++) {
        health->val -= 1;
        log_msg(&g_debug_log, L"Poisoned: %d", health->val);
    }
}

void StatusEffectTimer(ecs_iter_t *it)
{
    TimedStatusEffect *tse = ecs_field(it, TimedStatusEffect, 0);
    InitiativeData *init = ecs_field(it, InitiativeData, 1);

    for (int i = 0; i < it->count; i++) {
        init[i].points -= 100;
        if (--tse[i].turns > 0)
            continue;

        ecs_delete(it->world, it->entities[i]);
    }
}
