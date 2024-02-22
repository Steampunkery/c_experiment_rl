#include "monster.h"

#include "component.h"
#include "ai.h"
#include "map.h"

#include "flecs.h"

ecs_entity_t init_goblin(ecs_world_t *world, int x, int y)
{
    ecs_entity_t goblin = ecs_new(world, 0);
    ecs_set(world, goblin, Position, { x, y });
    ecs_set(world, goblin, Glyph, { 'g' });
    ecs_set(world, goblin, Renderable, { true });
    ecs_set(world, goblin, AIController, { do_nothing, NULL });
    ecs_set(world, goblin, Inventory, { 10, 0, { 0 } });
    ecs_add_id(world, goblin, MyTurn);

    return goblin;
}

void make_invisible(ecs_world_t *world, ecs_entity_t e)
{
    ecs_add_id(world, e, ecs_id(Invisible));
}

// Returns if the entity was successfully moved
bool try_move_entity(ecs_world_t *world, ecs_entity_t e, MovementAction *mov)
{
    if (entity_can_traverse(world, e, mov)) {
        ecs_set_id(world, e, ecs_id(MovementAction), sizeof(MovementAction), mov);
        return true;
    }

    return false;
}
