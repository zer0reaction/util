#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    size_t capacity;
    size_t used;
    void *data;
} Region;

Region region_alloc_alloc(size_t capacity);
void *region_alloc(Region *r, size_t bytes);
void region_free(Region *r);

#ifdef UTIL_IMPLEMENTATION

Region region_alloc_alloc(size_t capacity)
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

    // TODO: maybe do this in a friendlier way?
    assert(r->used + bytes <= r->capacity);

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

#endif // UTIL_IMPLEMENTATION

#endif // UTIL_H_
