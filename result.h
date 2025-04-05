#ifndef RESULT_H_
#define RESULT_H_

#include <assert.h>

enum Result_Code {
    RESULT_OK, RESULT_ERR
};
typedef enum Result_Code Result_Code;

/*
    Result structs are defined like this:

    typedef struct {
        Result_Code code;
        int value;
    } Result_Int;

    Then you can use this struct for the macros.
*/

#define RESULT_OK(r, v) do { r.code = RESULT_OK; r.value = v; } while (0)
#define RESULT_ERR(r) do { r.code = RESULT_ERR; } while (0)

/* if a function is passed it is ran twice, too bad */
/* #define RESULT_UNWRAP(r) (assert(r.code == RESULT_OK), r.value) */

#endif /* RESULT_H_ */
