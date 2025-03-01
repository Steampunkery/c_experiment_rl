#include "action.h"

#include "component.h"
#include "item.h"
#include "log.h"
#include "map.h"
#include "random.h"
#include "religion.h"

#define HAS_QUAFF_EFFECT(effect) (effect_type = ecs_get_mut_pair_second(world, quaff_e, HasQuaffEffect, effect))

void Move(ecs_world_t *world, ecs_entity_t e, MovementAction *ma)
{
    InitiativeData *init = ecs_get_mut(world, e, InitiativeData);
    if (!ma->x && !ma->y)
        goto out;

    Position *pos = ecs_get_mut(world, e, Position);
    Map *map = ecs_singleton_get_mut(world, Map);

    map_remove_entity(world, map, e, pos->x, pos->y);
    pos->x += ma->x;
    pos->y += ma->y;
    map_place_entity(world, map, e, pos->x, pos->y);

out:
    init->points -= ma->cost;
}

void Pickup(ecs_world_t *world, ecs_entity_t e, PickupAction *pa)
{
    Inventory *inv = ecs_get_mut(world, e, Inventory);
    Position const *pos = ecs_get(world, e, Position);
    InitiativeData *init = ecs_get_mut(world, e, InitiativeData);

    ecs_entity_t pickup_e = pickup_item(world, pa->entity, pos->x, pos->y);
    inv_insert(world, inv, e, pickup_e);
    init->points -= 50;
}

void Drop(ecs_world_t *world, ecs_entity_t e, DropAction *da)
{
    Inventory *inv = ecs_get_mut(world, e, Inventory);
    Position const *pos = ecs_get(world, e, Position);
    InitiativeData *init = ecs_get_mut(world, e, InitiativeData);

    assert(da->entity != 0);
    place_item(world, da->entity, pos->x, pos->y); // error handling here

    inv_delete(world, inv, e, da->entity);

    init->points -= 50;
}

void Quaff(ecs_world_t *world, ecs_entity_t e, QuaffAction *qa)
{
    Inventory *inv = ecs_get_mut(world, e, Inventory);
    InitiativeData *init = ecs_get_mut(world, e, InitiativeData);

    ecs_entity_t quaff_e = qa->entity;
    assert(quaff_e != 0);

    // Remove quaffable from inventory
    inv_delete(world, inv, e, quaff_e); // Note, this implies that potions _must_ be present in inventory when quaffed

    void *effect_type;
    if (HAS_QUAFF_EFFECT(TimedStatusEffect)) {
        TimedStatusEffect *tse = effect_type;
        tse->target= e;

        // ChildOf to do automatic deletion
        ecs_entity(world, {
                .set = ecs_values(
                        ecs_value_ptr(TimedStatusEffect, effect_type),
                        ecs_value(InitiativeData, { 0, 10 }),
                        { ecs_pair(Targets, tse->target), NULL }),
                .add = ecs_ids(tse->effect_comp, ecs_pair(EcsChildOf, tse->target)) // Use add to invoke constructor
        });
    } else if (HAS_QUAFF_EFFECT(EntityCallbackEffect)) {
        EntityCallbackEffect *ece = effect_type;
        ece->f(world, e, ece->arg);
    } else if (ecs_has_pair(world, quaff_e, HasQuaffEffect, EcsWildcard)) {
        assert(!"Unsupported quaff effect");
    }

    // After quaffing, the potion no longer exists
    ecs_delete(world, quaff_e);

    init->points -= 50;
}

void Attack(ecs_world_t *world, ecs_entity_t e, AttackAction *aa)
{
    InitiativeData *init = ecs_get_mut(world, e, InitiativeData);

    WieldDescriptor const *wd = ecs_get(world, e, WieldDescriptor);
    WeaponStats const *ws = wd && wd->main ? ecs_get(world, wd->main, WeaponStats) : NULL;

    Health *target_health = ecs_get_mut(world, aa->target, Health);

    // TODO: Calculate damage based on weapon, strength, defence, etc
    int val = ws ? roll(ws->n, ws->sides) + ws->offset : roll(1, 4) + 1;
    log_msg(&g_game_log, L"%S hits %S for %d dmg", GET_NAME_COMP(world, e), GET_NAME_COMP(world, aa->target), val);
    target_health->val -= val;

    if (target_health->val <= 0)
        ecs_add(world, aa->target, Dead);

    init->points -= 100;

}

void Prayer(ecs_world_t *world, ecs_entity_t e, void *)
{
    InitiativeData *init = ecs_get_mut(world, e, InitiativeData);

    if (!ecs_has_id(world, e, ecs_id(Religious)) && e == g_player_id){
        log_msg(&g_game_log, L"Your supplication falls upon deaf ears");
        goto out;
    }

    Religious *rel = ecs_get_mut(world, e, Religious);
    Religion *r = rel->religion;
    if (rel->favors_left > 0 && r->boons[r->boon_idx])
        bestow_boon(world, rel, e);
    else if (e == g_player_id)
        log_msg(&g_game_log, L"%S is indifferent to your plea", rel->religion->deity_name);

out:
    init->points -= 200;
}

