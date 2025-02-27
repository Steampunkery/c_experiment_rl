#include "random.h"

#include <stdlib.h>

int randint(int lower, int upper)
{
    return random() % (upper - lower + 1) + lower;
}

int roll(int n, int sides)
{
    int total = 0;
    for (; n > 0; n--)
        total += randint(1, sides);

    return total;
}
