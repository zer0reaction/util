#ifndef UTIL_ARENA_H_
#define UTIL_ARENA_H_

#include <stddef.h>
#include <stdlib.h>

#include "typedef.h"

#ifdef UTIL_ARENA_DEBUG
    #include <stdio.h>
#endif /* UTIL_ARENA_DEBUG */

#ifdef UTIL_ARENA_DEBUG
    #define UTIL_ARENA_DEBUG_INFO(info) do { \
        printf("[UTIL ARENA] ");             \
        printf info;                         \
        printf("\n");                        \
    } while (0)
#else
    #define UTIL_ARENA_DEBUG_INFO(info)
#endif

#ifndef UTIL_ARENA_REGION_DEFAULT_CAPACITY
    #define UTIL_ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif /* UTIL_ARENA_REGION_DEFAULT_CAPACITY */

#ifndef UTIL_ARENA_REGION_REALLOC_FACTOR
    #define UTIL_ARENA_REGION_REALLOC_FACTOR (2)
#endif /* UTIL_ARENA_REGION_REALLOC_FACTOR */

#ifndef UTIL_ARENA_REGION_ALLOC_FACTOR
    #define UTIL_ARENA_REGION_ALLOC_FACTOR (1.25)
#endif /* UTIL_ARENA_REGION_ALLOC_FACTOR */

#define UTIL_ARENA_MAX(a, b) ((a > b) ? a : b)

typedef struct Arena_Region Arena_Region;
struct Arena_Region {
    Arena_Region *next;
    size_t capacity;
    size_t used;
    void *data;
    Util_Bool reallocatable;
};

typedef struct Arena Arena;
struct Arena {
    Arena_Region *start;
    Arena_Region *start_reallocatable;
};

extern void *arena_alloc(Arena *a, size_t bytes);
extern void *arena_realloc(Arena *a, void *ptr, size_t old_size, size_t new_size);
extern void arena_free(Arena *a);

Arena_Region *internal_arena_region_create(size_t bytes);
Arena_Region *internal_arena_region_create_reallocatable(size_t bytes);
Arena_Region *internal_get_ptr_region(Arena *a, void *ptr);
void internal_arena_region_push(Arena *a, Arena_Region *r);
void internal_arena_region_push_reallocatable(Arena *a, Arena_Region *r);

#endif /* UTIL_ARENA_H_ */

#ifdef UTIL_ARENA_IMPLEMENTATION

Arena_Region *internal_arena_region_create(size_t bytes) {
    Arena_Region *r;
    size_t region_capacity;

    r = malloc(sizeof(Arena_Region));
    region_capacity = UTIL_ARENA_MAX(UTIL_ARENA_REGION_DEFAULT_CAPACITY, bytes * UTIL_ARENA_REGION_ALLOC_FACTOR);

    r->next = NULL;
    r->capacity = region_capacity;
    r->used = bytes;
    r->data = malloc(region_capacity);
    r->reallocatable = UTIL_FALSE;

    UTIL_ARENA_DEBUG_INFO(("Created regular region, %ld/%ld", r->used, r->capacity));

    return r;
}

Arena_Region *internal_arena_region_create_reallocatable(size_t bytes) {
    Arena_Region *r;
    size_t region_capacity;

    r = malloc(sizeof(Arena_Region));
    region_capacity = UTIL_ARENA_MAX(UTIL_ARENA_REGION_DEFAULT_CAPACITY, bytes * UTIL_ARENA_REGION_REALLOC_FACTOR);

    r->next = NULL;
    r->capacity = region_capacity;
    r->used = bytes;
    r->data = malloc(region_capacity);
    r->reallocatable = UTIL_TRUE;

    UTIL_ARENA_DEBUG_INFO(("Created reallocatable region, %ld/%ld", r->used, r->capacity));

    return r;
}

Arena_Region *internal_get_ptr_region(Arena *a, void *ptr) {
    Arena_Region *r;

    r = a->start_reallocatable;
    while (r != NULL) {
        UTIL_ARENA_DEBUG_INFO(("Going through reallocatable regions..."));
        if (ptr == r->data) return r;
        r = r->next;
    }
    UTIL_ARENA_DEBUG_INFO(("Searching for ptr in reallocatable regions failed, trying in regular"));

    r = a->start;
    while (r != NULL) {
        UTIL_ARENA_DEBUG_INFO(("Going through regular regions..."));
        if (ptr >= r->data && (char *)ptr < (char *)(r->data) + r->capacity) return r;
        r = r->next;
    }
    UTIL_ARENA_DEBUG_INFO(("Searching for ptr in regular regions failed"));

    return NULL;
}

void internal_arena_region_push(Arena *a, Arena_Region *r) {
    if (a->start == NULL) {
        UTIL_ARENA_DEBUG_INFO(("Pushing regular region to an empty list"));
        a->start = r;
    } else {
        UTIL_ARENA_DEBUG_INFO(("Pushing regular region"));
        r->next = a->start;
        a->start = r;
    }
}

void internal_arena_region_push_reallocatable(Arena *a, Arena_Region *r) {
    if (a->start_reallocatable == NULL) {
        UTIL_ARENA_DEBUG_INFO(("Pushing reallocatable region to an empty list"));
        a->start_reallocatable = r;
    } else {
        UTIL_ARENA_DEBUG_INFO(("Pushing reallocatable region"));
        r->next = a->start_reallocatable;
        a->start_reallocatable = r;
    }
}

void *arena_alloc(Arena *a, size_t bytes) {
    void *ptr;

    if (a == NULL) {
        UTIL_ARENA_DEBUG_INFO(("Arena pointer is NULL, allocating %ld bytes with malloc", bytes));
        return malloc(bytes);
    }

    if (a->start == NULL || a->start->used + bytes > a->start->capacity) {
        Arena_Region *r;

        r = internal_arena_region_create(bytes);
        internal_arena_region_push(a, r);
        return r->data;
    }

    UTIL_ARENA_DEBUG_INFO(("Allocating %ld bytes on an existing region", bytes));

    ptr = (char *)a->start->data + a->start->used;
    a->start->used += bytes;
    return ptr;
}

void *arena_realloc(Arena *a, void *ptr, size_t old_size, size_t new_size) {
    size_t i;
    Arena_Region *ptr_region;

    if (a == NULL) {
        UTIL_ARENA_DEBUG_INFO(("Arena pointer is NULL, reallocating %ld bytes with realloc", new_size));
        return realloc(ptr, new_size);
    }

    ptr_region = internal_get_ptr_region(a, ptr);

    if (ptr == NULL || ptr_region == NULL) {
        return arena_alloc(a, new_size);
    }

    if (!ptr_region->reallocatable || new_size > ptr_region->capacity) {
        Arena_Region *new_region;

        new_region = internal_arena_region_create_reallocatable(new_size);

        for (i = 0; i < old_size; ++i) {
            ((char *)new_region->data)[i] = ((char *)ptr)[i];
        }
        internal_arena_region_push_reallocatable(a, new_region);

        return new_region->data;
    }

    UTIL_ARENA_DEBUG_INFO(("Reallocating %ld -> %ld", old_size, new_size));
    ptr_region->used = new_size;
    return ptr;
}

void arena_free(Arena *a) {
    Arena_Region *r;

    r = a->start;
    while (r != NULL) {
        Arena_Region *next_region;

        UTIL_ARENA_DEBUG_INFO(("Freeing regular region (%ld/%ld)", r->used, r->capacity));

        next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    r = a->start_reallocatable;
    while (r != NULL) {
        Arena_Region *next_region;

        UTIL_ARENA_DEBUG_INFO(("Freeing reallocatable region (%ld/%ld)", r->used, r->capacity));
        
        next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    a->start = NULL;
    a->start_reallocatable = NULL;
}

#undef UTIL_ARENA_IMPLEMENTATION
#endif /* UTIL_ARENA_IMPLEMENTATION */
