#ifndef INFO_H_
#define INFO_H_

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

#endif /* INFO_H_ */
