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

#ifndef __KLAD_H__
#define __KLAD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int32_t klad_com_process(const rkp_deob_kdf *deob_kdf, common_hkl *com_klad,
			 const uint32_t check_wd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

