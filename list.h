#ifndef UTIL_LIST_H_
#define UTIL_LIST_H_

#include <stddef.h>

#include "typedef.h"
#include "arena.h"

#ifdef UTIL_LIST_DEBUG
  #include <stdio.h>
#endif /* UTIL_LIST_DEBUG */

#ifdef UTIL_LIST_DEBUG
  #define UTIL_LIST_DEBUG_INFO(info) do { \
    printf("[UTIL LIST] ");               \
    printf info;                           \
    printf("\n");                          \
  } while (0)
#else
  #define UTIL_LIST_DEBUG_INFO(info)
#endif

#define UTIL_LIST_HEADER(list) ((List_Header *)list - 1)
#define UTIL_LIST_BODY(header) ((char *)header + sizeof(List_Header))

typedef struct List_Header List_Header;
struct List_Header {
  size_t size;
  size_t stride;
};

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

void *internal_list_create(Arena *a, size_t size, size_t stride);
void *internal_list_resize(Arena *a, void *list, size_t new_size);

#endif /* UTIL_LIST_H_ */

#ifdef UTIL_LIST_IMPLEMENTATION

void *internal_list_create(Arena *a, size_t size, size_t stride) {
  size_t allocd_size;
  List_Header *header;

  UTIL_LIST_DEBUG_INFO(("Creating list of size %ld and stride %ld", size, stride));

  allocd_size = sizeof(List_Header) + (size * stride);
  header = arena_alloc(a, allocd_size);
  header->size = size;
  header->stride = stride;

  return UTIL_LIST_BODY(header);
}

void *internal_list_resize(Arena *a, void *list, size_t new_size) {
  List_Header *header;
  size_t old_alloc_size, new_alloc_size;

  header = UTIL_LIST_HEADER(list);
  old_alloc_size = sizeof(List_Header) + header->size * header->stride;
  new_alloc_size = sizeof(List_Header) + new_size * header->stride;

  UTIL_LIST_DEBUG_INFO(("Resizing list %ld -> %ld (%ld to %ld bytes)", header->size, new_size, old_alloc_size, new_alloc_size));

  if (new_alloc_size > old_alloc_size) {
    header = arena_realloc(a, header, old_alloc_size, new_alloc_size);
  }

  header->size = new_size;
  return UTIL_LIST_BODY(header);
}

size_t list_get_size(void *list) {
  List_Header *header;

  header = UTIL_LIST_HEADER(list);
  return header->size;
}

size_t list_get_stride(void *list) {
  List_Header *header;

  header = UTIL_LIST_HEADER(list);
  return header->stride;
}

#undef UTIL_LIST_IMPLEMENTATION
#endif /* UTIL_LIST_IMPLEMENTATION */
