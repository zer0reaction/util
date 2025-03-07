/*
Copyright (c) 2025 Nikita Rudakov (zer0reaction)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (4*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

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
String string_from_buffer(Arena *a, char8_t *buffer);
String string_cat_create(Arena *a, String s1, String s2);
void *string_cat(String *s1, String s2);
String string_read_file(Arena *a, const char *path);
void string_free(String *s1);

#endif // UTIL_H_

#ifdef UTIL_IMPLEMENTATION

Arena_Region *arena_region_create(size_t bytes)
{
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

    if (a->end->used + bytes <= a->end->capacity) {
        void *ptr = (uint8_t *)a->end->data + a->end->used;
        a->end->used += bytes;
        return ptr;
    } else {
        a->end->next = arena_region_create(bytes);
        a->end = a->end->next;
        return a->end->data;
    }
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

String string_from_buffer(Arena *a, char8_t *buffer)
{
    size_t buffer_length = 0;
    for (; buffer[buffer_length] != '\0'; buffer_length++);

    String s = {
        .capacity = buffer_length,
        .length = buffer_length,
        .data = arena_alloc(a, buffer_length + 1)
    };

    for (size_t i = 0; i < buffer_length + 1; i++) {
        s.data[i] = buffer[i];
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

void *string_cat(String *s1, String s2)
{
    if (s1->capacity < s1->length + s2.length) {
        // TODO: this is not good
        return NULL;
    }

    for (size_t i = 0; i < s2.length; i++) {
        s1->data[s1->length + i] = s2.data[i];
    }

    s1->length += s2.length;
    s1->data[s1->length] = '\0';

    return s1;
}

String string_read_file(Arena *a, const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        // TODO: this can be probably done better
        String s = {0};
        return s;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    String s = string_create(a, file_size);
    s.length = file_size;
    fread(s.data, file_size, 1, file);

    fclose(file);
    return s;
}

void string_free(String *s1)
{
    if (s1->data != NULL) {
        free(s1->data);

        s1->data = NULL;
        s1->capacity = 0;
        s1->length = 0;
    }
}

#endif // UTIL_IMPLEMENTATION
