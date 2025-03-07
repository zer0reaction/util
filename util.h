#ifndef UTIL_H_
#define UTIL_H_

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (4*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

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

void *arena_alloc(Arena *a, size_t bytes);
void arena_free(Arena *a);

char8_t *read_whole_file(Arena *a, const char *path);

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

char8_t *read_whole_file(Arena *a, const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char8_t *file_data = arena_alloc(a, file_size + 1);
    fread(file_data, file_size, 1, file);
    file_data[file_size] = '\0';

    return file_data;
}

#endif // UTIL_IMPLEMENTATION

#endif // UTIL_H_
