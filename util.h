#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* ---------- Info ---------- */

#ifdef ENABLE_INFO
    #include <stdio.h>
    #define INFO(caller, info) do { \
        printf("[%s] ", caller);               \
        printf info;                           \
        printf("\n");                          \
    } while (0)
#else
    #define INFO(caller, info)
#endif /* ENABLE_INFO */

/* ---------- Arena ---------- */

#ifdef ARENA_DEBUG
    #include <stdio.h>
#endif /* ARENA_DEBUG */

#ifdef ARENA_DEBUG
    #define ARENA_DEBUG_INFO(info) do { \
        printf("[UTIL ARENA] ");        \
        printf info;                    \
        printf("\n");                   \
    } while (0)
#else
    #define ARENA_DEBUG_INFO(info)
#endif

#ifndef ARENA_REGION_DEFAULT_CAPACITY
    #define ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif /* ARENA_REGION_DEFAULT_CAPACITY */

#ifndef ARENA_REGION_REALLOC_CAPACITY
    #define ARENA_REGION_REALLOC_CAPACITY (1024)
#endif /* ARENA_REGION_REALLOC_CAPACITY */

#ifndef ARENA_REGION_REALLOC_FACTOR
    #define ARENA_REGION_REALLOC_FACTOR (2)
#endif /* ARENA_REGION_REALLOC_FACTOR */

#ifndef ARENA_REGION_ALLOC_FACTOR
    #define ARENA_REGION_ALLOC_FACTOR (1.25)
#endif /* ARENA_REGION_ALLOC_FACTOR */

#define ARENA_MAX(a, b) ((a > b) ? a : b)

/* ---------- Dynamic array ---------- */

#define DA_HEADER(da) ((Da_Header *)(da) - 1)
#define DA_BODY(header) (void *)((char *)(header) + sizeof(Da_Header))

#define da_create(arena, T, size) internal_da_create(arena, size, sizeof(T))

#define da_push_back(da, value) do {       \
    u64 new_size = 0;                      \
                                           \
    new_size = da_size(da) + 1;            \
    da = internal_da_resize(da, new_size); \
    (da)[new_size - 1] = value;            \
} while (0)

#define da_push(da, _pos, value) do {                 \
    u64 i = 0;                                        \
    u64 new_size = 0;                                 \
    u64 pos = 0;                                      \
                                                      \
    pos = _pos;                                       \
                                                      \
    new_size = da_size(da) + 1;                       \
    assert(pos < new_size && "da_push: invalid pos"); \
                                                      \
    da = internal_da_resize(da, new_size);            \
    for (i = new_size - 1; i > pos; --i) {            \
        (da)[i] = (da)[i - 1];                        \
    }                                                 \
                                                      \
    (da)[pos] = value;                                \
} while (0)

#define da_pop(da, _pos) do {                    \
    u64 i = 0;                                   \
    u64 size = 0;                                \
    u64 pos = 0;                                 \
                                                 \
    pos = _pos;                                  \
                                                 \
    size = da_size(da);                          \
    assert(pos < size && "da_pop: invalid pos"); \
                                                 \
    for (i = pos; i < size - 1; ++i) {           \
        (da)[i] = (da)[i + 1];                   \
    }                                            \
                                                 \
    DA_HEADER(da)->size--;                       \
} while (0)

/* ---------- Type defenitions ---------- */

typedef unsigned char util_bool;
#define util_true (1)
#define util_false (0)

typedef char i8;
typedef short int i16;
typedef int i32;
typedef long int i64;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;

typedef float f32;
typedef double f64;

typedef unsigned char c8;
typedef unsigned short int c16;

/* ---------- Arena ---------- */

typedef struct Arena_Region Arena_Region;
struct Arena_Region {
    Arena_Region *next;
    u64 capacity;
    u64 used;
    void *data;
    util_bool reallocatable;
};

typedef struct Arena Arena;
struct Arena {
    Arena_Region *start;
    Arena_Region *start_reallocatable;
};

/* ---------- Dynamic array ---------- */

typedef struct Da_Header Da_Header;
struct Da_Header {
    Arena *arena;
    u64 size;
    u64 stride;
};

/* ---------- Arena ---------- */

extern void *arena_alloc(Arena *a, u64 bytes);
extern void *arena_realloc(Arena *a, void *ptr, u64 old_size, u64 new_size);
extern void arena_free(Arena *a);

Arena_Region *internal_arena_region_create(u64 bytes);
Arena_Region *internal_arena_region_create_reallocatable(u64 bytes);
Arena_Region *internal_get_ptr_region(Arena *a, void *ptr);
void internal_arena_region_push(Arena *a, Arena_Region *r);
void internal_arena_region_push_reallocatable(Arena *a, Arena_Region *r);

/* ---------- Dynamic array ---------- */

extern u64 da_size(void *da);
extern u64 da_stride(void *da);
extern void da_pop_back(void *da);
extern void *da_clone(Arena *a, void *orig);
extern void *da_append(void *dest, void *src);
extern void *da_insert(void *dest, void *src, u64 pos);

void *internal_da_create(Arena *a, u64 size, u64 stride);
void *internal_da_resize(void *da, u64 new_size);

#endif /* UTIL_H_ */

#ifdef UTIL_IMPLEMENTATION

/* ---------- Arena ---------- */

Arena_Region *internal_arena_region_create(u64 bytes) {
    Arena_Region *r = NULL;
    u64 region_capacity = 0;

    r = malloc(sizeof(Arena_Region));
    region_capacity = ARENA_MAX(ARENA_REGION_DEFAULT_CAPACITY, bytes * ARENA_REGION_ALLOC_FACTOR);

    r->next = NULL;
    r->capacity = region_capacity;
    r->used = bytes;
    r->data = malloc(region_capacity);
    r->reallocatable = util_false;

    ARENA_DEBUG_INFO(("Created regular region, %lu/%lu", r->used, r->capacity));

    return r;
}

Arena_Region *internal_arena_region_create_reallocatable(u64 bytes) {
    Arena_Region *r = NULL;
    u64 region_capacity = 0;

    r = malloc(sizeof(Arena_Region));
    region_capacity = ARENA_MAX(ARENA_REGION_REALLOC_CAPACITY, bytes * ARENA_REGION_REALLOC_FACTOR);

    r->next = NULL;
    r->capacity = region_capacity;
    r->used = bytes;
    r->data = malloc(region_capacity);
    r->reallocatable = util_true;

    ARENA_DEBUG_INFO(("Created reallocatable region, %lu/%lu", r->used, r->capacity));

    return r;
}

Arena_Region *internal_get_ptr_region(Arena *a, void *ptr) {
    Arena_Region *r = NULL;

    r = a->start_reallocatable;
    while (r != NULL) {
        if (ptr == r->data) return r;
        r = r->next;
    }

    r = a->start;
    while (r != NULL) {
        if (ptr >= r->data && (char *)ptr < (char *)(r->data) + r->capacity) return r;
        r = r->next;
    }

    return NULL;
}

void internal_arena_region_push(Arena *a, Arena_Region *r) {
    if (a->start == NULL) {
        a->start = r;
    } else {
        r->next = a->start;
        a->start = r;
    }
}

void internal_arena_region_push_reallocatable(Arena *a, Arena_Region *r) {
    if (a->start_reallocatable == NULL) {
        a->start_reallocatable = r;
    } else {
        r->next = a->start_reallocatable;
        a->start_reallocatable = r;
    }
}

void *arena_alloc(Arena *a, u64 bytes) {
    void *ptr = NULL;

    if (a == NULL) {
        ARENA_DEBUG_INFO(("Arena pointer is NULL, allocating %lu bytes with malloc", bytes));
        return malloc(bytes);
    }

    if (a->start == NULL || a->start->used + bytes > a->start->capacity) {
        Arena_Region *r = NULL;

        r = internal_arena_region_create(bytes);
        internal_arena_region_push(a, r);
        return r->data;
    }

    ARENA_DEBUG_INFO(("Allocating %lu bytes on an existing region", bytes));

    ptr = (char *)a->start->data + a->start->used;
    a->start->used += bytes;
    return ptr;
}

void *arena_realloc(Arena *a, void *ptr, u64 old_size, u64 new_size) {
    u64 i = 0;
    Arena_Region *ptr_region = NULL;

    if (a == NULL) {
        ARENA_DEBUG_INFO(("Arena pointer is NULL, reallocating %lu bytes with realloc", new_size));
        return realloc(ptr, new_size);
    }

    ptr_region = internal_get_ptr_region(a, ptr);

    if (ptr == NULL || ptr_region == NULL) {
        return arena_alloc(a, new_size);
    }

    if (!ptr_region->reallocatable || new_size > ptr_region->capacity) {
        Arena_Region *new_region = NULL;

        new_region = internal_arena_region_create_reallocatable(new_size);

        for (i = 0; i < old_size; ++i) {
            ((char *)new_region->data)[i] = ((char *)ptr)[i];
        }
        internal_arena_region_push_reallocatable(a, new_region);

        return new_region->data;
    }

    ARENA_DEBUG_INFO(("Reallocating %lu -> %lu", old_size, new_size));
    ptr_region->used = new_size;
    return ptr;
}

void arena_free(Arena *a) {
    Arena_Region *r = NULL;

    r = a->start;
    while (r != NULL) {
        Arena_Region *next_region = NULL;

        ARENA_DEBUG_INFO(("Freeing regular region (%lu/%lu)", r->used, r->capacity));

        next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    r = a->start_reallocatable;
    while (r != NULL) {
        Arena_Region *next_region = NULL;

        ARENA_DEBUG_INFO(("Freeing reallocatable region (%lu/%lu)", r->used, r->capacity));
        
        next_region = r->next;
        free(r->data);
        free(r);
        r = next_region;
    }

    a->start = NULL;
    a->start_reallocatable = NULL;
}

/* ---------- Dynamic array ---------- */

void *internal_da_create(Arena *a, u64 size, u64 stride) {
    u64 allocd_size = 0;
    Da_Header *header = NULL;

    allocd_size = sizeof(Da_Header) + (size * stride);
    header = arena_alloc(a, allocd_size);

    header->arena = a;
    header->size = size;
    header->stride = stride;

    memset(DA_BODY(header), 0, size * stride);

    return DA_BODY(header);
}

void *internal_da_resize(void *da, u64 new_size) {
    Arena *a = NULL;
    Da_Header *header = NULL;
    u64 old_alloc_size = 0;
    u64 new_alloc_size = 0;

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

u64 da_size(void *da) {
    Da_Header *header = NULL;

    header = DA_HEADER(da);
    return header->size;
}

u64 da_stride(void *da) {
    Da_Header *header = NULL;

    header = DA_HEADER(da);
    return header->stride;
}

void da_pop_back(void *da) {
    assert(da_size(da) > 0 && "da_pop_back: invalid size");
    DA_HEADER(da)->size--;
}

void *da_clone(Arena *a, void *orig) {
    Da_Header *head = NULL;
    void *clone = NULL;

    head = DA_HEADER(orig);
    clone = internal_da_create(a, head->size, head->stride);
    memcpy(clone, orig, head->size * head->stride);

    return clone;
}

void *da_append(void *dest, void *src) {
    u64 old_sizeb = 0;
    Da_Header *h_dest = NULL;
    Da_Header *h_src = NULL;

    h_dest = DA_HEADER(dest);
    h_src = DA_HEADER(src);
    old_sizeb = h_dest->size * h_dest->stride;

    dest = internal_da_resize(dest, h_dest->size + h_src->size);
    memcpy((char *)dest + old_sizeb, src, h_src->size * h_src->stride);

    return dest;
}

void *da_insert(void *dest, void *src, u64 pos) {
    void *mv_from = NULL;
    void *mv_to = NULL;
    u64 src_sizeb = 0;
    Da_Header *h_dest = NULL;
    Da_Header *h_src = NULL;

    h_dest = DA_HEADER(dest);
    h_src = DA_HEADER(src);
    src_sizeb = h_src->size * h_src->stride;

    assert(pos <= h_dest->size && "da_insert: invalid pos");

    dest = internal_da_resize(dest, h_dest->size + h_src->size);

    mv_from = (char *)dest + pos * h_dest->stride;
    mv_to = (char *)mv_from + src_sizeb;

    /* TODO: maybe rewrite this without memmove? */
    /*       and refactor */
    memmove(mv_to, mv_from, src_sizeb);
    memcpy(mv_from, src, src_sizeb);

    return dest;
}

#endif /* UTIL_IMPLEMENTATION */
