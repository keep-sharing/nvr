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

#include <types.h>
#include <common.h>
#include <log.h>
#include <com.h>
#include <rkp.h>
#include "hkl.h"
#include "klad.h"

static ALWAYS_INLINE int32_t _klad_func_start(const rkp_deob_kdf *deob_kdf,
		common_hkl *com_klad)
{
	int32_t level = 0;
	uint32_t tmp;
	int ret = 0;

	tmp = gen_wd_1(deob_kdf);
	ret = rkp_eff_rk_start(deob_kdf, tmp);
	if (ret != TD_SUCCESS)
		return TD_FAILURE;

	com_klad->key_size = 0; /* 128 bit key */
	tmp = gen_wd_2(com_klad, level);
	ret = hkl_content_start(com_klad, level, tmp);
	if (ret != TD_SUCCESS)
		return TD_FAILURE;

	level = 1;
	com_klad->cfg.is_odd = 0;
	com_klad->key_size = 1; /* 256 bit key */
	tmp = gen_wd_2(com_klad, level);
	ret = hkl_content_start(com_klad, level, tmp);
	if (ret != TD_SUCCESS)
		return TD_FAILURE;

	level = 2;
	com_klad->cfg.is_odd = 1;
	tmp = gen_wd_2(com_klad, level);
	ret = hkl_content_start(com_klad, level, tmp);
	if (ret != TD_SUCCESS)
		return TD_FAILURE;

	return TD_SUCCESS;
}

int32_t klad_com_process(const rkp_deob_kdf *deob_kdf, common_hkl *com_klad,
			 const uint32_t check_wd)
{
	volatile int32_t ret;

	chk_wd_2(deob_kdf, com_klad, check_wd);
	if (deob_kdf == NULL || com_klad == NULL)
		return TD_FAILURE;

	ret = hkl_com_lock();
	if (ret != TD_SUCCESS)
		return TD_FAILURE;

	ret = _klad_func_start(deob_kdf, com_klad);
	if (ret != TD_SUCCESS)
		print_error_func(_klad_func_start, ret);

	ret = hkl_com_unlock();
	if (ret != TD_SUCCESS)
		return TD_FAILURE;

	return ret;
}

