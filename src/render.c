#include "component.h"
#include "flecs.h"
#include "map.h"
#include "render.h"
#include "gui.h"
#include "log.h"

#include <uncursed/uncursed.h>
#include <glib.h>
#include <assert.h>

void render_gui_frame(gpointer item, gpointer win) {
    GuiFrame *gf = item;
    mvwaddstr((WINDOW *)win, gf->x, gf->y, gf->content);
}

void Render(ecs_iter_t *it) {
    const Map *map = ecs_singleton_get(it->world, Map);
    const GuiStack *gs = ecs_singleton_get(it->world, GuiStack);
    const Logger *l = ecs_singleton_get(it->world, Logger);

    WINDOW *basewin = ((WindowHolder *)it->param)->base;
    WINDOW *logwin = ((WindowHolder *)it->param)->log;

    // Logging
    const char *message = get_last_log_msg(l);
    mvwaddstr(logwin, 0, 0, message);
    for (int i = strlen(message); i < MAX_LOG_MSG_LEN; i++)
        mvwaddch(logwin, 0, i, ' ');

    // Map
    GString *map_str = g_string_sized_new(map->rows*map->cols + 1);
    for (int i = 0; i < map->rows; i++)
        for (int j = 0; j < map->cols; j++)
            g_string_append_c(map_str, tiletype_to_wchar[map->grid[i][j]]);

    mvwaddstr(basewin, 0, 0, map_str->str);
    g_string_free(map_str, TRUE);

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
    g_slist_foreach(gs->frames, (GFunc)render_gui_frame, basewin);

    wrefresh(basewin);
    wrefresh(logwin);
}
