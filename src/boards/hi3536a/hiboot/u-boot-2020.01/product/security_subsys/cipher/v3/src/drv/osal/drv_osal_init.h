// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_OSAL_INIT_H
#define DRV_OSAL_INIT_H

#include <asm/io.h>
#include <linux/delay.h>
#include <cpu_func.h>
#include <malloc.h>
#include "securec.h"
#include "drv_osal_chip.h"
#include "drv_cipher_kapi.h"
#include "drv_cipher_debug.h"

#define crypto_ioremap_nocache(addr, size)  (td_void *)(uintptr_t)(addr)
#define crypto_iounmap(addr, size)

#define crypto_read(addr)                   readl((volatile td_void *)(addr))
#define crypto_write(addr, val)             writel(val, (volatile td_void *)(addr))

#define crypto_msleep(msec)                 udelay((msec) * 1000)
#define crypto_udelay(usec)                 udelay(usec)

#define MAX_MALLOC_BUF_SIZE                 0x10000
td_void *crypto_calloc(size_t n, size_t size);
#define crypto_malloc(x)                    ((x) > 0 ? malloc(x) : TD_NULL)
#define crypto_free(x)              \
    do {                            \
        if ((x) != TD_NULL) {       \
            free((x));              \
            x = TD_NULL;            \
        }                           \
    } while (0)

#define CRYPTO_QUEUE_HEAD                   td_void *
#define crypto_queue_init(x)                (void)0
#define crypto_queue_wait_up(x)             (void)0

#define crypto_queue_wait_timeout(head, con, time) (con)

#define crypto_request_irq(irq, func, name) (td_s32)0

#define crypto_free_irq(irq, name)

#define CRYPTO_MUTEX                        td_s32
#define crypto_mutex_init(x)                (*(x)) = 0
#define crypto_mutex_lock(x)                0
#define crypto_mutex_unlock(x)
#define crypto_mutex_destroy(x)

/* task_tgid_nr(current): Thread group ID
 * current->pid         : Process ID
 */
#define CRYPTO_OWNER                        td_u32
#define crypto_get_owner(x)                 *(x) = 0

#define CRYPTO_IRQRETURN_T                  td_s32

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
#define OT_PROC_SUPPORT
#define CRYPTO_PROC_PRINT                   printf
#endif

#define crypto_flush_dcache_all()           flush_dcache_all()

#endif /* DRV_OSAL_INIT_H */
