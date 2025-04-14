#ifndef DA_H_
#define DA_H_

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arena.h"

#define DA_HEADER(da) ((Da_Header *)da - 1)
#define DA_BODY(header) ((char *)header + sizeof(Da_Header))

#define da_create(arena, T, size) internal_da_create(arena, size, sizeof(T))

#define da_push_back(da, value) do {       \
    size_t new_size;                       \
                                           \
    new_size = da_size(da) + 1;            \
    da = internal_da_resize(da, new_size); \
    da[new_size - 1] = value;              \
} while (0)

#define da_push(da, _pos, value) do {                 \
    size_t i, new_size, pos;                          \
                                                      \
    pos = _pos;                                       \
                                                      \
    new_size = da_size(da) + 1;                       \
    assert(pos < new_size && "da_push: invalid pos"); \
                                                      \
    da = internal_da_resize(da, new_size);            \
    for (i = new_size - 1; i > pos; --i) {            \
        da[i] = da[i - 1];                            \
    }                                                 \
                                                      \
    da[pos] = value;                                  \
} while (0)

#define da_append(a, b) {         \
    a = internal_da_append(a, b); \
} while (0)

#define da_insert(a, b, n) {         \
    a = internal_da_insert(a, b, n); \
} while (0)

/* TODO: rewrite as a function with memmove */
/* also add void * where the return value will be stored */
#define da_pop(da, _pos) do {                    \
    size_t i, size, pos;                         \
                                                 \
    pos = _pos;                                  \
                                                 \
    size = da_size(da);                          \
    assert(pos < size && "da_pop: invalid pos"); \
                                                 \
    for (i = pos; i < size - 1; ++i) {           \
        da[i] = da[i + 1];                       \
    }                                            \
                                                 \
    DA_HEADER(da)->size--;                       \
} while (0)

typedef struct Da_Header Da_Header;
struct Da_Header {
    Arena *arena;
    size_t size;
    size_t stride;
};

extern size_t da_size(void *da);
extern size_t da_stride(void *da);
extern void da_pop_back(void *da);
extern void *da_clone(Arena *a, void *orig);

void *internal_da_append(void *dest, void *src);
void *internal_da_insert(void *dest, void *src, size_t pos);
void *internal_da_create(Arena *a, size_t size, size_t stride);
void *internal_da_resize(void *da, size_t new_size);

#endif /* DA_H_ */

#ifdef DA_IMPLEMENTATION

void *internal_da_create(Arena *a, size_t size, size_t stride) {
    size_t allocd_size;
    Da_Header *header;

    allocd_size = sizeof(Da_Header) + (size * stride);
    header = arena_alloc(a, allocd_size);

    header->arena = a;
    header->size = size;
    header->stride = stride;

    memset(DA_BODY(header), 0, size * stride);

    return DA_BODY(header);
}

void *internal_da_resize(void *da, size_t new_size) {
    Arena *a;
    Da_Header *header;
    size_t old_alloc_size, new_alloc_size;

    header = DA_HEADER(da);
    a = header->arena;
    old_alloc_size = sizeof(Da_Header) + header->size * header->stride;
    new_alloc_size = sizeof(Da_Header) + new_size * header->stride;

    if (new_alloc_size > old_alloc_size) {
        header = arena_realloc(a, header, old_alloc_size, new_alloc_size);
    }

    header->size = new_size;
    return DA_BODY(header);
}

size_t da_size(void *da) {
    Da_Header *header;

    header = DA_HEADER(da);
    return header->size;
}

size_t da_stride(void *da) {
    Da_Header *header;

    header = DA_HEADER(da);
    return header->stride;
}

/* TODO: add void * where the return value will be stored */
void da_pop_back(void *da) {
    assert(da_size(da) > 0 && "da_pop_back: invalid size");
    DA_HEADER(da)->size--;
}

void *da_clone(Arena *a, void *orig) {
    Da_Header *head;
    void *clone;

    head = DA_HEADER(orig);
    clone = internal_da_create(a, head->size, head->stride);
    memcpy(clone, orig, head->size * head->stride);

    return clone;
}

void *internal_da_append(void *dest, void *src) {
    size_t old_sizeb;
    Da_Header *h_dest, *h_src;

    h_dest = DA_HEADER(dest);
    h_src = DA_HEADER(src);
    old_sizeb = h_dest->size * h_dest->stride;

    dest = internal_da_resize(dest, h_dest->size + h_src->size);
    memcpy((char *)dest + old_sizeb, src, h_src->size * h_src->stride);

    return dest;
}

void *internal_da_insert(void *dest, void *src, size_t pos) {
    void *mv_from, *mv_to;
    size_t src_sizeb;
    Da_Header *h_dest, *h_src;

    h_dest = DA_HEADER(dest);
    h_src = DA_HEADER(src);
    src_sizeb = h_src->size * h_src->stride;

    assert(pos <= h_dest->size && "da_insert: invalid pos");

    dest = internal_da_resize(dest, h_dest->size + h_src->size);

    mv_from = (char *)dest + pos * h_dest->stride;
    mv_to = (char *)mv_from + src_sizeb;

    memmove(mv_to, mv_from, src_sizeb);
    memcpy(mv_from, src, src_sizeb);

    return dest;
}

#undef DA_IMPLEMENTATION
#endif /* DA_IMPLEMENTATION */
