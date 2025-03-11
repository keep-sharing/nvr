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
#ifndef __RKP_KDF_H__
#define __RKP_KDF_H__

#include "rkp_com.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
	rkp_slot_choose_e rootkey_slot; /* delete in hpp rom but hrf hsl */
	uint32_t cas_kdf_static; /* delete in hpp rom , fixed set xx in hrf hsl */
	uint32_t unique_type; /* fixed set 0 */
	uint32_t sw_reg; /* sw reg */
	uint32_t remap; /* remap */
} rkp_kdf;

typedef struct {
	rkp_klad_sel_e klad_sel;
	rkp_klad_type_sel_e klad_type_sel;
} klad_sel;

typedef struct {
	uint8_t sck_crc;
	uint8_t sckv_crc;
	uint8_t seedv_crc;
	uint8_t modv_crc;
	uint8_t eff_rk_crc;
} rkp_kdf_rst;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __drv_rkp_h__ */

