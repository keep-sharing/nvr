/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Novatek Inc.
 */

#include "ahci_nvt.h"
#include <asm/arch/IOAddress.h>
#include <asm/arch/efuse_protected.h>

#define msleep(a) udelay(a * 1000)

static int nvt_sata_tune_en = 0;
static int rxclkinv;
static int nvt_sata_dma_opt = 0;
//static int nvt_ahci_hflag = AHCI_HFLAG_YES_NCQ;// | AHCI_HFLAG_YES_FBS;// // 0x4000
//static int nvt_sata_ddrabt[4] = {[0 ...(3)] = 0x0};

static void nvt_sata_phy_write(u32 val, void __iomem *phy_reg)
{
	if (nvt_sata_tune_en != 0) {
		printk("write phyreg:0x%lx val:0x%08x\r\n", (uintptr_t)(void __iomem *)phy_reg, val);
	}
	writel(val, phy_reg);
}

static u32 nvt_sata_phy_read(void __iomem *phy_reg)
{
	u32 val = 0;

	val = readl(phy_reg);
	if (nvt_sata_tune_en != 0) {
		printk("read phyreg:0x%lx val:0x%08x\r\n", (uintptr_t)(void __iomem *) phy_reg, val);
	}
	return val;
}

static inline void nvt_sata100_phy_parameter_setting(struct udevice *dev)
{
	struct ahci_nvt_plat_data *plat_data = dev_get_platdata(dev);
	u32 val = 0;

	if (rxclkinv) {
		nvt_sata_phy_write(0x11, (plat_data->top_va_base + 0x30));
	}

	nvt_sata_phy_read((plat_data->top_va_base + 0x30));

	val =  readl((IOADDR_CG_REG_BASE + 0x9c));
	val &= ~(0x1 << 5);
	writel(val, (IOADDR_CG_REG_BASE + 0x9c));

	msleep(1);

	val =  readl((IOADDR_CG_REG_BASE + 0x9c));
	val &= ~(0x1 << 5);
	val |= (0x1 << 5);
	writel(val, (IOADDR_CG_REG_BASE + 0x9c));

	writel(0x55, IOADDR_CG_REG_BASE + 0x4920);
	writel(0x55, IOADDR_CG_REG_BASE + 0x4924);
	writel(0x53, IOADDR_CG_REG_BASE + 0x4928);

	writel(0x55, IOADDR_CG_REG_BASE + 0x49a0);
	writel(0x55, IOADDR_CG_REG_BASE + 0x49a4);
	writel(0x53, IOADDR_CG_REG_BASE + 0x49a8);

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x225c));
	val &= ~(0x3 << 0);
	val |= (0x3 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x225c));

	val =  nvt_sata_phy_read((void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x00));
	val &= ~(0x1 << 20);
	val |= (0x1 << 20);
	nvt_sata_phy_write(val, (void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x00));

	val =  nvt_sata_phy_read((void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x00));
	val &= ~(0x1 << 22);
	val |= (0x1 << 22);
	nvt_sata_phy_write(val, (void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x00));

	val =  nvt_sata_phy_read((void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x00));
	val &= ~(0x1 << 13);
	val |= (0x1 << 13);
	nvt_sata_phy_write(val, (void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x00));

	val =  nvt_sata_phy_read((void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x140));
	val &= ~(0x1 << 6);
	val |= (0x1 << 6);
	nvt_sata_phy_write(val, (void __iomem *)(IOADDR_STBC_CG_REG_BASE + 0x140));

	//#Tx OOB waveform modify
	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2160));
	val &= ~(0x1 << 2);
	val |= (0x1 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2160));


	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0xff, (plat_data->phy_va_base + 0x20d0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2260));
	val &= ~(0x1 << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2260));

	nvt_sata_phy_write(0xca, (plat_data->phy_va_base + 0x2050));

	// SATA DPHY setting =============================
	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0x09, (plat_data->phy_va_base + 0x218c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cc8));
	val &= ~(0x1 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cc8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cc4));
	val &= ~(0x1 << 7);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cc4));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cc0));
	val &= ~(0x1 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cc0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cf8));
	val &= ~(0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cf8));

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0xc0, (plat_data->phy_va_base + 0x2944));
	nvt_sata_phy_write(0x88, (plat_data->phy_va_base + 0x294c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x29f0));
	val &= ~(0x1 << 1);
	val |= (0x1 << 1);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x29f0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x29cc));
	val &= ~(0xf << 0);
	val |= (0xf << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x29cc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2504));
	val &= ~(0x1 << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2504));

	nvt_sata_phy_write(0x08, (plat_data->phy_va_base + 0x21e8));

	val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0004));
	val &= ~(0x1 << 17);
	val |= (0x1 << 17);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0004));

	val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0004));
	val &= ~(0x1F << 12);
	val |= (0x00 << 12);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0004));

#if 0
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2008));
	val &= ~(0x3 << 29);
	val |= (0x3 << 29);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2008));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2008));
	val &= ~(0x1 << 14);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2008));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2008));
	val &= ~(0x1 << 13);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2008));
#endif

	val =  nvt_sata_phy_read((void __iomem *)(IOADDR_CG_REG_BASE + 0x94));
	val &= ~(0x1 << 26);
	nvt_sata_phy_write(val, (void __iomem *)(IOADDR_CG_REG_BASE + 0x94));

	val =  nvt_sata_phy_read((void __iomem *)(IOADDR_CG_REG_BASE + 0x94));
	val &= ~(0x1 << 26);
	val |= (0x1 << 26);
	nvt_sata_phy_write(val, (void __iomem *)(IOADDR_CG_REG_BASE + 0x94));

#if 0
	val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0004));
	val &= ~(0x1 << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0004));

	val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0004));
	val &= ~(0x1 << 2);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0004));

	val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0004));
	val &= ~(0x1 << 2);
	val |= (0x1 << 2);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0004));
#endif

	val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0040));
	val &= ~(0x1 << 16);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0040));

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x29ec));
	val &= ~(0xF << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x29ec));

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2d08));
	val &= ~(0x1 << 1);
	val |= (0x1 << 1);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2d08));

	nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2168));
	val &= ~(0x3 << 4);
	val |= (0x2 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2168));

	// USB30 SATA PHY common setting =============================
	nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2410));
	val &= ~(0x1 << 4);
	val |= (0x1 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2410));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2410));
	val &= ~(0x1 << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2410));

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0xff, (plat_data->phy_va_base + 0x2524));
	nvt_sata_phy_write(0xff, (plat_data->phy_va_base + 0x2514));
	nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x25a0));
	val &= ~(0x1 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x25a0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x25a0));
	val &= ~(0x1 << 1);
	val |= (0x1 << 1);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x25a0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x25a0));
	val &= ~(0x1 << 2);
	val |= (0x1 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x25a0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x25a0));
	val &= ~(0x1 << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x25a0));

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2420));
	val &= ~(0x1 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2420));

	nvt_sata_phy_write(0x5f, (plat_data->phy_va_base + 0x2680));
	nvt_sata_phy_write(0xf4, (plat_data->phy_va_base + 0x2688));

	// HDMI　DPHY RX  setting =============================
	nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0xff, (plat_data->phy_va_base + 0x2578));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2088));
	val &= ~(0x1 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2088));

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x25b0));
	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x25c0));
	nvt_sata_phy_write(0xb9, (plat_data->phy_va_base + 0x25d0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x208c));
	val &= ~(0xF << 4);
	val |= (0x2 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x208c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x208c));
	val &= ~(0xF << 0);
	val |= (0x3 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x208c));

	nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2694));
	val &= ~(0x3 << 4);
	val |= (0x1 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2694));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2694));
	val &= ~(0x7 << 0);
	val |= (0x4 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2694));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2168));
	val &= ~(0x1 << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2168));

	nvt_sata_phy_write(0xc0, (plat_data->phy_va_base + 0x22e4));

	// HDMI　APHY RX  setting =============================
	nvt_sata_phy_write(0x02, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0xf7, (plat_data->phy_va_base + 0x20a0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x202c));
	val &= ~(0x3 << 0);
	val |= (0x3 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x202c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x200c));
	val &= ~(0x7 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x200c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2008));
	val &= ~(0x1 << 7);
	val |= (0x1 << 7);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2008));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x200c));
	val &= ~(0xF << 4);
	val |= (0x2 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x200c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2078));
	val &= ~(0x1 << 4);
	val |= (0x1 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2078));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2078));
	val &= ~(0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2078));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2078));
	val &= ~(0x1 << 6);
	val |= (0x1 << 6);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2078));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x20cc));
	val &= ~(0x1 << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x20cc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x20cc));
	val &= ~(0x1 << 7);
	val |= (0x1 << 7);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x20cc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2028));
	val &= ~(0xF << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2028));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x202c));
	val &= ~(0x3 << 2);
	val |= (0x2 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x202c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2024));
	val &= ~(0x7 << 4);
	val |= (0x1 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2024));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2004));
	val &= ~(0xF << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2004));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x207c));
	val &= ~(0x7 << 4);
	val |= (0x7 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x207c));


	//SATA APHY Setting=============================
	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x21a8));
	val &= ~(0x1 << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x21a8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x21a8));
	val &= ~(0x1 << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x21a8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2194));
	val &= ~(0x7 << 5);
	val |= (0x4 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2194));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2198));
	val &= ~(0x7 << 5);
	val |= (0x2 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2198));

	// SATA DPHY setting =============================
	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0x09, (plat_data->phy_va_base + 0x218c));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cc8));
	val &= ~(0x1 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cc8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cc4));
	val &= ~(0x1 << 7);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cc4));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cc0));
	val &= ~(0x1 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cc0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2cf8));
	val &= ~(0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2cf8));

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));
	nvt_sata_phy_write(0x74, (plat_data->phy_va_base + 0x2158));
	nvt_sata_phy_write(0xF8, (plat_data->phy_va_base + 0x2048));
	nvt_sata_phy_write(0x8a, (plat_data->phy_va_base + 0x2050));

	nvt_sata_phy_write(0xfc, (plat_data->phy_va_base + 0x20d0));
	nvt_sata_phy_write(0x03, (plat_data->phy_va_base + 0x2450));
	nvt_sata_phy_write(0xf0, (plat_data->phy_va_base + 0x2408));

	//Apply trim data
	u32  trim;
	int  code;
	BOOL is_found;
	UINT32 internal_12k_trim = 0x8, tx_trim = 0x6, rx_trim = 0xc;

    u32 USB_SATA_Internal_12K_resistor;
    u32 USB_SATA_TX_TRIM, USB_SATA_RX_TRIM;
    code =  otp_key_manager(EFUSE_USB_SATA_TRIM_DATA);
    if (code == -33) {
        printf("[%d] data = NULL\n", EFUSE_USB_SATA_TRIM_DATA);
    } else {
        is_found = extract_trim_valid(code, (u32 *)&trim);
        if (is_found) {
            printf("  USB(SATA) Trim data = 0x%04x\r\n", (int)trim);
			USB_SATA_Internal_12K_resistor = trim & 0x1F;
			USB_SATA_TX_TRIM = ((trim >> 5) & 0xF);
			USB_SATA_RX_TRIM = ((trim >> 9) & 0xF);

			//12K resistor 1<= x <= 20(0x14)
            if (USB_SATA_Internal_12K_resistor > 0x14) {
				printf("12K_resistor Trim data error 0x%04x > 20(0x14)\r\n", (int)USB_SATA_Internal_12K_resistor);
            } else if (USB_SATA_Internal_12K_resistor < 0x1) {
				printf("12K_resistor Trim data error 0x%04x < 1(0x1)\r\n", (int)USB_SATA_Internal_12K_resistor);
            } else {
				printf("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB_SATA_Internal_12K_resistor);
				internal_12k_trim = USB_SATA_Internal_12K_resistor;
            }
            //TX term bit[8..5] 0x2 <= x <= 0xE
            if (USB_SATA_TX_TRIM > 0xE) {
                printf("USB(SATA) TX Trim data error 0x%04x > 0xE\r\n", (int)USB_SATA_TX_TRIM);
            } else if (USB_SATA_TX_TRIM < 0x2) {
                printf("USB(SATA) TX Trim data error 0x%04x < 0x2\r\n", (int)USB_SATA_TX_TRIM);
            } else {
                printf("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_TX_TRIM);
				tx_trim = USB_SATA_TX_TRIM;
            }
            //RX term bit[12..9] 0x8 <= x <= 0xE
            if (USB_SATA_RX_TRIM > 0xE) {
                printf("USB(SATA) RX Trim data error 0x%04x > 0xE\r\n", (int)USB_SATA_RX_TRIM);
            } else if (USB_SATA_RX_TRIM < 0x8) {
                printf("USB(SATA) RX Trim data error 0x%04x < 0x8\r\n", (int)USB_SATA_RX_TRIM);
            } else {
				printf("  *USB(SATA) RX Trim data 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_RX_TRIM);
				rx_trim = USB_SATA_RX_TRIM;
            }
        } else {
            //!!!Please apply default value here!!!
            printf("is found [%d][USB trim] = 0x%08x\r\n", is_found, (int)code);
        }
    }

	// RX trim value
	nvt_sata_phy_write(0x02, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2000));
	val &= ~(0xF << 0);
	val |= (rx_trim << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2000));

	//Tx trim value
	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2040));
	val &= ~(0xF << 4);
	val |= (tx_trim << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2040));

	//12k trim value
	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x23fc));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2680));
	val &= ~(0xF << 0);
	val |= (internal_12k_trim << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2680));

	msleep(1000);

	nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x2408));

	//nvt_sata_phy_write(0x40000000, (plat_data->top_va_base + 0x1C));

	//nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x18));			/*mode0 */
	//nvt_sata_phy_write(0x80004C26, (plat_data->top_va_base + 0x18));	/*mode1 */
	nvt_sata_phy_write(0x40004C26,  (plat_data->top_va_base + 0x18));	/*mode2 */

	if (nvt_sata_dma_opt) {
		nvt_sata_phy_write(0x100, (plat_data->top_va_base + 0x00));
	} else {
		nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x00));
	}
	printk("nvt_sata100_phy_parameter_setting\r\n");

	return;
}

int nvtsata_platform_init(struct udevice *dev)
{
	struct ahci_nvt_plat_data *plat_data = dev_get_platdata(dev);

	if (!plat_data->base) {
		pr_err("Error base addr\r\n");
		return -1;
	}

	if (plat_data->base == (void __iomem *) IOADDR_SATA0_REG_BASE) {
		writel(readl(IOADDR_STBC_CG_REG_BASE + 0x00) | (1 << 22),			IOADDR_STBC_CG_REG_BASE + 0x00);	//Enable SATA phy(pll22) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x9C) & ~(1 << 5),				IOADDR_CG_REG_BASE + 0x9C);			//Reset SATA0 phy clk
		writel(readl(IOADDR_CG_REG_BASE + 0x9C) | (1 << 5),					IOADDR_CG_REG_BASE + 0x9C);			//Release Reset SATA0 phy clk
		writel(readl(IOADDR_STBC_CG_REG_BASE + 0x00) | (1 << 13),			IOADDR_STBC_CG_REG_BASE + 0x00);	//Enable SATA  clk source(axi2) clk
		writel(readl(IOADDR_STBC_CG_REG_BASE + 0x140) | (1 << 6),			IOADDR_STBC_CG_REG_BASE + 0x140);	//Enable SATA  clk source(axi2) clk
		writel(readl(IOADDR_STBC_CG_REG_BASE + 0x14C) | (1 << 6),			IOADDR_STBC_CG_REG_BASE + 0x14C);	//Enable SATA  clk source(axi2) clk
	} else {
		pr_err("Wrong base addr\r\n");
		return -1;
	}

	nvt_sata100_phy_parameter_setting(dev);

	return 0;
}
