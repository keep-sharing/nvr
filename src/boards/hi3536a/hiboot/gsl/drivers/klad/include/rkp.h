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
#ifndef __RKP_DEOB_EFF_RK_H__
#define __RKP_DEOB_EFF_RK_H__

#include "rkp_com.h"
#include "rkp_deob.h"
#include "rkp_kdf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
	rkp_deob deob;
	rkp_kdf  kdf;
	klad_sel klad;
} rkp_deob_kdf;
int32_t rkp_get_klad_level(const int32_t module_id_0, const uint32_t check_wd);
int32_t rkp_eff_rk_start(const rkp_deob_kdf *deob_kdf, const uint32_t check_wd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __drv_rkp_h__ */

