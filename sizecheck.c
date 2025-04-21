#define DEBUG
#include "typedef.h"
#include "info.h"
#include <assert.h>

int main(void) {
    assert(sizeof(i8) == 1);
    assert(sizeof(i16) == 2);
    assert(sizeof(i32) == 4);
    assert(sizeof(i64) == 8);

    assert(sizeof(u8) == 1);
    assert(sizeof(u16) == 2);
    assert(sizeof(u32) == 4);
    assert(sizeof(u64) == 8);

    assert(sizeof(f32) == 4);
    assert(sizeof(f64) == 8);

    assert(sizeof(c8) == 1);
    assert(sizeof(c16) == 2);

    INFO("sizecheck", ("All types match their sizes"));

    return 0;
}
