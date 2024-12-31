#include "systems.h"

#include "component.h"
#include "render.h"
#include "log.h"
#include "religion.h"
#include "item.h"

#include "flecs.h"

ecs_entity_t render;

ECS_SYSTEM_DECLARE(Initiative);
ECS_SYSTEM_DECLARE(Move);
ECS_SYSTEM_DECLARE(Pickup);
ECS_SYSTEM_DECLARE(Drop);
ECS_SYSTEM_DECLARE(AI);
ECS_SYSTEM_DECLARE(Prayer);

void register_systems(ecs_world_t *world)
{
    ECS_SYSTEM_DEFINE(world, Initiative, EcsOnUpdate, InitiativeData);
    ECS_SYSTEM_DEFINE(world, Move, EcsOnUpdate, Position, MovementAction, InitiativeData);
    ECS_SYSTEM_DEFINE(world, Pickup, EcsOnUpdate, Inventory, PickupAction, Position, InitiativeData);
    ECS_SYSTEM_DEFINE(world, Drop, EcsOnUpdate, Inventory, DropAction, Position, InitiativeData);
    ECS_SYSTEM_DEFINE(world, AI, EcsOnUpdate, AIController, MyTurn);
    ECS_SYSTEM_DEFINE(world, Prayer, EcsOnUpdate, PrayerAction, InitiativeData);

    render = ecs_system(world, {
        .query.terms = {
            { .id = ecs_id(Position) },
            { .id = ecs_id(Glyph) },
            { .id = ecs_id(Renderable) },
        },
        .run = Render
    });
}

void Initiative(ecs_iter_t *it)
{
    InitiativeData *init = ecs_field(it, InitiativeData, 0);

    for (int i = 0; i < it->count; i++) {
        init[i].points += init[i].increment;
        if (init[i].points >= 0)
            ecs_add(it->world, it->entities[i], MyTurn);
        else
            ecs_remove(it->world, it->entities[i], MyTurn);
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
            inv_insert(&inv[i], e); // error handling here
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
        assert(inv_delete(&inv[i], da[i].entity));

        init[i].points -= 50;
        ecs_remove(it->world, it->entities[i], DropAction);
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
            rel = ecs_ensure(it->world, it->entities[i], Religious);
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
