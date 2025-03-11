// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_api.h"
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

int bsp_ddrtrn_training_item(enum ddrtrn_training_item item)
{
	int ret;
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	uintptr_t ctx_addr = (uintptr_t)&ctx;
	uintptr_t phy_all_addr = (uintptr_t)&phy_all;

	ddrtrn_hal_set_cfg_addr(ctx_addr, phy_all_addr);

	switch (item) {
	case DDR_HW_TRAINING:
		ret = ddrtrn_hal_hw_training_init();
		break;
	case DDR_SW_TRAINING:
		ret = ddrtrn_sw_training_if();
		break;
	case DDR_PCODE_TRAINING:
		ret = ddrtrn_pcode_training_func();
		break;
	default:
		return -1;
	}

	return ret;
}
