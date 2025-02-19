#include "render.h"

#include "component.h"
#include "map.h"
#include "log.h"

#include "rlsmenu.h"
#include "flecs.h"
#include <uncursed/uncursed.h>

void Render(ecs_iter_t *it)
{
    Map *map = ecs_singleton_get_mut(it->world, Map);
    rlsmenu_gui *gui = ecs_singleton_get_mut(it->world, rlsmenu_gui);

    WINDOW *basewin = ((WindowHolder *) it->param)->base;
    WINDOW *logwin = ((WindowHolder *) it->param)->log;

    // Logging
    static const char blank[] = {[0 ... MAX_LOG_MSG_LEN] = ' '};
    if (log_has_pending(&g_game_log)) {
        const wchar_t *message = get_last_log_msg(&g_game_log);
        mvwaddwstr(logwin, 0, 0, message);
        mvwaddnstr(logwin, 0, wcslen(message), blank, MAX_LOG_MSG_LEN - wcslen(message));
        wnoutrefresh(logwin);
    }

    // Map
    mvwaddstr(basewin, 0, 0, get_map_str(map));

    // Entities
    while (ecs_query_next(it)) {
        Position *pos = ecs_field(it, Position, 0);
        Glyph *glyph = ecs_field(it, Glyph, 1);
        Renderable *renderable = ecs_field(it, Renderable, 2);

        for (int i = 0; i < it->count; i++) {
            if (renderable[i].should_render)
                mvwaddch(basewin, pos[i].y, pos[i].x, A_NORMAL | glyph[i].c);
        }
    }

    // GUI
    rlsmenu_str str = rlsmenu_get_menu_str(gui);
    if (str.str)
        for (int i = 0; i < str.h; i++)
            mvwaddwnstr(basewin, i, 0, str.str + i * str.w, str.w);

    wrefresh(basewin);
}
