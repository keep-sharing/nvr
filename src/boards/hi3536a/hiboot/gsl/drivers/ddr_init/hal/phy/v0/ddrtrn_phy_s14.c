// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

void ddrtrn_hal_vref_get_host_max(unsigned int rank, unsigned int *val)
{
	if ((rank) == 0)
		(*val) = PHY_VRFTRES_HVREF_MASK;
	else
		(*val) = PHY_VRFTRES_RXDIFFCAL_MASK;
}

void ddrtrn_hal_vref_phy_host_get(unsigned long base_phy, unsigned int rank,
	unsigned int byte_index, unsigned int *val)
{
	if ((rank) == 0) {
		(*val) =
			ddrtrn_reg_read((base_phy) + ddrtrn_hal_phy_hvreft_status(rank, byte_index)) &
			PHY_VRFTRES_HVREF_MASK;
	} else {
		(*val) = (ddrtrn_reg_read((base_phy) +
			ddrtrn_hal_phy_hvreft_status(rank, byte_index)) >>
			PHY_VRFTRES_RXDIFFCAL_BIT) &
			PHY_VRFTRES_RXDIFFCAL_MASK;
	}
}

void ddrtrn_hal_vref_phy_dram_get(unsigned long base_phy, unsigned int *val, unsigned int byte_index)
{
	(*val) = ddrtrn_reg_read((base_phy) + ddrtrn_hal_phy_dvreft_status(byte_index)) &
		PHY_VRFTRES_DVREF_MASK;
}

/* PHY t28 DDR4 RDQS synchronize to RDM */
void ddrtrn_hal_phy_rdqs_sync_rdm(int val)
{
	ddrtrn_hal_rdqs_sync(val);
}
/* dqs swap */
void ddrtrn_hal_dqsswap_save_func(unsigned int *swapdfibyte_en, unsigned long base_phy)
{
	(*swapdfibyte_en) = ddrtrn_reg_read((base_phy) + DDR_PHY_DMSEL);
	ddrtrn_reg_write((*swapdfibyte_en) & PHY_DMSEL_SWAPDFIBYTE,
		(base_phy) + DDR_PHY_DMSEL);
}

void ddrtrn_hal_dqsswap_restore_func(unsigned int swapdfibyte_en, unsigned long base_phy)
{
	ddrtrn_reg_write(swapdfibyte_en, (base_phy) + DDR_PHY_DMSEL);
}

void ddrtrn_hal_phy_switch_rank(unsigned long base_phy, unsigned int val)
{
	ddrtrn_hal_switch_rank(base_phy, val);
}
