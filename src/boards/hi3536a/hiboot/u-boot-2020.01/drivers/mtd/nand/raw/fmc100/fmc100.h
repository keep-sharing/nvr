// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __FMC100_H__
#define __FMC100_H__

#include <nand.h>
#include <fmc_common.h>
#include <nfc_common.h>
#include <cpu_func.h>
#include "../../../fmc_spi_ids.h"
#include "securec.h"

/* These macroes are for debug only, reg option is slower then dma option */
#undef FMC100_SPI_NAND_SUPPORT_REG_READ
#undef FMC100_SPI_NAND_SUPPORT_REG_WRITE
/* open CONFIG_FS_MAY_NOT_YAFFS2 as you need ,just use for spi_nand */
#undef CONFIG_FS_MAY_NOT_YAFFS2

#define REG_CNT_HIGH_BLOCK_NUM_SHIFT        10

#define REG_CNT_BLOCK_NUM_MASK          0x3ff
#define REG_CNT_BLOCK_NUM_SHIFT         22

#define REG_CNT_PAGE_NUM_MASK           0x3f
#define REG_CNT_PAGE_NUM_SHIFT          16

#define REG_CNT_WRAP_MASK           0xf
#define REG_CNT_WRAP_SHIFT          12

#define ERR_STR_DRIVER "Driver does not support this configure "
#define ERR_STR_CHECK "Please make sure the hardware configuration is correct"

#define SPI_NAND_MAX_PAGESIZE           4096
#define SPI_NAND_MAX_OOBSIZE            256

#define FMC100_BUFFER_LEN (SPI_NAND_MAX_PAGESIZE + SPI_NAND_MAX_OOBSIZE)

#define FMC100_ADDR_CYCLE_MASK        0x2

struct fmc_host {
	struct mtd_info *mtd;
	struct nand_chip *chip;
	struct fmc_spi spi[CONFIG_SPI_NAND_MAX_CHIP_NUM];
	struct fmc_cmd_op cmd_op;

	void __iomem *iobase;
	void __iomem *regbase;

	unsigned int fmc_cfg;
	unsigned int fmc_cfg_ecc0;

	unsigned int offset;

	struct device *dev;

	/* This is maybe an un-aligment address, only for malloc or free */
	char *buforg;
	char *buffer;

	unsigned long dma_buffer;
	unsigned long dma_oob;

	unsigned int addr_cycle;
	unsigned int addr_value[2]; /* 2 addr */
	unsigned int cache_addr_value[2]; /* 2 addr */

	unsigned int column;
	unsigned int block_page_mask;

	unsigned int ecctype;
	unsigned int pagesize;
	unsigned int oobsize;

	int add_partition;

	/* BOOTROM read two bytes to detect the bad block flag */
#define FMC_BAD_BLOCK_POS     0
	unsigned char *bbm; /* nand bad block mark */
	unsigned short *epm;    /* nand empty page mark */

	unsigned int uc_er;

	void (*send_cmd_pageprog)(struct fmc_host *host);
	void (*send_cmd_status)(const struct fmc_host *host);
	void (*send_cmd_readstart)(struct fmc_host *host);
	void (*send_cmd_erase)(const struct fmc_host *host);
	void (*send_cmd_readid)(struct fmc_host *host);
	void (*send_cmd_reset)(const struct fmc_host *host);
	void (*set_system_clock)(const struct spi_op *op, int clk_en);
};

void fmc100_ecc0_switch(const struct fmc_host* host, unsigned char op);

int fmc100_host_init(struct fmc_host *host);

void fmc100_spi_nand_init(struct fmc_host *host);

void fmc_spi_nand_ids_register(void);
char spi_nand_feature_op(const struct fmc_spi* spi, unsigned char op,
				unsigned char addr, unsigned int* val);
void fmc_srst_init(void);
#endif /* End of __FMC100_H__ */
