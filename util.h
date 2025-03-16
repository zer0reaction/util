#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef UTIL_DEBUG
    #include <stdio.h>
#endif /* UTIL_DEBUG */

#ifdef UTIL_DEBUG
    #define UTIL_DEBUG_INFO(info) do { \
        printf("[UTIL_DEBUG] ");       \
        printf info ;                  \
        printf("\n");                  \
    } while (0)
#else
    #define UTIL_DEBUG_INFO(info)
#endif

#ifndef ARENA_REGION_DEFAULT_CAPACITY
    #define ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif /* ARENA_REGION_DEFAULT_CAPACITY */

#ifndef ARENA_REGION_REALLOC_FACTOR
    #define ARENA_REGION_REALLOC_FACTOR (2)
#endif /* ARENA_REGION_REALLOC_FACTOR */

#define UTIL_MAX(a, b) ((a > b) ? a : b)

#define list_create(arena, T, size) internal_list_create(arena, size, sizeof(T))

typedef struct Arena_Region Arena_Region;
struct Arena_Region {
    Arena_Region *next;
    size_t capacity;
    size_t used;
    void *data;
    bool reallocatable;
};

typedef struct Arena Arena;
struct Arena {
    Arena_Region *start, *end;
    Arena_Region *start_reallocatable, *end_reallocatable;
};

typedef struct List_Header List_Header;
struct List_Header {
    size_t size;
    size_t stride;
};

extern void *arena_alloc(Arena *a, size_t bytes);
extern void *arena_realloc(Arena *a, void *ptr, size_t old_size, size_t new_size);
extern void arena_free(Arena *a);

extern size_t list_get_size(void *list);
extern size_t list_get_stride(void *list);
/* TODO: add list_resize */

#endif /* UTIL_H_ */

#ifdef UTIL_IMPLEMENTATION

Arena_Region *internal_arena_region_create(size_t bytes) {
    Arena_Region *r;
    size_t region_capacity;

    r = malloc(sizeof(Arena_Region));
    region_capacity = UTIL_MAX(ARENA_REGION_DEFAULT_CAPACITY, bytes);

    r->next = NULL;
    r->capacity = region_capacity;
    r->used = bytes;
    r->data = malloc(region_capacity);
    r->reallocatable = false;

    UTIL_DEBUG_INFO(("Created regular region, %ld/%ld", r->used, r->capacity));

    return r;
}

Arena_Region *internal_arena_region_create_reallocatable(size_t bytes) {
    Arena_Region *r;
    size_t region_capacity;

    r = malloc(sizeof(Arena_Region));
    region_capacity = bytes * ARENA_REGION_REALLOC_FACTOR;

    r->next = NULL;
    r->capacity = region_capacity;
    r->used = bytes;
    r->data = malloc(region_capacity);
    r->reallocatable = true;

    UTIL_DEBUG_INFO(("Created reallocatable region, %ld/%ld", r->used, r->capacity));

    return r;
}

Arena_Region *internal_get_ptr_region(Arena *a, void *ptr) {
    Arena_Region *r;

    r = a->start_reallocatable;

    while (r != NULL) {
        if (ptr == r->data) return r;
        UTIL_DEBUG_INFO(("Going through reallocatable regions..."));
        r = r->next;
    }
    UTIL_DEBUG_INFO(("Searching for ptr in reallocatable regions failed, trying in regular"));

    r = a->start;
    while (r != NULL) {
        if (ptr >= r->data && (char *)ptr < (char *)(r->data) + r->capacity) return r;
        UTIL_DEBUG_INFO(("Going through regular regions..."));
        r = r->next;
    }
    UTIL_DEBUG_INFO(("Searching for ptr in regular regions failed"));

    return NULL;
}

void internal_arena_region_push_back(Arena *a, Arena_Region *r) {
    if (a->start == NULL) {
        UTIL_DEBUG_INFO(("Pushing back regular region to an empty arena"));
        a->start = a->end = r;
    } else {
        UTIL_DEBUG_INFO(("Pushing back regular region"));
        a->end->next = r;
        a->end = a->end->next;
    }
}

void internal_arena_region_push_back_reallocatable(Arena *a, Arena_Region *r) {
    if (a->start_reallocatable == NULL) {
        UTIL_DEBUG_INFO(("Pushing back reallocatable region to an empty arena"));
        a->start_reallocatable = a->end_reallocatable = r;
    } else {
        UTIL_DEBUG_INFO(("Pushing back reallocatable region"));
        a->end_reallocatable->next = r;
        a->end_reallocatable = a->end_reallocatable->next;
    }
}

void *internal_list_create(Arena *a, size_t size, size_t stride) {
    size_t allocd_size;
    void *list;
    List_Header *header;

    UTIL_DEBUG_INFO(("Creating list of size %ld and stride %ld", size, stride));

    allocd_size = sizeof(List_Header) + (size * stride);
    list = arena_alloc(a, allocd_size);
    header = (List_Header *)list;
    header->size = size;
    header->stride = stride;

    return (char *)list + sizeof(List_Header);
}

void *arena_alloc(Arena *a, size_t bytes) {
    void *ptr;

    if (a == NULL) {
        UTIL_DEBUG_INFO(("Arena pointer is NULL, allocating %ld bytes with malloc", bytes));
        return malloc(bytes);
    }

    if (a->start == NULL || a->end->used + bytes > a->end->capacity) {
        Arena_Region *r;

        UTIL_DEBUG_INFO(("Creating a new region and allocating %ld bytes on it", bytes));

        r = internal_arena_region_create(bytes);
        internal_arena_region_push_back(a, r);
        return r->data;
    }

    UTIL_DEBUG_INFO(("Allocating %ld bytes on an existing region", bytes));

    ptr = (char *)a->end->data + a->end->used;
    a->end->used += bytes;
    return ptr;
}

void *arena_realloc(Arena *a, void *ptr, size_t old_size, size_t new_size) {
    size_t i;
    Arena_Region *ptr_region;

    if (a == NULL) {
        UTIL_DEBUG_INFO(("Arena pointer is NULL, reallocating %ld bytes with realloc", new_size));
        return realloc(ptr, new_size);
    }

    UTIL_DEBUG_INFO(("Searching for pointer in regions (arena_realloc)"));
    ptr_region = internal_get_ptr_region(a, ptr);

    if (ptr == NULL || ptr_region == NULL) {
        UTIL_DEBUG_INFO(("Passed a NULL or invalid ptr to arena_realloc, allocating a new region of size %ld", new_size));
        return arena_alloc(a, new_size);
    }

    if (!ptr_region->reallocatable || new_size > ptr_region->capacity) {
        Arena_Region *new_region;

        UTIL_DEBUG_INFO(("Region is not reallocatable or there is not enough free space, creating a new one of size %ld", new_size));

        new_region = internal_arena_region_create_reallocatable(new_size);

        for (i = 0; i < old_size; ++i) {
            ((char *)new_region->data)[i] = ((char *)ptr)[i];
        }
        internal_arena_region_push_back_reallocatable(a, new_region);

        return new_region->data;
    }

    UTIL_DEBUG_INFO(("Reallocating %ld -> %ld", old_size, new_size));
    ptr_region->used = new_size;
    return ptr;
}

void arena_free(Arena *a) {
    Arena_Region *r;

    r = a->start;
    while (r != NULL) {
        Arena_Region *next_region;

        UTIL_DEBUG_INFO(("Freeing regular region"));

        next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    r = a->start_reallocatable;
    while (r != NULL) {
        Arena_Region *next_region;

        UTIL_DEBUG_INFO(("Freeing reallocatable region"));
        
        next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    a->start = a->end = NULL;
    a->start_reallocatable = a->end_reallocatable = NULL;
}

size_t list_get_size(void *list) {
    List_Header *header = (List_Header *)list - 1;
    return header->size;
}

size_t list_get_stride(void *list) {
    List_Header *header = (List_Header *)list - 1;
    return header->stride;
}

#undef UTIL_IMPLEMENTATION
#endif /* UTIL_IMPLEMENTATION */
