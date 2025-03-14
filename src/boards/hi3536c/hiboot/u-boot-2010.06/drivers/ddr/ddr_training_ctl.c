/**
 * Copyright (c) 2009-2014 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * ddr_training_ctl.c
 *
 * DDR training control.
 */
#include <ddr_interface.h>
#include "ddr_training_impl.h"

#ifdef DDR_SW_TRAINING_FUNC_PUBLIC
#ifdef DDR_TRAINING_CUT_CODE_CONFIG
/**
 * ddr_sw_training_func
 * @ddrtr_result
 *
 * Cut ddr training control code for less SRAM.
 * Support DDRC500.
 * Support DDRC510 with one PHY.
 */
int ddr_sw_training_func(void *ddrtr_result)
{
	unsigned int base_dmc = DDR_REG_BASE_DMC0;
	unsigned int base_phy = DDR_REG_BASE_PHY0;
	int result = 0;
	unsigned int auto_ref_timing = REG_READ(base_dmc + DDR_DMC_TIMING2);
	unsigned int misc_scramb = REG_READ(base_phy + DDR_PHY_MISC);
	unsigned int dramcfg_ma2t = REG_READ(base_phy + DDR_PHY_DRAMCFG)
		& PHY_DRAMCFG_MA2T;
	unsigned int acphyctl = REG_READ(base_phy + DDR_PHY_ACPHYCTL4);

	DDR_VARIABLE_DECLARE(swapdfibyte_en);

	/* check sw ddr training enable */
	if (DDR_BYPASS_ALL_MASK == REG_READ(DDR_REG_BASE_SYSCTRL
		+ SYSCTRL_DDR_TRAINING_CFG))
		return 0;

	ddr_training_start();

#ifdef DDR_TRAINING_STAT_CONFIG
	/* clear stat register */
	REG_WRITE(0x0, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_STAT);
#endif

	/* disable scramb */
	REG_WRITE(misc_scramb & PHY_MISC_SCRAMB_DIS,
		base_phy + DDR_PHY_MISC);

	/* disable rdqs swap */
	DDR_DQSSWAP_SAVE_FUNC(swapdfibyte_en, base_phy);

	/* check hardware gating */
	if (REG_READ(base_phy + DDR_PHY_PHYINITSTATUS)
		& PHY_INITSTATUS_GT_MASK) {
		DDR_FATAL("PHY[%x] hw gating fail.", base_phy);
		ddr_training_stat(DDR_ERR_HW_GATING,
			base_phy, -1, -1);
	}

#ifdef DDR_LPCA_TRAINING_CONFIG
	/* lpca */
	if (!ddr_training_check_bypass(DDR_BYPASS_LPCA_MASK)
		&& (PHY_DRAMCFG_TYPE_LPDDR3 ==
		(REG_READ(base_phy + DDR_PHY_DRAMCFG)
		& PHY_DRAMCFG_TYPE_LPDDR3))) {
		/* disable auto refresh */
		ddr_training_set_timing(base_dmc,
			auto_ref_timing & DMC_AUTO_TIMING_DIS);

		result += ddr_lpca_training(base_dmc, base_phy,
			ddrtr_result);

		/* enable auto refresh */
		ddr_training_set_timing(base_dmc, auto_ref_timing);
	}
#endif

#ifdef DDR_WL_TRAINING_CONFIG
	/* write leveling */
	if (!ddr_training_check_bypass(DDR_BYPASS_WL_MASK)) {
		/* disable auto refresh */
		ddr_training_set_timing(base_dmc,
			auto_ref_timing & DMC_AUTO_TIMING_DIS);
		result += ddr_write_leveling(base_dmc, base_phy);
		/* enable auto refresh */
		ddr_training_set_timing(base_dmc, auto_ref_timing);
	}
#endif

#ifdef DDR_DATAEYE_TRAINING_CONFIG
	/* dataeye */
	if (!ddr_training_check_bypass(DDR_BYPASS_DATAEYE_MASK)) {
		ddr_ddrt_init(base_dmc, DDR_DDRT_MODE_DATAEYE);
		result += ddr_dataeye_training(base_dmc, base_phy,
			ddrtr_result, DDR_DATAEYE_NORMAL_ADJUST);
	}
#endif

#ifdef DDR_HW_TRAINING_CONFIG
	/* hardware read */
	if (result && !ddr_training_check_bypass(DDR_BYPASS_HW_MASK)) {
		if (!dramcfg_ma2t) /* set 1T */
			REG_WRITE(0x0, base_phy + DDR_PHY_ACPHYCTL4);

		result = ddr_hw_training(base_dmc, base_phy);
		if (!dramcfg_ma2t) /* restore */
			REG_WRITE(acphyctl, base_phy + DDR_PHY_ACPHYCTL4);

		result += ddr_dataeye_training(base_dmc, base_phy,
			ddrtr_result, DDR_DATAEYE_ABNORMAL_ADJUST);
	}
#endif

#ifdef DDR_MPR_TRAINING_CONFIG
	/* mpr */
	if (result && !ddr_training_check_bypass(DDR_BYPASS_MPR_MASK)) {
		result = ddr_mpr_training(base_dmc, base_phy);
		result += ddr_dataeye_training(base_dmc, base_phy,
			ddrtr_result, DDR_DATAEYE_ABNORMAL_ADJUST);
	}
#endif

#ifdef DDR_GATE_TRAINING_CONFIG
	/* gate */
	if (!ddr_training_check_bypass(DDR_BYPASS_GATE_MASK)) {
		ddr_ddrt_init(base_dmc, DDR_DDRT_MODE_GATE);
		/* disable auto refresh */
		ddr_training_set_timing(base_dmc,
			auto_ref_timing & DMC_AUTO_TIMING_DIS);

		if (!dramcfg_ma2t) /* set 1T */
			REG_WRITE(0x0, base_phy + DDR_PHY_ACPHYCTL4);

		result += ddr_gate_training(base_dmc, base_phy);

		/* enable auto refresh */
		ddr_training_set_timing(base_dmc, auto_ref_timing);

		if (!dramcfg_ma2t) /* restore */
			REG_WRITE(acphyctl, base_phy + DDR_PHY_ACPHYCTL4);
	}
#endif

#ifdef DDR_VREF_TRAINING_CONFIG
	if (!ddr_training_check_bypass(DDR_BYPASS_VREF_MASK)) {
		ddr_ddrt_init(base_dmc, DDR_DDRT_MODE_DATAEYE);
		result += ddr_vref_training(base_dmc, base_phy,
			ddrtr_result);
	}
#endif

	/* restore scramb */
	REG_WRITE(misc_scramb, base_phy + DDR_PHY_MISC);

	/* restore rdqs swap */
	DDR_DQSSWAP_RESTORE_FUNC(swapdfibyte_en, base_phy);

	if (!result)
		ddr_training_suc();
	return result;
}
#else
/**
 * ddr_sw_training_func
 * @ddrtr_result
 *
 * Support DDRC510 with two PHY.
 */
int ddr_sw_training_func(void *ddrtr_result)
{
	unsigned int base_dmc, base_phy;
	struct tr_relate_reg relate_reg;
#if defined(DDR_WL_TRAINING_CONFIG) || defined(DDR_GATE_TRAINING_CONFIG) \
	|| defined(DDR_LPCA_TRAINING_CONFIG)
	struct tr_relate_reg relate_reg_timing;
#endif
	int result = 0;
	int i;

	/* check sw ddr training enable */
	if (DDR_BYPASS_ALL_MASK == REG_READ(DDR_REG_BASE_SYSCTRL
		+ SYSCTRL_DDR_TRAINING_CFG))
		return 0;

	ddr_training_start();

#ifdef DDR_TRAINING_STAT_CONFIG
	/* clear stat register */
	REG_WRITE(0x0, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_STAT);
#endif

	ddr_training_save_reg(&relate_reg, 0);

	for (i = 0; i < DDR_PHY_NUM; i++) {
		if (ddr_training_phy_disable(i))
			continue;

		ddr_training_get_base(i, &base_dmc, &base_phy);

		/* check hardware gating */
		if (REG_READ(base_phy + DDR_PHY_PHYINITSTATUS)
			& PHY_INITSTATUS_GT_MASK) {
			DDR_FATAL("PHY[%x] hw gating fail.", base_phy);
			ddr_training_stat(DDR_ERR_HW_GATING,
				base_phy, -1, -1);
		}

#ifdef DDR_LPCA_TRAINING_CONFIG
		/* lpca */
		if (!ddr_training_check_bypass(DDR_BYPASS_LPCA_MASK)
			&& (PHY_DRAMCFG_TYPE_LPDDR3 ==
			(REG_READ(base_phy + DDR_PHY_DRAMCFG)
			& PHY_DRAMCFG_TYPE_LPDDR3))) {
			ddr_training_save_reg(&relate_reg_timing,
				DDR_BYPASS_LPCA_MASK);
			result += ddr_lpca_training(base_dmc, base_phy,
				ddrtr_result);
			ddr_training_restore_reg(&relate_reg_timing);
		}
#endif

#ifdef DDR_WL_TRAINING_CONFIG
		/* write leveling */
		if (!ddr_training_check_bypass(DDR_BYPASS_WL_MASK)) {
			ddr_training_save_reg(&relate_reg_timing,
				DDR_BYPASS_WL_MASK);
			result += ddr_write_leveling(base_dmc, base_phy);
			ddr_training_restore_reg(&relate_reg_timing);
		}
#endif
		/* dataeye/gate/vref need switch axi */
		ddr_training_switch_axi(i, &relate_reg);

#ifdef DDR_DATAEYE_TRAINING_CONFIG
		/* dataeye */
		if (!ddr_training_check_bypass(DDR_BYPASS_DATAEYE_MASK)) {
			ddr_ddrt_init(base_dmc, DDR_DDRT_MODE_DATAEYE);
			result += ddr_dataeye_training(base_dmc, base_phy,
				ddrtr_result, DDR_DATAEYE_NORMAL_ADJUST);
		}
#endif

#ifdef DDR_HW_TRAINING_CONFIG
		/* hardware read */
		if (result && !ddr_training_check_bypass(DDR_BYPASS_HW_MASK)) {
			struct tr_relate_reg relate_reg_ac;
			ddr_training_save_reg(&relate_reg_ac,
				DDR_BYPASS_HW_MASK);
			result = ddr_hw_training(base_dmc, base_phy);
			ddr_training_restore_reg(&relate_reg_ac);
			result += ddr_dataeye_training(base_dmc, base_phy,
				ddrtr_result, DDR_DATAEYE_ABNORMAL_ADJUST);
		}
#endif

#ifdef DDR_MPR_TRAINING_CONFIG
		/* mpr */
		if (result && !ddr_training_check_bypass(DDR_BYPASS_MPR_MASK)) {
			result = ddr_mpr_training(base_dmc, base_phy);
			result += ddr_dataeye_training(base_dmc, base_phy,
				ddrtr_result, DDR_DATAEYE_ABNORMAL_ADJUST);
		}
#endif

#ifdef DDR_GATE_TRAINING_CONFIG
		/* gate */
		if (!ddr_training_check_bypass(DDR_BYPASS_GATE_MASK)) {
			ddr_training_save_reg(&relate_reg_timing,
				DDR_BYPASS_GATE_MASK);
			ddr_ddrt_init(base_dmc, DDR_DDRT_MODE_GATE);
			result += ddr_gate_training(base_dmc, base_phy);
			ddr_training_restore_reg(&relate_reg_timing);
		}
#endif

#ifdef DDR_VREF_TRAINING_CONFIG
		ddr_ddrt_init(base_dmc, DDR_DDRT_MODE_DATAEYE);
		if (!ddr_training_check_bypass(DDR_BYPASS_VREF_HOST_MASK)) {
			result += ddr_vref_training(base_dmc, base_phy,
				ddrtr_result, DDR_MODE_READ);
		}

#ifdef DDR_PHY_T28_CONFIG
		/* dram vref training enable && DDR4 */
		if (!ddr_training_check_bypass(DDR_BYPASS_VREF_DRAM_MASK)
			&& DMC_CFG_DRAM_TYPE_DDR4 ==
				(REG_READ(base_dmc + DDR_DMC_CFG_DDRMODE)
					& DMC_CFG_DRAM_TYPE_MASK)) {
			result += ddr_vref_training(base_dmc, base_phy,
				ddrtr_result, DDR_MODE_WRITE);
		}
#endif /* DDR_PHY_T28_CONFIG */
#endif /* DDR_VREF_TRAINING_CONFIG */
	}

	ddr_training_restore_reg(&relate_reg);

	if (!result)
		ddr_training_suc();
	return result;
}
#endif /* DDR_TRAINING_CUT_CODE_CONFIG */
#endif /* DDR_SW_TRAINING_FUNC_PUBLIC */

/**
 * ddr_sw_training_if
 * @ddrtr_result
 *
 *
 */
int ddr_sw_training_if(void *ddrtr_result)
{
	return DDR_SW_TRAINING_FUNC(ddrtr_result);
}
