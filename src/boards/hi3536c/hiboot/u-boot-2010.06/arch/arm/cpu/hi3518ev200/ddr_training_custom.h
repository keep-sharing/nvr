/*
* Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
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



/* config DDRC, PHY, DDRT typte */
#include <asm/arch/platform.h>

#define DDR_DDRC_V500_CONFIG
#define DDR_PHY_S40_CONFIG
#define DDR_DDRT_S40_CONFIG

/* config special item */
#define DDR_TRAINING_UART_DISABLE
#if 1
#define DDR_TRAINING_LOG_DISABLE
#define DDR_TRAINING_ADJUST_CMD_DISABLE
#else
#define DDR_HW_TRAINING_DISABLE
#define DDR_GATE_TRAINING_DISABLE
#define DDR_WL_TRAINING_DISABLE
#define DDR_DATAEYE_TRAINING_DISABLE
#define DDR_TRAINING_ADJUST_DISABLE
#define DDR_AC_TRAINING_CONFIG
#endif
#define DDR_TRAINING_CUT_CODE_CONFIG

/* config DDRC, PHY, DDRT base address */
/* [CUSTOM] DDR DMC base register */
#define DDR_REG_BASE_DMC0              DDR_DMC0_REG_BASE
/* [CUSTOM] DDR PHY base register */
#define DDR_REG_BASE_PHY0              DDR_PHY0_REG_BASE
/* [CUSTOM] DDR DDRT base register */
#define DDR_REG_BASE_DDRT              DDR_DDRT_REG_BASE
/* [CUSTOM] DDR training item system control */
#define DDR_REG_BASE_SYSCTRL           SYS_CTRL_REG_BASE

/* config offset address */
/* [CUSTOM] ddr training item */
#define SYSCTRL_DDR_TRAINING_CFG       0x90
/* [CUSTOM] ddrt reversed data */
#define SYSCTRL_DDRT_PATTERN           0x94
/* [CUSTOM] ddr training stat*/
#define SYSCTRL_DDR_TRAINING_STAT      0x98

/* config other special */
/* [CUSTOM] DDR training start address. MEM_BASE_DDR*/
#define DDRT_CFG_BASE_ADDR		0x80000000
/* [CUSTOM] SRAM start address.
NOTE: Makefile will parse it, plase define it as Hex. eg: 0xFFFF0C00 */
#define DDR_TRAINING_RUN_STACK		0x04010500

/* armv5 not support dsb, use nop */
#define DDR_ASM_DSB()                     { __asm__ __volatile__("nop"); }

#define DDR_TRAINING_DDRT_PREPARE_FUNC()   ddr_ddrt_prepare_custom()
void ddr_ddrt_prepare_custom(void);
