/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Novatek Inc.
 */

#include "ahci_nvt.h"
#include <ahci.h>

/*
 * Dummy implementation that can be overwritten by a board
 * specific function
 */
__weak int board_ahci_enable(void)
{
	return 0;
}

static int nvt_ahci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;
	int ret;

	ret = ahci_bind_scsi(dev, &scsi_dev);
	if (ret) {
		printf("%s: Failed to bind (err=%d\n)", __func__, ret);
		return ret;
	}

	return 0;
}

static int nvt_ahci_ofdata_to_platdata(struct udevice *dev)
{
	struct ahci_nvt_plat_data *plat_data = dev_get_platdata(dev);

	plat_data->base = (void __iomem *)devfdt_get_addr_index(dev, 0);
	if (IS_ERR(plat_data->base)) {
		return PTR_ERR(plat_data->base);
	}

	plat_data->top_va_base = (void __iomem *)devfdt_get_addr_index(dev, 1);
	if (IS_ERR(plat_data->top_va_base)) {
		return PTR_ERR(plat_data->top_va_base);
	}

	plat_data->phy_va_base = (void __iomem *)devfdt_get_addr_index(dev, 2);
	if (IS_ERR(plat_data->phy_va_base)) {
		return PTR_ERR(plat_data->phy_va_base);
	}

	printf("%s: base=0x%lx, top_va_base=0x%lx, phy_va_base=0x%lx\r\n", __func__, (uintptr_t)plat_data->base, (uintptr_t)plat_data->top_va_base, (uintptr_t)plat_data->phy_va_base);

	return 0;
}

static int nvt_ahci_probe(struct udevice *dev)
{
	/*
	 * Board specific SATA / AHCI enable code, e.g. enable the
	 * AHCI power or deassert reset
	 */
	board_ahci_enable();

	nvtsata_platform_init(dev);

	ahci_probe_scsi(dev, (ulong)devfdt_get_addr_ptr(dev));

	return 0;
}

static const struct udevice_id nvt_ahci_ids[] = {
	{ .compatible = "novatech,nvt_sata100" },
	{ }
};

U_BOOT_DRIVER(ahci_nvt_drv) = {
	.name		= "ahci_nvt",
	.id		= UCLASS_AHCI,
	.of_match	= nvt_ahci_ids,
	.bind		= nvt_ahci_bind,
	.ofdata_to_platdata = nvt_ahci_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct ahci_nvt_plat_data),
	.probe		= nvt_ahci_probe,
};
