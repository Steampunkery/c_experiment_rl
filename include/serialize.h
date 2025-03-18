#pragma once

#include "flecs.h"

#include <wchar.h>

typedef wchar_t const * wstr;
extern ECS_COMPONENT_DECLARE(wstr);

void register_serialize(ecs_world_t *world);
