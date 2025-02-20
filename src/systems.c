#include "systems.h"

#include "component.h"
#include "render.h"
#include "log.h"
#include "religion.h"
#include "item.h"

#include "flecs.h"

ecs_entity_t render;
ecs_entity_t initiative;
ecs_query_t *initiative_q;

ECS_SYSTEM_DECLARE(Move);
ECS_SYSTEM_DECLARE(Pickup);
ECS_SYSTEM_DECLARE(Drop);
ECS_SYSTEM_DECLARE(Quaff);
ECS_SYSTEM_DECLARE(AI);
ECS_SYSTEM_DECLARE(Prayer);
ECS_SYSTEM_DECLARE(ApplyPoison);
ECS_SYSTEM_DECLARE(StatusEffectTimer);

void register_systems(ecs_world_t *world)
{
    ECS_SYSTEM_DEFINE(world, Move, EcsOnUpdate, Position, MovementAction, InitiativeData);
    ECS_SYSTEM_DEFINE(world, Pickup, EcsOnUpdate, Inventory, PickupAction, Position, InitiativeData);
    ECS_SYSTEM_DEFINE(world, Drop, EcsOnUpdate, Inventory, DropAction, Position, InitiativeData);
    ECS_SYSTEM_DEFINE(world, Quaff, EcsOnUpdate, Inventory, QuaffAction, InitiativeData);
    ECS_SYSTEM_DEFINE(world, AI, EcsOnUpdate, AIController, MyTurn);
    ECS_SYSTEM_DEFINE(world, Prayer, EcsOnUpdate, PrayerAction, InitiativeData);
    ECS_SYSTEM_DEFINE(world, ApplyPoison, EcsOnUpdate, (Targets, $t), Health($t), Poison, MyTurn);
    ECS_SYSTEM_DEFINE(world, StatusEffectTimer, EcsOnUpdate, TimedStatusEffect, InitiativeData, MyTurn);

    render = ecs_system(world, {
        .query.terms = {
            { .id = ecs_id(Position) },
            { .id = ecs_id(Glyph) },
            { .id = ecs_id(Renderable) },
        },
        .run = Render
    });

    initiative_q = ecs_query(world, {
        .terms = { { .id = ecs_id(InitiativeData) } },
        .cache_kind = EcsQueryCacheAuto,
    });

    initiative = ecs_system(world, {
        .run = Initiative
    });
}

void Initiative(ecs_iter_t *it_)
{
    ecs_iter_t it = ecs_query_iter(it_->world, initiative_q);
    bool would_grant_turn = false;

    // Give initiatiive points until someone has a turn
    do {
        while (ecs_query_next(&it)) {
            InitiativeData *init = ecs_field(&it, InitiativeData, 0);

            for (int i = 0; i < it.count; i++) {
                init[i].points += init[i].increment;
                if (init[i].points >= 0) would_grant_turn = true;
            }
        }

        it = ecs_query_iter(it_->world, initiative_q);
    } while (!would_grant_turn);

    // Grant turns to all eligible entities
    while(ecs_query_next(&it)) {
        InitiativeData *init = ecs_field(&it, InitiativeData, 0);

        for (int i = 0; i < it.count; i++)
            if (init[i].points >= 0)
                ecs_add(it.world, it.entities[i], MyTurn);
            else
                ecs_remove(it.world, it.entities[i], MyTurn);
    }
}

void Move(ecs_iter_t *it)
{
    Position *pos = ecs_field(it, Position, 0);
    MovementAction *mov = ecs_field(it, MovementAction, 1);
    InitiativeData *init = ecs_field(it, InitiativeData, 2);

    for (int i = 0; i < it->count; i++) {
        pos[i].x += mov[i].x;
        pos[i].y += mov[i].y;
        init[i].points -= mov[i].cost;
        ecs_remove(it->world, it->entities[i], MovementAction);
    }
}

void AI(ecs_iter_t *it)
{
    AIController *aic = ecs_field(it, AIController, 0);

    for (int i = 0; i < it->count; i++) {
        aic[i].ai_func(it->world, it->entities[i], aic[i].state);
    }
}

void Pickup(ecs_iter_t *it)
{
    Inventory *inv = ecs_field(it, Inventory, 0);
    PickupAction *pa = ecs_field(it, PickupAction, 1);
    Position *pos = ecs_field(it, Position, 2);
    InitiativeData *init = ecs_field(it, InitiativeData, 3);

    for (int i = 0; i < it->count; i++) {
        if (!inv_full(&inv[i])) {
            ecs_entity_t e = pickup_item(it->world, pa[i].entity, pos[i].x, pos[i].y);

            // Consider rolling the below two into one function because they
            // should nearly always happen together
            if (inv_insert(&inv[i], e)) // error handling here
                ecs_add_pair(it->world, e, InInventory, it->entities[i]);
        }
        init[i].points -= 50;
        ecs_remove(it->world, it->entities[i], PickupAction);
    }
}

void Drop(ecs_iter_t *it)
{
    Inventory *inv = ecs_field(it, Inventory, 0);
    DropAction *da = ecs_field(it, DropAction, 1);
    Position *pos = ecs_field(it, Position, 2);
    InitiativeData *init = ecs_field(it, InitiativeData, 3);

    for (int i = 0; i < it->count; i++) {
        assert(da[i].entity != 0);
        place_item(it->world, da[i].entity, pos[i].x, pos[i].y); // error handling here

        // Consider rolling the below two into one function because they
        // should nearly always happen together
        assert(inv_delete(&inv[i], da[i].entity));
        ecs_remove_pair(it->world, da[i].entity, InInventory, it->entities[i]);

        init[i].points -= 50;
        ecs_remove(it->world, it->entities[i], DropAction);
    }
}

void Quaff(ecs_iter_t *it)
{
    Inventory *inv = ecs_field(it, Inventory, 0);
    QuaffAction *qa = ecs_field(it, QuaffAction, 1);
    InitiativeData *init = ecs_field(it, InitiativeData, 2);

    ecs_entity_t e;
    for (int i = 0; i < it->count; i++) {
        e = qa[i].entity;
        assert(e != 0);

        // Remove quaffable from inventory
        assert(inv_delete(&inv[i], e)); // Note, this implies that potions _must_ be present in inventory when quaffed
        ecs_remove_pair(it->world, e, InInventory, it->entities[i]);

        void *effect_type;
        if ((effect_type = ecs_get_mut_pair_second(it->world, e, HasQuaffEffect, TimedStatusEffect))) {
            TimedStatusEffect *tse = effect_type;
            tse->target= it->entities[i];

            ecs_entity(it->world, {
                    .set = ecs_values(
                            ecs_value_ptr(TimedStatusEffect, effect_type),
                            ecs_value(InitiativeData, { 0, 10 }),
                            { ecs_pair(Targets, tse->target), NULL }),
                    .add = ecs_ids(tse->effect_comp) // Use add to invoke constructor
            });
        } else if (ecs_has_pair(it->world, e, HasQuaffEffect, EcsWildcard)) {
            assert(!"Unsupported quaff effect");
        }

        // After quaffing, the potion no longer exists
        ecs_delete(it->world, e);

        init[i].points -= 50;
        ecs_remove(it->world, it->entities[i], QuaffAction);
    }

}

void Prayer(ecs_iter_t *it)
{
    InitiativeData *init = ecs_field(it, InitiativeData, 1);

    Religious *rel;
    for (int i = 0; i < it->count; i++) {
        if (!ecs_has_id(it->world, it->entities[i], ecs_id(Religious))) {
            if (it->entities[i] == g_player_id) {
                log_msg(&g_game_log, L"Your supplication falls upon deaf ears");
            }
        } else {
            rel = ecs_get_mut(it->world, it->entities[i], Religious);
            Religion *r = rel->religion;
            if (rel->favors_left > 0 && r->boons[r->boon_idx]) {
                bestow_boon(it->world, rel, it->entities[i]);
            } else {
                if (it->entities[i] == g_player_id) {
                    log_msg(&g_game_log, L"%S is indifferent to your plea", rel->religion->deity_name);
                }
            }
        }

        init[i].points -= 200;
        ecs_remove(it->world, it->entities[i], PrayerAction);
    }
}

void ApplyPoison(ecs_iter_t *it)
{
    Health *health = ecs_field(it, Health, 1);

    for (int i = 0; i < it->count; i++) {
        health[i].val -= 1;
        log_msg(&g_debug_log, L"Poisoned: %d", health[i].val);
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
