#ifndef UTIL_DA_H_
#define UTIL_DA_H_

#include <stddef.h>

#include "typedef.h"
#include "arena.h"

#ifdef UTIL_DA_DEBUG
    #include <stdio.h>
#endif /* UTIL_DA_DEBUG */

#ifdef UTIL_DA_DEBUG
    #define UTIL_DA_DEBUG_INFO(info) do { \
        printf("[UTIL DA] ");             \
        printf info;                        \
        printf("\n");                       \
    } while (0)
#else
    #define UTIL_DA_DEBUG_INFO(info)
#endif

#define UTIL_DA_HEADER(da) ((Da_Header *)da - 1)
#define UTIL_DA_BODY(header) ((char *)header + sizeof(Da_Header))

typedef struct Da_Header Da_Header;
struct Da_Header {
    size_t size;
    size_t stride;
};

#define da_create(arena, T, size) internal_da_create(arena, size, sizeof(T))
extern size_t da_get_size(void *da);
extern size_t da_get_stride(void *da);
#define da_push_back(arena, da, value) do {         \
    size_t new_size;                                    \
                                                        \
    new_size = da_get_size(da) + 1;                 \
    da = internal_da_resize(arena, da, new_size); \
    da[new_size - 1] = value;                         \
} while (0)
#define da_pop_back(da) (da[--(UTIL_DA_HEADER(da)->size)])

void *internal_da_create(Arena *a, size_t size, size_t stride);
void *internal_da_resize(Arena *a, void *da, size_t new_size);

#endif /* UTIL_DA_H_ */

#ifdef UTIL_DA_IMPLEMENTATION

void *internal_da_create(Arena *a, size_t size, size_t stride) {
    size_t allocd_size;
    Da_Header *header;

    UTIL_DA_DEBUG_INFO(("Creating da of size %ld and stride %ld", size, stride));

    allocd_size = sizeof(Da_Header) + (size * stride);
    header = arena_alloc(a, allocd_size);
    header->size = size;
    header->stride = stride;

    return UTIL_DA_BODY(header);
}

void *internal_da_resize(Arena *a, void *da, size_t new_size) {
    Da_Header *header;
    size_t old_alloc_size, new_alloc_size;

    header = UTIL_DA_HEADER(da);
    old_alloc_size = sizeof(Da_Header) + header->size * header->stride;
    new_alloc_size = sizeof(Da_Header) + new_size * header->stride;

    UTIL_DA_DEBUG_INFO(("Resizing da %ld -> %ld (%ld to %ld bytes)", header->size, new_size, old_alloc_size, new_alloc_size));

    if (new_alloc_size > old_alloc_size) {
        header = arena_realloc(a, header, old_alloc_size, new_alloc_size);
    }

    header->size = new_size;
    return UTIL_DA_BODY(header);
}

size_t da_get_size(void *da) {
    Da_Header *header;

    header = UTIL_DA_HEADER(da);
    return header->size;
}

size_t da_get_stride(void *da) {
    Da_Header *header;

    header = UTIL_DA_HEADER(da);
    return header->stride;
}

#undef UTIL_DA_IMPLEMENTATION
#endif /* UTIL_DA_IMPLEMENTATION */
