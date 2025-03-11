// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"
#include "training/ddrtrn_wakeup.h"

#ifdef DDR_WAKEUP_CONFIG
/* adjust wdqphse */
static void ddrtrn_wakeup_wdqphase_adjust(int offset)
{
	int wdqphase;
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int cur_mode = ddrtrn_hal_get_cur_mode();

	ddrtrn_hal_set_cur_mode(DDR_MODE_WRITE);
	if (byte_num > DDR_PHY_BYTE_MAX) {
		ddrtrn_error("byte num error, byte_num = %x", byte_num);
		return;
	}
	for (i = 0; i < byte_num; i++) {
		ddrtrn_hal_set_cur_byte(i);
		wdqphase = ddrtrn_hal_adjust_get_val();
		wdqphase += offset;
		wdqphase = (wdqphase < 0 ? 0 : wdqphase);
		wdqphase = (wdqphase > PHY_WDQ_PHASE_MASK ? PHY_WDQ_PHASE_MASK : wdqphase);
		ddrtrn_hal_adjust_set_val(wdqphase);
	}
	ddrtrn_hal_set_cur_mode(cur_mode); /* restore to current mode */
}

static int ddrtrn_wakeup_process(struct ddrtrn_dmc_cfg_sref *cfg_sref)
{
	int result = 0;
	unsigned int i, j;

	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return -1;

	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		/* exit powerdown */
		for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
			ddrtrn_hal_set_phy_id(i);
			ddrtrn_sref_cfg(cfg_sref, DMC_CFG_INIT_XSREF); /* bit[3:2] 0x2 */
		}
		/* RDET/WDET Method Selection */
		ddrtrn_hal_det_mod_sel();

		for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
			ddrtrn_hal_set_phy_id(i);
			ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));

			if (ddrtrn_hal_get_cur_phy_rank_num() > DDR_SUPPORT_RANK_MAX)
				return -1;

			/* adjust wdqphse */
			for (j = 0; j < ddrtrn_hal_get_cur_phy_rank_num(); j++) {
				ddrtrn_hal_set_rank_id(j);
				ddrtrn_wakeup_wdqphase_adjust(DDR_WDQPHASE_ADJUST_OFFSET);
			}

			ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
		}

		/* enable gt/rdet/wdet */
		ddrtrn_hal_hw_item_cfg(PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_GT_EN |
			PHY_PHYINITCTRL_RDET_EN | PHY_PHYINITCTRL_WDET_EN);
		result += ddrtrn_hw_training();
	} else {
		for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
			ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
			ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
		}
	}

	return result;
}

int ddrtrn_wakeup(void)
{
	int result;
	unsigned int i;

	struct ddrtrn_dmc_cfg_sref cfg_sref;

	ddrtrn_hal_cfg_init();

	/* enable PLL and dram reset */
	ddrtrn_hal_hw_item_cfg(PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_PLL_INIT_EN |
		PHY_PHYINITCTRL_DRAM_RST | PHY_PHYINITCTRL_DLYMEAS_EN);
	result = ddrtrn_hw_training();

	/* restore ddrc register */
	ddrtrn_hal_wakeup_restore();
	ddrtrn_hal_wakeup_lp_en_ctrl();
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return -1;

	if (ddrtrn_wakeup_process(&cfg_sref))
		return -1;

	/* exit auto self-refresh */
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
			return -1;

		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
			ddrtrn_sref_cfg(&cfg_sref, DMC_CFG_INIT_XSREF | DMC_CFG_SREF_PD); /* bit[3:2] 0x3 */
		else
			ddrtrn_sref_cfg(&cfg_sref, DMC_CFG_INIT_XSREF); /* bit[3:2] 0x2 */
	}

	return result;
}
#else
int ddrtrn_wakeup(void)
{
	return 0;
}
#endif /* DDR_WAKEUP_CONFIG */
