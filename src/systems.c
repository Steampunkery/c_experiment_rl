#include "flecs.h"
#include "systems.h"
#include "component.h"
#include "map.h"
#include "render.h"
#include "log.h"
#include "item.h"

ecs_entity_t render;

ECS_SYSTEM_DECLARE(Move);
ECS_SYSTEM_DECLARE(Pickup);
ECS_SYSTEM_DECLARE(Drop);
ECS_SYSTEM_DECLARE(AI);
ECS_SYSTEM_DECLARE(Prayer);

void register_systems(ecs_world_t *world) {
    ECS_SYSTEM_DEFINE(world, Move, EcsOnUpdate, Position, MovementAction);
    ECS_SYSTEM_DEFINE(world, Pickup, EcsOnUpdate, Inventory, PickupAction, Position);
    ECS_SYSTEM_DEFINE(world, Drop, EcsOnUpdate, Inventory, DropAction, Position);
    ECS_SYSTEM_DEFINE(world, AI, EcsOnUpdate, AIController);
    ECS_SYSTEM_DEFINE(world, Prayer, EcsOnUpdate, PrayerAction);

    render = ecs_system(world, {
        .query.filter.terms = {
            { .id = ecs_id(Position) },
            { .id = ecs_id(Glyph) },
            { .id = ecs_id(Renderable) },
        },
        .run = Render
    });
}

void Move(ecs_iter_t *it) {
    Position *pos = ecs_field(it, Position, 1);
    MovementAction *mov = ecs_field(it, MovementAction, 2);

    for (int i = 0; i < it->count; i++) {
        pos[i].x += mov[i].x;
        pos[i].y += mov[i].y;
        ecs_remove(it->world, it->entities[i], MovementAction);
    }
}

void AI(ecs_iter_t *it) {
    AIController *aic = ecs_field(it, AIController, 1);

    for (int i = 0; i < it->count; i++) {
        aic[i].ai_func(it->world, it->entities[i], aic[i].state);
    }
}

void Pickup(ecs_iter_t *it) {
    Map *map = ecs_singleton_get_mut(it->world, Map);
    Inventory *inv = ecs_field(it, Inventory, 1);
    PickupAction *pa = ecs_field(it, PickupAction, 2);
    Position *pos = ecs_field(it, Position, 3);

    ecs_entity_t *inv_slot;
    for (int i = 0; i < it->count; i++) {
        if (inv_slot = inv_get_free_slot(&inv[i]))
            *inv_slot = pickup_item(it->world, pa[i].entity, pos[i].x, pos[i].y); // error handling here
        ecs_remove(it->world, it->entities[i], PickupAction);
    }
}

void Drop(ecs_iter_t *it) {
    Map *map = ecs_singleton_get_mut(it->world, Map);
    Inventory *inv = ecs_field(it, Inventory, 1);
    DropAction *da = ecs_field(it, DropAction, 2);
    Position *pos = ecs_field(it, Position, 3);

    ecs_entity_t *inv_slot;
    for (int i = 0; i < it->count; i++) {
        assert(da[i].entity != 0);
        place_item(it->world, da[i].entity, pos[i].x, pos[i].y); // error handling here
        if (inv_slot = inv_get_slot_of_item(&inv[i], da[i].entity))
            *inv_slot = 0;
        ecs_remove(it->world, it->entities[i], DropAction);
    }
}

void Prayer(ecs_iter_t *it) {
    PrayerAction *pa = ecs_field(it, PrayerAction, 1);

    Religious *rel;
    for (int i = 0; i < it->count; i++) {
        if (!ecs_has_id(it->world, it->entities[i], ecs_id(Religious))) {
            if (it->entities[i] == g_player_id) {
                Logger *l = ecs_singleton_get_mut(it->world, Logger);
                log_msg(l, "Your supplication falls upon deaf ears");
            }
        } else {
            rel = ecs_get_mut(it->world, it->entities[i], Religious);
            if (rel->favors_left > 0) {
                bestow_boon(it->world, rel, it->entities[i]);
            } else {
                if (it->entities[i] == g_player_id) {
                    Logger *l = ecs_singleton_get_mut(it->world, Logger);
                    log_msg(l, "%s is indifferent to your plea", rel->religion->deity_name);
                }
            }
        }
        ecs_remove(it->world, it->entities[i], PrayerAction);
    }
}
