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
#ifndef __RKP_DEOB_H__
#define __RKP_DEOB_H__

#include "rkp_com.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
	rkp_slot_choose_e rootkey_slot; /* delte in hpp rom but hrf hsl */
} rkp_deob;

typedef struct {
	uint8_t vmask_crc;
	uint8_t clear_rk_crc;
} rkp_deob_rst;
int rkp_deob_start(rkp_deob *deob, rkp_deob_rst *deob_rst, rkp_deob_type_e deob_type);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __drv_rkp_h__ */

