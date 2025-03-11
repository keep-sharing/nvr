// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_OSAL_INIT_H
#define DRV_OSAL_INIT_H

#include <linux/delay.h>
#include <asm/io.h>
#include <malloc.h>
#include <string.h>

#define OTP_MUTEX_T                         td_s32
#define otp_mutex_init(x)                   (*(x) = *(x))
#define otp_mutex_lock(x)
#define otp_mutex_unlock(x)
#define otp_mutex_destroy(x)

#define otp_ioremap_nocache(addr, size)     (td_void*)(addr)
#define otp_iounmap(addr, size)

#define otp_malloc(x)                       malloc(x)
#define otp_free(x)                         \
    do {                                    \
        if (x) {                            \
            free(x);                        \
            x = TD_NULL;                    \
        }                                   \
    } while (0)

#define otp_write(addr, data)               writel(data, addr)
#define otp_read(addr)                      readl(addr)

#define otp_udelay(x)                       udelay(x)

#endif /* DRV_OSAL_INIT_H */
