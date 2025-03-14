// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __OT_TYPE_H__
#define __OT_TYPE_H__

#ifdef __KERNEL__

#include <linux/types.h>
#else

#include <stdint.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifndef NULL
    #define NULL                0L
#endif

#define TD_NULL                 0L
#define TD_SUCCESS              0
#define TD_FAILURE              (-1)

typedef unsigned char           td_uchar;
typedef unsigned char           td_u8;
typedef unsigned short          td_u16;
typedef unsigned int            td_u32;
typedef unsigned long           td_ulong;

typedef char                    td_char;
typedef signed char             td_s8;
typedef short                   td_s16;
typedef int                     td_s32;
typedef long                    td_slong;

typedef float                   td_float;
typedef double                  td_double;

typedef void                    td_void;

#ifndef _M_IX86
    typedef unsigned long long  td_u64;
    typedef long long           td_s64;
#else
    typedef unsigned __int64    td_u64;
    typedef __int64             td_s64;
#endif
typedef unsigned long int       uintptr_t;

typedef unsigned long           td_size_t;
typedef unsigned long           td_length_t;
typedef unsigned long int       td_phys_addr_t;
typedef td_u32                  td_handle;
typedef uintptr_t               td_uintptr_t;
typedef unsigned int            ot_fr32;

typedef enum {
    TD_FALSE = 0,
    TD_TRUE  = 1,
} td_bool;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __OT_TYPE_H__ */

