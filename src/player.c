#include "player.h"

#include "input.h"
#include "item.h"
#include "map.h"
#include "monster.h"
#include "religion.h"
#include "component.h"
#include "prefab.h"

#include "flecs.h"
#include "rogue.h"
#include <uncursed/uncursed.h>

ecs_entity_t init_player(ecs_world_t *world)
{
    ecs_entity_t player = ecs_entity_init(world, &(ecs_entity_desc_t) {
            .name = "player",
            .set = ecs_values(
                { ecs_isa(Player), NULL },
                ecs_value(Position, { 10, 10 }),
                ecs_value(Glyph, { '@' }),
                ecs_value(Inventory, INV_NEW(10))
            ),
            .add = ecs_ids(ActionFromSocket),
    });

    ecs_remove(world, player, AIController);
    ecs_enable_component(world, player, ActionFromSocket, false);
    add_follower(world, &pastafarianism, player);
    map_place_entity(world, ecs_singleton_get_mut(world, Map), player, 10, 10);

    return player;
}

// Returns a bool indicating if the player's turn is done
GameState process_player_input(ecs_world_t *world, KeyInfo *key)
{
    CommandType cmd = get_command(key, 7);
    if (cmd == GUICommand || cmd == PlayerGUICommand)
        return NewGUIFrame;
    else if (cmd == QuitCommand)
        return Quit;

    if (key->status != OK)
        return PlayerTurn;

    InputType in = get_input_type(key);
    Position const *pos, *pos2;
    ecs_entity_t e;

    switch (in) {
    case MovementInput:
        pos = &direction9[key->key - '1'];
        pos2 = ecs_get(world, g_player_id, Position);

        // TODO: Factions!
        e = first_prefab_at_pos(world, ecs_singleton_get(world, Map), Goblin, pos2->x + pos->x, pos2->y + pos->y, &(int){0});
        if (e != 0) {
            ecs_set_pair_second(world, g_player_id, HasAction, AttackAction, { e });
            return RunSystems;
        }

        int cost = get_cost_for_movement(pos->x, pos->y);
        MovementAction mov = { pos->x, pos->y, cost };

        return try_move_entity(world, g_player_id, &mov) ? RunSystems : PlayerTurn;

    case WaitInput:
        ecs_set_pair_second(world, g_player_id, HasAction, MovementAction, { 0, 0, 10 });
        return RunSystems;

    case PickupInput:
        if (inv_full(ecs_get(world, g_player_id, Inventory)))
            return PlayerTurn;

        int n;
        pos = ecs_get(world, g_player_id, Position);
        e = first_prefab_at_pos(world, ecs_singleton_get(world, Map), Item, pos->x, pos->y, &n);

        if (e == 0)
            return PlayerTurn;

        if (n > 1)
            return NewGUIFrame;

        ecs_set_pair_second(world, g_player_id, HasAction, PickupAction, { e });
        return RunSystems;

    case PrayerInput:
        ecs_set_pair_second(world, g_player_id, HasAction, PrayerAction, { '\0' });
        return RunSystems;

    case NotImplemented:
        return PlayerTurn;
    }

    assert(!"Unreachable");
}
