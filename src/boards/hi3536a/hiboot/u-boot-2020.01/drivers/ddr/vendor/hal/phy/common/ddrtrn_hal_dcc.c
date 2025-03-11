// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"
#include "hal/ddrtrn_hal_context.h"

/* s40/t28/t16 not support dcc training */
#ifdef DDR_DCC_TRAINING_CONFIG
/* Save two rank RDET result */
void ddrtrn_hal_dcc_save_two_rank_bdl(struct ddrtrn_hal_trianing_dq_data *rank_dcc_data)
{
	unsigned int byte_idx;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();

	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		rank_dcc_data->dq03[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank_idx, byte_idx));
		rank_dcc_data->dq47[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank_idx, byte_idx));
		rank_dcc_data->rdm[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl2(rank_idx, byte_idx));
		rank_dcc_data->rdqs[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx));

		ddrtrn_debug("rank[%x] dq03[%x] dq47[%x] rdm[%x] rdqs[%x]", rank_idx,
			rank_dcc_data->dq03[byte_idx],
			rank_dcc_data->dq47[byte_idx],
			rank_dcc_data->rdm[byte_idx],
			rank_dcc_data->rdqs[byte_idx]);
	}
}

/* Restore two rank RDET result */
void ddrtrn_hal_dcc_restore_two_rank_bdl(struct ddrtrn_hal_trianing_dq_data *rank_dcc_data)
{
	unsigned int byte_idx;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();

	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		ddrtrn_reg_write(rank_dcc_data->dq03[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank_idx, byte_idx));
		ddrtrn_reg_write(rank_dcc_data->dq47[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank_idx, byte_idx));
		ddrtrn_reg_write(rank_dcc_data->rdm[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl2(rank_idx, byte_idx));
		ddrtrn_reg_write(rank_dcc_data->rdqs[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx));
	}
}

unsigned int ddrtrn_hal_dcc_get_ioctl21(void)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_ACIOCTL21);
}

void ddrtrn_hal_dcc_set_ioctl21(unsigned int ioctl21)
{
	ddrtrn_reg_write(ioctl21, ddrtrn_hal_get_cur_phy() + DDR_PHY_ACIOCTL21);
}

unsigned int ddrtrn_hal_dcc_get_gated_bypass(void)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_AC_GATED_BYPASS);
}

void ddrtrn_hal_dcc_set_gated_bypass(unsigned int gated_bypass_temp)
{
	ddrtrn_reg_write(gated_bypass_temp, ddrtrn_hal_get_cur_phy() + DDR_PHY_AC_GATED_BYPASS);
}

void ddrtrn_hal_dcc_rdet_enable(void)
{
	ddrtrn_reg_write(PHY_PHYINITCTRL_RDET_EN,
		ddrtrn_hal_get_cur_phy() + DDR_PHY_PHYINITSTATUS);
}

unsigned int ddrtrn_hal_dcc_get_dxnrdbound(unsigned int byte_index)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdbound(byte_index));
}
#endif /* DDR_DCC_TRAINING_CONFIG */
