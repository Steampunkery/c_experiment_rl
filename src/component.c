#include "component.h"

#include "map.h"
#include "gui.h"

#define COMPONENT(c) ECS_COMPONENT_DECLARE(c);
COMPONENTS
#undef COMPONENT
ECS_ON_ADD(InitiativeData, ptr, { ecs_add(_it->world, entity, MyTurn); });
ECS_ON_SET(InitiativeData, ptr, { ecs_add(_it->world, entity, MyTurn); });

#define TAG(t) ECS_TAG_DECLARE(t);
TAGS
#undef TAG

void register_components(ecs_world_t *world)
{
#define COMPONENT(c) ECS_COMPONENT_DEFINE(world, c);
COMPONENTS
#undef COMPONENT
    ecs_set_hooks(world, InitiativeData, { .on_set = ecs_on_set(InitiativeData), .on_add = ecs_on_add(InitiativeData) });

#define TAG(t) ECS_TAG_DEFINE(world, t);
TAGS
#undef TAG
    ecs_add_id(world, MyTurn, EcsCanToggle);
    // NOTE: Union relationships cannot have data
    ecs_add_id(world, InInventory, EcsUnion);
    // TODO: Find a better way to store this information? This causes a table
    // move every wield/unwield
    ecs_add_id(world, IsWielding, EcsUnion);
    ecs_add_id(world, ActionFromSocket, EcsCanToggle);
}

bool inv_full(const Inventory *inv)
{
    return inv->end >= inv->capacity;
}

bool inv_insert(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e)
{
    if (inv->end >= inv->capacity || !e) return false;

    inv->items[inv->end++] = e;
    inv->data_id++;
    ecs_add_pair(world, e, InInventory, owner);

    return true;
}

bool inv_delete(ecs_world_t *world, Inventory *inv, ecs_entity_t owner, ecs_entity_t e)
{
    for (int i = 0; i < inv->capacity; i++)
        if (inv->items[i] == e) {
            inv->items[i] = inv->items[--inv->end];
            inv->data_id++;
            ecs_remove_pair(world, e, InInventory, owner);
            return true;
        }

    return false;
}
