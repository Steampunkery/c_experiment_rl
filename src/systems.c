#include "systems.h"

#include "component.h"
#include "input.h"
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

void Move(ecs_iter_t *it)
{
    Position *pos = ecs_field(it, Position, 0);
    MovementAction *mov = ecs_field(it, MovementAction, 1);
    InitiativeData *init = ecs_field(it, InitiativeData, 2);
    Map *map = ecs_singleton_get_mut(it->world, Map);

    for (int i = 0; i < it->count; i++) {
        map_remove_entity(it->world, map, it->entities[i], pos[i].x, pos[i].y);
        pos[i].x += mov[i].x;
        pos[i].y += mov[i].y;
        map_place_entity(it->world, map, it->entities[i], pos[i].x, pos[i].y);
        init[i].points -= mov[i].cost;
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
    }

}

void Attack(ecs_iter_t *it)
{
    AttackAction *aa = ecs_field(it, AttackAction, 0);
    Position *pos = ecs_field(it, Position, 1);
    InitiativeData *init = ecs_field(it, InitiativeData, 2);

    for (int i = 0; i < it->count; i++) {
        Position const *target_pos = ecs_get(it->world, aa[i].target, Position);
        Health *target_health = ecs_get_mut(it->world, aa[i].target, Health);

        int j;
        for (j = 0; j < 9; j++) {
            if (target_pos->x == (pos->x + direction9[j].x) && target_pos->y == (pos->y + direction9[j].y))
                break;
        }

        if (j != 9)
            // TODO: Calculate damage based on weapon, strength, defence, etc
            target_health->val -= 20;

        init[i].points -= 100;
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
