// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __FMC100_NAND_H__
#define __FMC100_NAND_H__

#include <nand.h>
#include <nfc_common.h>
#include <fmc_common.h>
#include "securec.h"
/* These macroes are for debug only, reg option is slower then dma option */
#undef FMC100_NAND_SUPPORT_REG_READ
#undef FMC100_NAND_SUPPORT_REG_WRITE
/* open CONFIG_FS_MAY_NOT_YAFFS2 as you need, just use for nand */
#undef CONFIG_FS_MAY_NOT_YAFFS2
#define FMC100_NAND_OOBSIZE_FOR_YAFFS     32

#define REG_CNT_HIGH_BLOCK_NUM_SHIFT        10

#define REG_CNT_BLOCK_NUM_MASK          0x3ff
#define REG_CNT_BLOCK_NUM_SHIFT         22

#define REG_CNT_PAGE_NUM_MASK           0x3f
#define REG_CNT_PAGE_NUM_SHIFT          16

#define REG_CNT_WRAP_MASK           0xf
#define REG_CNT_WRAP_SHIFT          12

#define NAND_BUFFER_LEN             (2048 + 128)

#define FMC100_ADDR_CYCLE_MASK        0x4

struct fmc_host {
	struct nand_chip *chip;
	struct mtd_info  *mtd;

	struct fmc_cmd_op cmd_op;
	void __iomem *regbase;

	/* Controller config option nand flash */
	unsigned int nand_cfg;
	unsigned int nand_cfg_ecc0;

	unsigned int offset;
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

	int  need_rr_data;
#define FMC100_READ_RETRY_DATA_LEN         128
	char rr_data[FMC100_READ_RETRY_DATA_LEN];
	struct read_retry_t *read_retry;

	int  version;

	/* BOOTROM read two bytes to detect the bad block flag */
#define FMC100_BAD_BLOCK_POS              0
	unsigned char *bbm;  /* nand bad block mark */
	unsigned short *epm;  /* nand empty page mark */
	unsigned int flags;

#define FMC100_PS_UC_ECC        0x01 /* page has ecc error */
#define FMC100_PS_BAD_BLOCK     0x02 /* bad block */
#define FMC100_PS_EMPTY_PAGE    0x04 /* page is empty */
#define FMC100_PS_EPM_ERROR     0x0100 /* empty page mark word has error. */
#define FMC100_PS_BBM_ERROR     0x0200 /* bad block mark word has error. */
	unsigned int page_status;

	void (*send_cmd_pageprog)(struct fmc_host *host);
	void (*send_cmd_status)(struct fmc_host *host);
	void (*send_cmd_readstart)(struct fmc_host *host);
	void (*send_cmd_erase)(struct fmc_host *host);
	void (*send_cmd_readid)(struct fmc_host *host);
	void (*send_cmd_reset)(struct fmc_host *host);

	void (*enable_ecc_randomizer)(struct fmc_host *host,
				      int ecc_en, int randomizer_en);

	void (*detect_ecc)(struct fmc_host *host);
};

int fmc100_nand_init(struct nand_chip *chip);

void fmc_spl_ids_register(void);

void fmc100_nand_controller_enable(int enable);

#endif /* End of__FMC100_NAND_H__ */
