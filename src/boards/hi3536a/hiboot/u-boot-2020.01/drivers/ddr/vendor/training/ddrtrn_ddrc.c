// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#if defined(DDR_HW_TRAINING_CONFIG) || defined(DDR_DCC_TRAINING_CONFIG)
/* DDR hw/dcc training exit or enter auto self-refresh */
int ddrtrn_training_ctrl_easr(unsigned int sref_req)
{
	int result = 0;
	unsigned int i;

	if (ddrtrn_hal_get_cur_phy_dmc_num() > DDR_DMC_PER_PHY_MAX) {
		ddrtrn_error("loop upper limit cfg->dmc_num out of range");
		return -1;
	}
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		result += ddrtrn_hal_ddrc_easr(ddrtrn_hal_get_cur_dmc_addr(i), sref_req);

	return result;
}

void ddrtrn_training_restore_timing(struct ddrtrn_hal_timing *timing)
{
	unsigned int i;

	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		ddrtrn_hal_set_timing(ddrtrn_hal_get_cur_dmc_addr(i), timing->val[i]);
}
#endif /* DDR_HW_TRAINING_CONFIG ||  DDR_DCC_TRAINING_CONFIG */