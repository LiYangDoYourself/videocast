#ifndef __UCONFIG_H__
#define __UCONFIG_H__

/**
 *******************************************************************************
 * prompt
 *******************************************************************************
 */
#ifndef O
# define O
#endif

#ifndef _user_
# define _user_
#endif

#ifndef _kernel_
# define _kernel_
#endif

#ifndef _safe_
# define _safe_
#endif

#ifndef _maybe_
# define _maybe_
#endif

#ifndef _force_
# define _force_
#endif

#ifndef _ignore_
# define _ignore_
#endif

#ifndef _restrict_
# define _restrict_
#endif

#ifndef _const_
# define _const_
#endif

#ifndef _static_
# define _static_
#endif

#ifndef _friend_
# define _friend_
#endif

#ifndef _explicit_
# define _explicit_
#endif

#ifndef _virtual_
# define _virtual_
#endif

#ifndef _inline_
# define _inline_
#endif

#ifndef _public_
# define _public_
#endif

#ifndef _protected_
# define _protected_
#endif

#ifndef _private_
# define _private_
#endif

#ifndef _function_
# define _function_
#endif

/**
 *******************************************************************************
 * typedef enum
 *******************************************************************************
 */
typedef enum _state_
{
    OFF = 0,
    ON = 1,
} state_t;

typedef enum _if_
{
    NO = 0,
    YES = 1,
} if_t;

typedef enum _GlobalAlgNums_
{
    MaxAlgNum=16,
}GlobalAlgNums_t;

typedef volatile unsigned char		regb_t;
typedef volatile unsigned short 	regw_t;
typedef volatile unsigned long		regl_t;
typedef volatile unsigned char *	regb_ptr;
typedef volatile unsigned short *	regw_ptr;
typedef volatile unsigned long *	regl_ptr;
typedef signed long			flag_t;
typedef unsigned long		addr_t;


#endif // __UCONFIG_H__
