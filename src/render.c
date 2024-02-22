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
    const Logger *l = ecs_singleton_get(it->world, Logger);

    WINDOW *basewin = ((WindowHolder *) it->param)->base;
    WINDOW *logwin = ((WindowHolder *) it->param)->log;

    // Logging
    const wchar_t *message = get_last_log_msg(l);
    mvwaddwstr(logwin, 0, 0, message);
    for (int i = wcslen(message); i < MAX_LOG_MSG_LEN; i++)
        mvwaddch(logwin, 0, i, ' ');

    // Map
    mvwaddstr(basewin, 0, 0, get_map_str(map));

    // Entities
    while (ecs_query_next(it)) {
        Position *pos = ecs_field(it, Position, 1);
        Glyph *glyph = ecs_field(it, Glyph, 2);
        Renderable *renderable = ecs_field(it, Renderable, 3);

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
    wrefresh(logwin);
}
