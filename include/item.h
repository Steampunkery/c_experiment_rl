#pragma once
#include <stdint.h>

typedef int wchar_t;
typedef uint64_t ecs_entity_t;
typedef struct ecs_world_t ecs_world_t;

ecs_entity_t create_item(ecs_world_t *world, wchar_t glyph, char *name);
ecs_entity_t place_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
ecs_entity_t pickup_item(ecs_world_t *world, ecs_entity_t e, int x, int y);
