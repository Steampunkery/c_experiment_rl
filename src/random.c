#include "random.h"

#include <stdlib.h>

// Return an integer in [lower, upper], inclusive on both ends
int randint(int lower, int upper)
{
    return random() % (upper - lower + 1) + lower;
}

// Returns a dice roll equal to n, "sides"-sided dice
int roll(int n, int sides)
{
    int total = 0;
    for (; n > 0; n--)
        total += randint(1, sides);

    return total;
}
