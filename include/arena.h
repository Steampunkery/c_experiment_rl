#pragma once

#include <stddef.h>

#define new(a, t, n)  (t *)alloc(a, sizeof(t), n)

typedef struct {
    char *beg;
    char *end;
} arena;

void *alloc(arena *a, ptrdiff_t size, ptrdiff_t count);
void *peeka(arena *a);
arena new_arena(ptrdiff_t cap);
