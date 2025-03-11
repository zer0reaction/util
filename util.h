#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (4*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

#define UTIL_MAX(a, b) ((a > b) ? a : b)

#define array_create(arena, T, size) internal_array_create(arena, size, sizeof(T))

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

typedef struct Array_Header Array_Header;
struct Array_Header {
    size_t size;
    size_t stride;
};

extern void *arena_alloc(Arena *a, size_t bytes);
extern void arena_free(Arena *a);

extern size_t array_get_size(void *array);
extern size_t array_get_stride(void *array);

#endif // UTIL_H_

#ifdef UTIL_IMPLEMENTATION

Arena_Region *internal_arena_region_create(size_t bytes) {
    Arena_Region *r = malloc(sizeof(Arena_Region));
    size_t region_capacity = UTIL_MAX(ARENA_REGION_DEFAULT_CAPACITY, bytes);

    r->next = NULL;
    r->capacity = region_capacity;
    r->used = bytes;
    r->data = malloc(region_capacity);

    return r;
}

void *arena_alloc(Arena *a, size_t bytes) {
    if (a == NULL) return malloc(bytes);

    if (a->start == NULL) {
        a->start = a->end = internal_arena_region_create(bytes);
        return a->end->data;
    }

    if (a->end->used + bytes <= a->end->capacity) {
        void *ptr = (char *)a->end->data + a->end->used;
        a->end->used += bytes;
        return ptr;
    } else {
        a->end->next = internal_arena_region_create(bytes);
        a->end = a->end->next;
        return a->end->data;
    }
}

void arena_free(Arena *a) {
    Arena_Region *r = a->start;

    while (r != NULL) {
        Arena_Region *next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    a->start = a->end = NULL;
}

void *internal_array_create(Arena *a, size_t size, size_t stride) {
    size_t allocd_size = sizeof(Array_Header) + (size * stride);

    void *array = arena_alloc(a, allocd_size);
    Array_Header *header = (Array_Header *)array;
    header->size = size;
    header->stride = stride;

    return (char *)array + sizeof(Array_Header);
}

size_t array_get_size(void *array) {
    Array_Header *header = (Array_Header *)array - 1;
    return header->size;
}

size_t array_get_stride(void *array) {
    Array_Header *header = (Array_Header *)array - 1;
    return header->stride;
}

#undef UTIL_IMPLEMENTATION
#endif // UTIL_IMPLEMENTATION
