#include "render.h"

#include "rogue.h"
#include "component.h"
#include "map.h"
#include "log.h"

#include "rlsmenu.h"
#include "flecs.h"
#include <uncursed/uncursed.h>

void Render(ecs_iter_t *it)
{
    Map *map = ecs_singleton_get_mut(it->world, Map);
    GameVars *vars = it->param;

    // Logging
    static const char blank[] = {[0 ... MAX_LOG_MSG_LEN] = ' '};
    if (log_has_pending(&g_game_log)) {
        const wchar_t *message = get_last_log_msg(&g_game_log);
        mvwaddwstr(vars->logwin, 0, 0, message);
        mvwaddnstr(vars->logwin, 0, wcslen(message), blank, MAX_LOG_MSG_LEN - wcslen(message));
        wnoutrefresh(vars->logwin);
    }

    // Map
    switch(vars->state) {
    case PlayerTurn:
    case RunSystems:
        mvwaddstr(vars->basewin, 0, 0, get_map_str(map));

        // Entities
        while (ecs_query_next(it)) {
            Position *pos = ecs_field(it, Position, 0);
            Glyph *glyph = ecs_field(it, Glyph, 1);
            Renderable *renderable = ecs_field(it, Renderable, 2);

            for (int i = 0; i < it->count; i++) {
                if (renderable[i].should_render)
                    mvwaddch(vars->basewin, pos[i].y, pos[i].x, A_NORMAL | glyph[i].c);
            }
        }

        Health const *health = ecs_get(it->world, g_player_id, Health);
        Position const *pos = ecs_get(it->world, g_player_id, Position);
        mvwprintw(vars->statuswin, 0, 0, "Health: %3d\tPos: (%3d, %3d)", health->val, pos->x, pos->y);
        wnoutrefresh(vars->statuswin);

        break;
    case GUI:
    case NewGUIFrame:
        rlsmenu_str str = rlsmenu_get_menu_str(vars->gui);
        if (str.str)
            for (int i = 0; i < str.h; i++)
                mvwaddwnstr(vars->basewin, i, 0, str.str + i * str.w, str.w);
        /* FALLTHROUGH */
    default:
        ecs_iter_fini(it);
    }

    wrefresh(vars->basewin);
}
