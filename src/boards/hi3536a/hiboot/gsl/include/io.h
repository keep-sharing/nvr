/*
 *
 * Copyright (c) 2020-2021 Shenshu Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#include <types.h>

#define __arch_getl(a)      (*(volatile unsigned int *)(a))
#define __arch_putl(v, a)   (*(volatile unsigned int *)(unsigned long)(a) = (v))

#define dsb()	asm volatile ("dsb sy" : : : "memory")
#define dmb()	asm volatile ("dmb sy" : : : "memory")

#define mb()        dsb()
#define __iormb()   dmb()
#define __iowmb()   dmb()

#define writel(v, c) ({ uint32_t __v = v; __iowmb(); __arch_putl(__v, c); __v; })
#define readl(c)    ({ uint32_t __v = __arch_getl(c); __iormb(); __v; })

#endif  /* __ASM_ARM_IO_H */
