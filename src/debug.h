#ifndef UNPACK_DEBUG_H
#define UNPACK_DEBUG_H

#include <stdio.h>

#ifdef DEBUG

#define DP(fmt, args...) \
    fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#define DF(...)                                                              \
    do {                                                                     \
        FILE *fp = fopen("/tmp/unpack.debug.txt","a");                       \
        fprintf(fp,"%s:%s:%d:\t", __FILE__, __func__, __LINE__);             \
        fprintf(fp,__VA_ARGS__);                                             \
        fprintf(fp,"\n");                                                    \
        fclose(fp);                                                          \
    } while (0)
#else /* do nothing */

#define DP(fmt, args...)
#define DF(...)

#endif

#endif // UNPACK_DEBUG_H
