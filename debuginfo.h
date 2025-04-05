#ifndef DEBUGINFO_H_
#define DEBUGINFO_H_

#ifdef DEBUG
    #include <stdio.h>
    #define DEBUG_INFO(caller, info) do { \
        printf("[%s] ", caller);               \
        printf info;                           \
        printf("\n");                          \
    } while (0)
#else
    #define DEBUG_INFO(caller, info)
#endif

#endif /* DEBUGINFO_H_ */
