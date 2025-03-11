// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_HW_TRAINING_CONFIG
#ifdef DDR_HW_READ_ADJ_CONFIG
/*
 * Adjust rdqs and dq after hw read training.
 * When define ddrtrn_TRAINING_ADJUST_DISABLE, MUST define ddrtrn_HW_READ_ADJ_CONFIG.
 */
void ddrtrn_hal_hw_read_adj(void)
{
	int i;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();

	ddrtrn_debug("DDR hw read adjust");
	/* check hw read adjust bypass bit */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_HW_ADJ_MASK) != DDR_FALSE)
		return;

	/* assume read dataeye window on left */
	for (i = 0; i < byte_num; i++) {
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), i))
			+ (PHY_DQ_MIDDLE_VAL << PHY_BDL_DQ_BIT),
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), i));
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), i))
			+ (PHY_DQ_MIDDLE_VAL << PHY_BDL_DQ_BIT),
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), i));
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(i))
			+ (PHY_RDQS_MIDDLE_VAL << PHY_RDQS_BDL_BIT),
			base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
	}
}
#else
void ddrtrn_hal_hw_read_adj(void) {}
#endif /* ddrtrn_HW_READ_ADJ_CONFIG */

/* sync rank1 WDQSPH/WDQPH to rank0 */
static void ddrtrn_hal_set_rank1_wdq_to_rank0(unsigned int base_phy, unsigned int byte_num)
{
	unsigned int byte_idx;

	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(1, byte_idx)),
			base_phy + ddrtrn_hal_phy_dxwdqsdly(0, byte_idx));
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(1, byte_idx)),
			base_phy + ddrtrn_hal_phy_dxnwdqdly(0, byte_idx));
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
}

static void ddrtrn_hal_wdqs_bdl2phase(unsigned int base_phy, unsigned int rank, unsigned int byte_idx)
{
	unsigned int wdqsdly, wdqdly, wdqsbdl, wdqsphase, wdqphase;
	unsigned int wlsl;
	unsigned int phase2bdl;

	ddrtrn_debug("wdqsbdl adjust phy[%x] rank[%x] byte[%x]", base_phy, rank, byte_idx);

	phase2bdl = ((ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >> PHY_RDQS_CYC_BIT) &
		PHY_RDQS_CYC_MASK) / PHY_WDQSPHASE_NUM_T;
	wdqsdly = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(rank, byte_idx));
	wdqdly = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(rank, byte_idx));
	wdqsbdl = (wdqsdly >> PHY_WDQS_BDL_BIT) & PHY_WDQS_BDL_MASK;
	wdqsphase = (wdqsdly >> PHY_WDQS_PHASE_BIT) & PHY_WDQS_PHASE_MASK;
	wdqphase = (wdqdly >> PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK;
	wlsl = (ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwlsl(rank, byte_idx)) >>
		PHY_WLSL_BIT) & PHY_WLSL_MASK; /* [17:16] valid value:0/1/2 */

	if ((wdqsbdl > PHY_WDQS_BDL_MASK) || (phase2bdl == 0))
		return;

	while (wdqsbdl > phase2bdl) {
		if (wdqsphase < PHY_WDQSPHASE_REG_MAX) {
			wdqsbdl -= phase2bdl;
			wdqsphase++;
			/* if bit[1:0] value is 0x3, avoid phase cavity */
			if ((wdqsphase & 0x3) == 0x3) /* 0x3:bit[1:0] */
				wdqsphase++;
		} else if (wlsl < PHY_WLSL_MAX) {
			wdqsbdl -= phase2bdl;
			wlsl++;
			wdqsphase = PHY_WDQSPHASE_REG_MIN;
			if (wdqphase < PHY_WDQPHASE_REG_NUM_T) {
				wdqphase = 0;
			} else {
				wdqphase -= PHY_WDQPHASE_REG_NUM_T;
			}
		} else {
			break;
		}
	}

	wdqsdly = (wdqsdly & (~(PHY_WDQS_BDL_MASK << PHY_WDQS_BDL_BIT))) | (wdqsbdl << PHY_WDQS_BDL_BIT);
	wdqsdly = (wdqsdly & (~(PHY_WDQS_PHASE_MASK << PHY_WDQS_PHASE_BIT))) | (wdqsphase << PHY_WDQS_PHASE_BIT);
	ddrtrn_reg_write(wdqsdly, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank, byte_idx));
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwlsl(rank, byte_idx)) &
		(~(PHY_WLSL_MASK << PHY_WLSL_BIT))) | (wlsl << PHY_WLSL_BIT),
		base_phy + ddrtrn_hal_phy_dxnwlsl(rank, byte_idx));
	ddrtrn_reg_write((wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) | (wdqphase << PHY_WDQ_PHASE_BIT),
		base_phy + ddrtrn_hal_phy_dxnwdqdly(rank, byte_idx));
	ddrtrn_hal_phy_cfg_update(base_phy);
}

/* This function is used to prevent logic bugs */
int ddrtrn_hal_hw_training_normal_conf(void)
{
	int result;
	unsigned int byte_idx;
	unsigned int base_phy;
	unsigned int byte_num;
	unsigned int rank_idx;
	unsigned int cur_item = ddrtrn_hal_get_cur_item();
	struct ddrtrn_hal_training_dq_adj_rank wdq_rank0_byte;

	ddrtr_set_data(&wdq_rank0_byte, 0, sizeof(struct ddrtrn_hal_training_dq_adj_rank));

	base_phy = ddrtrn_hal_get_cur_phy();
	byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	rank_idx = ddrtrn_hal_get_rank_id();
	if (rank_idx == 0) {
		/* WL */
		ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_WL_EN);
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
			ddrtrn_hal_set_cur_byte(byte_idx);
			ddrtrn_hal_wdqs_bdl2phase(base_phy, 0, byte_idx);
		}
		result = ddrtrn_hal_hw_training_process(cur_item & PHY_HW_GP_NORMAL);
	} else { /* rank1 */
		/* save rank0 WDQSPH/WDQPH of all byte */
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++)
			ddrtrn_hal_get_dly_value(&wdq_rank0_byte.byte_dq_adj[byte_idx], base_phy, 0, byte_idx);
		/* WL */
		result = ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_WL_EN);
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
			ddrtrn_hal_set_cur_byte(byte_idx);
			ddrtrn_hal_wdqs_bdl2phase(base_phy, 1, byte_idx);
		}

		/* sync rank1 WDQSPH/WDQPH to rank0 */
		ddrtrn_hal_set_rank1_wdq_to_rank0(base_phy, byte_num);

		/* GATE/GDS/WL2/RDET/WDET */
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_HW_GP_NORMAL_RANK1);
		/* sync rank1 WDQSPH/WDQPH to rank0 */
		ddrtrn_hal_set_rank1_wdq_to_rank0(base_phy, byte_num);

		/* HVREFT/DVREFT */
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_HVREFT_EN);
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_DVREFT_EN);
		/* sync rank1 WDQSPH/WDQPH to rank0 */
		ddrtrn_hal_set_rank1_wdq_to_rank0(base_phy, byte_num);

		/* TDQSST */
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_PIC_TDQSST);

		/* restore rank0 WDQSPH/WDQPH of all byte */
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++)
			ddrtrn_hal_restore_dly_value(&wdq_rank0_byte.byte_dq_adj[byte_idx], base_phy, 0, byte_idx);
	}

	return result;
}

void ddrtrn_hal_training_get_rdqs(struct ddrtrn_hal_bdl *rdqs)
{
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();

	for (i = 0; i < byte_num; i++)
		rdqs->bdl[i] = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
}

void ddrtrn_hal_training_set_rdqs(struct ddrtrn_hal_bdl *rdqs)
{
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();

	for (i = 0; i < byte_num; i++)
		ddrtrn_reg_write(rdqs->bdl[i], base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
}

/* ca odt disable, DRAM_RST and DRAM_INIT are required to take effect
 * The DRAM_RST cannot be performed more than once
 */
static int ddrtrn_hal_hw_ca_odt_disable(void)
{
	int result;
	unsigned int temp;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();

	temp = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG01);
	ddrtrn_reg_write(temp & 0x8fffffff, base_phy + DDR_PHY_MODEREG01); /* ca odt disable */
	result = ddrtrn_hal_hw_training_process(item & PHY_HW_GP_DRAM_RESET);

	ddrtrn_reg_write(temp, base_phy + DDR_PHY_MODEREG01); /* restore */

	return result;
}

/* CA Vref Sync, rank0 and rank1 */
static int ddrtrn_hal_hw_ca_vref_sync(void)
{
	int result;
	unsigned int temp;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();

	temp = ddrtrn_reg_read(base_phy + DDR_PHY_TRAINCTRL0);
	ddrtrn_reg_write(temp & (~PHY_TRAINCTRL0_MASK), base_phy + DDR_PHY_TRAINCTRL0); /* select rank0 */
	result = ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_AC);

	ddrtrn_reg_write((temp & (~PHY_TRAINCTRL0_MASK)) | 0x1,
		base_phy + DDR_PHY_TRAINCTRL0); /* select rank1 */
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_AC);

	ddrtrn_reg_write(temp, base_phy + DDR_PHY_TRAINCTRL0); /* restore */

	return result;
}

static int ddrtrn_hal_hw_dram_mr_init(void)
{
	int result;
	unsigned int tx_odt_mode;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();

	tx_odt_mode = (ddrtrn_reg_read(base_phy + DDR_PHY_ACIOCTL) >> PHY_AC_IOCTL_TX_MODE_BIT) &
		PHY_AC_IOCTL_TX_MODE_MASK;
	if (tx_odt_mode == DDR_PHY_LPDDR4X_MODE) {
		unsigned int temp;
		unsigned int temp1;

		/* rank0 */
		temp = ddrtrn_reg_read(base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write((temp & (~DDR_PHY_RANKEN_MASK)) | 0x1,
			base_phy + DDR_PHY_RANKEN); /* select rank0 */

		temp1 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG23); /* store the contents of the Mode Register */
		ddrtrn_reg_write(temp1 & 0xffffffc7, base_phy + DDR_PHY_MODEREG23); /* rank0 ck/cs/ca odt enable */
		result = ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN); /* rank0 draminit */
		/* restore */
		ddrtrn_reg_write(temp, base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write(temp1, base_phy + DDR_PHY_MODEREG23);

		/* rank1 */
		temp = ddrtrn_reg_read(base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write((temp & (~DDR_PHY_RANKEN_MASK)) | 0x2, /* 0x2:bit1 set 1 */
			base_phy + DDR_PHY_RANKEN); /* select rank1 */

		temp1 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG23);
		/* rank1 ck/caodt diable, rank1 cs odt enable */
		ddrtrn_reg_write((temp1 & 0xffffffc7) | 0x28, base_phy + DDR_PHY_MODEREG23);
		result += ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN);
		/* restore */
		ddrtrn_reg_write(temp, base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write(temp1, base_phy + DDR_PHY_MODEREG23);
	} else {
		result = ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN);
	}

	return result;
}

/* DDR HW training adapt dram type */
int ddrtrn_hal_hw_dataeye_adapt(unsigned int *ddrtrn_temp)
{
	int result;
	unsigned int modereg45;
	unsigned int modereg67;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();

	modereg45 = 0;
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		unsigned int dramtimer1;

		dramtimer1 = ddrtrn_reg_read(base_phy + DDR_PHY_DRAMTIMER1);
		ddrtrn_reg_write(dramtimer1 & (~(DDR_PHY_T_MOD_MASK << DDR_PHY_T_MOD_BIT)),
			base_phy + DDR_PHY_DRAMTIMER1); /* TMOD:0 */

		result = ddrtrn_hal_hw_ca_odt_disable(); /* CA odt disable */
		result += ddrtrn_hal_hw_ca_vref_sync(); /* CA vref sync */
		result += ddrtrn_hal_hw_dram_mr_init(); /* in WR0 */

		modereg67 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG67);
		/* turn to WR1 */
		ddrtrn_reg_write(modereg67 | (0x1 << PHY_MODEREG67_LP4_FSPWR_BIT),
			base_phy + DDR_PHY_MODEREG67); /* bit6 set 1 */
		result += ddrtrn_hal_hw_dram_mr_init();
		result += ddrtrn_hal_hw_ca_vref_sync(); /* CA vref sync */

		/* turn to WR0 */
		ddrtrn_reg_write(modereg67 & (~(0x1 << PHY_MODEREG67_LP4_FSPWR_BIT)),
			base_phy + DDR_PHY_MODEREG67); /* bit6 set 0 */
		result += ddrtrn_hal_hw_dram_mr_init();

		/* restore DRAMTIMER1 */
		ddrtrn_reg_write(dramtimer1, base_phy + DDR_PHY_DRAMTIMER1);
	} else {
#ifdef DDR_WRITE_DM_DISABLE
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_DDR4) {
			modereg45 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG45);
			ddrtrn_reg_write((modereg45 & 0xFBFFFFFF) | 0x8000000, base_phy + DDR_PHY_MODEREG45); /* write dm disable */
		}
#endif
		*ddrtrn_temp = modereg45; /* for restore 0xe0 in ddr_hw_training_ctl */
		result = ddrtrn_hal_hw_training_process(ddrtrn_hal_get_cur_item() & PHY_HW_GP_DRAM_RESET);
	}

	return result;
}


int ddrtrn_hal_hw_dataeye_vref_set(void)
{
	int result;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();
	unsigned int dvrft_ctrl;

	dvrft_ctrl = ddrtrn_reg_read(base_phy + DDR_PHY_DVRFTCTRL);
	ddrtrn_reg_write(dvrft_ctrl & (~PHY_DVRFTCTRL_PDAEN_EN),
		base_phy + DDR_PHY_DVRFTCTRL);
	/* DDR_PHY_VREFTCTRL 31bit:1 do vref dram set twice */
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_VREFTCTRL) &
		(~((unsigned int)0x1 << PHY_VREFS_MRS_ENTER_BIT))) |
		((unsigned int)0x1 << PHY_VREFS_MRS_ENTER_BIT),
		base_phy + DDR_PHY_VREFTCTRL);
	result = ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_DQ);
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_DQ);
	/* DDR_PHY_VREFTCTRL 31bit:0 do vref dram set once */
	ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_VREFTCTRL) &
		(~((unsigned int)0x1 << PHY_VREFS_MRS_ENTER_BIT)),
		base_phy + DDR_PHY_VREFTCTRL);
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_DQ);
	ddrtrn_reg_write(dvrft_ctrl, base_phy + DDR_PHY_DVRFTCTRL);

	return result;
}

/* DDR HW training process */
int ddrtrn_hal_hw_training_process(unsigned int item)
{
	unsigned int count;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int init_ctrl = ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITCTRL);

	if (item == 0)
		return 0;

	ddrtrn_debug("base_phy[%x] itme[%x]", base_phy, item);
	/* hardware training enable */
	ddrtrn_reg_write(item | PHY_PHYINITCTRL_INIT_EN | init_ctrl,
		base_phy + DDR_PHY_PHYINITCTRL);

	if ((item & PHY_PHYINITCTRL_DRAM_RST) &&
		(item & PHY_PHYINITCTRL_DRAM_INIT_EN)) {
		if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
			return -1;
	}

	count = DDR_HWR_WAIT_TIMEOUT;
	/* auto cleared to 0 after training finished */
	while (count--) {
		if ((ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITCTRL) &
			PHY_PHYINITCTRL_MASK) == 0) {
			break;
		}
	}

	if (count == 0xffffffff) {
		ddrtrn_fatal("HWR wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_HW_RD_DATAEYE, base_phy, item,
			ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS));
		return -1;
	}

	if (ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS) &
		(~PHY_PHYINITSTATUS_ZCAL_ERROR)) {
		ddrtrn_fatal("Phy[%x] hw[%x] failed[%x]", base_phy, item,
			ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS));
		ddrtrn_hal_training_stat(DDR_ERR_HW_RD_DATAEYE, base_phy, item,
			ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS));
		return -1;
	}
	return 0;
}

void ddrtrn_hal_hw_clear_rdq(unsigned int i)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();

	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), i));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), i));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
}

static void ddrtrn_hal_set_phy_dxnrdqsdly(unsigned int index, unsigned int value)
{
	ddrtrn_reg_write(value,
			ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(index));
}
static unsigned int ddrtrn_hal_get_phy_dxnrdqsdly(unsigned int index)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(index));
}

static void ddrtrn_hal_set_phy_rain_ctrl(unsigned int value)
{
	ddrtrn_reg_write(value, ddrtrn_hal_get_cur_phy() + DDR_PHY_TRAINCTRL12);
}

static unsigned int ddrtrn_hal_get_phy_rain_ctrl(void)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_TRAINCTRL12);
}

int ddrtrn_hal_hw_restore_rdqsbdl(unsigned int bdl_0, unsigned int bdl_1, unsigned int index)
{
	unsigned int rdqs_rank0, rdqs_rank1;
	int offset;
	/* struct ddrtrn_rdqs_data store the whole register value */
	rdqs_rank0 =
		(bdl_0 >> PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK;
	rdqs_rank1 =
		(bdl_1 >> PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK;

	ddrtrn_hal_set_cur_byte(index);
	if (rdqs_rank0 > rdqs_rank1) {
		offset = rdqs_rank0 - rdqs_rank1;
		ddrtrn_hal_set_phy_dxnrdqsdly(index, bdl_0);
		ddrtrn_hal_set_rank_id(1); /* switch to rank1 for sync rank1 rdq */
	} else {
		offset = rdqs_rank1 - rdqs_rank0;
		ddrtrn_hal_set_phy_dxnrdqsdly(index, bdl_1);
		ddrtrn_hal_set_rank_id(0); /* switch to rank0 for sync rank0 rdq */
	}

	return offset;
}

void ddrtrn_hal_hw_save_rdqsbdl(void)
{
	unsigned int temp;

	temp = ((ddrtrn_hal_get_phy_dxnrdqsdly(0) >>
		PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK) >> 2; /* right shift 2: 1/4T */
	temp = (ddrtrn_hal_get_phy_rain_ctrl() &
		(~(PHY_WL_FALLEDGE_BDL_JSTEP_R_MASK <<
		PHY_WL_FALLEDGE_BDL_JSTEP_R_BIT))) |
		(temp << PHY_WL_FALLEDGE_BDL_JSTEP_R_BIT);
	ddrtrn_hal_set_phy_rain_ctrl(temp);
}
#ifdef DDR_WRITE_DM_DISABLE
int ddrtrn_hal_hw_restore_write_dm(unsigned int ddrtrn_temp)
{
	int result;
	unsigned int temp1;
	unsigned int temp;
	unsigned int item = ddrtrn_hal_get_cur_item();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();

	ddrtrn_reg_write(ddrtrn_temp, base_phy + DDR_PHY_MODEREG45); /* restore 0xe0 */
	temp = ddrtrn_reg_read(base_phy + DDR_PHY_MRS_SEQ_PROG);
	temp1 = ddrtrn_reg_read(base_phy + DDR_PHY_DRAMCFG);
	ddrtrn_reg_write(PHY_MRS_SEQ_PROG_VAL, base_phy + DDR_PHY_MRS_SEQ_PROG); /* init MR5 */
	ddrtrn_reg_write(temp1 | PHY_WDM_DISABLE_VAL, base_phy + DDR_PHY_DRAMCFG); /* write dm disable */
	result = ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN);
	ddrtrn_reg_write(temp, base_phy + DDR_PHY_MRS_SEQ_PROG); /* restore */
	ddrtrn_reg_write(temp1, base_phy + DDR_PHY_DRAMCFG); /* restore */

	return result;
}
#endif /* DDR_WRITE_DM_DISABLE */
#endif /* DDR_HW_TRAINING_CONFIG */
