#include "render.h"

#include "rogue.h"
#include "component.h"
#include "map.h"
#include "log.h"

#include "rlsmenu.h"
#include "flecs.h"
#include "uncursed.h"

void Render(ecs_iter_t *it)
{
    Map *map = ecs_singleton_get_mut(it->world, Map);
    Position const *player_pos = ecs_get(it->world, g_player_id, Position);
    GameVars *vars = it->param;

    // Logging
    static unsigned int render_log_data_id = -1;
    static const char blank[] = {[0 ... MAX_LOG_MSG_LEN] = ' '};
    if (g_game_log.data_id != render_log_data_id) {
        render_log_data_id = g_game_log.data_id;
        for (int i = 0; i < 3; i++) {
            const wchar_t *message = nth_log_msg(&g_game_log, i);
            if (message == NULL)
                continue;
            mvwaddwstr(vars->logwin, 2-i, 0, message);
            mvwaddnstr(vars->logwin, 2-i, wcslen(message), blank, MAX_LOG_MSG_LEN - wcslen(message));
            wnoutrefresh(vars->logwin);
        }
    }

    // Map
    switch(vars->state) {
    case PlayerTurn:
    case RunSystems:
        curs_set(1);
        mvwaddstr(vars->basewin, 0, 0, get_map_str(map));

        // Entities
        while (ecs_query_next(it)) {
            Position *pos = ecs_field(it, Position, 0);
            Glyph *glyph = ecs_field(it, Glyph, 1);

            for (int i = 0; i < it->count; i++)
                mvwaddch(vars->basewin, pos[i].y, pos[i].x, A_NORMAL | glyph[i].c);
        }

        Health const *health = ecs_get(it->world, g_player_id, Health);
        mvwprintw(vars->statuswin, 0, 0, "Health: %3d\tPos: (%3d, %3d)", health->val, player_pos->x, player_pos->y);
        wnoutrefresh(vars->statuswin);
        // TODO: This is probably a hack
        mvwchgat(vars->basewin, player_pos->y, player_pos->x, 1, A_NORMAL, 1, NULL);

        break;
    case GUI:
    case NewGUIFrame:
        curs_set(0);
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
