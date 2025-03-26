#ifndef UTIL_DEBUGINFO_H_
#define UTIL_DEBUGINFO_H_

#ifdef UTIL_DEBUG
    #include <stdio.h>
    #define UTIL_DEBUG_INFO(caller, info) do { \
        printf("[%s] ", caller);               \
        printf info;                           \
        printf("\n");                          \
    } while (0)
#else
    #define UTIL_DEBUG_INFO(caller, info)
#endif

#endif /* UTIL_DEBUGINFO_H_ */
