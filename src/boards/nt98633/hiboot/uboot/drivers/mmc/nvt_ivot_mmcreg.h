/**
    NVT mmc function
    Register settings
    @file       nvt_ivot_mmcreg.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __NVT_IVOT_MMCREG_H__
#define __NVT_IVOT_MMCREG_H__

#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>

#define SDIO_CMD_REG_OFS            0x00
union SDIO_CMD_REG {
	uint32_t reg;
	struct {
	unsigned int CMD_IDX:6;
	unsigned int NEED_RSP:1;
	unsigned int LONG_RSP:1;
	unsigned int RSP_TIMEOUT_TYPE:1;
	unsigned int CMD_EN:1;
	unsigned int SDC_RST:1;
	unsigned int ENABLE_SDIO_INT_DETECT:1;
	unsigned int ENABLE_VOL_SWITCH_DET:1;
	unsigned int ENABLE_EMMC_BOOT:1;
	unsigned int EMMC_BOOT_ACK:1;
	unsigned int EMMC_BOOT_MODE:1;
	unsigned int EMMC_BOOT_CLK:1;
	unsigned int Reserved0:15;
	} bit;
};

#define SDIO_ARGU_REG_OFS           0x04
union SDIO_ARGU_REG {
	uint32_t reg;
	struct {
	unsigned int ARGUMENT:32;
	} bit;
};

#define SDIO_RSP0_REG_OFS           0x08
union SDIO_RSP0_REG {
	uint32_t reg;
	struct {
	unsigned int RSP:32;
	} bit;
};

#define SDIO_RSP1_REG_OFS           0x0C
union SDIO_RSP1_REG {
	uint32_t reg;
	struct {
	unsigned int RSP_3RD_MSB:32;
	} bit;
};

#define SDIO_RSP2_REG_OFS           0x10
union SDIO_RSP2_REG {
	uint32_t reg;
	struct {
	unsigned int RSP_2ND_MSB:32;
	} bit;
};

#define SDIO_RSP3_REG_OFS           0x14
union SDIO_RSP3_REG {
	uint32_t reg;
	struct {
	unsigned int RSP_1ST_MSB:32;
	} bit;
};

#define SDIO_RSP_CMD_REG_OFS        0x18
union SDIO_RSP_CMD_REG {
	uint32_t reg;
	struct {
	unsigned int RSP_CMD_IDX:6;
	unsigned int Reserved0:26;
	} bit;
};

#define SDIO_DATA_CTRL_REG_OFS      0x1C
union SDIO_DATA_CTRL_REG {
	uint32_t reg;
	struct {
	unsigned int Reserved0:6;
	unsigned int DATA_EN:1;
	unsigned int READ_WAIT_EN:1;
	unsigned int SUSPEND_DATA:1;
	unsigned int DIS_SDIO_INT_PERIOD:1;
	unsigned int Reserved1:3;
	unsigned int WAIT_VOL_SWITCH_EN:1;
	unsigned int Reserved2:2;
	unsigned int BLK_SIZE:16;
	} bit;
};

#define SDIO_DATA_TIMER_REG_OFS     0x20
union SDIO_DATA_TIMER_REG {
	uint32_t reg;
	struct {
	unsigned int TIMEOUT:32;
	} bit;
};

#define SDIO_VOL_SWITCH_TIMER_REG_OFS     0x24
union SDIO_VOL_SWITCH_TIMER_REG {
	uint32_t reg;
	struct {
	unsigned int VOL_TIMEOUT:12;
	unsigned int Reserved1:20;
	} bit;
};

#define SDIO_STATUS_REG_OFS  0x28
union SDIO_STATUS_REG {
	uint32_t reg;
	struct {
	unsigned int RSP_CRC_FAIL:1;
	unsigned int DATA_CRC_FAIL:1;
	unsigned int RSP_TIMEOUT:1;
	unsigned int DATA_TIMEOUT:1;
	unsigned int RSP_CRC_OK:1;
	unsigned int DATA_CRC_OK:1;
	unsigned int CMD_SENT:1;
	unsigned int DATA_END:1;
	unsigned int SDIO_INT:1;
	unsigned int READ_WAIT:1;
	unsigned int CARD_BUSY2READY:1;
	unsigned int VOL_SWITCH_END:1;
	unsigned int VOL_SWITCH_TIMEOUT:1;
	unsigned int RSP_VOL_SWITCH_FAIL:1;
	unsigned int VOL_SWITCH_GLITCH:1;
	unsigned int EMMC_BOOT_ACK_RECEIVE:1;
	unsigned int EMMC_BOOT_ACK_TIMEOUT:1;
	unsigned int EMMC_BOOT_END:1;
	unsigned int EMMC_BOOT_ACK_ERROR:1;
	unsigned int DMA_ERROR:1;
	unsigned int PHASE_COMP:1;
	unsigned int Reserved0:11;
	} bit;
};

#define SDIO_INT_MASK_REG_OFS       0x30
union SDIO_INT_MASK_REG {
	uint32_t reg;
	struct {
	unsigned int RSP_CRC_FAIL_INT_EN:1;
	unsigned int DATA_CRC_FAIL_INT_EN:1;
	unsigned int RSP_TIMEOUT_INT_EN:1;
	unsigned int DATA_TIMEOUT_INT_EN:1;
	unsigned int RSP_CRC_OK_INT_EN:1;
	unsigned int DATA_CRC_OK_INT_EN:1;
	unsigned int CMD_SENT_INT_EN:1;
	unsigned int DATA_END_INT_EN:1;
	unsigned int SDIO_INT_INT_EN:1;
	unsigned int READ_WAIT_INT_EN:1;
	unsigned int CARD_BUSY2READY_INT_EN:1;
	unsigned int VOL_SWITCH_END_INT_EN:1;
	unsigned int VOL_SWITCH_TIMEOUT_INT_EN:1;
	unsigned int RSP_VOL_SWITCH_FAIL_INT_EN:1;
	unsigned int VOL_SWITCH_GLITCH_INT_EN:1;
	unsigned int EMMC_BOOT_ACK_RECEIVE_INT_EN:1;
	unsigned int EMMC_BOOT_ACK_TIMEOUT_INT_EN:1;
	unsigned int EMMC_BOOT_END_INT_EN:1;
	unsigned int EMMC_BOOT_ACK_ERROR_INT_EN:1;
	unsigned int Reserved0:13;
	} bit;
};

#define SDIO_CLOCK_CTRL_REG_OFS     0x38
union SDIO_CLOCK_CTRL_REG {
	UINT32 reg;
	struct {
	unsigned int REV0:8;
	unsigned int CLK_AUTOGATE_HIGH:1;
	unsigned int CLK_SD:1;
	unsigned int CLK_DIS:1;
	unsigned int CLK_DDR:1;
	unsigned int DLY_ACT:1;
	unsigned int DLY_TYPE:1;
	unsigned int REV1:2;
	unsigned int DLY_POLARITY:1;
	unsigned int REV2:3;
	unsigned int SMP_EDGE_SEL:1;
	unsigned int CLK_SD_CMD:1;
	unsigned int SMP_EDGE_SEL_CMD:1;
	unsigned int REV3:1;
	unsigned int DEL_SEL:6;
	unsigned int REV4:2;
	} bit;
};

#define SDIO_BUS_WIDTH_REG_OFS      0x3C
union SDIO_BUS_WIDTH_REG {
	uint32_t reg;
	struct {
	unsigned int BUS_WIDTH:2;
	unsigned int Reserved0:30;
	} bit;
};

#define SDIO_BUS_STATUS_REG_OFS     0x40
union SDIO_BUS_STATUS_REG {
	uint32_t reg;
	struct {
	unsigned int CARD_BUSY:1;
	unsigned int BUS_STS_CMD:1;
	unsigned int Reserved0:6;
	unsigned int BUS_STS_D0:1;
	unsigned int BUS_STS_D1:1;
	unsigned int BUS_STS_D2:1;
	unsigned int BUS_STS_D3:1;
	unsigned int BUS_STS_D4:1;
	unsigned int BUS_STS_D5:1;
	unsigned int BUS_STS_D6:1;
	unsigned int BUS_STS_D7:1;
	unsigned int Reserved1:16;
	} bit;
};

#define SDIO_CLOCK_CTRL2_REG_OFS    0x44
union SDIO_CLOCK_CTRL2_REG {
	uint32_t reg;
	struct {
	unsigned int OUTDLY_SEL:6;
	unsigned int Reserved0:10;
	unsigned int INDLY_SEL:6;
	unsigned int Reserved1:10;
	} bit;
};

#define SDIO_EMMC_BOOT_ACK_TIMER_REG_OFS    0x48
union SDIO_EMMC_BOOT_ACK_TIMER_REG {
	uint32_t reg;
	struct {
	unsigned int TIMEOUT:24;
	unsigned int Reserved0:8;
	} bit;
};

#define SDIO_PHY_REG_OFS    0x4C
union SDIO_PHY_REG {
	uint32_t reg;
	struct {
	unsigned int PHY_SW_RST:1;
	unsigned int Reserved0:3;
	unsigned int BLK_FIFO_EN:1;
	unsigned int Reserved1:27;
	} bit;
};

#define SDIO_DLY0_REG_OFS           0x50
union SDIO_DLY0_REG {
	uint32_t reg;
	struct {
	unsigned int DLY_PHASE_SEL:4;
	unsigned int DLY_SEL:3;
	unsigned int Reserved0:5;
	unsigned int SAMPLE_CLK_EDGE:1;
	unsigned int SRC_CLK_SEL:1;
	unsigned int PAD_CLK_SEL:1;
	unsigned int Reserved1:1;
	unsigned int DET_MODE:1;
	unsigned int DET_1_0_DIS:1;
	unsigned int DET_0_1_DIS:1;
	unsigned int DET_DATA:1;
	unsigned int Reserved2:6;
	unsigned int DDR_DATA_SWAP:1;
	unsigned int DDR_START_EDGE:1;
	unsigned int DATA_DLY_PHASE_SEL:4;
	} bit;
};

#define SDIO_DLY1_REG_OFS           0x54
union SDIO_DLY1_REG {
	uint32_t reg;
	struct {
	unsigned int Reserved0:27;
	unsigned int HS400_READ_DLY:1;
	unsigned int DATA_READ_DLY:2;
	unsigned int DET_READ_DLY:2;
	} bit;
};

#define SDIO_DLY2_REG_OFS           0x58
union SDIO_DLY2_REG {
	uint32_t reg;
	struct {
	unsigned int DET_DATA_OUT:32;
	} bit;
};

#define SDIO_DLY3_REG_OFS           0x5C
union SDIO_DLY3_REG {
	uint32_t reg;
	struct {
	unsigned int DET_CLR:1;
	unsigned int Reserved0:31;
	} bit;
};

#define SDIO_DLY4_REG_OFS           0x60
union SDIO_DLY4_REG {
	uint32_t reg;
	struct {
	unsigned int PHASE_COMP_EN:32;
	} bit;
};

#define SDIO_DLY5_REG_OFS           0x64
union SDIO_DLY5_REG {
	uint32_t reg;
	struct {
	unsigned int Reserved0:4;
	unsigned int OUT_DLY_EN:1;
	unsigned int DATA_OUT_DLY_INV:1;
	unsigned int CMD_OUT_DLY_INV:1;
	unsigned int Reserved1:1;
	unsigned int OUT_DLY_SEL:5;
	unsigned int Reserved2:19;
	} bit;
};

#define SDIO_DLY6_REG_OFS           0x7C
union SDIO_DLY6_REG {
	uint32_t reg;
	struct {
	unsigned int DET_CMD_OUT:32;
	} bit;
};

#define SDIO_DMA_DES_HIGH_START_ADDR_REG_OFS  0xF0
union SDIO_DMA_DES_HIGH_START_ADDR_REG {
	uint32_t reg;
	struct {
	unsigned int DES_HIGH_ADDR:4;
	unsigned int Reserved0:28;
	} bit;
};

#define SDIO_DMA_DES_HIGH_CURR_ADDR_REG_OFS   0xF4
union SDIO_DMA_DES_HIGH_CURR_ADDR_REG {
	uint32_t reg;
	struct {
	unsigned int DRAM_HIGH_ADDR:4;
	unsigned int Reserved0:28;
	} bit;
};

#define SDIO_DATA_PORT_REG_OFS     0x100
union SDIO_DATA_PORT_REG {
	uint32_t reg;
	struct {
	unsigned int FIFO:32;
	} bit;
};

#define SDIO_DATA_LENGTH_REG_OFS   0x104
union SDIO_DATA_LENGTH_REG {
	uint32_t reg;
	struct {
	unsigned int LENGTH:26;
	unsigned int Reserved0:6;
	} bit;
};

#define SDIO_FIFO_STATUS_REG_OFS   0x108
union SDIO_FIFO_STATUS_REG {
	uint32_t reg;
	struct {
	unsigned int FIFO_CNT:7;
	unsigned int Reserved0:1;
	unsigned int FIFO_EMPTY:1;
	unsigned int FIFO_FULL:1;
	unsigned int Reserved1:22;
	} bit;
};

#define SDIO_FIFO_CONTROL_REG_OFS  0x10C
union SDIO_FIFO_CONTROL_REG {
	uint32_t reg;
	struct {
	unsigned int FIFO_EN:1;
	unsigned int FIFO_MODE:1;
	unsigned int FIFO_DIR:1;
	unsigned int Reserved0:29;
	} bit;
};

#define SDIO_FIFO_SWITCH_REG_OFS   0x1B0
union SDIO_FIFO_SWITCH_REG {
	uint32_t reg;
	struct {
	unsigned int Reserved0:4;
	unsigned int FIFO_SWITCH_DLY:1;
	unsigned int Reserved1:27;
	} bit;
};

#define SDIO_DMA_START_ADDR_REG_OFS      0x110
union SDIO_DMA_START_ADDR_REG {
	uint32_t reg;
	struct {
	unsigned int DRAM_ADDR:32;
	} bit;
};

#define SDIO_DMA_CURR_ADDR_REG_OFS       0x114
union SDIO_DMA_CURR_ADDR_REG {
	uint32_t reg;
	struct {
	unsigned int DRAM_ADDR:32;
	} bit;
};

#endif
