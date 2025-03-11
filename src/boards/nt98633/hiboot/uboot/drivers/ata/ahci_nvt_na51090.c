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

static int ssc;
static int phy_txffepost1 = 0x4;//R-0x820[3:0]
static int phy_txffepost2 = 0x0;//R-0x820[5:4]
static int phy_txffepre   = 0x0;//R-0x820[8:6]
static int phy_txampboost = 0x1;//R-0x820[10:9]
static int phy_txampredu  = 0x0;//R-0x820[15:11]
static int phy_txslewadj  = 0x7;//R-0x820[18:16]
static int phy_losrefgendecoderbinin = 0x4;//LOS_REFGEN_DECODER_BIN_IN[15:12]

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
	u32 scontrol = 0;

	if (rxclkinv) {
		//nvt_sata_phy_write(0x11, (plat_data->top_va_base + 0x30));
		val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0));
		val |= (0x1 << 6);
		nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0));
	}

	nvt_sata_phy_write(0x40000000, (plat_data->top_va_base + 0x1C));

	nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x18));			/*mode0 */
	//nvt_sata_phy_write(0x80004C26, (plat_data->top_va_base + 0x18));	/*mode1 */
	//nvt_sata_phy_write(0x40004C26,  (plat_data->top_va_base + 0x18));	/*mode2 */

	if (nvt_sata_dma_opt) {
		nvt_sata_phy_write(0x100, (plat_data->top_va_base + 0x00));
	} else {
		nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x00));
	}

	/*
	if (nvt_sata_ddrabt[(pdev->id - 1)] != 0x0) {
		val = nvt_sata_phy_read((plat_data->top_va_base + 0x0C));
		val &= (~(0xF << 16));
		val |= (nvt_sata_ddrabt[(pdev->id -1)] << 16);
		nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0C));
	}
	*/

	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x20)) & ~(0xF << 20), (plat_data->top_va_base + 0x20));

	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x04)) & ~0x3, (plat_data->top_va_base + 0x04));
	msleep(5);

	// Release PHY_I2C_APB_RESETN
	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x04)) | 0x2, (plat_data->top_va_base + 0x04));
	msleep(5);

	scontrol = readl(plat_data->base + 0x12C);

	if ((scontrol & 0xF0) != 0x00) {
		nvt_sata_phy_write(0x04040000, (plat_data->phy_va_base + 0x850));
	}

	if ((scontrol & 0xF0) == 0x10) {
		// RATE_SET_1P5G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01430000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040008, (plat_data->phy_va_base + 0x850));
	}

	if ((scontrol & 0xF0) == 0x20) {
		// RATE_SET_3G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01420000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040004, (plat_data->phy_va_base + 0x850));
	}

	if ((scontrol & 0xF0) == 0x30) {
		// RATE_SET_6G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01410000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040000, (plat_data->phy_va_base + 0x850));
	}
	msleep(1);

	if (ssc) {
		//printk("TURN ON SSC\r\n");
		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x406C));
		val &= ~(0xFF << 0);
		val |= (0x79 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x406C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x40AC));
		val &= ~(0xFF << 0);
		val |= (0xC8 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40AC));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x40A8));
		val &= ~(0xFF << 0);
		val |= (0x0D << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40A8));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xC7 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xCF << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xC7 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));
	}

	nvt_sata_phy_write((0x06400000 + ((phy_txffepost1 & 0xF) << 0) +
			    ((phy_txffepost2 & 0x3) << 4) + ((phy_txffepre & 0x7) << 6) +
			    ((phy_txampboost & 0x3) << 9) + ((phy_txampredu & 0x1F) << 11) +
			    ((phy_txslewadj & 0x7) << 16)), (plat_data->phy_va_base + 0x820));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x834));
	val &= ~(0xF << 12);
	val |= (phy_losrefgendecoderbinin << 12);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x834));


	// BEST_SETTING
	if (plat_data->base != (void __iomem *) IOADDR_SATA2_REG_BASE) {
		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x91C));
		val &= ~(0xFFFF << 8);
		val |= (0x01F0 << 8);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x91C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0xF0));
		val &= ~(0xFFF << 16);
		val |= (0x3F << 16);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xF0));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x910));
		val &= ~(0xFFFFFF << 0);
		val |= (0x032060 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x910));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x914));
		val &= ~(0xFFFF << 0);
		val |= (0x3050 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x914));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0xA0));
		val |= (0x7F << 24);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xA0));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x918));
		val |= (0x1 << 15);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x918));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0xA4));
		val &= ~(0xFF  << 24);
		val |= (0x08 << 24);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xA4));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x0C));
		val &= ~(0xF  << 20);
		val |= (0xB << 20);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x0C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x00));
		val |= (0xFF << 16);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x00));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1008));
		val |= (0x1 << 5);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1008));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x874));
		val &= ~(0x1  << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x874));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x854));
		val &= ~(0xF  << 14);
		val |= (0x7 << 14);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x854));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x85C));
		val |= (0x1 << 23);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x85C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1084));
		val &= ~((0x3  << 1) | (0x3F << 16) | (0x3F << 24));
		val |= ((0x3  << 1) | (0x0F << 16) | (0x01 << 24));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1084));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1070));
		val &= ~(0xFF  << 0);
		val |= (0x81 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1070));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1068));
		val |= (0x1 << 3);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1068));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x210));
		val |= (0x3 << 18);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x210));
	} else {
		//PCIE COMBO PHY
		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0xA0));
		val |= (0x7F << 24);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xA0));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x0C));
		val &= ~(0xF  << 20);
		val |= (0xB << 20);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x0C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x00));
		val |= (0xFF << 16);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x00));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1008));
		val |= (0x1 << 5);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1008));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x874));
		val &= ~(0x1  << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x874));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x854));
		val &= ~(0xF  << 14);
		val |= (0x7 << 14);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x854));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1210));
		val &= ~(0x3  << 5);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1210));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1084));
		val &= ~((0x3  << 1) | (0x3F << 16) | (0x3F << 24));
		val |= ((0x3  << 1) | (0x0F << 16) | (0x01 << 24));
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1084));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1070));
		val &= ~(0xFF  << 0);
		val |= (0x81 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1070));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1068));
		val |= (0x1 << 3);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1068));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x210));
		val |= (0x3 << 18);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x210));
	}

	// Release PORn
	nvt_sata_phy_write((nvt_sata_phy_read(plat_data->top_va_base + 0x04) | 0x1), (plat_data->top_va_base + 0x04));
	msleep(10);

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
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 5),		IOADDR_CG_REG_BASE + 0x00);					//Enable SATA phy(pll5) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) & ~(1 << 10),		IOADDR_CG_REG_BASE + 0x98);					//Reset SATA0 clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) | (1 << 10),		IOADDR_CG_REG_BASE + 0x98);					//Release Reset SATA0 phy clk
		writel(readl(IOADDR_CG_REG_BASE + 0x74) | (1 << 19),		IOADDR_CG_REG_BASE + 0x74);					//Release SATA0 clk
		writel(readl(IOADDR_CG_REG_BASE + 0xE4) | (1 << 4),		IOADDR_CG_REG_BASE + 0xE4);					//Release SATA0 program clk
	} else if (plat_data->base == (void __iomem *) IOADDR_SATA1_REG_BASE) {
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 5),		IOADDR_CG_REG_BASE + 0x00);					//Enable SATA phy(pll5) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) & ~(1 << 11),		IOADDR_CG_REG_BASE + 0x98);					//Reset SATA1 clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) | (1 << 11),		IOADDR_CG_REG_BASE + 0x98);					//Release Reset SATA1 phy clk
		writel(readl(IOADDR_CG_REG_BASE + 0x74) | (1 << 20),		IOADDR_CG_REG_BASE + 0x74);					//Release SATA1 clk
		writel(readl(IOADDR_CG_REG_BASE + 0xE4) | (1 << 5),		IOADDR_CG_REG_BASE + 0xE4);					//Release SATA1 program clk
	} else if (plat_data->base == (void __iomem *) IOADDR_SATA2_REG_BASE) {
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 5),		IOADDR_CG_REG_BASE + 0x00);					//Enable SATA phy(pll5) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) & ~(1 << 12),		IOADDR_CG_REG_BASE + 0x98);					//Reset SATA2 clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) | (1 << 12),		IOADDR_CG_REG_BASE + 0x98);					//Release Reset SATA02 phy clk
		writel(readl(IOADDR_CG_REG_BASE + 0x74) | (1 << 21),		IOADDR_CG_REG_BASE + 0x74);					//Release SATA2 clk
		writel(readl(IOADDR_CG_REG_BASE + 0xE4) | (1 << 6),		IOADDR_CG_REG_BASE + 0xE4);					//Release SATA2 program clk
	} else {
		pr_err("Wrong base addr\r\n");
		return -1;
	}

	nvt_sata100_phy_parameter_setting(dev);

	return 0;
}