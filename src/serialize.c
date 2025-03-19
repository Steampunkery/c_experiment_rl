#include "serialize.h"

#include "component.h"

#include <wchar.h>
#include <stdlib.h>

static_assert(sizeof(wchar_t) == sizeof(ecs_i32_t));

static size_t Name_count(void const *ptr)
{
    Name const *data = ptr;
    return data->count;
}

static int Name_serialize(ecs_serializer_t const *ser, void const *ptr)
{
    Name const *data = ptr;

    for (int32_t i = 0; i < data->count; i++)
        ser->value(ser, ecs_id(ecs_i32_t), data + i);

    return 0;
}

static void Name_resize(void *ptr, size_t size)
{
    Name *data = ptr;

    if (data->count == (int32_t) size)
        return;

    data->count = size;
    if (!data->count) {
        free(data->s);
        data->s = NULL;
        data->count = 0;
        return;
    }

    data->s = reallocarray(data->s, sizeof(int32_t), size+1);
    data->s[size] = L'\0';
}

static void* Name_ensure_element(void *ptr, size_t index) {
    Name *data = ptr;

    if (data->count <= (int32_t) index)
        Name_resize(ptr, index + 1);

    return &data->s[index];
}

void register_serialize(ecs_world_t *world)
{
    ecs_opaque(world, {
        .entity = ecs_id(Name),
        .type = {
            .as_type = ecs_vector(world, { .type = ecs_id(ecs_i32_t) }),
            .count = Name_count,
            .serialize = Name_serialize,
            .resize = Name_resize,
            .ensure_element = Name_ensure_element
        }
    });
}
