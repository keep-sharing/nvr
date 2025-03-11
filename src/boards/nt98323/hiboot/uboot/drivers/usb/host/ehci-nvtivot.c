/*
 * SAMSUNG EXYNOS USB HOST EHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <usb.h>
#include <asm/io.h>
#include "ehci.h"
#include <asm/arch/IOAddress.h>

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;
#define U2PHY_SETREG(ofs,value)  writel((value), (volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + ofs))
#define U2PHY_GETREG(ofs)        readl((volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + ofs))
#define U2PHY2_SETREG(ofs,value) writel((value), (volatile void __iomem *)(IOADDR_USB2PHY_REG_BASE + ofs))
#define U2PHY2_GETREG(ofs)       readl((volatile void __iomem *)(IOADDR_USB2PHY_REG_BASE + ofs))

#ifdef CONFIG_DM_USB
struct uboot_ehci_nvt_priv_data{
	struct ehci_ctrl ctrl; /* Needed by EHCI */
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
};

static int ehci_usb_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}
#endif

static int nvt_get_port_speed(struct ehci_ctrl *ctrl, uint32_t reg)
{
	unsigned tmp = ehci_readl(((void *)ctrl->hcor) + 0x70);

	switch ((tmp >> 22) & 3) {
		case 0:
			return PORTSC_PSPD_FS;
		case 1:
			return PORTSC_PSPD_LS;
		case 2:
		default:
			return PORTSC_PSPD_HS;
	}
}

const struct ehci_ops nvt_ehci_ops = {
	.set_usb_mode		= NULL,
	.get_port_speed		= nvt_get_port_speed,
	.powerup_fixup		= NULL,
	.get_portsc_register	= NULL,
};

#ifndef CONFIG_DM_USB
static void nvtim_init_usbhc(int index)
{
	u32 usbbase;
	u32 tmpval = 0;
	static int do_once = 0;

	if (!do_once) {
		/* Reset PHY*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval &= ~(0x3<<4);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x3<<4;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		do_once++;
	}

	if (index == 0) {
		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x1<<4;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));
		tmpval |= 0x1<<12;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));

		mdelay(10);

		/* Enable Bus Clk*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));
		tmpval &= ~(0x1<<4);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));
		tmpval &= ~(0x1<<12);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));

		usbbase = IOADDR_USB_REG_BASE;
	} else {
		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x1<<5;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));
		tmpval |= 0x1<<13;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));

		mdelay(10);

		/* Enable Bus Clk*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));
		tmpval &= ~(0x1<<5);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));
		tmpval &= ~(0x1<<13);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));

		usbbase = IOADDR_USB2_REG_BASE;
	}

	/* Set INT_POLARITY */
	tmpval = readl((volatile unsigned long *)(usbbase+0xC4));
	tmpval |= (0x1 << 0x3);
	writel(tmpval, (volatile unsigned long *)(usbbase+0xC4));

	/* Clear DEVPHY_SUSPEND[5] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C8));
	tmpval &= ~(0x1<<31);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C8));

	/* Set USB ID & VBUSI */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
	tmpval &= ~0xFF;
	tmpval |= 0x22;
	tmpval |= (0x1<<20);
	tmpval &= ~(0x1<<21);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x100));
	tmpval &= ~(0x1<<9);
	#ifdef CONFIG_FPGA_EMULATION
	tmpval |=  (0x1<<1);
	#endif
	writel(tmpval, (volatile unsigned long *)(usbbase+0x100));

	/* Clear HOSTPHY_SUSPEND[6] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
	tmpval &= ~(0x1<<20);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));

	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval |= (0x1<<6);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	mdelay(1);

	tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));

	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x3F << 16);
	tmpval |=  (0x22 << 16);
	tmpval &= ~(0x1<<6);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	mdelay(25);

	/* A_BUS_DROP[5] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	udelay(2);

	/* A_BUS_REQ[4] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval |= (0x1<<4);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x3 << 4);
	tmpval &= ~(0x3 << 2);
	tmpval |=  (0x3 << 2);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* Configure PHY related settings below */
	{
		u8 u2_trim_swctrl=6, u2_trim_sqsel=5, u2_trim_resint=8;

		if (index == 0) {
			U2PHY_SETREG(0x144, 0x20);
			U2PHY_SETREG(0x140, 0x30);
			U2PHY_SETREG(0x148, 0x60 + u2_trim_resint);
			mdelay(5);
			U2PHY_SETREG(0x144, 0x00);
			mdelay(5);

			tmpval = U2PHY_GETREG(0x18);
			tmpval &= ~(0x7<<1);
			tmpval |= (u2_trim_swctrl << 0x1);
			U2PHY_SETREG(0x18, tmpval);

			tmpval = U2PHY_GETREG(0x14);
			tmpval &= ~(0x7 << 0x2);
			tmpval |= (u2_trim_sqsel << 0x2);
			U2PHY_SETREG(0x14, tmpval);
		} else {
			tmpval = U2PHY2_GETREG(0x18);
			tmpval &= ~(0x7<<1);
			tmpval |= (u2_trim_swctrl << 0x1);
			U2PHY2_SETREG(0x18, tmpval);

			tmpval = U2PHY2_GETREG(0x14);
			tmpval &= ~(0x7 << 0x2);
			tmpval |= (u2_trim_sqsel << 0x2);
			U2PHY2_SETREG(0x14, tmpval);
		}

		tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
		tmpval &= ~(0x1F << 8);
		tmpval |= (u2_trim_resint << 8);
		tmpval |= (0x1 << 15);
		writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));
	}
}
/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	nvtim_init_usbhc(index);

	if (index == 0)
		*hccr = (struct ehci_hccr *)IOADDR_USB_REG_BASE;
	else
		*hccr = (struct ehci_hccr *)IOADDR_USB2_REG_BASE;

	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	ehci_set_controller_priv(index, NULL, &nvt_ehci_ops);
	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	u32 usbbase;
	u32 tmpval = 0;

	if (index == 0)
		usbbase = IOADDR_USB_REG_BASE;
	else
		usbbase = IOADDR_USB2_REG_BASE;

	/* Host Reset */
	writel((readl((volatile unsigned long *)(usbbase+0x10)) | 0x2), (volatile unsigned long *)(usbbase+0x10));

	/* A_BUS_REQ[4] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<4);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	/* A_BUS_DROP[5] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval |= (0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));


	if (index == 0) {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval &= ~(0x1<<4);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x1<<4;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
	} else {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval &= ~(0x1<<5);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x1<<5;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
	}

	return 0;
}

#else
static void nvtim_init_usbhc_dm(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	u32 usbbase;
	u32 tmpval = 0;
	static int do_once = 0;

	if (!do_once) {
		/* Reset PHY*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval &= ~(0x3<<4);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x3<<4;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		do_once++;
	}

	if (index == 0) {
		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x1<<4;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));
		tmpval |= 0x1<<12;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));

		mdelay(10);

		/* Enable Bus Clk*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));
		tmpval &= ~(0x1<<4);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));
		tmpval &= ~(0x1<<12);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));

		usbbase = IOADDR_USB_REG_BASE;
	} else {
		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));
		tmpval |= 0x1<<5;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x54));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));
		tmpval |= 0x1<<13;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x58));

		mdelay(10);

		/* Enable Bus Clk*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));
		tmpval &= ~(0x1<<5);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x68));

		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));
		tmpval &= ~(0x1<<13);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x70));

		usbbase = IOADDR_USB2_REG_BASE;
	}

	/* Set INT_POLARITY */
	tmpval = readl((volatile unsigned long *)(usbbase+0xC4));
	tmpval |= (0x1 << 0x3);
	writel(tmpval, (volatile unsigned long *)(usbbase+0xC4));

	/* Clear DEVPHY_SUSPEND[5] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C8));
	tmpval &= ~(0x1<<31);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C8));

	/* Set USB ID & VBUSI */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
	tmpval &= ~0xFF;
	tmpval |= 0x22;
	tmpval |= (0x1<<20);
	tmpval &= ~(0x1<<21);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x100));
	tmpval &= ~(0x1<<9);
	#ifdef CONFIG_FPGA_EMULATION
	tmpval |=  (0x1<<1);
	#endif
	writel(tmpval, (volatile unsigned long *)(usbbase+0x100));

	/* Clear HOSTPHY_SUSPEND[6] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
	tmpval &= ~(0x1<<20);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));

	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval |= (0x1<<6);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	mdelay(1);

	tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));

	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x3F << 16);
	tmpval |=  (0x22 << 16);
	tmpval &= ~(0x1<<6);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	mdelay(25);

	/* A_BUS_DROP[5] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	udelay(2);

	/* A_BUS_REQ[4] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval |= (0x1<<4);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x3 << 4);
	tmpval &= ~(0x3 << 2);
	tmpval |=  (0x3 << 2);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* Configure PHY related settings below */
	{
		u8 u2_trim_swctrl=6, u2_trim_sqsel=5, u2_trim_resint=8;

		if (index == 0) {
			U2PHY_SETREG(0x144, 0x20);
			U2PHY_SETREG(0x140, 0x30);
			U2PHY_SETREG(0x148, 0x60 + u2_trim_resint);
			mdelay(5);
			U2PHY_SETREG(0x144, 0x00);
			mdelay(5);

			tmpval = U2PHY_GETREG(0x18);
			tmpval &= ~(0x7<<1);
			tmpval |= (u2_trim_swctrl << 0x1);
			U2PHY_SETREG(0x18, tmpval);

			tmpval = U2PHY_GETREG(0x14);
			tmpval &= ~(0x7 << 0x2);
			tmpval |= (u2_trim_sqsel << 0x2);
			U2PHY_SETREG(0x14, tmpval);
		} else {
			tmpval = U2PHY2_GETREG(0x18);
			tmpval &= ~(0x7<<1);
			tmpval |= (u2_trim_swctrl << 0x1);
			U2PHY2_SETREG(0x18, tmpval);

			tmpval = U2PHY2_GETREG(0x14);
			tmpval &= ~(0x7 << 0x2);
			tmpval |= (u2_trim_sqsel << 0x2);
			U2PHY2_SETREG(0x14, tmpval);
		}

		tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
		tmpval &= ~(0x1F << 8);
		tmpval |= (u2_trim_resint << 8);
		tmpval |= (0x1 << 15);
		writel(tmpval, (volatile unsigned long *)(usbbase+0x1D8));
	}

	*hccr = (struct ehci_hccr *)usbbase;
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct uboot_ehci_nvt_priv_data *ctx = dev_get_priv(dev);
	u32 base_addr = (u32) dev_read_addr(dev);
	int index;

	if (base_addr == IOADDR_USB_REG_BASE)
		index = 0;
	else
		index = 1;

	nvtim_init_usbhc_dm(index, &ctx->hccr , &ctx->hcor);

	return ehci_register(dev, ctx->hccr, ctx->hcor, &nvt_ehci_ops, 0, USB_INIT_HOST);
}

static int ehci_usb_remove(struct udevice *dev)
{
	int ret;

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "nvt,ehci-nvtivot" },
	{ }
};

U_BOOT_DRIVER(usb_ehci) = {
	.name	= "ehci_nvt",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.ofdata_to_platdata = ehci_usb_ofdata_to_platdata,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct uboot_ehci_nvt_priv_data),
//	.platdata_auto_alloc_size = sizeof(struct exynos_ehci_platdata),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
#endif