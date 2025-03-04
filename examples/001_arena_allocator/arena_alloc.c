#define UTIL_IMPLEMENTATION
#include "util.h"

#include <stdio.h>

int main(void)
{
    Arena arena = {0};

    int *a = arena_alloc(&arena, sizeof(int));
    int *b = arena_alloc(&arena, sizeof(int));
    int *c = arena_alloc(NULL, sizeof(int)); // behaves like malloc

    *a = 34;
    *b = 36;
    *c = -1;

    printf("%d\n", *a + *b + *c);

    arena_free(&arena);
    free(c);
    return 0;
}
