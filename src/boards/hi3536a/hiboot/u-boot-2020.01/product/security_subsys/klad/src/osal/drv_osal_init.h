// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_OSAL_INIT_H
#define DRV_OSAL_INIT_H

#include <linux/delay.h>
#include <malloc.h>
#include <asm/io.h>

#define KLAD_MUTEX_T                         td_s32
#define klad_mutex_init(x)                   (*(x) = *(x))
#define klad_mutex_lock(x)
#define klad_mutex_unlock(x)
#define klad_mutex_destroy(x)

#define klad_ioremap_nocache(addr, size)     (td_void*)(addr)
#define klad_iounmap(addr, size)

#define klad_malloc(x)                       malloc(x)
#define klad_free(x)                         free(x)

#define klad_write(addr, data)               writel(data, addr)
#define klad_read(addr)                      readl(addr)

#define klad_udelay(x)                       udelay(x)

#define KLAD_QUEUE_HEAD                      td_s32
#define klad_queue_init(x)
#define klad_queue_wait_up(x)
#define klad_queue_destroy(x)

#define klad_queue_wait_timeout(head, func, param, time)

#define klad_request_irq(irq, func, name)

#define klad_free_irq(irq, name)

#endif /* DRV_OSAL_INIT_H */

