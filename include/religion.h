#pragma once
#include "rogue.h"
#include <stddef.h>
#include <stdint.h>

typedef struct Religious Religious;
typedef struct Position Position;
typedef struct ecs_world_t ecs_world_t;

enum boon_type { BOONTYPE_ABILITY, BOONTYPE_ITEM };

// TODO: Rewrite the boon system with better ECS principles (?)
typedef struct Boon {
    wchar_t name[NAME_LEN_MAX];
    enum boon_type type;
} Boon;

typedef struct AbilityBoon {
    Boon super;
    ecs_entity_t *ability_id;
    void *ability_data;
} AbilityBoon;

typedef struct ItemBoon {
    Boon super;
    ecs_entity_t *item;
} ItemBoon;

typedef struct Religion {
    wchar_t religion_name[NAME_LEN_MAX];
    wchar_t deity_name[NAME_LEN_MAX];
    wchar_t follower_title[NAME_LEN_MAX];
    unsigned int num_followers;
    Boon *boons[3];
    uint8_t boon_idx;
    // TODO: Put more fields in here
} Religion;

extern Religion pastafarianism;
extern AbilityBoon AB_SeeInvisible;
extern ItemBoon IB_Mjolnir;

void add_follower(ecs_world_t *, Religion *, ecs_entity_t);
void bestow_boon(ecs_world_t *, Religious *, ecs_entity_t);
void bestow_ability(ecs_world_t *, AbilityBoon *, ecs_entity_t);
void bestow_item(ecs_world_t *, ItemBoon *, const Position *);
