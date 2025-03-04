#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (4*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

typedef unsigned char char8_t;
typedef struct Arena_Region Arena_Region;

struct Arena_Region {
    Arena_Region *next;
    size_t capacity;
    size_t used;
    void *data;
};

typedef struct {
    Arena_Region *start, *end;
} Arena;

typedef struct {
    size_t capacity;
    size_t length;
    char8_t *data;
} String;

void *arena_alloc(Arena *a, size_t bytes);
void arena_free(Arena *a);

String string_create(Arena *a, size_t capacity);
String string_from_literal(Arena *a, const char *literal);
String string_cat_create(Arena *a, String s1, String s2);
void string_cat(String *s1, String s2);

#ifdef UTIL_IMPLEMENTATION

Arena_Region *arena_region_create(size_t bytes) {
    Arena_Region *r = malloc(sizeof(Arena_Region));

    r->next = NULL;

    if (bytes > ARENA_REGION_DEFAULT_CAPACITY) {
        r->capacity = bytes;
        r->data = malloc(bytes);
    } else {
        r->capacity = ARENA_REGION_DEFAULT_CAPACITY;
        r->data = malloc(ARENA_REGION_DEFAULT_CAPACITY);
    }

    r->used = bytes;

    return r;
}

void *arena_alloc(Arena *a, size_t bytes)
{
    if (a == NULL) {
        return malloc(bytes);
    }

    if (a->start == NULL) {
        a->start = a->end = arena_region_create(bytes);
        return a->end->data;
    }

    Arena_Region *current_region = a->start;

    while (current_region != NULL) {
        if (current_region->used + bytes <= current_region->capacity) {
            void *ptr = (uint8_t *)current_region->data + current_region->used;
            current_region->used += bytes;
            return ptr;
        }

        current_region = current_region->next;
    }

    a->end->next = arena_region_create(bytes);
    a->end = a->end->next;
    return a->end->data;
}

void arena_free(Arena *a)
{
    Arena_Region *r = a->start;

    while (r != NULL) {
        Arena_Region *next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    a->start = a->end = NULL;
}

String string_create(Arena *a, size_t capacity)
{
    String s = {
        .capacity = capacity,
        .length = 0,
        .data = arena_alloc(a, capacity + 1)
    };

    for (size_t i = 0; i < capacity + 1; i++) {
        s.data[i] = 0;
    }

    return s;
}

String string_from_literal(Arena *a, const char *literal)
{
    size_t literal_length = 0;
    for (; literal[literal_length] != '\0'; literal_length++);

    String s = {
        .capacity = literal_length,
        .length = literal_length,
        .data = arena_alloc(a, literal_length + 1)
    };

    for (size_t i = 0; i < literal_length + 1; i++) {
        s.data[i] = literal[i];
    }

    return s;
}

String string_cat_create(Arena *a, String s1, String s2)
{
    size_t total_length = s1.length + s2.length;

    String s = {
        .capacity = total_length,
        .length = total_length,
        .data = arena_alloc(a, total_length + 1)
    };

    for (size_t i = 0; i < s1.length; i++) {
        s.data[i] = s1.data[i];
    }

    for (size_t i = 0; i < s2.length; i++) {
        s.data[i + s1.length] = s2.data[i];
    }

    s.data[total_length] = '\0';

    return s;
}

void string_cat(String *s1, String s2)
{
    assert(s1->capacity >= s1->length + s2.length &&
           "error in string_cat: not enough capacity.");

    for (size_t i = 0; i < s2.length; i++) {
        s1->data[s1->length + i] = s2.data[i];
    }

    s1->length += s2.length;
    s1->data[s1->length] = '\0';
}

#endif // UTIL_IMPLEMENTATION

#endif // UTIL_H_
