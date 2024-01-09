#include "component.h"
#include "map.h"
#include "gui.h"
#include "log.h"

#include "flecs.h"

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

ECS_TAG_DECLARE(Invisible);
ecs_entity_t MyTurn;

void register_components(ecs_world_t *world) {
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

    ECS_TAG_DEFINE(world, Invisible);
    MyTurn = ecs_new_id(world);

    ECS_COMPONENT_DEFINE(world, Map);
    ECS_COMPONENT_DEFINE(world, GuiStack);
    ECS_COMPONENT_DEFINE(world, Logger);
}

ecs_entity_t *inv_get_free_slot(Inventory *inv) {
    int i;
    for (i = 0; i < inv->capacity; i++)
        if (inv->items[i] == 0) return &inv->items[i];

    return NULL;
}

ecs_entity_t *inv_get_slot_of_item(Inventory *inv, ecs_entity_t e) {
    int i;
    for (i = 0; i < inv->capacity; i++)
        if (inv->items[i] == e) return &inv->items[i];

    return NULL;
}
