#include "systems.h"

#include "component.h"
#include "log.h"
#include "map.h"
#include "random.h"
#include "render.h"
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

    for (int i = 0; i < it->count; i++)
        health->val -= 1;
}

// TODO: Burn stuff
void ApplyOnFire(ecs_iter_t *it)
{
    Health *health = ecs_field(it, Health, 1);

    for (int i = 0; i < it->count; i++)
        health->val -= roll(1, 4);
}

// For status effect timers, treat param as turns until effect is done.
void ProcessStatusEffects(ecs_iter_t *it)
{
    GenStatusEffect *gse = ecs_field(it, GenStatusEffect, 0);
    InitiativeData *init = ecs_field(it, InitiativeData, 1);

    for (int i = 0; i < it->count; i++) {
        init[i].points -= 100;

        switch (gse->param.type) {
        case SE_Timed:
            if (--gse[i].param.arg > 0)
                continue;
            break;
        case SE_Probability:
            if (!perc_roll(gse[i].param.arg))
                continue;
            break;
        }

        ecs_delete(it->world, it->entities[i]);
    }
}

void DeathCleanup(ecs_iter_t *it)
{
    Health *health = ecs_field(it, Health, 0);
    Position *pos = ecs_field(it, Position, 1);
    Inventory *inv = ecs_field(it, Inventory, 2);

    Map *map = ecs_singleton_get_mut(it->world, Map);
    for (int i = 0; i < it->count; i++) {
        if (health[i].val > 0)
            continue;

        map_remove_entity(it->world, map, it->entities[i], pos[i].x, pos[i].y);

        if (inv)
            for (int j = 0; j < inv[i].end; j++) {
                ecs_remove_pair(it->world, inv[i].items[j], InInventory, it->entities[i]);
                place_item(it->world, inv[i].items[j], pos[i].x, pos[i].y);
            }

        WieldDescriptor const *wd;
        if ((wd = ecs_get(it->world, it->entities[i], WieldDescriptor)) && wd->main)
            place_item(it->world, wd->main, pos[i].x, pos[i].y);

        ecs_delete(it->world, it->entities[i]);
    }
}
