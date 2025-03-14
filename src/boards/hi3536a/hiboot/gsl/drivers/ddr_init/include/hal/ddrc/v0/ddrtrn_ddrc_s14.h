// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DDR_DDRC_V520_H
#define DDR_DDRC_V520_H

/* ******* DMC ************************* */
/* base address: DDR_REG_BASE_DMC0 DDR_REG_BASE_DMC1 */
/* register offset address */
#define DDR_DMC_CTRL_SREF     0X0  /* DDRC self-refresh control. */
#define DDR_DMC_CFG_SREF      0x20 /* DDRC self-refresh config. */
#define DDR_DMC_CFG_PD        0x28 /* PowerDown */
#define DDR_DMC_CFG_DDRMODE   0x50
#define DDR_DMC_CFG_SCRAMB    0x58 /* DDR scramb config */
#define ddr_dmc_cfg_rnkvol(n) (0x60 + ((n) << 2))
#define DDR_DMC_CFG_EMRS01    0xf0
#define DDR_DMC_TIMING2       0x108
#define DDR_DMC_SFCREQ        0xc
#define DDR_DMC_SFCCMD        0x210
#define DDR_DMC_SFCADDR       0x214 /* read col and row */
#define DDR_DMC_SFCBANK       0x218
#define DDR_DMC_CURR_FUNC     0x294
#ifndef DDR_DMC_SFC_RDATA0
#define DDR_DMC_SFC_RDATA0 0x4A8 /* SFC read data[31:0] */
#endif
#ifndef DDR_DMC_SFC_RDATA1
#define DDR_DMC_SFC_RDATA1 0x4AC /* SFC read data[63:32] */
#endif
#ifndef DDR_DMC_SFC_RDATA2
#define DDR_DMC_SFC_RDATA2 0x4B0 /* SFC read data[95:64] */
#endif
#ifndef DDR_DMC_SFC_RDATA3
#define DDR_DMC_SFC_RDATA3 0x4B4 /* SFC read data[127:96] */
#endif
#ifndef DDR_DMC_SFC_RDATA4
#define DDR_DMC_SFC_RDATA4 0x4B8 /* SFC read data[159:128] */
#endif
#ifndef DDR_DMC_SFC_RDATA5
#define DDR_DMC_SFC_RDATA5 0x4BC /* SFC read data[191:160] */
#endif
#ifndef DDR_DMC_SFC_RDATA6
#define DDR_DMC_SFC_RDATA6 0x4C0 /* SFC read data[223:192] */
#endif
#ifndef DDR_DMC_SFC_RDATA7
#define DDR_DMC_SFC_RDATA7 0x4C4 /* SFC read data[255:224] */
#endif

/* register mask */
#define DMC_CMD_MRS_MASK            0xffff
/* storing data bus width. [00]8bit, [01]16bit, [10]32bit, [11]64bit */
#define DMC_MEM_WIDTH_MASK          0x3
#define DMC_MRS_MASK                0xffff /* [15:0] Mode Register mask */
#define DMC_MR0_BL_MASK             0x3
#define DMC_CFG_DRAM_TYPE_MASK      0xf /* [3:0]101:DDR2, 110:DDR3, 111:DDR4 */
#define DMC_CFG_MEM_BG_MASK         0x3 /* [11:10]0:1, 1:2, 2:4 Bank Group */
#define DMC_CURR_FUNC_IN_SREF_MASK  0x1
#define DMC_RNKVOL_MEM_BANK_MASK    0x3 /* [9:8] */
#define DMC_RNKVOL_MEM_ROW_MASK     0x7 /* [6:4] */
#define DMC_RNKVOL_MEM_COL_MASK     0x7 /* [2:0] */
#define DMC_CFG_INIT_XSREF_PD_MASK  0xc /* [3:2] */
#define DMC_CFG_PD_PRD_MASK         0xfff /* [15:4]pd_prd */

/* register bit */
#define DMC_MEM_WIDTH_BIT       4  /* storing data bus width */
/* [CUSTOM] precharge disable/enable bit */
#define DMC_SFC_PRE_DIS_BIT     30
/* [CUSTOM] [29:12]config MR when LMR command */
#define DMC_SFC_CMD_MRS_BIT     12
#define DMC_SFC_RANK_BIT        16 /* [CUSTOM] [31:16]sfc_rank */
#define DMC_CFG_MEM_BG_BIT      10 /* [11:10] mem_bankgroup */
#define DMC_RNKVOL_MEM_BANK_BIT 8  /* [9:8] */
#define DMC_RNKVOL_MEM_ROW_BIT  4  /* [6:4] */
#define DMC_CFG_PD_PRD_BIT      4  /* [15:4]pd_prd */
#define DMC_CFG_PD_EN_BIT       0  /* [0] pd_en */

/* register value */
#define DMC_BANK_MR1                1
#define DMC_BANK_MR3                0x3
#define DMC_CMD_TYPE_LMR            0x2
#define DMC_CMD_TYPE_READ           0x5  /* read  */
#define DMC_CMD_TYPE_PRECHARGE_ALL  0x6  /* precharge all */
#define DMC_CMD_MRS_MR3             0x4  /* MR3 equal 0x4 */
#define DMC_CMD_MRS_A7              0x80
/* value 1 means exexute command. cmd_rank[0] control DDR RANK0 */
#define DMC_CMD_RANK0               0x1
#define DMC_MR0_BL_BUST8            0x0  /* BC8 (fixed) */
#define DMC_MR0_BL_BUST4            0x2  /* BC4 (fixed) */
#define DMC_AUTO_TIMING_DIS         0xfffff000 /* auto refresh disable */
#define DMC_POWER_DOWN_DIS          0xfffffffe /* powerDown disable */
#define DMC_SCRAMB_DIS              0xffffbfff /* [14] scramb disable */
/* [4] scramb_seed_type, [2:0] scramb_seed_sort */
#define DMC_SCRAMB_CFG              0xffffffe8
#define DMC_CFG_DRAM_TYPE_DDR4      0x7  /* DDR4 */
#define DMC_CFG_DRAM_TYPE_LPDDR4    0x8  /* LPDDR4 */
#define DMC_CFG_MEM_2BG             0x1  /* 2 Bank Group */
#define DMC_CFG_INIT_XSREF          0x8  /* bit[3] */
#define DMC_CFG_SREF_PD             0x4  /* bit[2] */

#define DMC_CTRL_SREF_ENTER   0x1  /* 1 Enter Auto-self refresh */
#define DMC_CTRL_SREF_EXIT    0x2  /* 2 Exit Auto-self refresh */
#define DMC_RNKVOL_MEM_ROW_11 0x0  /* 000: 11 bit */

#ifndef DDR_DMC_PER_PHY_MAX
#define DDR_DMC_PER_PHY_MAX 2
#endif

#ifndef DDR_AXI_SWITCH_NUM
#define DDR_AXI_SWITCH_NUM 4 /* ddr training axi switch number */
#endif

#ifndef DDR_RANK_NUM
#define DDR_RANK_NUM 2 /* rank number */
#endif


/* ******* AXI ************************* */
/* *
 * DMC -- PHY
 * /
 * DDRT -- AXI
 * \
 * DMC -- PHY
 */
/* base address: DDR_REG_BASE_AXI */
/* register offset address */
#define DDR_AXI_CHSEL_REMAP    0x40  /* Channel selection remapping */
#define DDR_AXI_REGION_ATTRIB0 0x104 /* region 0 */
#define DDR_AXI_REGION_ATTRIB1 0x114 /* region 1 */

/* register mask */
#define AXI_REGION_ATTRIB_CH_MASK 0xffffffc0 /* channel mask */

/* register value */
/* Map to the single channel, independent address */
#define AXI_RNG_ATTR_CH_MODE    0x8
#define AXI_RNG_ATTR_CH_START_0 0x0
#define AXI_RNG_ATTR_CH_START_1 0x1
#define AXI_RNG_ATTR_CH_START_2 0x2
#define AXI_RNG_ATTR_CH_START_3 0x3
#define AXI_RNG_NUM             2 /* region number */

/* *******data define*********************************** */
struct ddrtrn_ddrc_data {
	unsigned int region_attrib[AXI_RNG_NUM];
	unsigned int rnkvol;
};
#endif /* DDR_DDRC_V520_H */
