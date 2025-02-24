#include "player.h"

#include "input.h"
#include "map.h"
#include "monster.h"
#include "religion.h"
#include "component.h"
#include "prefab.h"

#include "flecs.h"
#include <uncursed/uncursed.h>

ecs_entity_t init_player(ecs_world_t *world)
{
    ecs_entity_t player = ecs_entity_init(world, &(ecs_entity_desc_t) {
            .name = "player",
            .set = ecs_values(
                { ecs_isa(Player), NULL },
                ecs_value(Position, { 10, 10 }),
                ecs_value(Glyph, { '@' }),
                ecs_value( Inventory, INV_NEW(10))
            ),
            .add = ecs_ids(ActionFromSocket),
    });

    ecs_remove(world, player, AIController);
    ecs_enable_component(world, player, ActionFromSocket, false);
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
            Position *pos = &direction9[key.key - '1'];
            int cost = get_cost_for_movement(pos->x, pos->y);
            MovementAction mov = { pos->x, pos->y, cost };
            ret = try_move_entity(world, g_player_id, &mov);

            if (ret) {
                Map *map = ecs_singleton_get_mut(world, Map);
                map->dijkstra_maps[DM_ORDER_PLAYER].dirty = true;
            }

            break;
        case WaitInput:
            ecs_set_pair_second(world, g_player_id, HasAction, MovementAction, { 0, 0, 10 });
            ret = true;
            break;
        case PickupInput:
            if (!inv_full(ecs_get(world, g_player_id, Inventory))) {
                ecs_set_pair_second(world, g_player_id, HasAction, PickupAction, { 0 });
                ret = true;
            }
            break;
        case PrayerInput:
            ecs_set_pair_second(world, g_player_id, HasAction, PrayerAction, { '\0' });
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
