/*
 * Novatek IVOT USB HOST EHCI Controller
 *
 * Copyright (C) 2020 NOVATEK MICROELECTRONICS CORP
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
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <linux/libfdt.h>
#if defined(CONFIG_TARGET_NA51090_A64)
#include <asm/arch/efuse_protected.h>
#endif

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;
#define U2PHY_SETREG(ofs,value)         writel((value), (volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + ((ofs)<<2)))
#define U2PHY_GETREG(ofs)               readl((volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + ((ofs)<<2)))
#define DRV_VERSION                     "1.00.02"

static void nvtim_set_gpio_power(int index)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int gpio_pin, gpio_value, err;

	if (index) {
		sprintf(path,"/usb20host@2,%lx",(IOADDR_USB2_REG_BASE & 0xFFFFFFFF));
	} else {
		sprintf(path,"/usb20host@2,%lx",(IOADDR_USB_REG_BASE & 0xFFFFFFFF));
	}

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "power_gpio", NULL);

	if (cell > 0) {
		gpio_pin = __be32_to_cpu(cell[0]);
		gpio_value = __be32_to_cpu(cell[1]);

#if defined(CONFIG_TARGET_NA51090_A64)
		if ((gpio_pin >= J_GPIO(0)) && (gpio_pin <= J_GPIO(4))) {
			u32 pinmux_value;
			unsigned long tmpval;
			sprintf(path,"/top@%lx/misc",(IOADDR_TOP_REG_BASE & 0xFFFFFFFF));
			nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
			cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", NULL);
			pinmux_value = __be32_to_cpu(cell[0]);

			if ((pinmux_value & 0x1) == 0x0) {
				tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x04));
				tmpval &= ~(0x1 << 16);
				writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x04));

				tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0xA4));
				tmpval |= (0x1 << (gpio_pin & 0xF));
				writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0xA4));
			}
		}
#endif
		err = gpio_request(gpio_pin, "vbus");
		if (err) {
			printf("#### failed to request vbus, %d\n", gpio_pin);
			return;
		}

		gpio_direction_output(gpio_pin, gpio_value);
		gpio_free(gpio_pin);
	}
}


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
	unsigned long usbbase;
	unsigned long tmpval = 0;
#if defined(CONFIG_TARGET_NA51090_A64)
	u8 u2_trim_swctrl = 4, u2_trim_sqsel = 4, u2_trim_resint = 8, u3_trim_rint_sel = 8;
	u32 trim;
	u16 remap_data;
	int code;
	bool is_found;

	code = otp_key_manager(OTP_USB_TRIM_FIELD);
	if (code == -33) {
		printf("Read usb trim error\n");
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			remap_data = trim & 0xFFFF;
			u3_trim_rint_sel = remap_data & 0x1F;
			u2_trim_resint   = (remap_data >> 9) & 0x1F;
		}
	}

	usbbase = IOADDR_USB3PHY_REG_BASE;
	tmpval = readl((volatile unsigned long *)(usbbase+0x680));
	tmpval = 0x40 | (u3_trim_rint_sel);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x680));

	if (index)
		usbbase = IOADDR_USB2PHY_REG_BASE;
	else
		usbbase = IOADDR_USBPHY_REG_BASE;
	writel(0x20, (volatile unsigned long *)(usbbase+0x144));
	writel(0x30, (volatile unsigned long *)(usbbase+0x140));
	writel(0x60 + u2_trim_resint, (volatile unsigned long *)(usbbase+0x148));
	mdelay(5);
	writel(0x00, (volatile unsigned long *)(usbbase+0x144));
	mdelay(5);

	tmpval = readl((volatile unsigned long *)(usbbase+0x18));
	tmpval &= ~(0x7 << 1);
	tmpval |= (u2_trim_swctrl << 0x1);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x18));

	tmpval = readl((volatile unsigned long *)(usbbase+0x14));
	tmpval &= ~(0x7 << 0x2);
	tmpval |= (u2_trim_sqsel << 0x2);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x14));
#endif

	if (index) {
		usbbase = IOADDR_USB2_REG_BASE;

		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<16);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<16;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		/* Release sram shutdown*/
		tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
		tmpval &= ~(0x1<<8);
		writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

		/* Enable clock*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
		tmpval |= (0x1<<25);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	} else {
		usbbase = IOADDR_USB_REG_BASE;

		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<15);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<15;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		/* Release sram shutdown*/
		tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
		tmpval &= ~(0x1<<7);
		writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

		/* Enable clock*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
		tmpval |= (0x1<<24);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	}

	mdelay(10);

	/* Set USB ID & VBUSI */
	tmpval = readl((volatile unsigned long *)(usbbase+0x400));
	tmpval |= 0x1 << 20;
	writel(tmpval, (volatile unsigned long *)(usbbase+0x400));

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x100));
	tmpval &= ~(0x1<<9);
	#ifdef CONFIG_NVT_FPGA_EMULATION
	tmpval |=  (0x1<<1);
	#endif
	writel(tmpval, (volatile unsigned long *)(usbbase+0x100));

	/* Clear DEVPHY_SUSPEND[5] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C8));
	tmpval &= ~(0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C8));

	/* Clear HOSTPHY_SUSPEND[6] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x1<<6);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* USB_ACCESS_SELECT[2] to 0 (DRAM only) */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C4));
	tmpval &= ~(0x1<<2);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C4));

	/* Host EOF_BEHAVE[31] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<31);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x3<<4);
	tmpval |=  (0x3<<2);
	tmpval &= ~(0x3F<<8);
	tmpval |=  (0x22<<8);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* A_BUS_DROP[5] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	mdelay(2);

	/* A_BUS_REQ[4] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval |= (0x1<<4);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));
}
/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	nvtim_init_usbhc(index);

	if (index)
		*hccr = (struct ehci_hccr *)IOADDR_USB2_REG_BASE;
	else
		*hccr = (struct ehci_hccr *)IOADDR_USB_REG_BASE;

	*hcor = (struct ehci_hcor *)((unsigned long)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	nvtim_set_gpio_power(index);

	ehci_set_controller_priv(index, NULL, &nvt_ehci_ops);

	printf("nvtimusb 0x%lx %s\n", (unsigned long) *hccr, DRV_VERSION);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	unsigned long usbbase;
	unsigned long tmpval = 0;

	if (index)
		usbbase = IOADDR_USB2_REG_BASE;
	else
		usbbase = IOADDR_USB_REG_BASE;

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


#if defined(CONFIG_TARGET_NA51090_A64)
        if (index == 0) {
                /* Enable Reset*/
                tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
                tmpval &= ~(0x1<<15);
                writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

                udelay(10);

                /* Disable Reset*/
                tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
                tmpval |= 0x1<<15;
                writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
        } else {
                /* Enable Reset*/
                tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
                tmpval &= ~(0x1<<16);
                writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

                udelay(10);

                /* Disable Reset*/
                tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
                tmpval |= 0x1<<16;
                writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
        }
#else
	/* Enable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x84));
	tmpval &= ~(0x1<<19);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x84));

	udelay(10);

	/* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x84));
	tmpval |= 0x1<<19;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x84));

#endif
	return 0;
}

#else
static void nvtim_init_usbhc_dm(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	unsigned long usbbase;
	unsigned long tmpval = 0;
#if defined(CONFIG_TARGET_NA51090_A64)
	u8 u2_trim_swctrl = 4, u2_trim_sqsel = 4, u2_trim_resint = 8, u3_trim_rint_sel = 8;
	u32 trim;
	u16 remap_data;
	int code;
	bool is_found;

	code = otp_key_manager(OTP_USB_TRIM_FIELD);
	if(code == -33) {
		printf("Read usb trim error\n");
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			remap_data = trim & 0xFFFF;
			u3_trim_rint_sel = remap_data & 0x1F;
			u2_trim_resint   = (remap_data >> 9) & 0x1F;
		}
	}

	usbbase = IOADDR_USB3PHY_REG_BASE;
	tmpval = readl((volatile unsigned long *)(usbbase+0x680));
	tmpval = 0x40 | (u3_trim_rint_sel);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x680));

	if (index)
		usbbase = IOADDR_USB2PHY_REG_BASE;
	else
		usbbase = IOADDR_USBPHY_REG_BASE;
	writel(0x20, (volatile unsigned long *)(usbbase+0x144));
	writel(0x30, (volatile unsigned long *)(usbbase+0x140));
	writel(0x60 + u2_trim_resint, (volatile unsigned long *)(usbbase+0x148));
	mdelay(5);
	writel(0x00, (volatile unsigned long *)(usbbase+0x144));
	mdelay(5);

	tmpval = readl((volatile unsigned long *)(usbbase+0x18));
	tmpval &= ~(0x7 << 1);
	tmpval |= (u2_trim_swctrl << 0x1);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x18));

	tmpval = readl((volatile unsigned long *)(usbbase+0x14));
	tmpval &= ~(0x7 << 0x2);
	tmpval |= (u2_trim_sqsel << 0x2);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x14));
#endif

	if (index) {
		usbbase = IOADDR_USB2_REG_BASE;

		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<16);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<16;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		/* Release sram shutdown*/
		tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
		tmpval &= ~(0x1<<8);
		writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

		/* Enable clock*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
		tmpval |= (0x1<<25);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	} else {
		usbbase = IOADDR_USB_REG_BASE;

		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<15);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<15;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		/* Release sram shutdown*/
		tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
		tmpval &= ~(0x1<<7);
		writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

		/* Enable clock*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
		tmpval |= (0x1<<24);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	}

	mdelay(10);

	/* Set USB ID & VBUSI */
	tmpval = readl((volatile unsigned long *)(usbbase+0x400));
	tmpval |= 0x1 << 20;
	writel(tmpval, (volatile unsigned long *)(usbbase+0x400));

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x100));
	tmpval &= ~(0x1<<9);
	#ifdef CONFIG_NVT_FPGA_EMULATION
	tmpval |=  (0x1<<1);
	#endif
	writel(tmpval, (volatile unsigned long *)(usbbase+0x100));

	/* Clear DEVPHY_SUSPEND[5] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C8));
	tmpval &= ~(0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C8));

	/* Clear HOSTPHY_SUSPEND[6] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x1<<6);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* USB_ACCESS_SELECT[2] to 0 (DRAM only) */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C4));
	tmpval &= ~(0x1<<2);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C4));

	/* Host EOF_BEHAVE[31] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<31);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x3<<4);
	tmpval |=  (0x3<<2);
	tmpval &= ~(0x3F<<8);
	tmpval |=  (0x22<<8);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* A_BUS_DROP[5] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	mdelay(2);

	/* A_BUS_REQ[4] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval |= (0x1<<4);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	*hccr = (struct ehci_hccr *)usbbase;
	*hcor = (struct ehci_hcor *)((unsigned long)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct uboot_ehci_nvt_priv_data *ctx = dev_get_priv(dev);
	unsigned long base_addr = dev_read_addr(dev);
	int index;

	printf("0x%lx ver %s\n", base_addr, DRV_VERSION);

	if (base_addr == IOADDR_USB_REG_BASE)
		index = 0;
	else
		index = 1;

	nvtim_init_usbhc_dm(index, &ctx->hccr , &ctx->hcor);

	nvtim_set_gpio_power(index);

	return ehci_register(dev, ctx->hccr, ctx->hcor, &nvt_ehci_ops, 0, USB_INIT_HOST);
}

static int ehci_usb_remove(struct udevice *dev)
{
	int ret;
	unsigned long usbbase;
	unsigned long tmpval = 0;
	unsigned long base_addr = dev_read_addr(dev);

	usbbase = base_addr;


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

#if defined(CONFIG_TARGET_NA51090_A64)
	if (usbbase == IOADDR_USB_REG_BASE) {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<15);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<15;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	} else {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<16);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<16;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	}
#else
	/* Enable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval &= ~(0x1<<15);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	udelay(10);

	/* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval |= 0x1<<15;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));


#endif
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
