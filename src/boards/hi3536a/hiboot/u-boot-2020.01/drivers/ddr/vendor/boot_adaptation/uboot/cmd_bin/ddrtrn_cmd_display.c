// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

void ddrtrn_hal_phy_wdqs_display_cmd(unsigned int base_phy,
	unsigned int rank, unsigned int byte_num)
{
	unsigned int i;

	for (i = 0; i < byte_num; i++)
		ddrtrn_info("[%x = %x] WDQS Byte(%x) ",
			base_phy + ddrtrn_hal_phy_dxwdqsdly(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(rank, i)), i);
}

void ddrtrn_hal_phy_wdqphase_display_cmd(unsigned int base_phy,
	unsigned int rank, unsigned int byte_num)
{
	unsigned int i;

	for (i = 0; i < byte_num; i++)
		ddrtrn_info("[%x = %x] WDQ Phase Byte(%x)",
			base_phy + ddrtrn_hal_phy_dxnwdqdly(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(rank, i)), i);
}

void ddrtrn_hal_phy_wdqbdl_display_cmd(unsigned int base_phy,
	unsigned int rank, unsigned int byte_num)
{
	unsigned int i;

	for (i = 0; i < byte_num; i++) {
		/* DQ0-DQ3 */
		ddrtrn_info("[%x = %x] WDQ BDL DQ(%x-%x)",
			base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank, i)),
			(i << 3), ((i << 3) + 3)); /* DQ0-DQ3, Shift left 3:8 dq */

		/* DQ4-DQ7 */
		ddrtrn_info("[%x = %x] WDQ BDL DQ(%x-%x)",
			base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank, i)),
			((i << 3) + 4), ((i << 3) + 7)); /* DQ4-DQ7, Shift left 3:8 dq */
	}
}

void ddrtrn_hal_phy_wdm_display_cmd(unsigned int base_phy,
	unsigned int rank, unsigned int byte_num)
{
	unsigned int i;

	for (i = 0; i < byte_num; i++)
		ddrtrn_info("[%x = %x] WDM Byte(%x)",
			base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank, i)), i);
}

void ddrtrn_hal_phy_wdq_wdqs_oe_display_cmd(unsigned int base_phy,
	unsigned int rank, unsigned int byte_num)
{
	unsigned int i;

	for (i = 0; i < byte_num; i++)
		ddrtrn_info("[%x = %x] Write DQ/DQS OE Byte(%x)",
			base_phy + ddrtrn_hal_phy_dxnoebdl(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnoebdl(rank, i)), i);
}

void ddrtrn_hal_phy_rdqs_display_cmd(unsigned int base_phy, unsigned int byte_num)
{
	unsigned int i;

	for (i = 0; i < byte_num; i++)
		ddrtrn_info("[%x = %x] RDQS Byte(%x)", base_phy + ddrtrn_hal_phy_dxnrdqsdly(i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(i)), i);
}

void ddrtrn_hal_phy_rdqbdl_display_cmd(unsigned int base_phy,
	unsigned int rank, unsigned int byte_num)
{
	unsigned int i;

	for (i = 0; i < byte_num; i++) {
		/* DQ0-DQ3 */
		ddrtrn_info("[%x = %x] RDQ BDL DQ(%x-%x)",
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank, i)),
			(i << 3), ((i << 3) + 3)); /* DQ0-DQ3, Shift left 3:8 dq */

		/* DQ4-DQ7 */
		ddrtrn_info("[%x = %x] RDQ BDL DQ(%x-%x)",
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank, i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank, i)),
			((i << 3) + 4), ((i << 3) + 7)); /* DQ4-DQ7, Shift left 3:8 dq */
	}
}

void ddrtrn_hal_phy_gate_display_cmd(unsigned int base_phy,
	unsigned int rank, unsigned int byte_num)
{
	unsigned int _i;

	for (_i = 0; _i < byte_num; _i++)
		ddrtrn_info("[%x = %x] Gate Byte(%x)",
			base_phy + ddrtrn_hal_phy_dxnrdqsgdly(rank, _i),
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsgdly(rank, _i)), _i);
}

void ddrtrn_hal_phy_cs_display_cmd(unsigned int base_phy)
{
	ddrtrn_info("[%x = %x] CS", base_phy + DDR_PHY_ACCMDBDL2,
		ddrtrn_reg_read(base_phy + DDR_PHY_ACCMDBDL2));
}

void ddrtrn_hal_phy_clk_display_cmd(unsigned int base_phy, unsigned int acphyctl7)
{
	ddrtrn_info("[%x = %x] CLK", base_phy + DDR_PHY_ACPHYCTL7, acphyctl7);
}

void ddrtrn_hal_phy_vref_host_display_cmd(__attribute__((unused)) unsigned long base_phy,
	__attribute__((unused)) unsigned int rank, unsigned int byte_num)
{
	unsigned int _i;

	for (_i = 0; _i < (byte_num); _i++)
		ddrtrn_info("[%x = %x] Host Vref Byte(%x)",
			(base_phy) + ddrtrn_hal_phy_hvreft_status(rank, _i),
			ddrtrn_reg_read((base_phy) + ddrtrn_hal_phy_hvreft_status(rank, _i)), _i);
}

void ddrtrn_hal_phy_vref_dram_display_cmd(__attribute__((unused)) unsigned long base_phy,
	unsigned int byte_num)
{
	unsigned int _i;

	for (_i = 0; _i < (byte_num); _i++)
		ddrtrn_info("[%x = %x] DRAM Vref Byte(%x)",
			(base_phy) + ddrtrn_hal_phy_dvreft_status(_i),
			ddrtrn_reg_read((base_phy) + ddrtrn_hal_phy_dvreft_status(_i)), _i);
}

void ddrtrn_hal_phy_dx_dpmc_display_cmd(__attribute__((unused)) unsigned long base_phy,
	unsigned int byte_num)
{
	unsigned int _i;
	for (_i = 0; _i < (byte_num); _i++)
		ddrtrn_info("[%x = %x] Dpmc Byte(%x)",
			(base_phy) + dx_dxnmiscctrl3(_i),
			ddrtrn_reg_read((base_phy) + dx_dxnmiscctrl3(_i)), _i);
}

void ddrtrn_hal_phy_dcc_display_cmd(unsigned long base_phy)
{
	__attribute__((unused)) unsigned int val;
	val = ddrtrn_reg_read((base_phy) + DDR_PHY_ACIOCTL21);
	val = ddrtrn_reg_read((base_phy) + DDR_PHY_ACIOCTL21);
	ddrtrn_info("[%x = %x] DCC duty", (base_phy) + DDR_PHY_ACIOCTL21, val);
}

void ddrtrn_hal_phy_addrph_display_cmd(__attribute__((unused)) unsigned long base_phy)
{
	ddrtrn_info("[%x = %x] CA Phase", (base_phy) + DDR_PHY_ADDRPHBOUND,
		ddrtrn_reg_read((base_phy) + DDR_PHY_ADDRPHBOUND));
}

void ddrtrn_hal_phy_addrbdl_display_cmd(__attribute__((unused)) unsigned long base_phy)
{
	unsigned int _i;

	for (_i = 0; _i < DDR_PHY_CA_REG_MAX; _i++)
		ddrtrn_info("[%x = %x] ACADDRBDL(%x)", (base_phy) + ddr_phy_acaddrbdl(_i),
			ddrtrn_reg_read((base_phy) + ddr_phy_acaddrbdl(_i)), _i);
}