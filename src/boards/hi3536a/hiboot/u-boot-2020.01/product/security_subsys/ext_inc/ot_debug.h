// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __OT_DEBUG_H__
#define __OT_DEBUG_H__

#ifndef __KERNEL__
#include <stdio.h>
#include <stdarg.h>
#else
#include <asm/io.h>
#endif

#include "ot_type.h"
#include "ot_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef __KERNEL__

#define OT_PRINT      printf

#else

#define OT_PRINT      printk

#endif /* end of __KERNEL__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __OT_DEBUG_H__ */
