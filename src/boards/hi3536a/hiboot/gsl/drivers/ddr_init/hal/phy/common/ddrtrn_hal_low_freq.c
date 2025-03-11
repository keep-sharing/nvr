// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_LOW_FREQ_CONFIG
unsigned int ddrtrn_hal_low_freq_get_dficlk_ratio(unsigned int base_phy)
{
	return (ddrtrn_reg_read(base_phy + DDR_PHY_PHYCTRL0) >> PHY_DFICLK_RATIO_BIT) &
			PHY_DFICLK_RATIO_MASK;
}

unsigned int ddrtrn_hal_low_freq_get_pll7(void)
{
	return ddrtrn_reg_read(CRG_REG_BASE_ADDR + PERI_CRG_PLL7);
}

unsigned int ddrtrn_hal_low_freq_get_pll6(void)
{
	return ddrtrn_reg_read(CRG_REG_BASE_ADDR + PERI_CRG_PLL6);
}

void ddrtrn_hal_low_freq_phy_clk(unsigned int clk_status)
{
	unsigned int gated;
	unsigned int base_phy;
	unsigned int phy_idx;
	/* bit[31], bit[14:0] */
	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		if (ddrtrn_hal_get_rank_item_hw(phy_idx, 0) == 0)
			continue;

		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);
		gated = ddrtrn_reg_read(base_phy + DDR_PHY_CLKGATED);
		ddrtrn_reg_write((gated & (~PHY_CLKGATED_MASK)) | clk_status,
			base_phy + DDR_PHY_CLKGATED);
	}
}

/* config DDR PLL power down or power up */
void ddrtrn_hal_low_freq_pll_power(unsigned int pll_status)
{
	unsigned int pll_ctrl;
	unsigned int base_phy;
	unsigned int phy_idx;

	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		if (ddrtrn_hal_get_rank_item_hw(phy_idx, 0) == 0)
			continue;

		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);
		pll_ctrl = ddrtrn_reg_read(base_phy + DDR_PHY_PLLCTRL);
		ddrtrn_reg_write((pll_ctrl & (~(PHY_PLL_PWDN_MASK << PHY_PLL_PWDN_BIT))) |
			(pll_status << PHY_PLL_PWDN_BIT), base_phy + DDR_PHY_PLLCTRL);
	}

	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_100US); /* wait 100us */
}

void ddrtrn_hal_get_ck_phase(unsigned int base_phy, struct ddrtrn_ck_phase *ck_phase)
{
	unsigned int acphyctl7;

	acphyctl7 = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL7);
	ck_phase->ck_phase[0] = ((acphyctl7 >> PHY_ACPHY_DCLK0_BIT) & PHY_ACPHY_DCLK_MASK) |
		(((acphyctl7 >> PHY_ACPHY_DRAMCLK0_BIT) & PHY_ACPHY_DRAMCLK_MASK) << PHY_ACPHY_DRAMCLK_EXT_BIT);
	ck_phase->ck_phase[1] = ((acphyctl7 >> PHY_ACPHY_DCLK1_BIT) & PHY_ACPHY_DCLK_MASK) |
		(((acphyctl7 >> PHY_ACPHY_DRAMCLK1_BIT) & PHY_ACPHY_DRAMCLK_MASK) << PHY_ACPHY_DRAMCLK_EXT_BIT);
}

void ddrtrn_hal_clear_wdqs_register(unsigned int base_phy, unsigned int rank_idx,
	unsigned int byte_idx)
{
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx));

	ddrtrn_hal_phy_cfg_update(base_phy);
}

void ddrtrn_hal_get_wdqs_average_in_lowfreq_by_rank(const struct ddrtrn_delay_ckph *delay_ck,
	struct ddrtrn_hal_training_dq_adj_all *dq_adj_all, int cycle)
{
	int wdqsphase;
	unsigned int byte_idx;
	unsigned int wdqsdly_lowfreq;
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int cur_phy = ddrtrn_hal_get_cur_phy();
	struct ddrtrn_hal_training_dq_adj *byte_dq_st = NULL;

	if (ddrtrn_hal_get_cur_phy_total_byte_num() > DDR_PHY_BYTE_MAX)
		return;

	for (byte_idx = 0; byte_idx < ddrtrn_hal_get_cur_phy_total_byte_num(); byte_idx++) {
		byte_dq_st = &dq_adj_all->phy_dq_adj[phy_idx].rank_dq_adj[rank_idx].byte_dq_adj[byte_idx];

		ddrtrn_hal_get_dly_value(byte_dq_st, cur_phy, rank_idx, byte_idx);
		wdqsphase = (int)(byte_dq_st->wdqsphase);
		wdqsphase = wdqsphase - (wdqsphase + 1) / 4; /* 4 wdqphase */

		if ((wdqsphase > PHY_WDQS_PHASE_MASK) || (byte_dq_st->wlsl > PHY_WLSL_MASK))
			return;

		if (delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx] > PHY_RDQS_CYC_MASK)
			return;

		if (delay_ck->tr_delay_ck[phy_idx].phase2bdl_lowfreq[byte_idx] >
			(PHY_RDQS_CYC_MASK / PHY_WDQSPHASE_NUM_T))
			return;

		wdqsdly_lowfreq = (unsigned int)(wdqsphase * (int)(delay_ck->tr_delay_ck[phy_idx].phase2bdl_lowfreq[byte_idx]) +
			(int)(byte_dq_st->wdqsbdl) + ((int)(byte_dq_st->wlsl) - 1) *
			(int)(delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx]));

		byte_dq_st->wdqs_sum += wdqsdly_lowfreq;

		/* get wdqs average */
		if (cycle == (WL_TIME - 1))
			byte_dq_st->wdqs_average = byte_dq_st->wdqs_sum / WL_TIME;

		/* set wdq register value to 0 */
		ddrtrn_hal_clear_wdqs_register(cur_phy, rank_idx, byte_idx);
	}
}

void ddrtrn_hal_get_wdqs_average_in_lowfreq_by_phy(const struct ddrtrn_delay_ckph *delay_ck,
	struct ddrtrn_hal_training_dq_adj_all *dq_adj_all, int cycle)
{
	unsigned int rank_idx;

	for (rank_idx = 0; rank_idx < ddrtrn_hal_get_cur_phy_rank_num(); rank_idx++) {
		ddrtrn_hal_set_rank_id(rank_idx);

		ddrtrn_hal_get_wdqs_average_in_lowfreq_by_rank(delay_ck, dq_adj_all, cycle);
	}
}

int ddrtrn_hal_get_wdqs_average_in_lowfreq(const struct ddrtrn_delay_ckph *delay_ck,
	struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	int result = 0;
	int i;
	unsigned int phy_idx;

	for (i = 0; i < WL_TIME; i++) {
		ddrtrn_hal_hw_item_cfg(PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_WL_EN); /* bit[4] bit[0] */
		result += ddrtrn_hw_training();

		for (phy_idx = 0; phy_idx < DDR_PHY_NUM; phy_idx++) {
			ddrtrn_hal_set_phy_id(phy_idx);
			ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_cur_phy_addr());
			if (ddrtrn_hal_get_rank_item_hw(phy_idx, 0) == 0)
				continue;

			ddrtrn_hal_get_wdqs_average_in_lowfreq_by_phy(delay_ck, dq_adj_all, i);
		}
	}

	return result;
}

/* get ck_phase and rdqscyc in low freq */
void ddrtrn_hal_freq_change_get_val_in_lowfreq(struct ddrtrn_delay_ckph *delay_ck)
{
	unsigned int phy_idx, byte_idx;
	unsigned int base_phy;

	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return;

	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);

		ddrtrn_hal_get_ck_phase(base_phy, &(delay_ck->tr_delay_ck[phy_idx].ckph_st[0])); /* 0: in low freq */

		if (ddrtrn_hal_get_phy_total_byte_num(phy_idx) > DDR_PHY_BYTE_MAX)
			return;

		for (byte_idx = 0; byte_idx < ddrtrn_hal_get_phy_total_byte_num(phy_idx); byte_idx++) {
			delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx] =
				(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >>
				PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK;
			delay_ck->tr_delay_ck[phy_idx].phase2bdl_lowfreq[byte_idx] =
				delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx] / PHY_WDQSPHASE_NUM_T;
		}
	}
}

void ddrtrn_hal_freq_change_get_val_in_highfreq(struct ddrtrn_delay_ckph *delay_ck)
{
	unsigned int phy_idx, byte_idx;
	unsigned int base_phy;

	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return;

	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);

		ddrtrn_hal_get_ck_phase(base_phy, &(delay_ck->tr_delay_ck[phy_idx].ckph_st[1])); /* 1: in high freq */

		if (ddrtrn_hal_get_phy_total_byte_num(phy_idx) > DDR_PHY_BYTE_MAX)
			return;

		for (byte_idx = 0; byte_idx < ddrtrn_hal_get_phy_total_byte_num(phy_idx); byte_idx++) {
			delay_ck->tr_delay_ck[phy_idx].rdqscyc_highfreq[byte_idx] =
				(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >>
				PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK;
			delay_ck->tr_delay_ck[phy_idx].phase2bdl_highfreq[byte_idx] =
				delay_ck->tr_delay_ck[phy_idx].rdqscyc_highfreq[byte_idx] / PHY_WDQSPHASE_NUM_T;
		}
	}
}

void ddrtrn_hal_freq_change_set_wdqs_val_process(const struct ddrtrn_delay_ckph *delay_ck,
	const struct ddrtrn_hal_training_dq_adj *byte_dq_st, unsigned int wlsl, int wdqsdly_highfreq)
{
	unsigned int wdqsdly;
	unsigned int wdqsphase;
	unsigned int wdqsbdl;
	unsigned int wdqphase;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int byte_idx = ddrtrn_hal_get_cur_byte();

	wdqsphase = 0;
	wdqsdly = byte_dq_st->wdqsdly;
	wdqsbdl = (unsigned int)wdqsdly_highfreq;

	while ((wdqsbdl > delay_ck->tr_delay_ck[ddrtrn_hal_get_phy_id()].phase2bdl_highfreq[byte_idx]) &&
		(wdqsphase < (PHY_WDQSPHASE_REG_MAX - 8))) { /* 8: 1/2T */
		wdqsbdl = wdqsbdl - delay_ck->tr_delay_ck[ddrtrn_hal_get_phy_id()].phase2bdl_highfreq[byte_idx];
		wdqsphase++;
		/* if bit[1:0] value is 0x3, avoid phase cavity */
		if ((wdqsphase & 0x3) == 0x3) /* 0x3:bit[1:0] */
			wdqsphase++;
	}

	wdqphase = 0;
	wdqsdly = (wdqsdly & (~(PHY_WDQS_BDL_MASK << PHY_WDQS_BDL_BIT))) | (wdqsbdl << PHY_WDQS_BDL_BIT);
	wdqsdly = (wdqsdly & (~(PHY_WDQS_PHASE_MASK << PHY_WDQS_PHASE_BIT))) |
		(wdqsphase << PHY_WDQS_PHASE_BIT);
	ddrtrn_reg_write(wdqsdly, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_idx, byte_idx));
	ddrtrn_reg_write((byte_dq_st->dxnwlsl & (~(PHY_WLSL_MASK << PHY_WLSL_BIT))) | (wlsl << PHY_WLSL_BIT),
		base_phy + ddrtrn_hal_phy_dxnwlsl(rank_idx, byte_idx));
	ddrtrn_reg_write((byte_dq_st->wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) |
		(wdqphase << PHY_WDQ_PHASE_BIT),
		base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx));

	/* set wdq */
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_idx, byte_idx));
	ddrtrn_hal_phy_cfg_update(base_phy);
}

void ddrtrn_hal_freq_change_set_wdqs_val_by_rank(const struct ddrtrn_delay_ckph *delay_ck,
	const struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	unsigned int byte_idx;
	unsigned int wlsl;
	unsigned int clk_index;
	unsigned int ck_phase_freq[PHY_ACCTL_DCLK_NUM]; /* [0]:lowfreq [1]:highfreq */
	int wdqsdly_highfreq;
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	const struct ddrtrn_delay_ckph_phy *tr_delay_ck = &delay_ck->tr_delay_ck[phy_idx];

	if (ddrtrn_hal_get_cur_phy_total_byte_num() > DDR_PHY_BYTE_MAX)
		return;

	for (byte_idx = 0; byte_idx < ddrtrn_hal_get_cur_phy_total_byte_num(); byte_idx++) {
		ddrtrn_hal_set_cur_byte(byte_idx);
		const struct ddrtrn_hal_training_dq_adj *byte_dq_st =
			&dq_adj_all->phy_dq_adj[phy_idx].rank_dq_adj[rank_idx].byte_dq_adj[byte_idx];
		wlsl = 0x1;

		clk_index = (byte_idx < 2) ? 0 : 1; /* 2: byte 0/1 */
		ck_phase_freq[0] = tr_delay_ck->ckph_st[0].ck_phase[clk_index] -
			(tr_delay_ck->ckph_st[0].ck_phase[clk_index] + 1) / 4; /* 4 wdqsphase */
		ck_phase_freq[1] = tr_delay_ck->ckph_st[1].ck_phase[clk_index] -
			(tr_delay_ck->ckph_st[1].ck_phase[clk_index] + 1) / 4; /* 4 wdqsphase */
		if (ck_phase_freq[0] > CK_PHASE_MAX || ck_phase_freq[1] > CK_PHASE_MAX)
			return;
		if (tr_delay_ck->phase2bdl_lowfreq[byte_idx] > (PHY_RDQS_CYC_MASK / PHY_WDQSPHASE_NUM_T) ||
			tr_delay_ck->phase2bdl_highfreq[byte_idx] > (PHY_RDQS_CYC_MASK / PHY_WDQSPHASE_NUM_T))
			return;
		wdqsdly_highfreq = (int)(ck_phase_freq[1] * tr_delay_ck->phase2bdl_highfreq[byte_idx]) -
			((int)(ck_phase_freq[0] * tr_delay_ck->phase2bdl_lowfreq[byte_idx]) - byte_dq_st->wdqs_average);

		/* wdqs back 120 */
		wdqsdly_highfreq -= (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx]) / 3; /* 1/3 T */
		while ((wdqsdly_highfreq < 0) && (wlsl > 0)) {
			wdqsdly_highfreq = wdqsdly_highfreq + (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx]);
			wlsl--;
		}
		while ((wdqsdly_highfreq >= (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx])) &&
			(wlsl < 2)) { /* 2: wlsl val 0/1/2 */
			wdqsdly_highfreq = wdqsdly_highfreq - (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx]);
			wlsl++;
		}

		if (wdqsdly_highfreq < 0)
			wdqsdly_highfreq = 0;
		else if (wdqsdly_highfreq > PHY_BDL_MASK)
			wdqsdly_highfreq = PHY_BDL_MASK;

		ddrtrn_hal_freq_change_set_wdqs_val_process(delay_ck, byte_dq_st, wlsl, wdqsdly_highfreq);
	}
}

void ddrtrn_hal_freq_change_set_wdqs_val_by_phy(const struct ddrtrn_delay_ckph *delay_ck,
	const struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	unsigned int i;

	for (i = 0; i < ddrtrn_hal_get_cur_phy_rank_num(); i++) {
		ddrtrn_hal_set_rank_id(i);

		ddrtrn_hal_freq_change_set_wdqs_val_by_rank(delay_ck, dq_adj_all);
	}
}

void ddrtrn_hal_freq_change_set_wdqs_val(const struct ddrtrn_delay_ckph *delay_ck,
	const struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	unsigned int i;

	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		if (ddrtrn_hal_get_rank_item_hw(i, 0) == 0)
			continue;

		ddrtrn_hal_freq_change_set_wdqs_val_by_phy(delay_ck, dq_adj_all);
	}
}

/* Configure ddr frequency */
void ddrtrn_hal_low_freq_cfg_freq_process(unsigned int crg_pll7, unsigned int crg_pll6)
{
	unsigned int clk_reset;
	unsigned int soc_freq_conf;
	unsigned int ddraxi_sc_seled;

	clk_reset = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_CLK_RESET);
	soc_freq_conf = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_SOC_FREQ_CONFIG);

	/* DDR switch to 24MHZ */
	ddrtrn_reg_write((clk_reset & (~(DDR_CKSEL_MASK << DDR_CKSEL_BIT))) | (CRG_DDR_CKSEL_24M << DDR_CKSEL_BIT),
		CRG_REG_BASE_ADDR + CRG_CLK_RESET);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_10US); /* wait 10us */
	/* ddraxi cksel switch to 24MHZ */
	ddrtrn_reg_write((soc_freq_conf & (~(DDR_AXI_CKSEL_MASK << DDR_AXI_CKSEL_BIT))) |
		(CRG_DDRAXI_CKSEL_24M << DDR_AXI_CKSEL_BIT),
		CRG_REG_BASE_ADDR + CRG_SOC_FREQ_CONFIG);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_10US); /* wait 10us */
	/* check ddr axi clock stat */
	ddraxi_sc_seled = (ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_SOC_FREQ_INDICATE) >>
		DDR_AXI_SC_SELED_BIT) & DDR_AXI_SC_SELED_MASK;
	if (ddraxi_sc_seled != CRG_DDRAXI_CKSEL_24M)
		return;

	/* Configure DPLL */
	ddrtrn_reg_write(crg_pll7 | (0x1 << 20), CRG_REG_BASE_ADDR + PERI_CRG_PLL7); /* bit[20] */
	ddrtrn_reg_write(crg_pll6, CRG_REG_BASE_ADDR + PERI_CRG_PLL6);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_1US); /* wait 1us */
	ddrtrn_reg_write(crg_pll7 & (~(0x1 << 20)), CRG_REG_BASE_ADDR + PERI_CRG_PLL7); /* bit[20] */
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_100US); /* wait 100us */

	/* restore  CRG_CLK_RESET and CRG_SOC_FREQ_CONFIG */
	ddrtrn_reg_write(clk_reset, CRG_REG_BASE_ADDR + CRG_CLK_RESET);
	ddrtrn_reg_write(soc_freq_conf, CRG_REG_BASE_ADDR + CRG_SOC_FREQ_CONFIG);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_10US);
}
#endif /* DDR_LOW_FREQ_CONFIG */
