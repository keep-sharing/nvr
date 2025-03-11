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

#ifndef __COM_H__
#define __COM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define chk_ret(func) \
	do { \
		if ((func) != TD_SUCCESS) { \
			print_error_func(func, TD_FAILURE); \
			return TD_FAILURE; \
		} \
	} while (0)

#define neq_ret(a, b) \
	do { \
		if ((a) != (b)) { \
			print_error_check(a, b); \
			return TD_FAILURE; \
		} \
	} while (0)

#define chk_wd_1(data0, check_wd) \
	neq_ret((uintptr_t)(data0) ^ ((uintptr_t)(data0) << 16), check_wd)
#define chk_wd_2(data0, data1, check_wd) \
	neq_ret((uintptr_t)(data0) ^ (uintptr_t)(data1), check_wd)
#define chk_wd_3(data0, data1, data2, check_wd) \
	neq_ret((uintptr_t)(data0) ^ (uintptr_t)(data1) ^ (uintptr_t)(data2), check_wd)
#define chk_wd_4(data0, data1, data2, data3, check_wd) \
	neq_ret((uintptr_t)(data0) ^ (uintptr_t)(data1) ^ (uintptr_t)(data2) ^ (uintptr_t)(data3), check_wd)
#define chk_wd_5(data0, data1, data2, data3, data4, check_wd) \
	neq_ret((uintptr_t)(data0) ^ (uintptr_t)(data1) ^ (uintptr_t)(data2) ^ (uintptr_t)(data3) ^ \
			(uintptr_t)(data4), check_wd)

#define gen_wd_1(data0) ((uintptr_t)(data0) ^ ((uintptr_t)(data0) << 16))
#define gen_wd_2(data0, data1) ((uintptr_t)(data0) ^ (uintptr_t)(data1))
#define gen_wd_3(data0, data1, data2) ((uintptr_t)(data0) ^ (uintptr_t)(data1) ^ (uintptr_t)(data2))
#define gen_wd_4(data0, data1, data2, data3) \
	((uintptr_t)(data0) ^ (uintptr_t)(data1) ^ (uintptr_t)(data2) ^ (uintptr_t)(data3))
#define gen_wd_5(data0, data1, data2, data3, data4) \
	((uintptr_t)(data0) ^ (uintptr_t)(data1) ^ (uintptr_t)(data2) ^ (uintptr_t)(data3) ^ (uintptr_t)(data4))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
