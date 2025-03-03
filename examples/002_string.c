#define UTIL_IMPLEMENTATION
#include "util.h"

#include <stdio.h>

int main(void)
{
    Region r = region_create(1024 * sizeof(char8_t));

    String s1 = string_from_literal(&r, "Hello, ");
    String s2 = string_from_literal(&r, "Utility Library!");
    String s3 = string_cat_create(&r, s1, s2);

    printf("%s\n", s3.data);

    String s = string_create(&r, 128);
    string_cat(&s, string_from_literal(&r, "1, "));
    string_cat(&s, string_from_literal(&r, "2, 3"));

    printf("%s\n", s.data);

    region_free(&r);
    return 0;
}
