/*
 * SPI flash internal definitions
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _NVT_FLASH_SPI_NOR_INT_H_
#define _NVT_FLASH_SPI_NOR_INT_H_
/* Enum list - Full read commands */
enum spi_read_cmds {
	ARRAY_SLOW		= BIT(0),
	ARRAY_FAST		= BIT(1),
	DUAL_OUTPUT_FAST	= BIT(2),
	QUAD_OUTPUT_FAST	= BIT(3),
	DUAL_IO_FAST		= BIT(4),
	QUAD_IO_FAST		= BIT(5),
};

/* Normal - Extended - Full command set */
#define RD_NORM		(ARRAY_SLOW | ARRAY_FAST)
#define RD_EXTN		(RD_NORM | DUAL_OUTPUT_FAST | DUAL_IO_FAST)
#define RD_FULL		(RD_EXTN | QUAD_OUTPUT_FAST | QUAD_IO_FAST)

/* sf param flags */
enum {
#ifndef CONFIG_SPI_FLASH_USE_4K_SECTORS
	SECT_4K		= 0,
#else
	SECT_4K		= BIT(0),
#endif
	SECT_32K	= BIT(1),
	E_FSR		= BIT(2),
	SST_WR		= BIT(3),
	WR_QPP		= BIT(4),
};

enum spi_nor_option_flags {
	SNOR_F_SST_WR		= BIT(0),
	SNOR_F_USE_FSR		= BIT(1),
};
#define SPI_FLASH_3B_ADDR_LEN		3
#define SPI_FLASH_CMD_LEN		(1 + SPI_FLASH_3B_ADDR_LEN)
#define SPI_FLASH_16MB_BOUN		0x1000000

/* CFI Manufacture ID's */
#define SPI_FLASH_CFI_MFR_SPANSION	0x01
#define SPI_FLASH_CFI_MFR_STMICRO	0x20
#define SPI_FLASH_CFI_MFR_MACRONIX	0xc2
#define SPI_FLASH_CFI_MFR_SST		0xbf
#define SPI_FLASH_CFI_MFR_WINBOND	0xef
#define SPI_FLASH_CFI_MFR_GIGADEVICE	0xc8
#define SPI_FLASH_CFI_MFR_EON		0x1c

/* Erase commands */
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_4K_4BYTE		0x21
#define CMD_ERASE_CHIP			0xc7
#define CMD_ERASE_64K			0xd8
#define CMD_ERASE_64K_4BYTE		0xdc

/* Write commands */
#define CMD_WRITE_STATUS		0x01
#define CMD_PAGE_PROGRAM		0x02
#define CMD_WRITE_DISABLE		0x04
#define CMD_WRITE_ENABLE		0x06
#define CMD_QUAD_PAGE_PROGRAM		0x32
#define CMD_WRITE_EVCR			0x61

/* Read commands */
#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_READ_DUAL_OUTPUT_FAST	0x3b
#define CMD_READ_DUAL_IO_FAST		0xbb
#define CMD_READ_QUAD_OUTPUT_FAST	0x6b
#define CMD_READ_QUAD_IO_FAST		0xeb
#define CMD_READ_ID			0x9f
#define CMD_READ_STATUS			0x05
#define CMD_READ_STATUS1		0x35
#define CMD_READ_CONFIG			0x35
#define CMD_FLAG_STATUS			0x70
#define CMD_READ_EVCR			0x65

/* Bank addr access commands */
#ifdef CONFIG_SPI_FLASH_BAR
# define CMD_BANKADDR_BRWR		0x17
# define CMD_BANKADDR_BRRD		0x16
# define CMD_EXTNADDR_WREAR		0xC5
# define CMD_EXTNADDR_RDEAR		0xC8
#endif

/* Common status */
#define STATUS_WIP			BIT(0)
#define STATUS_QEB_WINSPAN		BIT(1)
#define STATUS_QEB_MXIC			BIT(6)
#define STATUS_PEC			BIT(7)
#define STATUS_QEB_MICRON		BIT(7)
#define SR_BP0				BIT(2)  /* Block protect 0 */
#define SR_BP1				BIT(3)  /* Block protect 1 */
#define SR_BP2				BIT(4)  /* Block protect 2 */

#define FLASH_STATUS_WP_BITS            (0x1C)
#define FLASH_STATUS_WEL_BITS           (0x02)
#define FLASH_STATUS_TYPE1_QE_BITS      (1<<6)
#define FLASH_STATUS_TYPE2_QE_BITS      (1<<9)

/* Flash timeout values */
#define SPI_FLASH_PROG_TIMEOUT		(2 * CONFIG_SYS_HZ)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT	(5 * CONFIG_SYS_HZ)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10 * CONFIG_SYS_HZ)
/**
 * struct spi_flash_params - SPI/QSPI flash device params structure
 *
 * @name:		Device name ([MANUFLETTER][DEVTYPE][DENSITY][EXTRAINFO])
 * @jedec:		Device jedec ID (0x[1byte_manuf_id][2byte_dev_id])
 * @ext_jedec:		Device ext_jedec ID
 * @sector_size:	Isn't necessarily a sector size from vendor,
 *			the size listed here is what works with CMD_ERASE_64K
 * @nr_sectors:		No.of sectors on this device
 * @e_rd_cmd:		Enum list for read commands
 * @flags:		Important param, for flash specific behaviour
 */
struct spi_flash_params {
	const char *name;
	u32 jedec;
	u16 ext_jedec;
	u32 sector_size;
	u32 nr_sectors;
	u8 e_rd_cmd;
	u16 flags;
};
#endif