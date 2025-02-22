#include "component.h"

#include "map.h"
#include "gui.h"

ECS_COMPONENT_DECLARE(Name);
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Weight);
ECS_COMPONENT_DECLARE(Value);
ECS_COMPONENT_DECLARE(Satiation);
ECS_COMPONENT_DECLARE(Stack);
ECS_COMPONENT_DECLARE(Health);
ECS_COMPONENT_DECLARE(Inventory);
ECS_COMPONENT_DECLARE(MovementAction);
ECS_COMPONENT_DECLARE(PickupAction);
ECS_COMPONENT_DECLARE(DropAction);
ECS_COMPONENT_DECLARE(QuaffAction);
ECS_COMPONENT_DECLARE(AIController);
ECS_COMPONENT_DECLARE(Glyph);
ECS_COMPONENT_DECLARE(MyTurn);
ECS_COMPONENT_DECLARE(Religious);
ECS_COMPONENT_DECLARE(PrayerAction);
ECS_COMPONENT_DECLARE(SeeInvisible);
ECS_COMPONENT_DECLARE(Renderable);
ECS_COMPONENT_DECLARE(rlsmenu_gui);
ECS_COMPONENT_DECLARE(Map);
ECS_COMPONENT_DECLARE(InitiativeData);
ECS_ON_ADD(InitiativeData, ptr, { ecs_add(_it->world, entity, MyTurn); });
ECS_ON_SET(InitiativeData, ptr, { ecs_add(_it->world, entity, MyTurn); });
ECS_COMPONENT_DECLARE(MenuNetWrapper);
ECS_COMPONENT_DECLARE(TimedStatusEffect);

ECS_TAG_DECLARE(Invisible);
ECS_TAG_DECLARE(MyTurn);
ECS_TAG_DECLARE(Targets);
ECS_TAG_DECLARE(HasQuaffEffect);
ECS_TAG_DECLARE(InInventory);
ECS_TAG_DECLARE(Poison);
ECS_TAG_DECLARE(HasAction);
ECS_TAG_DECLARE(ActionFromSocket);

void register_components(ecs_world_t *world)
{
    ECS_COMPONENT_DEFINE(world, Name);
    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Weight);
    ECS_COMPONENT_DEFINE(world, Value);
    ECS_COMPONENT_DEFINE(world, Satiation);
    ECS_COMPONENT_DEFINE(world, Stack);
    ECS_COMPONENT_DEFINE(world, Health);
    ECS_COMPONENT_DEFINE(world, Inventory);
    ECS_COMPONENT_DEFINE(world, MovementAction);
    ECS_COMPONENT_DEFINE(world, PickupAction);
    ECS_COMPONENT_DEFINE(world, DropAction);
    ECS_COMPONENT_DEFINE(world, QuaffAction);
    ECS_COMPONENT_DEFINE(world, AIController);
    ECS_COMPONENT_DEFINE(world, Glyph);
    ECS_COMPONENT_DEFINE(world, Religious);
    ECS_COMPONENT_DEFINE(world, PrayerAction);
    ECS_COMPONENT_DEFINE(world, SeeInvisible);
    ECS_COMPONENT_DEFINE(world, Renderable);
    ECS_COMPONENT_DEFINE(world, InitiativeData);
    ecs_set_hooks(world, InitiativeData, { .on_set = ecs_on_set(InitiativeData), .on_add = ecs_on_add(InitiativeData) });
    ECS_COMPONENT_DEFINE(world, MenuNetWrapper);
    ECS_COMPONENT_DEFINE(world, TimedStatusEffect);

    ECS_TAG_DEFINE(world, Invisible);
    ECS_TAG_DEFINE(world, MyTurn);
    ecs_add_id(world, MyTurn, EcsCanToggle);
    ECS_TAG_DEFINE(world, Targets);
    ECS_TAG_DEFINE(world, HasQuaffEffect);
    ECS_TAG_DEFINE(world, InInventory);
    // NOTE: Union relationships cannot have data
    ecs_add_id(world, InInventory, EcsUnion);
    ECS_TAG_DEFINE(world, Poison);
    ECS_TAG_DEFINE(world, HasAction);
    ecs_add_id(world, HasAction, EcsExclusive);
    ECS_TAG_DEFINE(world, ActionFromSocket);
    ecs_add_id(world, ActionFromSocket, EcsCanToggle);

    ECS_COMPONENT_DEFINE(world, Map);
    ECS_COMPONENT_DEFINE(world, rlsmenu_gui);
}

bool inv_full(const Inventory *inv)
{
    return inv->end >= inv->capacity;
}

bool inv_insert(Inventory *inv, ecs_entity_t e)
{
    if (inv->end >= inv->capacity || !e) return false;
    inv->items[inv->end++] = e;
    inv->data_id++;
    return true;
}

bool inv_delete(Inventory *inv, ecs_entity_t e)
{
    for (int i = 0; i < inv->capacity; i++)
        if (inv->items[i] == e) {
            inv->items[i] = inv->items[--inv->end];
            inv->data_id++;
            return true;
        }

    return false;
}
