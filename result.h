#ifndef RESULT_H_
#define RESULT_H_

#include <assert.h>

enum Result_Code {
  UTIL_RESULT_OK, UTIL_RESULT_ERR
};
typedef enum Result_Code Result_Code;

#define UTIL_RESULT_OK(r, v) do { r.code = UTIL_RESULT_OK; r.value = v; } while (0)
#define UTIL_RESULT_ERR(r) do { r.code = UTIL_RESULT_ERR; } while (0)
#define UTIL_RESULT_UNWRAP(r) (assert(r.code == UTIL_RESULT_OK), r.value)

#endif /* RESULT_H_ */
