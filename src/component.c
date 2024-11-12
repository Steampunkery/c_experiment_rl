#include "component.h"

#include "map.h"
#include "gui.h"
#include "log.h"

#include "rlsmenu.h"

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Inventory);
ECS_COMPONENT_DECLARE(MovementAction);
ECS_COMPONENT_DECLARE(PickupAction);
ECS_COMPONENT_DECLARE(DropAction);
ECS_COMPONENT_DECLARE(AIController);
ECS_COMPONENT_DECLARE(Glyph);
ECS_COMPONENT_DECLARE(MyTurn);
ECS_COMPONENT_DECLARE(Item);
ECS_COMPONENT_DECLARE(Religious);
ECS_COMPONENT_DECLARE(PrayerAction);
ECS_COMPONENT_DECLARE(SeeInvisible);
ECS_COMPONENT_DECLARE(Renderable);
ECS_COMPONENT_DECLARE(rlsmenu_gui);
ECS_COMPONENT_DECLARE(Logger);
ECS_COMPONENT_DECLARE(Map);
ECS_COMPONENT_DECLARE(InitiativeData);
ECS_COMPONENT_DECLARE(StatBlock);
ECS_COMPONENT_DECLARE(StatBlockMod);

ECS_TAG_DECLARE(Invisible);
ECS_TAG_DECLARE(MyTurn);

void register_components(ecs_world_t *world)
{
    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Inventory);
    ECS_COMPONENT_DEFINE(world, MovementAction);
    ECS_COMPONENT_DEFINE(world, PickupAction);
    ECS_COMPONENT_DEFINE(world, DropAction);
    ECS_COMPONENT_DEFINE(world, AIController);
    ECS_COMPONENT_DEFINE(world, Glyph);
    ECS_COMPONENT_DEFINE(world, Item);
    ECS_COMPONENT_DEFINE(world, Religious);
    ECS_COMPONENT_DEFINE(world, PrayerAction);
    ECS_COMPONENT_DEFINE(world, SeeInvisible);
    ECS_COMPONENT_DEFINE(world, Renderable);
    ECS_COMPONENT_DEFINE(world, InitiativeData);
    ECS_COMPONENT_DEFINE(world, StatBlock);
    ECS_COMPONENT_DEFINE(world, StatBlockMod);

    ECS_TAG_DEFINE(world, Invisible);
    ECS_TAG_DEFINE(world, MyTurn);

    ECS_COMPONENT_DEFINE(world, Map);
    ECS_COMPONENT_DEFINE(world, rlsmenu_gui);
    ECS_COMPONENT_DEFINE(world, Logger);
}

bool inv_full(const Inventory *inv)
{
    return inv->end >= inv->capacity;
}

bool inv_insert(Inventory *inv, ecs_entity_t e)
{
    if (inv->end >= inv->capacity || !e) return false;
    inv->items[inv->end++] = e;
    return true;
}

bool inv_delete(Inventory *inv, ecs_entity_t e)
{
    for (int i = 0; i < inv->capacity; i++)
        if (inv->items[i] == e) {
            inv->items[i] = inv->items[--inv->end];
            return true;
        }

    return false;
}
