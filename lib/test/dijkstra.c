#include "dijkstra.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAP_HEIGHT 50
#define MAP_WIDTH 200
#define COLOR_DIV 12
#define COLOR_CAP 230

int myround(float x) {
    return (int)(x < 0 ? (x - 0.49999997f) : (x + 0.49999997f));
}

int x_dir[] = { 1, 0, -1, 0, 1, -1, -1, 1 };
int y_dir[] = { 0, 1, 0, -1, 1, 1, -1, -1 };

void successors8(size_t idx, const void *, successor_t **exits, uint8_t *n_exits) {
    static successor_t all[8];

    *n_exits = 0;
    for (int i = 0; i < 8; i++) {
        int x = idx % MAP_WIDTH + x_dir[i];
        int y = idx / MAP_WIDTH + y_dir[i];
        if (x < 0 || x > MAP_WIDTH-1 || y < 0 || y > MAP_HEIGHT-1)
            continue;

        all[*n_exits].idx = (int)idx + x_dir[i] + y_dir[i] * MAP_WIDTH;
        all[*n_exits].cost = i > 3 ? 1.4f : 1.0f;
        (*n_exits)++;
    }

    *exits = all;
}

int get_green(int i) {
    if (!i) return 0;

    if (i > COLOR_DIV)
        return COLOR_CAP;
    else {
        int color = (COLOR_CAP/COLOR_DIV)*i;
        return color > COLOR_CAP ? COLOR_CAP : color;
    }
}

int get_red(int i) {
    if (!i) return 255;

    if (i > COLOR_DIV) {
        int color = COLOR_CAP-(COLOR_CAP/COLOR_DIV)*(i-COLOR_DIV);
        return color < 0 ? 0 : color;
    } else {
        return COLOR_CAP;
    }
}

int main(void) {
    size_t w = MAP_WIDTH, h = MAP_HEIGHT;

    srandom(time(NULL));
    uint32_t n_sources = random() % 20 + 1;
    size_t *sources = malloc(n_sources * sizeof(*sources));

    for (int i = 0; i < n_sources; i++)
        sources[i] = random() % (MAP_WIDTH * MAP_HEIGHT);

    DijkstraMap dm;
    set_successor_fn(&dm, successors8, NULL);
    build_dijkstra_map(&dm, MAP_WIDTH, MAP_HEIGHT, sources, n_sources);

    for (int i = 0; i < w * h; i++) {
        int rounded = myround(dm.map[i]);

        printf("\033[48;2;%d;%d;0m \e[0m", get_red(rounded), get_green(rounded));
        if (i % w == w - 1) printf("\n");
    }

    free(sources);
    destroy_dijkstra_map(&dm);
    return 0;
}
