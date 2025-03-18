typedef unsigned char bool;
#define true 1
#define false 0

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

  return 0;
}
