/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Novatek Inc.
 */

#ifndef AHCI_NVT_H
#define AHCI_NVT_H

#include <common.h>
#include <dm.h>

struct ahci_nvt_plat_data {
	void __iomem *base;
	void __iomem *top_va_base;
	void __iomem *phy_va_base;
};

int nvtsata_platform_init(struct udevice *dev);

#endif
