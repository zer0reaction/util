#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (4*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

#define list_create(arena, T, size) internal_list_create(arena, size, sizeof(T))

typedef unsigned char char8_t;

typedef struct Arena_Region Arena_Region;
struct Arena_Region {
    Arena_Region *next;
    size_t capacity;
    size_t used;
    void *data;
};

typedef struct Arena Arena;
struct Arena {
    Arena_Region *start, *end;
};

typedef struct List_Header List_Header;
struct List_Header {
    size_t size;
    size_t stride;
};

void *arena_alloc(Arena *a, size_t bytes);
void arena_free(Arena *a);
void *internal_list_create(Arena *a, size_t size, size_t stride);
size_t list_get_size(void *list);

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

void *internal_list_create(Arena *a, size_t size, size_t stride) {
    size_t allocd_size = sizeof(List_Header) + (size * stride);

    void *list = arena_alloc(a, allocd_size);
    ((List_Header *)list)->size = size;
    ((List_Header *)list)->stride = stride;

    return (char *)list + sizeof(List_Header);
}

size_t list_get_size(void *list) {
    List_Header *header = (List_Header *)list - 1;
    return header->size;
}

#endif // UTIL_IMPLEMENTATION

#endif // UTIL_H_
