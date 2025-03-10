#include "religion.h"

#include "component.h"
#include "item.h"
#include "log.h"

#include "flecs.h"

Religion pastafarianism = {
    .religion_name = L"Pastafarianism",
    .deity_name = L"The Flying Spaghetti Monster",
    .follower_title = L"Pastafarian",
    .num_followers = 0,
    .boons = { &AB_SeeInvisible.super, &IB_Mjolnir.super, NULL },
    .boon_idx = 0,
};

AbilityBoon AB_SeeInvisible = {
    .super = {
        .name = L"See Invisible",
        .type = BOONTYPE_ABILITY,
    },
    .ability_id = &ecs_id(SeeInvisible),
    .ability_data = &(SeeInvisible) { '\0' },
};

ItemBoon IB_Mjolnir = {
    .super = {
        .name = L"The Warhammer Mjolnir",
        .type = BOONTYPE_ITEM,
    },
    .item = &mjolnir,
};

// Add a follower to a given religion
void add_follower(ecs_world_t *world, Religion *religion, ecs_entity_t e)
{
    ecs_set(world, e, Religious, { religion, 3 });
    religion->num_followers++;
}

void bestow_boon(ecs_world_t *world, Religious *rel, ecs_entity_t e)
{
    Religion *r = rel->religion;
    Boon *boon = r->boons[r->boon_idx];

    switch (boon->type) {
    case BOONTYPE_ABILITY:
        bestow_ability(world, (AbilityBoon *) boon, e);
        break;
    case BOONTYPE_ITEM:
        const Position *pos = ecs_get(world, e, Position);
        if (!pos) break;
        bestow_item(world, (ItemBoon *) boon, pos);
        break;
    }

    if (e == g_player_id) {
        log_msg(&g_game_log, L"%S grants you %S!", r->deity_name, boon->name);
    }

    rel->favors_left--;
    r->boon_idx++;
}

void bestow_ability(ecs_world_t *world, AbilityBoon *boon, ecs_entity_t e)
{
    const ecs_type_info_t *info = ecs_get_type_info(world, *boon->ability_id);
    ecs_set_id(world, e, *boon->ability_id, info->size, boon->ability_data);
}

void bestow_item(ecs_world_t *world, ItemBoon *boon, const Position *pos)
{
    place_item(world, *boon->item, pos->x, pos->y);
}
