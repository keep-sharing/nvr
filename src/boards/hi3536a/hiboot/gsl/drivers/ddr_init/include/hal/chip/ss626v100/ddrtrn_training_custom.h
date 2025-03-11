// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DDR_TRAINING_CUSTOM_H
#define DDR_TRAINING_CUSTOM_H

/* boot adaptation */
#if BOOTLOADER == DDR_BOOT_TYPE_UBOOT
#define DDR_VREF_TRAINING_CONFIG
#define DDR_VREF_WITHOUT_BDL_CONFIG
#elif BOOTLOADER == DDR_BOOT_TYPE_CMD_BIN
#define DDR_VREF_TRAINING_CONFIG
#define DDR_VREF_WITHOUT_BDL_CONFIG
#elif BOOTLOADER == DDR_BOOT_TYPE_HSL
#define DDR_TRAINING_LOG_DISABLE
#elif BOOTLOADER == DDR_BOOT_TYPE_AUX_CODE
#define DDR_TRAINING_LOG_DISABLE
#else
# error Unknown boot Type
#endif

/* config DDRC, PHY, DDRT typte */
#define DDR_DDRC_S14_CONFIG
#define DDR_PHY_S14_CONFIG
#define DDR_DDRT_V2_0_SHF0_CONFIG

/* config special item */
/* s40/t28/t16 not support dcc training */
#define DDR_CHANNEL_MAP_PHY0_DMC0_PHY1_DMC2
#define DDR_AUTO_PD_CONFIG
#define DDR_LOW_FREQ_CONFIG
#define DDR_RETRAIN_CONFIG

#define DDR_WL_TRAINING_DISABLE
#define DDR_GATE_TRAINING_DISABLE
#define DDR_TRAINING_UART_DISABLE

#define DDR_PHY_NUM 2 /* phy number */

#define DDR_DMC_PER_PHY_MAX 2 /* dmc number per phy max */

/* config DDRC, PHY, DDRT base address */
/* [CUSTOM] DDR PHY0 base register */
#define DDR_REG_BASE_PHY0        0x11150000
/* [CUSTOM] DDR PHY1 base register */
#define DDR_REG_BASE_PHY1        0x11152000
/* [CUSTOM] DDR DMC0 base register */
#define DDR_REG_BASE_DMC0        0x11148000
/* [CUSTOM] DDR DMC1 base register */
#define DDR_REG_BASE_DMC1        0x11149000
/* [CUSTOM] DDR DMC2 base register */
#define DDR_REG_BASE_DMC2        0x1114a000
/* [CUSTOM] DDR DMC3 base register */
#define DDR_REG_BASE_DMC3        0x1114b000
/* [CUSTOM] DDR DDRT base register */
#define DDR_REG_BASE_DDRT        0x11160000
/* [CUSTOM] DDR training item system control */
#define DDR_REG_BASE_SYSCTRL     0x11020000
#define DDR_REG_BASE_AXI         0x11140000
/* Serial Configuration */
#define DDR_REG_BASE_UART0       0x11040000

/* config offset address */
/* Assume sysctrl offset address for DDR training as follows,
if not please define. */
/* [CUSTOM] PHY1 ddrt reversed data */
#define SYSCTRL_DDRT_PATTERN            0xa8
/* [CUSTOM] PHY2 ddrt reversed data */
#define SYSCTRL_DDRT_PATTERN_SEC        0xac
/* [CUSTOM] ddr sw training item */
#define SYSCTRL_DDR_TRAINING_CFG        0xa0 /* RANK0 */
#define SYSCTRL_DDR_TRAINING_CFG_SEC    0xa4 /* RANK1 */
/* [CUSTOM] ddr training stat */
#define SYSCTRL_DDR_TRAINING_STAT       0xb0
/* [CUSTOM] ddr training version flag */
#define SYSCTRL_DDR_TRAINING_VERSION_FLAG	0xb4
#define SYSCTRL_DDR_TRAINING_CTX        0xb8
#define SYSCTRL_DDR_TRAINING_PHY        0xbc
#define SYSCTRL_DDR_TRAINING_ADJUST     0xc0
#define SYSCTRL_DDR_TRAINING_DQ_TYPE    0xc4

/* [CUSTOM] ddr hw training item */
#define SYSCTRL_DDR_HW_PHY0_RANK0       0x90
#define SYSCTRL_DDR_HW_PHY0_RANK1       0x94
#define SYSCTRL_DDR_HW_PHY1_RANK0       0x98
#define SYSCTRL_DDR_HW_PHY1_RANK1       0x9c

/* config other special */
/* [CUSTOM] DDR training start address. MEM_BASE_DDR */
#define DDRT_CFG_BASE_ADDR       0x40000000
/* [CUSTOM] SRAM start address.
NOTE: Makefile will parse it, plase define it as Hex. eg: 0xFFFF0C00 */
#define DDR_TRAINING_RUN_STACK   0x04020c00

#define CRG_REG_BASE_ADDR      0x11010000
#define PERI_CRG_PLL6          0x180  /* DPLL-1 configuration register */
#define PERI_CRG_PLL7          0x184  /* DPLL-2 configuration register */
#define PERI_CRG_DDRT          0x22a0 /* DDRT clock and soft reset control register */
#define CRG_SOC_FREQ_CONFIG    0x2000 /* SOC frequency configuration register */
#define CRG_SOC_FREQ_INDICATE  0x2020 /* SOC frequency indication register */
#define CRG_CLK_RESET          0x2280 /* DDR clock reset configuration register */
#define SYSCTRL_DDRT_CTRL      0x4030 /* DDRT control register */

/* mask */
#define DDR_CKSEL_MASK         0x3 /* [13:12]ddrtrn_cksel */
#define DDR_AXI_CKSEL_MASK     0x3 /* [13:12]ddraxi_cksel */
#define DDR_AXI_SC_SELED_MASK  0x3 /* [13:12]ddraxi_sc_seled */
#define DDR_TRFC_EN_MASK       0x1 /* bit[0] trfc_en */

/* bit */
#define DDR_CKSEL_BIT          12 /* [13:12]ddr_cksel */
#define DDR_AXI_CKSEL_BIT      12 /* [13:12]ddraxi_cksel */
#define DDR_AXI_SC_SELED_BIT   12 /* [13:12]ddraxi_sc_seled */
#define DDR_TRFC_EN_BIT        0  /* bit[0] trfc_en */
#define DDR_CFG_RX_AGE_COMPST_EN_BIT 31 /* bit[31] cfg_rx_age_compst_en */
#define DDR_TEST0_CKEN_BIT     4  /* ddrtest0_cken */
#define DDR_DDRT_CTRL_DDRT0_BIT 0 /* bit[0]ddrt0_mst_sel */
#define DDR_DDRT_CTRL_DDRT1_BIT 1 /* bit[1]ddrt1_mst_sel */

/* value */
#define CRG_DDR_CKSEL_24M    0x0 /* ddrtrn_cksel 000:24MHz */
#define CRG_DDRAXI_CKSEL_24M 0x0 /* ddraxi_cksel 00:24MHz */
#define CRG_DDR_CKSEL_PLL    0x1 /* ddrtrn_cksel 001:clk_dpll_pst */
/* PLL 200MHz */
#define CRG_PLL7_TMP_1TO4 0x00001042
#define CRG_PLL6_TMP_1TO4 0x24aaaaaa
/* PLL 400MHz */
#define CRG_PLL7_TMP_1TO2 0x00001042
#define CRG_PLL6_TMP_1TO2 0x14aaaaaa
/* PLL 800MHz */
#define CRG_PLL7_TMP_1TO1 0x00001042
#define CRG_PLL6_TMP_1TO1 0x12aaaaaa

#define DDR_FROM_VALUE0 0x0024140F /* partial boot form value */
#define DDR_FROM_VALUE1 0xFFDBEBF5 /* partial boot form value */

#define WL_TIME                 1 /* Number of write leveling times */

#define DDR_RELATE_REG_DECLARE
struct ddrtrn_hal_training_custom_reg {
	unsigned int ddrt_clk_reg;
	unsigned int age_compst_en[DDR_PHY_NUM];
	unsigned int trfc_en[DDR_PHY_NUM];
	unsigned int ddrt_ctrl;
};
int ddrtrn_hal_boot_cmd_save(struct ddrtrn_hal_training_custom_reg *custom_reg);
void ddrtrn_hal_boot_cmd_restore(const struct ddrtrn_hal_training_custom_reg *custom_reg);
#endif /* DDR_TRAINING_CUSTOM_H */
