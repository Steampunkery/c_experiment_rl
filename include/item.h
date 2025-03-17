#pragma once
#include "rogue.h"

#include <stdint.h>
#include <stddef.h>

typedef int wchar_t;
typedef uint64_t ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;
typedef struct Map Map;
typedef struct DamageRoll DamageRoll;

void item_init(ecs_world_t *world);
ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
ecs_entity_t first_prefab_at_pos(ecs_world_t *world, Map const *map, ecs_entity_t type, int x, int y, int *n);
void health_potion_cb(ecs_world_t *world, ecs_entity_t e, union cb_arg arg);
void apply_weapon_effects(ecs_world_t *world, ecs_entity_t w, ecs_entity_t, ecs_entity_t t, DamageRoll *);

extern ecs_entity_t mjolnir;
extern ecs_entity_t brisingr;

