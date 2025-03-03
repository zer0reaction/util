#define UTIL_IMPLEMENTATION
#include "util.h"

#include <stdio.h>

int main(void)
{
    Region r = region_alloc_alloc(1024 * sizeof(int));

    int *a = region_alloc(&r, 512 * sizeof(int));
    int *b = region_alloc(&r, 512 * sizeof(int));

    region_free(&r);
    return 0;
}
