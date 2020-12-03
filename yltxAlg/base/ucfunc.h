#ifndef __UCFUNC_H__
#define __UCFUNC_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *******************************************************************************
 * standard
 *******************************************************************************
 */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif


/**
 *******************************************************************************
 * macro
 *******************************************************************************
 */
#ifndef NULL
# define NULL				(0)
#endif //NULL

/**
 *******************************************************************************
 * likey and unlikely
 *******************************************************************************
 */
#ifndef likely
# define likely(x)			__builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
# define unlikely(x)		__builtin_expect(!!(x), 0)
#endif

/**
 *******************************************************************************
 * array operation
 *******************************************************************************
 */
#ifndef ARRAY_SIZE
# define ARRAY_SIZE(ptr)		(sizeof(ptr) / sizeof((ptr)[0]))
#endif

/**
 *******************************************************************************
 * return struct header
 *******************************************************************************
 */
#ifndef offsetof
# define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif

#ifndef container_of
# define container_of(ptr, type, member) ({ \
    const decltype(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member));})
#endif /* container_of */

/**
 *******************************************************************************
 * assert
 *******************************************************************************
 */
#define USE_ASSERT_DEBUG

#ifdef USE_ASSERT_DEBUG
#define usr_assert(condition) \
    do { \
        if (unlikely(condition)) { \
            fprintf(stdout, "> File:<%s>, Func:<%s>, Line:<%d>, <%s>\n", \
                __FILE__, __func__, __LINE__, #condition); \
            fflush(stdout); \
            while(1); \
        } \
    } while(0)
#else
#define usr_assert(condition)
#endif

/**
 *******************************************************************************
 * prif
 *******************************************************************************
 */
#define USE_DEBUG

/** ------------------------------------------------------------------------
 * user space */
#define prif(fmt, ...)	do {printf(fmt, ##__VA_ARGS__);fflush(stdout);} while (0)
#define pret(s, d)	do {fprintf(stdout, "%s - RET:%d\n", s, d);fflush(stdout);} while (0)
#define perr(s) 	do {fprintf(stdout, "> File:<%s>, Func:<%s>, Line:<%d>\n  %s - Err:%s\n",\
    __FILE__, __func__, __LINE__, s, strerror(errno));fflush(stdout);} while (0)

#ifdef USE_DEBUG
#define dprif(fmt, ...) do {printf(fmt, ##__VA_ARGS__);fflush(stdout);} while (0)
#else
#define dprif(fmt, ...)
#endif



#ifdef __cplusplus
} /* end of the 'extern "C"' block */
#endif

#endif // __UCFUNC_H__
