// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "fmc100_os.h"
#include <common.h>

static struct fmc_host fmc100_host = {
	.chip = NULL,
};

static void fmc100_driver_probe(struct nand_chip *chip, unsigned char cs)
{
	int ret;
	struct fmc_host *host = &fmc100_host;

	fmc_pr(BT_DBG, "\t*-Start SPI Nand flash driver probe\n");

	if (!host->chip) {
		/* CONFIG FMC SRST */
		fmc_srst_init();
		/* FMC ip version check */
		if (fmc_ip_ver_check())
			db_bug("Error: fmc IP version unknown!\n");

		/* FMC current SPI device type check */
		fmc_dev_type_switch(FLASH_TYPE_SPI_NAND);

		/* FMC SPI nand init */
		if (memset_s((char *)host, sizeof(struct fmc_host), 0,
				sizeof(struct fmc_host))) {
			db_bug("Error: memset_s fail! %s %d\n", __func__, __LINE__);
			return;
		}
		ret = fmc100_host_init(host);
		if (ret) {
			db_msg("Error: Host init failed, result: %d\n", ret);
			/* Change SPI device type to default */
			fmc_dev_type_switch(FLASH_TYPE_DEFAULT);
			return;
		}
	} else {
		fmc_pr(BT_DBG, "\t*-SPI Nand host is initialized.\n");
	}

	host->cmd_op.cs = cs;
	host->chip = chip;
	fmc100_spi_nand_init(host);

	fmc_pr(BT_DBG, "\t*-End SPI Nand flash driver probe.\n");

	return;
}

static int fmc100_spi_nand_pre_probe(struct nand_chip *chip)
{
	uint8_t nand_maf_id;
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct fmc_host *host = chip->priv;
	int ret;
	/* Reset the chip first */
	host->send_cmd_reset(host);
	chip->dev_ready(mtd);

	/* Check the ID */
	host->offset = 0;
	ret = memset_s((unsigned char *)(chip->IO_ADDR_R), IO_ADDR_R_SIZE, 0, 0x10);
	if (ret)
		printf("ERR:memset_s fail %s %d\n", __func__, __LINE__);

	host->send_cmd_readid(host);
	nand_maf_id = readb(chip->IO_ADDR_R);
	if (nand_maf_id == 0x00 || nand_maf_id == 0xff) {
		printf("Cannot found a valid SPI Nand Device\n");
		return 1;
	}

	return 0;
}

int board_nand_init(struct nand_chip *chip)
{
	unsigned char chip_num = CONFIG_SPI_NAND_MAX_CHIP_NUM;
	static unsigned char cs = 0;
	unsigned char *fmc_cs = NULL;

	for (cs = 0; chip_num && (cs < CONFIG_FMC_MAX_CS_NUM); cs++) {
		fmc_cs = get_cs_number(cs);
		if (*fmc_cs) {
			fmc_pr(BT_DBG, "\t\t*-Current CS(%d) is occupied.\n",
			       cs);
			continue;
		}

		fmc100_driver_probe(chip, cs);
		chip_num--;
	}

	if (chip_num)
		return 1;

	if (fmc100_spi_nand_pre_probe(chip))
		return 1;

	return 0;
}

static int fmc100_spi_nand_get_ecctype(void)
{
	struct fmc_host *host = &fmc100_host;

	if (!host->chip) {
		printf("SPI Nand flash uninitialized.\n");
		return -1;
	}

	return match_ecc_type_to_yaffs(fmc100_host.ecctype);
}

int nand_get_ecctype(void)
{
	return fmc100_spi_nand_get_ecctype();
}
