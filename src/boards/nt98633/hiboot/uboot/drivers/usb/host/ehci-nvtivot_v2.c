/*
 * NVT USB HOST EHCI Controller V2 (Project: 331)
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
#include <asm/arch/hardware.h>

#include <asm/gpio.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <linux/libfdt.h>
#include <asm/arch/efuse_protected.h>
#include "ehci_nvtivot_reg_v2.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

#define DRV_VERSION                     "1.00.01"

static REGVALUE usb_getreg(u32 usbbase, u32 offset)
{
	return readl((volatile unsigned long *)(usbbase + offset));
}

static void usb_setreg(u32 usbbase, u32 offset, REGVALUE value)
{
	writel(value, (volatile unsigned long *)(usbbase + offset));
}


static void nvtim_set_gpio_power(int index)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int gpio_pin, gpio_value, err;

	if (index) {
		sprintf(path,"/usb20host@%lx",(IOADDR_USB2_REG_BASE & 0xFFFFFFFF));
	} else {
		sprintf(path,"/usb20host@%lx",(IOADDR_USB_REG_BASE & 0xFFFFFFFF));
	}

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "power_gpio", NULL);

	if (cell > 0) {
		gpio_pin = __be32_to_cpu(cell[0]);
		gpio_value = __be32_to_cpu(cell[1]);

		err = gpio_request(gpio_pin, "vbus");
		if (err) {
			printf("#### failed to request vbus, %d\n", gpio_pin);
			return;
		}

		gpio_direction_output(gpio_pin, gpio_value);
		gpio_free(gpio_pin);
	} else {
		printf("no power gpio found\n");
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
#define TRIM_RESINT_UPPER 0x14
#define TRIM_RESINT_LOWER 0x1
static void nvtim_init_usbhc(int index)
{

	u32 usbbase;
	u32 tmpval = 0;

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

	/* Set INT_POLARITY */
	union USB_GLOBALINTMASK_REG usbgintmskreg;
	usbgintmskreg.reg = usb_getreg(usbbase, USB_GLOBALINTMASK_REG_OFS);
	usbgintmskreg.bit.INT_POLARITY = 1;
	usb_setreg(usbbase, USB_GLOBALINTMASK_REG_OFS, usbgintmskreg.reg);

	/* Clear DEVPHY_SUSPEND[5] */
	union USB_DEVDMACTRL1_REG usbctrl1reg;
	usbctrl1reg.reg = usb_getreg(usbbase, USB_DEVDMACTRL1_REG_OFS);
	usbctrl1reg.bit.DEVPHY_SUSPEND = 0;
	usb_setreg(usbbase, USB_DEVDMACTRL1_REG_OFS, usbctrl1reg.reg);

	/* Set USB ID & VBUSI */
	union USB_PHYTOP_REG usbphytopreg;
	usbphytopreg.reg &= ~0xFF;
	usbphytopreg.reg |= 0x22;
	usbphytopreg.bit.USB_VBUSI = 1;
	usbphytopreg.bit.USB_ID = 0;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	union USB_DEVMAINCTRL_REG devmainctrlreg;
	devmainctrlreg.reg = usb_getreg(usbbase, USB_DEVMAINCTRL_REG_OFS);
	devmainctrlreg.bit.FORCE_FS = 0;
#ifdef CONFIG_FPGA_EMULATION
	devmainctrlreg.bit.HALF_SPEED = 1;
#endif
	usb_setreg(usbbase, USB_DEVMAINCTRL_REG_OFS, devmainctrlreg.reg);

	/* Clear HOSTPHY_SUSPEND[6] */
	usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
	usbphytopreg.bit.USB_VBUSI = 0;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);


	union USB_HCMISC_REG usbhcmiscreg;
	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.bit.HostPhy_Suspend = 1;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	mdelay(1);

	usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
	usbphytopreg.bit.USB_VBUSI = 1;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);

	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.reg &= ~(0x3F << 16);
	usbhcmiscreg.reg |=  (0x22 << 16);
	usbhcmiscreg.bit.HostPhy_Suspend = 0;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	mdelay(25);

	/* A_BUS_DROP[5] = 0 */
	union USB_OTGCTRLSTATUS_REG otgctrlstsreg;
	otgctrlstsreg.reg = usb_getreg(usbbase,USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_DROP = 0;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	udelay(2);

	/* A_BUS_REQ[4] = 1 */
        otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
        otgctrlstsreg.bit.A_BUS_REQ = 1;
        usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.bit.EOF2_Time = 0;
	usbhcmiscreg.bit.EOF1_Time = 0;
	usbhcmiscreg.bit.EOF1_Time = 0x3;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	/* Configure PHY related settings below */
	{
		u8 u2_trim_swctrl=4, u2_trim_sqsel=4, u2_trim_resint=8;
		u32 trim;
		int code;
		bool is_found;

		// get trim value of usb column
		code = otp_key_manager(OTP_USB_SET1_TRIM_FIELD);
		if(code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB trim error\r\n");
		} else {
			// check if usb trim value is valid
			is_found = extract_2_set_type_trim_valid(TRIM_USB_FIELD, code, (u32 *)&trim);
			if(is_found) {
				if ((TRIM_RESINT_LOWER <= trim) && (trim <= TRIM_RESINT_UPPER)) {
					u2_trim_resint = trim;
					printf("is found [%d][%d][USB] = 0x%04x\r\n", is_found, OTP_USB_SET1_TRIM_FIELD, (int)trim);
					printf("  	=> 12K_resistor[0x%04x]\r\n", (int)u2_trim_resint);
				} else {
					printf("data is out of valid range\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		if (index)
			usbbase = IOADDR_USB2PHY_REG_BASE;
		else
			usbbase = IOADDR_USBPHY_REG_BASE;
		writel(0x20, (volatile unsigned long *)(usbbase+USB_PHYCTRL51_REG_OFS));
		writel(0x30, (volatile unsigned long *)(usbbase+USB_PHYCTRL50_REG_OFS));
		writel(0x60 + u2_trim_resint, (volatile unsigned long *)(usbbase+USB_PHYCTRL52_REG_OFS));
		mdelay(5);
		writel(0x00, (volatile unsigned long *)(usbbase+USB_PHYCTRL51_REG_OFS));
		mdelay(5);

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYCTRL06_REG_OFS));
		tmpval &= ~(0x7 << 1);
		tmpval |= (u2_trim_swctrl << 0x1);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYCTRL06_REG_OFS));

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYCTRL05_REG_OFS));
		tmpval &= ~(0x7 << 0x2);
		tmpval |= (u2_trim_sqsel << 0x2);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYCTRL05_REG_OFS));


		if (index)
			usbbase = IOADDR_USB2_REG_BASE;
		else
			usbbase = IOADDR_USB_REG_BASE;

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));
		tmpval &= ~(0x1F << 8);
		tmpval |= (u2_trim_resint << 8);
		tmpval |= (0x1 << 15);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));
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

	nvtim_set_gpio_power(index);

	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

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
	u32 usbbase;
	u32 tmpval = 0;

	if (index)
		usbbase = IOADDR_USB2_REG_BASE;
	else
		usbbase = IOADDR_USB_REG_BASE;

	/* Host Reset */
	union USB_MCR_REG usbmcrreg;
	usbmcrreg.reg = usb_getreg(usbbase, USB_MCR_REG_OFS);
	usbmcrreg.bit.HC_RESET = 1;
	usb_setreg(usbbase, USB_MCR_REG_OFS, usbmcrreg.reg);

	/* A_BUS_REQ[4] = 0 */
	union USB_OTGCTRLSTATUS_REG otgctrlstsreg;
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_REQ = 0;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	/* A_BUS_DROP[5] = 1 */
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_DROP = 1;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

#if defined(CONFIG_TARGET_NA51103)
	if (index) {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<16);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<16;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	} else {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<15);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<15;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	}
#else
	if (index) {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<16);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<16;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	} else {
		/* Enable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval &= ~(0x1<<15);
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

		udelay(10);

		/* Disable Reset*/
		tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
		tmpval |= 0x1<<15;
		writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	}
#endif
	return 0;
}

#else
#define TRIM_RESINT_UPPER 0x14
#define TRIM_RESINT_LOWER 0x1
static void nvtim_init_usbhc_dm(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	unsigned long usbbase;
	unsigned long tmpval = 0;

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

	/* Set INT_POLARITY */
	union USB_GLOBALINTMASK_REG usbgintmskreg;
	usbgintmskreg.reg = usb_getreg(usbbase, USB_GLOBALINTMASK_REG_OFS);
	usbgintmskreg.bit.INT_POLARITY = 1;
	usb_setreg(usbbase, USB_GLOBALINTMASK_REG_OFS, usbgintmskreg.reg);

	/* Clear DEVPHY_SUSPEND[5] */
	union USB_DEVDMACTRL1_REG usbctrl1reg;
	usbctrl1reg.reg = usb_getreg(usbbase, USB_DEVDMACTRL1_REG_OFS);
	usbctrl1reg.bit.DEVPHY_SUSPEND = 0;
	usb_setreg(usbbase, USB_DEVDMACTRL1_REG_OFS, usbctrl1reg.reg);

	/* Set USB ID & VBUSI */
	union USB_PHYTOP_REG usbphytopreg;
	usbphytopreg.reg &= ~0xFF;
	usbphytopreg.reg |= 0x22;
	usbphytopreg.bit.USB_VBUSI = 1;
	usbphytopreg.bit.USB_ID = 0;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);


	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	union USB_DEVMAINCTRL_REG devmainctrlreg;
	devmainctrlreg.reg = usb_getreg(usbbase, USB_DEVMAINCTRL_REG_OFS);
	devmainctrlreg.bit.FORCE_FS = 0;
#ifdef CONFIG_FPGA_EMULATION
	devmainctrlreg.bit.HALF_SPEED = 1;
#endif
	usb_setreg(usbbase, USB_DEVMAINCTRL_REG_OFS, devmainctrlreg.reg);

	/* Clear HOSTPHY_SUSPEND[6] */
	usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
	usbphytopreg.bit.USB_VBUSI = 0;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);


	union USB_HCMISC_REG usbhcmiscreg;
	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.bit.HostPhy_Suspend = 1;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	mdelay(1);

	usbphytopreg.reg = usb_getreg(usbbase, USB_PHYTOP_REG_OFS);
	usbphytopreg.bit.USB_VBUSI = 1;
	usb_setreg(usbbase, USB_PHYTOP_REG_OFS, usbphytopreg.reg);

	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.reg &= ~(0x3F << 16);
	usbhcmiscreg.reg |=  (0x22 << 16);
	usbhcmiscreg.bit.HostPhy_Suspend = 0;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	mdelay(25);

	/* A_BUS_DROP[5] = 0 */
	union USB_OTGCTRLSTATUS_REG otgctrlstsreg;
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_DROP = 0;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	udelay(2);

	/* A_BUS_REQ[4] = 1 */
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_REQ = 1;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	usbhcmiscreg.reg = usb_getreg(usbbase, USB_HCMISC_REG_OFS);
	usbhcmiscreg.bit.EOF2_Time = 0;
	usbhcmiscreg.bit.EOF1_Time = 0;
	usbhcmiscreg.bit.EOF1_Time = 0x3;
	usb_setreg(usbbase, USB_HCMISC_REG_OFS, usbhcmiscreg.reg);

	/* Configure PHY related settings below */
	{
		u8 u2_trim_swctrl=4, u2_trim_sqsel=4, u2_trim_resint=8;
		u32 trim;
		int code;
		bool is_found;

		// get trim value of usb column
		code = otp_key_manager(OTP_USB_SET1_TRIM_FIELD);
		if(code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB trim error\r\n");
		} else {
			// check if usb trim value is valid
			is_found = extract_2_set_type_trim_valid(TRIM_USB_FIELD, code, (u32 *)&trim);
			if(is_found) {
				if ((TRIM_RESINT_LOWER <= trim) && (trim <= TRIM_RESINT_UPPER)) {
					u2_trim_resint = trim;
					printf("is found [%d][%d][USB] = 0x%04x\r\n", is_found, OTP_USB_SET1_TRIM_FIELD, (int)trim);
					printf("  	=> 12K_resistor[0x%04x]\r\n", (int)u2_trim_resint);
				} else {
					printf("data is out of valid range\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		if (index)
			usbbase = IOADDR_USB2PHY_REG_BASE;
		else
			usbbase = IOADDR_USBPHY_REG_BASE;
		writel(0x20, (volatile unsigned long *)(usbbase+USB_PHYCTRL51_REG_OFS));
		writel(0x30, (volatile unsigned long *)(usbbase+USB_PHYCTRL50_REG_OFS));
		writel(0x60 + u2_trim_resint, (volatile unsigned long *)(usbbase+USB_PHYCTRL52_REG_OFS));
		mdelay(5);
		writel(0x00, (volatile unsigned long *)(usbbase+USB_PHYCTRL51_REG_OFS));
		mdelay(5);

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYCTRL06_REG_OFS));
		tmpval &= ~(0x7 << 1);
		tmpval |= (u2_trim_swctrl << 0x1);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYCTRL06_REG_OFS));

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYCTRL05_REG_OFS));
		tmpval &= ~(0x7 << 0x2);
		tmpval |= (u2_trim_sqsel << 0x2);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYCTRL05_REG_OFS));


		if (index)
			usbbase = IOADDR_USB2_REG_BASE;
		else
			usbbase = IOADDR_USB_REG_BASE;

		tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));
		tmpval &= ~(0x1F << 8);
		tmpval |= (u2_trim_resint << 8);
		tmpval |= (0x1 << 15);
		writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));
	}

	*hccr = (struct ehci_hccr *)usbbase;
	*hcor = (struct ehci_hcor *)((unsigned long)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct uboot_ehci_nvt_priv_data *ctx = dev_get_priv(dev);

	u32 base_addr = (u32) dev_read_addr(dev);
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
	u32 usbbase;
	u32 tmpval = 0;

	unsigned long base_addr = dev_read_addr(dev);

	usbbase = base_addr;

	/* Host Reset */
	union USB_MCR_REG usbmcrreg;
	usbmcrreg.reg = usb_getreg(usbbase, USB_MCR_REG_OFS);
	usbmcrreg.bit.HC_RESET = 1;
	usb_setreg(usbbase, USB_MCR_REG_OFS, usbmcrreg.reg);

	/* A_BUS_REQ[4] = 0 */
	union USB_OTGCTRLSTATUS_REG otgctrlstsreg;
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_REQ = 0;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

	/* A_BUS_DROP[5] = 1 */
	otgctrlstsreg.reg = usb_getreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS);
	otgctrlstsreg.bit.A_BUS_DROP = 1;
	usb_setreg(usbbase, USB_OTGCTRLSTATUS_REG_OFS, otgctrlstsreg.reg);

#if defined(CONFIG_TARGET_NA51103)
	if (usbbase == IOADDR_USB_REG_BASE) {
		printf("usb1 remove\n");
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
		printf("usb2 remove\n");
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
