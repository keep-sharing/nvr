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
#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned long int uintptr_t;

typedef unsigned long size_t;

typedef int errno_t;

#define BITS_PER_LONG 32

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

typedef int bool;

#define TRUE    1
#define FALSE   0

#define ERROR   -1
#define OK      0
#define TD_SUCCESS	0
#define TD_FAILURE	-1
#define DATA_EMPTY	-2

#define AUTH_SUCCESS	(0x3CA5965A)
#define AUTH_FAILURE	(0xC35A69A5)

#define IN
#define OUT

#define PRIVATE static

typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed int      int32_t;
typedef unsigned int    uint32_t;
typedef long            int64_t;
typedef unsigned long   uint64_t;

#endif

