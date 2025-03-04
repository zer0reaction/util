# My C utility library

I started developing this because *using C without libraries is a pain*.

Start exploring the library by visiting the [examples](./examples)

## Current functionality

- Arena allocator
- Strings (support arena allocation)

## Quick start

You can just simply copy-paste the library header and start using it.

```c
#define UTIL_IMPLEMENTATION
#include "util.h"

#include <stdio.h>

int main(void)
{
    Arena a = {0};

    String s = string_from_literal(&a, "Hello, world!");
    printf("%s\n", s.data);

    arena_free(&a);
    return 0;
}
```
