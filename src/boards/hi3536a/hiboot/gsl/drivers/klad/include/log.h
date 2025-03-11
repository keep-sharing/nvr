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
#ifndef LOG_H
#define LOG_H

#include "lib.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define print_error_check(a, b) \
	printf("[%s][%04d]check %s and %s failed.\n", __FUNCTION__, __LINE__, #a, #b)
#define print_error_func(func, ret) \
	printf("[%s][%04d]call [%s] failed, return 0x%x.\n", __FUNCTION__, __LINE__, #func, ret)
#define print_debug_func() \
	printf("[%s][%04d]mark.\n", __FUNCTION__, __LINE__)
#define log_printf(fmt...) \
	do { \
		printf("\033[1;31m"); \
		printf("[%s][%d] ", __func__, __LINE__); \
		printf(fmt); \
		printf("\033[0m"); \
	} while (0)
#define print_val(v) \
	printf("[%-40s]  0x%08x.\n", #v, v)
#define print_str(v, l) \
	do { \
		int j = 0; \
		printf("[%-40s]  ", #v); \
		for (j = 0; j < (l); j++) \
			printf("%02x", (v)[j]); \
		printf(".\n"); \
	} while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* deob.h */
