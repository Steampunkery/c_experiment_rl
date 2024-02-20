#include "rogue.h"
#include "flecs.h"
#include "gui.h"
#include "render.h"
#include "component.h"
#include "input.h"
#include <glib.h>
#include <uncursed/uncursed.h>
#include <assert.h>

ECS_COMPONENT_DECLARE(GuiStack);

char *items_to_menu_body(ecs_world_t *world, const void *args, int len, FilterFunc f);
bool drop_item_cb(ecs_world_t *world, const void *e);

GuiFrame gui_frames[52] = {
    [alpha_to_idx('d')] = {
        0, 0, 0,
        NULL,
        inventory_cb,
        NULL,
        drop_item_cb,
        true
    },
    [alpha_to_idx('i')] = {
        0, 0, 0,
        NULL,
        inventory_cb,
        NULL,
        NULL,
        false
    },
};

bool drop_item_cb(ecs_world_t *world, const void *e) {
    ecs_set(world, g_player_id, DropAction, { *(ecs_entity_t *) e });
    return true;
}

CommandType inventory_cb(ecs_world_t *world, GuiFrame *gf, KeyInfo *key) {
    const Inventory *inv = ecs_get_id(world, g_player_id, ecs_id(Inventory));
    if (!gf->content) {
        gf->content = items_to_menu_body(world, inv->items, inv->end, gf->filter);
        if (!gf->content) return CancelCommand;
    }

    if (!key) return InvalidCommand;

    CommandType ret = InvalidCommand;
    switch (key->status) {
        case KEY_CODE_YES:
        case OK:
            if (key->key == KEY_ESCAPE) ret = CancelCommand;
            else if (is_alpha(key->key) && gf->action) {
                int idx = alpha_to_idx(key->key);
                bool is_valid_choice = (idx < inv->end)
                    && (!gf->filter || gf->filter(world, &inv->items[idx]));
                ret = is_valid_choice && gf->action(world, &inv->items[idx])
                    ? SuccessCommand : InvalidCommand;
            }
            break;
    }

    return ret;
}

// TODO: Abstract body constructors into FP?
// Caller owns the returned string
char *items_to_menu_body(ecs_world_t *world, const void *args, int len, FilterFunc f) {
    assert(len < 52);
    const ecs_entity_t *items = args;

    const Item *item;
    GString *str = g_string_sized_new(32);
    for (int i = 0; i < len; i++) {
        if (!items[i]) continue;

        item = ecs_get_id(world, items[i], ecs_id(Item));
        assert(item);

        if (f && !f(world, item)) continue;
        g_string_append_printf(str, "(%c) %s\n", idx_to_alpha[i], item->name);
    }

    return g_string_free(str, str->len < 1);
}

