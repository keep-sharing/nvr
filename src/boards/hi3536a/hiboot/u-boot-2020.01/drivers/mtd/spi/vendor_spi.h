// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __FMC100_SPI_H__
#define __FMC100_SPI_H__

#include <spi_flash.h>
#include <linux/mtd/mtd.h>

struct spi_flash *fmc100_spi_nor_probe(struct mtd_info_ex **spi_nor_info);
struct mtd_info_ex *fmc100_get_spi_nor_info(struct spi_flash *spi_nor_flash);

#endif

