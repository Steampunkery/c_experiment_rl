#pragma once
#include "rogue.h"

typedef struct Religious Religious;
typedef struct Position Position;
typedef struct ecs_world_t ecs_world_t;

typedef enum { BOONTYPE_ABILITY, BOONTYPE_ITEM } BoonType;

typedef struct Boon {
    char name[NAME_LEN_MAX];
    BoonType type;
} Boon;

typedef struct AbilityBoon {
    Boon super;
    ecs_entity_t* ability_id;
    void *ability_data;
} AbilityBoon;

typedef struct ItemBoon {
    Boon super;
    char item_name[NAME_LEN_MAX];
} ItemBoon;

typedef struct Religion {
    char religion_name[NAME_LEN_MAX];
    char deity_name[NAME_LEN_MAX];
    char follower_title[NAME_LEN_MAX];
    unsigned int num_followers;
    Boon *boons[3];
    unsigned char boon_idx;
    // TODO: Put more fields in here
} Religion;

extern Religion pastafarianism;
extern AbilityBoon AB_SeeInvisible;
extern ItemBoon IB_Mjolnir;

void add_follower(ecs_world_t *, Religion *, ecs_entity_t);
void bestow_boon(ecs_world_t *, Religious *, ecs_entity_t);
void bestow_ability(ecs_world_t *, AbilityBoon *, ecs_entity_t);
void bestow_item(ecs_world_t *, ItemBoon *, const Position *);
