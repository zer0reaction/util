#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <stdlib.h>

#ifdef UTIL_DEBUG
  #include <stdio.h>
#endif /* UTIL_DEBUG */

#ifdef UTIL_DEBUG
  #define UTIL_DEBUG_INFO(info) do { \
    printf("[UTIL_DEBUG] ");     \
    printf info ;          \
    printf("\n");          \
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

#ifndef ARENA_REGION_ALLOC_FACTOR
  #define ARENA_REGION_ALLOC_FACTOR (1.25)
#endif /* ARENA_REGION_ALLOC_FACTOR */

#define UTIL_MAX(a, b) ((a > b) ? a : b)
#define LIST_HEADER(list) ((List_Header *)list - 1)
#define LIST_BODY(header) ((char *)header + sizeof(List_Header))

typedef unsigned char bool;
#define true 1
#define false 0

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
  Arena_Region *start;
  Arena_Region *start_reallocatable;
};

typedef struct List_Header List_Header;
struct List_Header {
  size_t size;
  size_t stride;
};

extern void *arena_alloc(Arena *a, size_t bytes);
extern void *arena_realloc(Arena *a, void *ptr, size_t old_size, size_t new_size);
extern void arena_free(Arena *a);

#define list_create(arena, T, size) internal_list_create(arena, size, sizeof(T))
extern size_t list_get_size(void *list);
extern size_t list_get_stride(void *list);
#define list_push_back(arena, list, value) do {        \
  size_t new_size;                                     \
                                                       \
  new_size = list_get_size(list) + 1;                  \
  list = internal_list_resize(arena, list, new_size);  \
  list[new_size - 1] = value;                          \
} while (0)

Arena_Region *internal_arena_region_create(size_t bytes);
Arena_Region *internal_arena_region_create_reallocatable(size_t bytes);
Arena_Region *internal_get_ptr_region(Arena *a, void *ptr);
void internal_arena_region_push(Arena *a, Arena_Region *r);
void internal_arena_region_push_reallocatable(Arena *a, Arena_Region *r);
void *internal_list_create(Arena *a, size_t size, size_t stride);
void *internal_list_resize(Arena *a, void *list, size_t new_size);

#endif /* UTIL_H_ */

#ifdef UTIL_IMPLEMENTATION

Arena_Region *internal_arena_region_create(size_t bytes) {
  Arena_Region *r;
  size_t region_capacity;

  r = malloc(sizeof(Arena_Region));
  region_capacity = UTIL_MAX(ARENA_REGION_DEFAULT_CAPACITY, bytes * ARENA_REGION_ALLOC_FACTOR);

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
  region_capacity = UTIL_MAX(ARENA_REGION_DEFAULT_CAPACITY, bytes * ARENA_REGION_REALLOC_FACTOR);

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
    UTIL_DEBUG_INFO(("Going through reallocatable regions..."));
    if (ptr == r->data) return r;
    r = r->next;
  }
  UTIL_DEBUG_INFO(("Searching for ptr in reallocatable regions failed, trying in regular"));

  r = a->start;
  while (r != NULL) {
    UTIL_DEBUG_INFO(("Going through regular regions..."));
    if (ptr >= r->data && (char *)ptr < (char *)(r->data) + r->capacity) return r;
    r = r->next;
  }
  UTIL_DEBUG_INFO(("Searching for ptr in regular regions failed"));

  return NULL;
}

void internal_arena_region_push(Arena *a, Arena_Region *r) {
  if (a->start == NULL) {
    UTIL_DEBUG_INFO(("Pushing regular region to an empty list"));
    a->start = r;
  } else {
    UTIL_DEBUG_INFO(("Pushing regular region"));
    r->next = a->start;
    a->start = r;
  }
}

void internal_arena_region_push_reallocatable(Arena *a, Arena_Region *r) {
  if (a->start_reallocatable == NULL) {
    UTIL_DEBUG_INFO(("Pushing reallocatable region to an empty list"));
    a->start_reallocatable = r;
  } else {
    UTIL_DEBUG_INFO(("Pushing reallocatable region"));
    r->next = a->start_reallocatable;
    a->start_reallocatable = r;
  }
}

void *internal_list_create(Arena *a, size_t size, size_t stride) {
  size_t allocd_size;
  List_Header *header;

  UTIL_DEBUG_INFO(("Creating list of size %ld and stride %ld", size, stride));

  allocd_size = sizeof(List_Header) + (size * stride);
  header = arena_alloc(a, allocd_size);
  header->size = size;
  header->stride = stride;

  return LIST_BODY(header);
}

void *internal_list_resize(Arena *a, void *list, size_t new_size) {
  List_Header *header;
  size_t old_alloc_size, new_alloc_size;

  header = LIST_HEADER(list);
  old_alloc_size = sizeof(List_Header) + header->size * header->stride;
  new_alloc_size = sizeof(List_Header) + new_size * header->stride;

  UTIL_DEBUG_INFO(("Resizing list %ld -> %ld (%ld to %ld bytes)", header->size, new_size, old_alloc_size, new_alloc_size));

  if (new_alloc_size > old_alloc_size) {
    header = arena_realloc(a, header, old_alloc_size, new_alloc_size);
  }

  header->size = new_size;
  return LIST_BODY(header);
}

void *arena_alloc(Arena *a, size_t bytes) {
  void *ptr;

  if (a == NULL) {
    UTIL_DEBUG_INFO(("Arena pointer is NULL, allocating %ld bytes with malloc", bytes));
    return malloc(bytes);
  }

  if (a->start == NULL || a->start->used + bytes > a->start->capacity) {
    Arena_Region *r;

    r = internal_arena_region_create(bytes);
    internal_arena_region_push(a, r);
    return r->data;
  }

  UTIL_DEBUG_INFO(("Allocating %ld bytes on an existing region", bytes));

  ptr = (char *)a->start->data + a->start->used;
  a->start->used += bytes;
  return ptr;
}

void *arena_realloc(Arena *a, void *ptr, size_t old_size, size_t new_size) {
  size_t i;
  Arena_Region *ptr_region;

  if (a == NULL) {
    UTIL_DEBUG_INFO(("Arena pointer is NULL, reallocating %ld bytes with realloc", new_size));
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

  UTIL_DEBUG_INFO(("Reallocating %ld -> %ld", old_size, new_size));
  ptr_region->used = new_size;
  return ptr;
}

void arena_free(Arena *a) {
  Arena_Region *r;

  r = a->start;
  while (r != NULL) {
    Arena_Region *next_region;

    UTIL_DEBUG_INFO(("Freeing regular region (%ld/%ld)", r->used, r->capacity));

    next_region = r->next;
    free(r->data);
    free(r);
    r = next_region;
  }

  r = a->start_reallocatable;
  while (r != NULL) {
    Arena_Region *next_region;

    UTIL_DEBUG_INFO(("Freeing reallocatable region (%ld/%ld)", r->used, r->capacity));
    
    next_region = r->next;
    free(r->data);
    free(r);
    r = next_region;
  }

  a->start = NULL;
  a->start_reallocatable = NULL;
}

size_t list_get_size(void *list) {
  List_Header *header;

  header = LIST_HEADER(list);
  return header->size;
}

size_t list_get_stride(void *list) {
  List_Header *header;

  header = LIST_HEADER(list);
  return header->stride;
}

#undef UTIL_IMPLEMENTATION
#endif /* UTIL_IMPLEMENTATION */
