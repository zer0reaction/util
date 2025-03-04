# Arena Allocator

[Region-based memory management](https://en.wikipedia.org/wiki/Region-based_memory_management)

## My implementation

Regions are stored as a linked list and have a default capacity of 4096 bytes (you can change that with `ARENA_REGION_DEFAULT_CAPACITY` preprocessor directive). Pointers to the first and the last region are stored in the arena struct.

When the `arena_alloc` is called, it allocatates memory on the first region that has enough free space to store the data. If there are no such regions, one is created and placed at the end of the linked list. If you are allocating more than `ARENA_REGION_DEFAULT_CAPACITY`, then a region of such size will be created and placed at the end of the linked list.

`arena_alloc` behaves like `malloc` if `NULL` is passed instead of arena pointer.

When `arena_free` is called, arena is freed by walking through the linked list and freeing both the region data and the region itself.

## Function description

(Do not forget to add `UTIL_IMPLEMENTATION` preprocessor directive if you need the implementations of the functions and not just the library header)

### `arena_alloc`

```c
void *arena_alloc(Arena *a, size_t bytes);
```

Allocates `bytes` amount of bytes in the arena. Behaves like `malloc` if `NULL` is passed instead of an arena pointer.

### `arena_free`

```c
void arena_free(Arena *a);
```

Frees all the regions and memory in them.

## Basic Usage

To use the arena allocator you first need to create an empty `Arena` struct:

```c
Arena arena = {0};
```

Then you can start allocating memory in it by calling `arena_alloc`:

```c
int *a = arena_alloc(&arena, sizeof(int));
int *b = arena_alloc(&arena, sizeof(int));
```

This will create the first block with the default size of 4096 bytes. To change the default region capacity define `ARENA_REGION_DEFAULT_CAPACITY` preprocessor directive with the desired size in bytes.

After you have finished allocating all the memory and no longer need it, you can free all the regions by calling `arena_free`:

```c
arena_free(&arena);
```

Now the arena is back to it's original state.
