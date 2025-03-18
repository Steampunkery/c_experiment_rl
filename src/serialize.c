#include "serialize.h"
#include <wchar.h>

ECS_COMPONENT_DECLARE(wstr);
static_assert(sizeof(wchar_t) == sizeof(ecs_i32_t));

static size_t wstr_count(void const *ptr)
{
    return wcslen(ptr);
}

static int wstr_serialize(ecs_serializer_t const *ser, void const *ptr)
{
    wstr data = *(wstr *)ptr;
    size_t count = wstr_count(data);

    for (size_t i = 0; i < count; i++)
        ser->value(ser, ecs_id(ecs_i32_t), data + i);

    return 0;
}

void register_serialize(ecs_world_t *world)
{
    ECS_COMPONENT_DEFINE(world, wstr);

    ecs_opaque(world, {
        .entity = ecs_id(wstr),
        .type = {
            .as_type = ecs_vector(world, { .type = ecs_id(ecs_i32_t) }),
            .count = wstr_count,
            .serialize = wstr_serialize
        }
    });
}
