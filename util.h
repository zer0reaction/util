#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

typedef unsigned char char8_t;

typedef struct {
    size_t capacity;
    size_t used;
    void *data;
} Region;

typedef struct {
    size_t capacity;
    size_t length;
    char8_t *data;
} String;

Region region_create(size_t capacity);
void *region_alloc(Region *r, size_t bytes);
void region_free(Region *r);

String string_create(Region *r, size_t capacity);
String string_from_literal(Region *r, const char8_t *literal);
String string_cat_create(Region *r, String s1, String s2);
void string_cat(String *s1, String s2);

#ifdef UTIL_IMPLEMENTATION

Region region_create(size_t capacity)
{
    Region r = {
        .capacity = capacity,
        .used = 0,
        .data = malloc(capacity)
    };

    return r;
}

void *region_alloc(Region *r, size_t bytes)
{
    if (r == NULL) {
        return malloc(bytes);
    }

    // TODO: do this in a friendlier way?
    assert(r->used + bytes <= r->capacity); // TODO: add string message

    void *ptr = (uint8_t *)r->data + r->used;
    r->used += bytes;
    return ptr;
}

void region_free(Region *r)
{
    if (r->capacity) {
        free(r->data);
        r->capacity = 0;
        r->used = 0;
    }
}

String string_create(Region *r, size_t capacity)
{
    String s = {
        .capacity = capacity,
        .length = 0,
        .data = region_alloc(r, capacity + 1)
    };

    for (int i = 0; i < capacity + 1; i++) {
        s.data[i] = 0;
    }

    return s;
}

String string_from_literal(Region *r, const char8_t *literal)
{
    size_t literal_length = 0;
    for (; literal[literal_length] != '\0'; literal_length++);

    String s = {
        .capacity = literal_length,
        .length = literal_length,
        .data = region_alloc(r, literal_length + 1)
    };

    for (int i = 0; i < literal_length + 1; i++) {
        s.data[i] = literal[i];
    }

    return s;
}

String string_cat_create(Region *r, String s1, String s2)
{
    size_t total_length = s1.length + s2.length;

    String s = {
        .capacity = total_length,
        .length = total_length,
        .data = region_alloc(r, total_length + 1)
    };

    for (int i = 0; i < s1.length; i++) {
        s.data[i] = s1.data[i];
    }

    for (int i = 0; i < s2.length; i++) {
        s.data[i + s1.length] = s2.data[i];
    }

    s.data[total_length] = '\0';

    return s;
}

void string_cat(String *s1, String s2)
{
    assert(s1->capacity >= s1->length + s2.length); // TODO: add string message

    for (int i = 0; i < s2.length; i++) {
        s1->data[s1->length + i] = s2.data[i];
    }

    s1->length += s2.length;
    s1->data[s1->length] = '\0';
}

#endif // UTIL_IMPLEMENTATION

#endif // UTIL_H_
