// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_DATAEYE_TRAINING_CONFIG
void ddrtrn_hal_dataeye_set_dq_sum(unsigned int dq_sum)
{
	unsigned int byte_index = ddrtrn_hal_get_cur_byte();

	ddrtrn_reg_write((dq_sum & PHY_BDL_MASK) << PHY_WDM_BDL_BIT,
		ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqnbdl2(ddrtrn_hal_get_rank_id(), byte_index));
}

unsigned int ddrtrn_hal_get_dq_type(void)
{
	return ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_DQ_TYPE);
}

void ddrtrn_hal_set_dq_type(unsigned int dq_check_type)
{
	ddrtrn_reg_write(dq_check_type, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_DQ_TYPE);
}
#endif /* DDR_DATAEYE_TRAINING_CONFIG */
