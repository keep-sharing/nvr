// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_LOW_FREQ_CONFIG

void ddrtrn_low_freq_cfg_freq(unsigned int base_phy)
{
	unsigned int dficlk_ratio;
	unsigned int crg_pll7_tmp;
	unsigned int crg_pll6_tmp;

	/* dramCK Clock Ratio */
	dficlk_ratio = ddrtrn_hal_low_freq_get_dficlk_ratio(base_phy);
	switch (dficlk_ratio) {
	case 0x0: /* 1:1 */
		crg_pll7_tmp = CRG_PLL7_TMP_1TO1; /* PLL 1600MHz */
		crg_pll6_tmp = CRG_PLL6_TMP_1TO1;
		break;
	case 0x1: /* 1:2 */
		crg_pll7_tmp = CRG_PLL7_TMP_1TO2; /* PLL 800MHz */
		crg_pll6_tmp = CRG_PLL6_TMP_1TO2;
		break;
	case 0x2: /* 1:4 */
	/* Fall through to PLL 400MHz */
	default:
		crg_pll7_tmp = CRG_PLL7_TMP_1TO4; /* PLL 400MHz */
		crg_pll6_tmp = CRG_PLL6_TMP_1TO4;
		break;
	}

	/* Configure DDR low frequency */
	ddrtrn_hal_low_freq_cfg_freq_process(crg_pll7_tmp, crg_pll6_tmp);
}

int ddrtrn_low_freq_start(struct ddrtrn_hal_training_custom_reg *reg)
{
	int result;
	unsigned int i;
	unsigned int crg_pll7_init_val, crg_pll6_init_val;

	struct ddrtrn_dmc_cfg_sref cfg_sref;
	struct ddrtrn_hal_training_dq_adj_all dq_adj_all;
	struct ddrtrn_delay_ckph delay_ck;

	ddrtr_set_data(&dq_adj_all, 0, sizeof(struct ddrtrn_hal_training_dq_adj_all));

	crg_pll7_init_val = ddrtrn_hal_low_freq_get_pll7();
	crg_pll6_init_val = ddrtrn_hal_low_freq_get_pll6();

	ddrtrn_hal_cfg_init();

	/* Configure DDR low frequency, does not distinguish PHY */
	ddrtrn_low_freq_cfg_freq(ddrtrn_hal_get_phy_addr(0));

	/* Perform partial initialization of DDR */
	ddrtrn_hal_hw_item_cfg(DDR_FROM_VALUE0);
	result = ddrtrn_hw_training();

	/* get value in low freq */
	ddrtrn_hal_freq_change_get_val_in_lowfreq(&delay_ck);
	ddrtrn_hal_get_wdqs_average_in_lowfreq(&delay_ck, &dq_adj_all);

	/* enter auto self-refresh */
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
			ddrtrn_sref_cfg(&cfg_sref, DMC_CFG_INIT_XSREF | DMC_CFG_SREF_PD); /* bit[3:2] 0x3 */

		if (ddrtrn_training_ctrl_easr(DDR_ENTER_SREF))
			return -1;
	}

	/* close phy clock gated */
	ddrtrn_hal_low_freq_phy_clk(PHY_CLK_GATED_CLOSE);

	/* DDR PLL power down */
	ddrtrn_hal_low_freq_pll_power(PHY_PLL_POWER_DOWN);

	/* Configure DDR high frequency */
	ddrtrn_hal_low_freq_cfg_freq_process(crg_pll7_init_val, crg_pll6_init_val);

	/* DDR PLL power up */
	ddrtrn_hal_low_freq_pll_power(PHY_PLL_POWER_UP);

	/* open phy clock gated */
	ddrtrn_hal_low_freq_phy_clk(PHY_CLK_GATED_OPEN);

	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++)
		ddrtrn_hal_ck_cfg(ddrtrn_hal_get_phy_addr(i));

	/* exit auto self-refresh */
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
			ddrtrn_sref_cfg(&cfg_sref, DMC_CFG_INIT_XSREF | DMC_CFG_SREF_PD); /* bit[3:2] 0x3 */

		if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
			return -1;
	}

	ddrtrn_hal_hw_item_cfg(PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_DLYMEAS_EN); /* bit[2][0] */
	result += ddrtrn_hw_training();
	/* get rdqscyc in high freq */
	ddrtrn_hal_freq_change_get_val_in_highfreq(&delay_ck);
	ddrtrn_hal_freq_change_set_wdqs_val(&delay_ck, &dq_adj_all);

	/* Perform partial initialization of DDR */
	ddrtrn_hal_hw_item_cfg(DDR_FROM_VALUE1);
	result += ddrtrn_hw_training();
	/* open rdqs anti-aging */

	return result;
}
#else
int ddrtrn_low_freq_start(struct ddrtrn_hal_training_custom_reg *custom_reg)
{
	return 0;
}
#endif /* DDR_LOW_FREQ_CONFIG */
