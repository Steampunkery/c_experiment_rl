#include "flecs.h"
#include "component.h"
#include "monster.h"
#include "input.h"

void left_walker(ecs_world_t *world, ecs_entity_t e, void *arg) {
    try_move_entity(world, e, &input_to_movement['4']);
}

void do_nothing(ecs_world_t *world, ecs_entity_t e, void *arg) {
    try_move_entity(world, e, &input_to_movement['5']);
}

