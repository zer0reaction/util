#define UTIL_IMPLEMENTATION
#include "util.h"

#include <stdio.h>

int main(void)
{
    Region r = region_create(1024 * sizeof(int));

    int *a = region_alloc(&r, sizeof(int));
    int *b = region_alloc(&r, sizeof(int));
    int *c = region_alloc(NULL, sizeof(int)); // behaves like malloc

    *a = 34;
    *b = 36;
    *c = -1;

    printf("%d\n", *a + *b + *c);

    region_free(&r);
    free(c);
    return 0;
}
