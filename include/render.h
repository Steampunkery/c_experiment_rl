#pragma once
typedef struct ecs_iter_t ecs_iter_t;
typedef struct WINDOW WINDOW;

void Render(ecs_iter_t *it);

typedef struct WindowHolder {
    WINDOW *base;
    WINDOW *log;
} WindowHolder;

