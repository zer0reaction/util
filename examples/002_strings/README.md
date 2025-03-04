# Strings

## Function and structs description

(Do not forget to add `UTIL_IMPLEMENTATION` preprocessor directive if you need the implementations of the functions and not just the library header)

### `String`

```c
typedef struct {
    size_t capacity;
    size_t length;
    char8_t *data; // aka unsigned char
} String;
```

- `capacity` - number of characters that a string can hold (not including the null-termination byte)
- `length` - current number of characters in the string (not including the null-termination byte)
- `data` - pointer to the start of the string

### `string_create`

```c
String string_create(Arena *a, size_t capacity);
```

Allocates an UTF-8 encoded string of the specified capacity on the arena (or on the heap if `NULL` is passed) and fills it with zeros. The string is automatically null-terminated. Note, that capacity does not include the null-termination byte, so creating a string of size `0` you allocate one byte of memory.

### `string_from_literal`

```c
String string_from_literal(Arena *a, const char *literal);
```

Allocates an UTF-8 encoded string on the arena (or on the heap if `NULL` is passed) and copies the characters from the string literal to the string. The string is automatically null-terminated. The capacity of the new string is the length of the string literal. Note, that capacity does not include the null-termination byte.

### `string_cat_create`

```c
String string_cat_create(Arena *a, String s1, String s2);
```

Concatenates `s1` and `s2` by creating a new string on the arena (or on the heap if `NULL` is passed) with the capacity of `s1.length + s2.length`. String is automatically null-terminated.

### `string_cat`

```c
void *string_cat(String *s1, String s2);
```

Concatenates `s1` and `s2` by appending `s2`'s contents to the end of `s1`. The string is automatically null-terminated. On success, returns `s1`. If there is not enough capacity, `NULL` is returned. Note, that nor capacity neither length do not include the null-termination byte.
