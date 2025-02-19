#include "observer.h"

#include "component.h"

#include "flecs.h"

/* NOTE: Many functions in this file assume that any entity with the
 * Invisible tag also have the Renderable component. Look at this code
 * again if this assumption changes.
 */

void AddInvisible(ecs_iter_t *it);
void RemoveInvisible(ecs_iter_t *it);
void AddSeeInvisible(ecs_iter_t *it);
void RemoveSeeInvisible(ecs_iter_t *it);

void register_observers(ecs_world_t *world)
{
    ECS_OBSERVER(world, AddInvisible, EcsOnAdd, Invisible);
    ECS_OBSERVER(world, RemoveInvisible, EcsOnRemove, Invisible);
    ECS_OBSERVER(world, AddSeeInvisible, EcsOnAdd, SeeInvisible);
    ECS_OBSERVER(world, RemoveSeeInvisible, EcsOnRemove, SeeInvisible);
}

void AddInvisible(ecs_iter_t *it)
{
    bool si = ecs_has_id(it->world, g_player_id, ecs_id(SeeInvisible));

    for (int i = 0; i < it->count; i++) {
        Renderable *renderable = ecs_get_mut(it->world, it->entities[i], Renderable);
        renderable->should_render = false || !!si;
    }
}

void RemoveInvisible(ecs_iter_t *it)
{
    for (int i = 0; i < it->count; i++) {
        Renderable *renderable = ecs_get_mut(it->world, it->entities[i], Renderable);
        renderable->should_render = true;
    }
}

void ToggleSeeInvisible(ecs_world_t *world, bool toggle)
{
    // clang-format off
    ecs_query_t *f = ecs_query(world, {
        .terms = {
            { .id = ecs_id(Invisible) },
        },
        .cache_kind = EcsQueryCacheNone
    });
    // clang-format on

    ecs_iter_t it = ecs_query_iter(world, f);
    while (ecs_query_next(&it)) {
        for (int i = 0; i < it.count; i++) {
            Renderable *renderable = ecs_get_mut(world, it.entities[i], Renderable);
            renderable->should_render = toggle;
        }
    }

    ecs_query_fini(f);
}

void AddSeeInvisible(ecs_iter_t *it)
{
    for (int i = 0; i < it->count; i++) {
        if (it->entities[i] == g_player_id) break;
        if (i == it->count - 1) return;
    }

    ToggleSeeInvisible(it->world, true);
}

void RemoveSeeInvisible(ecs_iter_t *it)
{
    for (int i = 0; i < it->count; i++) {
        if (it->entities[i] == g_player_id) break;
        if (i == it->count - 1) return;
    }

    ToggleSeeInvisible(it->world, false);
}
