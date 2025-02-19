#include "arena.h"

#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

void *alloc(arena *a, ptrdiff_t size, ptrdiff_t count)
{
    ptrdiff_t alignment = -(uintptr_t)a->beg & (sizeof(void *) - 1);
    ptrdiff_t available = a->end - a->beg - alignment;
    if (count > available/size) {
        assert(0);  // out of memory
    }
    char *r = a->beg + alignment;
    a->beg += alignment + size*count;
    return memset(r, 0, size*count);
}

void *peeka(arena *a)
{
    return (-(uintptr_t)a->beg & (sizeof(void *) - 1)) + a->beg;
}

arena new_arena(ptrdiff_t cap)
{
    arena a = {0};
    a.beg = malloc(cap);
    a.end = a.beg ? a.beg+cap : 0;
    return a;
}

