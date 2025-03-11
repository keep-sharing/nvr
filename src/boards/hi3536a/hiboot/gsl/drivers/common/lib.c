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
#include <lib.h>
#include <common.h>
#include <platform.h>

#define BIT_LEN_OF_BYTE	8

size_t strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc);

	return sc - s;
}

int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1 = NULL;
	const unsigned char *su2 = NULL;
	int res = 0;

	for (su1 = cs, su2 = ct; count > 0; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

#ifdef CFG_DEBUG
#ifdef PRINT_TO_MEM
#define MEM_FOR_PRINT_BASE 0xf8400000
u32 mem_print_base;
#endif

#define CFG_PBSIZE 256

void printf(const char *fmt, ...)
{
}

size_t strnlen(const char *s, size_t count)
{
	const char *sc;
	for (sc = s; count-- && *sc != '\0'; ++sc);
	return sc - s;
}

void set_time_mark()
{
	timer_start();
}

unsigned long  get_time_ms()
{
	return timer_get_val() / timer_get_divider();
}
#endif

void debug_info(const char *printbuffer, char value)
{
}

/**
 * strncmp - Compare two length-limited strings
 * @cs: One string
 * @ct: Another string
 * @count: The maximum number of bytes to compare
 */
int strncmp(const char *cs, const char *ct, size_t count)
{
	register signed char __res = 0;

	while (count) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}

	return __res;
}

/**
 * strcmp - Compare two strings
 * @cs: One string
 * @ct: Another string
 */
int strcmp(const char *cs, const char *ct)
{
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

/**
 * memset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
void *memset(void *s, unsigned int c, size_t count)
{
	unsigned long *sl = (unsigned long *) s;
	unsigned long cl = 0;
	char *s8 = NULL;
	int i;

	/* do it one word at a time (32 bits or 64 bits) while possible */
	if (((uintptr_t)s & (sizeof(*sl) - 1)) == 0) {
		for (i = 0; i < sizeof(*sl); i++) {
			cl <<= BIT_LEN_OF_BYTE;
			cl |= c & 0xff;
		}
		while (count >= sizeof(*sl)) {
			*sl++ = cl;
			count -= sizeof(*sl);
		}
	}
	/* fill 8 bits at a time */
	s8 = (char *)sl;
	while (count--)
		*s8++ = c;

	return s;
}

/**
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void *memcpy(void *dest, const void *src, size_t count)
{
	unsigned long *dl = (unsigned long *)dest;
	unsigned long *sl = (unsigned long *)src;
	char *d8 = NULL;
	char *s8 = NULL;

	if (src == dest)
		return dest;

	/* while all data is aligned (common case), copy a word at a time */
	if ((((uintptr_t)dest | (uintptr_t)src) & (sizeof(*dl) - 1)) == 0) {
		while (count >= sizeof(*dl)) {
			*dl++ = *sl++;
			count -= sizeof(*dl);
		}
	}
	/* copy the reset one byte at a time */
	d8 = (char *)dl;
	s8 = (char *)sl;
	while (count--)
		*d8++ = *s8++;

	return dest;
}

