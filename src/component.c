#include "component.h"

#include "map.h"
#include "gui.h"

#include <stdlib.h>
#include <wchar.h>

#define COMPONENT(c) ECS_COMPONENT_DECLARE(c);
COMPONENTS
#undef COMPONENT
ECS_ON_ADD(InitiativeData, ptr, { ecs_add(_it->world, entity, MyTurn); });
ecs_iter_action_t ecs_on_set(InitiativeData) = ecs_on_add(InitiativeData);

void ecs_on_add(Invisible)(ecs_iter_t *it)
{
    for (int32_t i = 0; i < it->count; i++)
        if (!ecs_has(it->world, g_player_id, SeeInvisible))
            ecs_remove(it->world, it->entities[i], Renderable);
}
ecs_iter_action_t ecs_on_set(Invisible) = ecs_on_add(Invisible);

void ecs_on_remove(Invisible)(ecs_iter_t *it)
{
    for (int32_t i = 0; i < it->count; i++)
        ecs_add(it->world, it->entities[i], Renderable);
}

void ToggleSeeInvisible(ecs_world_t *world, bool toggle)
{
    ecs_query_t *q = ecs_query_init (world, &(ecs_query_desc_t) {
        .terms = { { .id = ecs_id(Invisible) } },
        .cache_kind = EcsQueryCacheNone
    });

    void (*add_or_remove)(ecs_world_t *, ecs_entity_t, ecs_id_t) = toggle ? ecs_add_id : ecs_remove_id;
    ecs_iter_t it = ecs_query_iter(world, q);
    while (ecs_query_next(&it))
        for (int i = 0; i < it.count; i++)
            add_or_remove(world, it.entities[i], Renderable);

    ecs_query_fini(q);
}

void ecs_on_add(SeeInvisible)(ecs_iter_t *it)
{
    for (int32_t i = 0; i < it->count; i++)
        if (it->entities[i] == g_player_id)
            goto found;
    return;

found:
    ToggleSeeInvisible(it->world, true);
}
ecs_iter_action_t ecs_on_set(SeeInvisible) = ecs_on_add(SeeInvisible);

void ecs_on_remove(SeeInvisible)(ecs_iter_t *it)
{
    for (int32_t i = 0; i < it->count; i++)
        if (it->entities[i] == g_player_id)
            goto found;
    return;

found:
    ToggleSeeInvisible(it->world, false);
}

ECS_CTOR(Name, ptr, {
    ptr->s = NULL;
    ptr->count = 0;
});
ECS_DTOR(Name, ptr, {
    free((void *) ptr->s);
    ptr->count = 0;
});
ECS_COPY(Name, dst, src, {
    dst->s = wcsdup(src->s);
    dst->count = src->count;
});
ECS_MOVE(Name, dst, src, {
    dst->s = src->s;
    dst->count = src->count;
    src->s = NULL;
    src->count = 0;
});

#define META_COMP(c, t, ...) ECS_META_IMPL_CALL(ECS_##t##_, IMPL, c, #__VA_ARGS__)
META_COMPS
#undef META_COMP

#define TAG(t) ECS_TAG_DECLARE(t);
TAGS
#undef TAG

void register_components(ecs_world_t *world)
{
#define META_COMP(c, ...) ECS_META_COMPONENT(world, c);
    META_COMPS
#undef META_COMP

#define COMPONENT(c) ECS_COMPONENT_DEFINE(world, c);
    COMPONENTS
#undef COMPONENT
    ecs_set_hooks(world, InitiativeData,
                  { .on_set = ecs_on_set(InitiativeData), .on_add = ecs_on_add(InitiativeData) });
    ecs_set_hooks(world, Name,
                  { .ctor = ecs_ctor(Name),
                    .move = ecs_move(Name),
                    .copy = ecs_copy(Name),
                    .copy_ctor = ecs_copy(Name),
                    .dtor = ecs_dtor(Name) });

#define TAG(t) ECS_TAG_DEFINE(world, t);
    TAGS
#undef TAG
    ecs_set_hooks(world, Invisible, { .on_add = ecs_on_add(Invisible),
            .on_set = ecs_on_set(Invisible), .on_remove = ecs_on_remove(Invisible) });
    ecs_set_hooks(world, SeeInvisible, { .on_add = ecs_on_add(SeeInvisible),
            .on_set = ecs_on_set(SeeInvisible), .on_remove = ecs_on_remove(SeeInvisible) });

    ecs_add_id(world, MyTurn, EcsCanToggle);
    // NOTE: Union relationships cannot have data
    ecs_add_id(world, InInventory, EcsUnion);
    // TODO: Find a better way to store this information? This causes a table
    // move every wield/unwield
    ecs_add_id(world, ActionFromSocket, EcsCanToggle);
}

bool inv_full(Inventory const *inv)
{
    assert(inv->count <= inv->capacity);
    return inv->count == inv->capacity;
}

void inv_insert(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e)
{
    assert(inv->count < inv->capacity);
    assert(e != 0);

    inv->data_id++;
    inv->count++;
    ecs_add_pair(world, e, InInventory, owner);
}

void inv_delete(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e)
{
    assert(inv->count > 0);
    ecs_remove_pair(world, e, InInventory, owner);
    inv->count--;
}
