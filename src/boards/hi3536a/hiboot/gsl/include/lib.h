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
#ifndef __LIB_H__
#define __LIB_H__

#include <types.h>

int strcmp(const char *cs, const char *ct);
int strncmp(const char *cs, const char *ct, size_t count);

void *memset(void *s, unsigned int c, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
int  memcmp(const void *cs, const void *ct, size_t count);

size_t strlen(const char *s);

errno_t memcpy_s(void *dest, size_t destMax, const void *src, size_t count);
errno_t memset_s(void *dest, size_t destMax, int c, size_t count);
errno_t strcpy_s(char *strDest, size_t destMax, const char *strSrc);

#ifdef CFG_DEBUG
size_t strnlen(const char *s, size_t count);
void printf(const char *fmt, ...);
void set_time_mark();
unsigned long get_time_ms();

#else /* CFG_DEBUG */

#define printf(...)
#define set_time_mark(...)
#define get_time_ms(...)

#endif /* CFG_DEBUG */

int malloc_init(void);
errno_t memmove_s(void *dest, size_t destMax, const void *src, size_t count);
void *malloc(size_t size);
void free(void *mem_ptr);

void udelay(unsigned long us);
void mdelay(unsigned long msec);

void debug_info(const char *printbuffer, char value);

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#undef reg_get
#define reg_get(addr) (*(volatile unsigned int *)(uintptr_t)(addr))

#undef reg_set
#define reg_set(addr, val) (*(volatile unsigned int *)(uintptr_t)(addr) = (val))

#ifndef readl
#define readl(addr) (*(volatile unsigned int *)(uintptr_t)(addr))
#endif

#ifndef writel
#define writel(val, addr) (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#endif

#ifndef readw
#define readw(addr) (*(volatile unsigned short *)(uintptr_t)(addr))
#endif

#ifndef writew
#define writew(val, addr) (*(volatile unsigned short *)(uintptr_t)(addr) = (val))
#endif

#ifndef readb
#define readb(addr) (*(volatile unsigned char *)(uintptr_t)(addr))
#endif

#ifndef writeb
#define writeb(val, addr) (*(volatile unsigned char *)(uintptr_t)(addr) = (val))
#endif

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

#endif /* __LIB_H__ */

