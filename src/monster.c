#include "monster.h"

#include "component.h"
#include "ai.h"
#include "map.h"

#include "flecs.h"
#include <math.h>

int get_cost_for_movement(int x, int y)
{
    // Waiting should cost a full turn
    if (!x && !y) return 100;
    double dist = round(sqrt(x*x + y*y) * 10) / 10;
    return (int) (dist * 100);
}

void make_invisible(ecs_world_t *world, ecs_entity_t e)
{
    ecs_add_id(world, e, ecs_id(Invisible));
}

// Returns if the entity was successfully moved
bool try_move_entity(ecs_world_t *world, ecs_entity_t e, MovementAction *mov)
{
    if (entity_can_traverse(world, e, &(Position){ .x = mov->x, .y = mov->y })) {
        ecs_set_id(world, e, ecs_id(MovementAction), sizeof(MovementAction), mov);
        return true;
    }

    return false;
}
