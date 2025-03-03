#define UTIL_IMPLEMENTATION
#include "util.h"

#include <stdio.h>

int main(void)
{
    Arena arena = {0};

    String s1 = string_from_literal(&arena, "Hello, ");
    String s2 = string_from_literal(&arena, "Utility Library!");
    String s3 = string_cat_create(&arena, s1, s2);
    printf("%s\n", s3.data);

    if (string_cat(&s3, string_from_literal(&arena, " And World too!")) == NULL) {
        printf("Error concatenating s3 and s.\n");
    }
    printf("%s\n", s3.data);

    String s = string_create(&arena, 128);
    string_cat(&s, string_from_literal(&arena, "1, "));
    string_cat(&s, string_from_literal(&arena, "2, 3"));

    printf("%s\n", s.data);

    String on_heap = string_from_literal(NULL, "This string is allocated using malloc directly, use string_free in this case and only in this case");
    printf("%s\n", on_heap.data);
    string_free(&on_heap);
    string_free(&on_heap); // does nothing

    arena_free(&arena);
    return 0;
}
