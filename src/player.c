#include "player.h"

#include "monster.h"
#include "religion.h"

#include "flecs.h"
#include <uncursed/uncursed.h>

ecs_entity_t init_player(ecs_world_t *world)
{
    ecs_entity_t player = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = "player" });
    ecs_set(world, player, Position, { 10, 10 });
    ecs_set(world, player, Glyph, { '@' });
    ecs_set(world, player, Renderable, { true });
    ecs_set(world, player, Inventory, { 10, 0, { 0 } });
    ecs_add_id(world, player, MyTurn);

    add_follower(world, &pastafarianism, player);

    return player;
}

// Returns a bool indicating if the player's turn is done
bool process_player_input(ecs_world_t *world, KeyInfo key)
{
    bool ret = false;
    switch (key.status) {
    case OK:
        InputType in = get_input_type(&key);

        // TODO?: Separate these cases out into callbacks
        switch (in) {
        case MovementInput:
            ret = try_move_entity(world, g_player_id, &input_to_movement[key.key]);
            break;
        case WaitInput:
            ecs_set(world, g_player_id, MovementAction, { 0, 0 });
            ret = true;
            break;
        case PickupInput:
            if (!inv_full(ecs_get(world, g_player_id, Inventory))) {
                ecs_set(world, g_player_id, PickupAction, { 0 });
                ret = true;
            }
            break;
        case PrayerInput:
            ecs_set(world, g_player_id, PrayerAction, { '\0' });
            ret = true;
            break;
        case NotImplemented:
            break;
        }
        break;
    case KEY_CODE_YES:
        break;
    default:
        ret = false;
    }

    return ret;
}
