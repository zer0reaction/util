#define UTIL_IMPLEMENTATION
#include "util.h"

#include <stdio.h>

int main(void)
{
    Region r = region_alloc_alloc(1024 * sizeof(char8_t));

    String s1 = string_from_literal(&r, "Hello, ");
    String s2 = string_from_literal(&r, "Utility Library!");
    String s3 = string_cat_create(&r, s1, s2);

    printf("%s\n", s3.data);

    region_free(&r);
    return 0;
}
