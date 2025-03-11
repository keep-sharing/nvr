/*
 * Copyright (c) 2021, Novatek Microelectronics Corp.
 * All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <usb.h>

#include "xhci.h"
#ifdef CONFIG_DM_USB
#include <dm.h>
#include <fdtdec.h>
#endif
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <linux/libfdt.h>
#if (defined(CONFIG_TARGET_NA51090_A64)|| defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64))
#include <asm/arch/efuse_protected.h>
#endif

#define DRV_VERSION                     "1.00.04"
#ifdef CONFIG_DM_USB
/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

struct uboot_xhci_priv {
        struct xhci_ctrl ctrl; /* Needed by EHCI */
        struct xhci_hccr *hccr;
        struct xhci_hcor *hcor;
};
#endif

#define USB3PHY_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(IOADDR_USB3CTRL_REG_BASE+0x2000+((ofs)<<2)))
#define USB3PHY_GETREG(ofs)         readl((volatile void __iomem *)(IOADDR_USB3CTRL_REG_BASE+0x2000+((ofs)<<2)))

#define USB3U2PHY_SETREG(ofs,value) writel((value), (volatile void __iomem *)(IOADDR_USB3CTRL_REG_BASE+0x1000+((ofs)<<2)))
#define USB3U2PHY_GETREG(ofs)       readl((volatile void __iomem *)(IOADDR_USB3CTRL_REG_BASE+0x1000+((ofs)<<2)))

#if defined(CONFIG_TARGET_NA51090_A64)
static void nvtim_set_gpio_power(void)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int gpio_pin, gpio_value, err;

	sprintf(path,"/u3hst@2,%lx",(IOADDR_USB3_REG_BASE & 0xFFFFFFFF));

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
#endif

#if (defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64))
static void na51102_phyinit(void)
{
	unsigned long tmpval = 0, temp = 0;
	u8 tx_term_trim_val = 6;
	u8 rx_term_trim_val = 12;
	u8 u3_trim_rint_sel=8;
	u32 trim;
	int code;
	bool is_found;

	code = otp_key_manager(EFUSE_USB_SATA_TRIM_DATA);
	if (code == -33) {
		printf("Read usb trim error\n");
		printf("[%d] data = NULL\n", 5);
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			printf("  USB(SATA) Trim data = 0x%04x\r\n", (int)trim);
			u3_trim_rint_sel = trim & 0x1F;
			tx_term_trim_val = ((trim >> 5) & 0xF);
			rx_term_trim_val = ((trim >> 9) & 0xF);
			if (u3_trim_rint_sel > 0x14) {
				u3_trim_rint_sel=8;
				printf("12K_resistor Trim data error 0x%04x > 20(0x14)\r\n", (int)u3_trim_rint_sel);
			} else if (u3_trim_rint_sel < 0x1) {
				u3_trim_rint_sel=8;
				printf("12K_resistor Trim data error 0x%04x < 1(0x1)\r\n", (int)u3_trim_rint_sel);
			} else {
				printf("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)u3_trim_rint_sel);
			}
			//TX term bit[8..5] 0x2 <= x <= 0xE
			if (tx_term_trim_val > 0xE) {
				tx_term_trim_val = 6;
				printf("USB(SATA) TX Trim data error 0x%04x > 0xE\r\n", (int)tx_term_trim_val);
			} else if (tx_term_trim_val < 0x2) {
				tx_term_trim_val = 6;
				printf("USB(SATA) TX Trim data error 0x%04x < 0x2\r\n", (int)tx_term_trim_val);
			} else {
				printf("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)tx_term_trim_val);
			}
			//RX term bit[12..9] 0x8 <= x <= 0xE
			if (rx_term_trim_val > 0xE) {
				rx_term_trim_val = 12;
				printf("USB(SATA) RX Trim data error 0x%04x > 0xE\r\n", (int)rx_term_trim_val);
			} else if (rx_term_trim_val < 0x8) {
				rx_term_trim_val = 12;
				printf("USB(SATA) RX Trim data error 0x%04x < 0x8\r\n", (int)rx_term_trim_val);
			} else {
				printf("  *USB(SATA) RX Trim data 0x8 <= [0x%04x] <= 0xE\r\n", (int)rx_term_trim_val);
			}
		} else {
			//!!!Please apply default value here!!!
			printf("is found [%d][USB trim] = 0x%08x\r\n", is_found, (int)code);
		}
	}

	//======================================================
	//(1) PHY RINT enable										[NEW]
	//======================================================
	// PHY_RES = 0xD
	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x38));
	tmpval |= 0xD01;
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x38));

	// Boundary /PHY control
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	// RINT_EN bias enable internal control selection
	// 0 : from boundary
	// 1 : from PHY internal register
	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x2680));
	tmpval &= ~0x20;
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x2680));
	//======================================================
	//(2) USB30 PHY 12k trim bits mux selection  0X1a1[7] = 0		[NEW]
	//======================================================
	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x2684));
	tmpval &= ~0x80;
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x2684));

	USB3PHY_SETREG(0x1A0, 0x40+u3_trim_rint_sel);
	USB3PHY_SETREG(0x1A3, 0x60);
	//======================================================
	//(3) USB20 swing & sqsel trim bits removed in INT98530
	//======================================================
	//========================================================
	//add additonal setting for 0.85V
	//========================================================
	// Bank 2
	USB3PHY_SETREG(0xFF, 0x02);
	// bit[5:4]  = LDO_VLDO_SEL_DCO[1:0] ; 00 = 0.85v
	USB3PHY_SETREG(0x2A, 0x80);
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);
	// bit[1:0]  = LDO_SEL_18[1:0] ; 00 = 0.85v
	USB3PHY_SETREG(0x64, 0x44);
	// bit[1:0] = TXPLL_CP[1:0] = 01
	USB3PHY_SETREG(0x65, 0X99);

	//========================================================
	//swing
	//========================================================
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);
	// bit[5:4] = V_VCHSEL[1:0] = 11
	USB3PHY_SETREG(0x56, 0X74);
	// TX_AMP_CTL=2, TX_DEC_EM_CTL=8
	USB3PHY_SETREG(0x14, 0X8A);
	// bit[5:4] = TX_AMP_CTL_LFPS[1:0] (from 10 -> 11)
	//USB3PHY_SETREG(0x12, 0XE8);
	USB3PHY_SETREG(0x12, 0XF8);
	// bit3 : TX_DE_EN manual mode
	// bit[7:4]: TX_DEC_EM_CTL manual mode
	USB3PHY_SETREG(0x34, 0Xfc);

	//======================================================
	//(4) TX termination trim bits								[NEW]
	//======================================================
	// 0xF05F2040[7:4] = tx_term_trim_val[3:0]
	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	temp = USB3PHY_GETREG(0x10);
	temp &= ~(0xF0);
	temp |= (tx_term_trim_val<<4);
	USB3PHY_SETREG(0x10 , temp);

	//======================================================
	//(5) RX termination trim bits								[NEW]
	//======================================================
	// Bank 2
	USB3PHY_SETREG(0xFF, 0x02);

	temp = USB3PHY_GETREG(0x0);
	temp &= ~(0xF);
	temp |= (rx_term_trim_val);
	USB3PHY_SETREG(0x00 , temp);

	// Bank 0
	USB3PHY_SETREG(0xFF, 0x00);

	//======================================================
	//(6) T12 USB30 APHY setting								[NEW]
	//======================================================
	USB3PHY_SETREG(0xFF ,0x00);
	USB3PHY_SETREG(0x149,0xFF);
	USB3PHY_SETREG(0x145,0xFF);
	USB3PHY_SETREG(0xFF ,0x01);
	USB3PHY_SETREG(0x168,0xEF);
	USB3PHY_SETREG(0x108,0x02);
	USB3PHY_SETREG(0x1A0,0x5F);
	USB3PHY_SETREG(0x1A2,0xF4);
	USB3PHY_SETREG(0x15E,0xFF);
	USB3PHY_SETREG(0x22 ,0x00);
	USB3PHY_SETREG(0x16C,0x00);
	USB3PHY_SETREG(0x170,0x00);
	USB3PHY_SETREG(0x174,0xB9);
	USB3PHY_SETREG(0x23 ,0x13);
	USB3PHY_SETREG(0x1A5,0x2C);
	USB3PHY_SETREG(0x5A ,0x08);
	USB3PHY_SETREG(0xB9 ,0xC0);
	USB3PHY_SETREG(0xFF ,0x02);
	USB3PHY_SETREG(0x28 ,0xF7);
	USB3PHY_SETREG(0x3  ,0x21);
	USB3PHY_SETREG(0x2  ,0xEC);
	USB3PHY_SETREG(0x1E ,0x10);
	USB3PHY_SETREG(0x33 ,0xA0);
	USB3PHY_SETREG(0xA  ,0x0F);
	USB3PHY_SETREG(0xB  ,0x3B);
	USB3PHY_SETREG(0x1F ,0x7C);
	//USB3PHY_SETREG(0x2A ,0x90);
	USB3PHY_SETREG(0xFF ,0x00);

	/* asd_mode=1 (toggle rate) */
	USB3PHY_SETREG(0x198, 0x04);
	/* RX_ICTRL's offset = 0 */
	//USB3PHY_SETREG(0x1BF, 0x40);
	/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
	USB3PHY_SETREG(0x014, 0x8a);
	/* TX_LFPS_AMP_CTL = 1 */
	USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
	/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */

	//USB3PHY_SETREG(0x114, 0x0B);
	USB3PHY_SETREG(0x114, 0x04);
	//USB3PHY_SETREG(0x152, 0x2E);
	//USB3PHY_SETREG(0x153, 0x01);
	//USB3PHY_SETREG(0x1B0, 0xC0);
	//USB3PHY_SETREG(0x1B1, 0x91);
	//USB3PHY_SETREG(0x1B2, 0x00);
	//USB3PHY_SETREG(0x135, 0x88);
	//USB3PHY_SETREG(0x12A, 0x50);
	//USB3PHY_SETREG(0x1F0, 0x80);
	//USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
	//USB3PHY_SETREG(0x105, 0x01);
	USB3PHY_SETREG(0x056, 0x74);//disconnect level to 0x3
	udelay(2);

	USB3PHY_SETREG(0x102, 0xF0);
	udelay(10);
	USB3PHY_SETREG(0x102, 0x00);
	udelay(300);

	//USB3PHY_SETREG(0x103, 0x01);
	//udelay(100);

	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if(1) {
		temp = USB3PHY_GETREG(0x011);
		temp |= 0x1;
		USB3PHY_SETREG(0x011, temp);

		temp = USB3PHY_GETREG(0x031);
		temp |= 0x1;
		USB3PHY_SETREG(0x031, temp);
	}

	USB3PHY_SETREG(0x029, 0x1);

	USB3U2PHY_SETREG(0x09, 0xb0);
	USB3U2PHY_SETREG(0x12, 0x31);
}
#endif

#ifndef CONFIG_DM_USB
/*
 * Create the appropriate control structures to manage a new XHCI host
 * controller.
 */
int xhci_hcd_init(int index, struct xhci_hccr **ret_hccr,
		  struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	int len;
	unsigned long tmpval, temp;
	u8 u3_trim_rint_sel=8,u3_trim_swctrl=4, u3_trim_sqsel=4,u3_trim_icdr=0xB;

#if defined(CONFIG_TARGET_NA51090_A64)
	/* init OTG controller */
	/* Enable phy clk*/
	tmpval = readl((volatile unsigned long *)IOADDR_CG_REG_BASE);
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)IOADDR_CG_REG_BASE);

	/* Enable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval &= ~(0x1<<18);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval &= ~(0x1<<15);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

	udelay(10);

	/* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval |= 0x1<<18;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval |= 0x1<<15;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

	/* Release sram shutdown*/
	tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
	tmpval &= ~(0x1<<7);
	writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

	/* Enable clock*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	tmpval |= (0x1<<26);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));

	mdelay(10);

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x28));
	tmpval = 0x1;
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x28));

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));
	tmpval &= ~0x2;
	tmpval &= ~(0x1 << 11);
	tmpval |= (0x1 << 0);
	tmpval &= ~((0x7 << 16) | (0x7 << 24) | (0x1 << 4));
	tmpval |= ((0x4 << 16) | (0x4 << 24));
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));

	USB3PHY_SETREG(0x0D, 0x01);
#elif (defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64))
	/* Enable PLL20 */
	tmpval = readl((volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE));
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE));

	udelay(10);

	//Reset u2/u3 phy
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval &= ~(0x5<<5);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	udelay(10);

        /* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval |= 0x5<<5;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

       /* Enable USB3 clock*/
	tmpval = readl((volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE+0x140));
	tmpval |= (0x1<<5);
	writel(tmpval, (volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE+0x140));

	mdelay(10);

       /* Close channel disable*/
	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x14));
	tmpval &= 0xFFFFFDFF;
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x14));

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));
	tmpval &= ~0x2;
	tmpval &= ~(0x1 << 11);
	tmpval |= (0x1 << 0);
	tmpval &= ~((0x7 << 16) | (0x7 << 24) | (0x1 << 4));
	tmpval |= ((0x4 << 16) | (0x4 << 24));
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));

	udelay(100);
#endif

#if defined(CONFIG_TARGET_NA51090_A64)
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
				u3_trim_rint_sel = remap_data&0x1F;
				u3_trim_icdr     = (remap_data>>5)&0xF;
			}

			//usb3_validateTrim();
			USB3PHY_SETREG(0x1A0, 0x40+u3_trim_rint_sel);
			USB3PHY_SETREG(0x1A3, 0x60);

			temp = USB3U2PHY_GETREG(0x06);
			temp &= ~(0x7<<1);
			temp |= (u3_trim_swctrl<<1);
			USB3U2PHY_SETREG(0x06, temp);

			temp = USB3U2PHY_GETREG(0x05);
			temp &= ~(0x7<<2);
			temp |= (u3_trim_sqsel<<2);
			USB3U2PHY_SETREG(0x05, temp);
		}

		/* asd_mode=1 (toggle rate) */
		USB3PHY_SETREG(0x198, 0x04);
		/* RX_ICTRL's offset = 0 */
		USB3PHY_SETREG(0x1BF, 0x40);
		/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
		USB3PHY_SETREG(0x014, 0x8a);
		/* TX_LFPS_AMP_CTL = 1 */
		USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
		/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */

		USB3PHY_SETREG(0x114, 0x0B);
		USB3PHY_SETREG(0x152, 0x2E);
		USB3PHY_SETREG(0x153, 0x01);
		USB3PHY_SETREG(0x1B0, 0xC0);
		USB3PHY_SETREG(0x1B1, 0x91);
		USB3PHY_SETREG(0x1B2, 0x00);
		USB3PHY_SETREG(0x135, 0x88);
		USB3PHY_SETREG(0x12A, 0x50);
		USB3PHY_SETREG(0x1F0, 0x80);
		USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
		USB3PHY_SETREG(0x105, 0x01);
		USB3PHY_SETREG(0x056, 0x74);//disconnect level to 0x3
		udelay(2);

		USB3PHY_SETREG(0x102, 0x20);
		udelay(10);
		USB3PHY_SETREG(0x102, 0x00);
		udelay(300);

		USB3PHY_SETREG(0x103, 0x01);
		udelay(100);

		USB3U2PHY_SETREG(0x51, 0x20);
		udelay(2);
		USB3U2PHY_SETREG(0x51, 0x00);
		udelay(2);

		if(1) {
			temp = USB3PHY_GETREG(0x011);
			temp |= 0x1;
			USB3PHY_SETREG(0x011, temp);

			temp = USB3PHY_GETREG(0x031);
			temp |= 0x1;
			USB3PHY_SETREG(0x031, temp);
		}

		USB3PHY_SETREG(0x029, 0x1);
#elif defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
	na51102_phyinit();
#endif
	hccr = (struct xhci_hccr *)IOADDR_USB3_REG_BASE;
	len = HC_LENGTH(xhci_readl(&hccr->cr_capbase));
	hcor = (struct xhci_hcor *)((unsigned long)hccr + len);

	printk("XHCI init hccr 0x%lx and hcor 0x%lx hc_length 0x%x ver %s\n",
		(unsigned long)hccr, (unsigned long)hcor, len, DRV_VERSION);

	*ret_hccr = hccr;
	*ret_hcor = hcor;
#if defined(CONFIG_TARGET_NA51090_A64)
	nvtim_set_gpio_power();
#endif
	return 0;
}

/*
 * Destroy the appropriate control structures corresponding * to the XHCI host
 * controller
 */
void xhci_hcd_stop(int index)
{
}


#else
/*
 * Create the appropriate control structures to manage a new XHCI host
 * controller.
 */
int dm_xhci_hcd_init(int index, struct xhci_hccr **ret_hccr,
		struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	int len;
	unsigned long tmpval = 0, temp = 0;
#if defined(CONFIG_TARGET_NA51090_A64)
	u8 u3_trim_swctrl=4, u3_trim_sqsel=4,u3_trim_icdr=0xB;
	u8 u3_trim_rint_sel=8;
#endif

#if defined(CONFIG_TARGET_NA51090_A64)
	/* init OTG controller */
	/* Enable phy clk*/
	tmpval = readl((volatile unsigned long *)IOADDR_CG_REG_BASE);
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)IOADDR_CG_REG_BASE);

	/* Enable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval &= ~(0x1<<18);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval &= ~(0x1<<15);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

	udelay(10);

	/* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval |= 0x1<<18;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval |= 0x1<<15;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

	/* Release sram shutdown*/
	tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
	tmpval &= ~(0x1<<25);
	writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

	/* Enable clock*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	tmpval |= (0x1<<26);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));

	mdelay(10);

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));
	tmpval &= ~0x2;
	tmpval &= ~(0x1 << 11);
	tmpval |= (0x1 << 0);
	tmpval &= ~((0x7 << 16) | (0x7 << 24) | (0x1 << 4));
	tmpval |= ((0x4 << 16) | (0x4 << 24));
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));

	udelay(100);

	USB3PHY_SETREG(0x0D, 0x01);
#elif (defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64))
	/* Enable PLL20 */
	tmpval = readl((volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE));
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE));

	udelay(10);

	//Reset u2/u3 phy
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval &= ~(0x5<<5);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	udelay(10);

        /* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));
	tmpval |= 0x5<<5;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x9C));

       /* Enable USB3 clock*/
	tmpval = readl((volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE+0x140));
	tmpval |= (0x1<<5);
	writel(tmpval, (volatile unsigned long *)(IOADDR_STBC_CG_REG_BASE+0x140));

	mdelay(10);

       /* Close channel disable*/
	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x14));
	tmpval &= 0xFFFFFDFF;
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x14));

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));
	tmpval &= ~0x2;
	tmpval &= ~(0x1 << 11);
	tmpval |= (0x1 << 0);
	tmpval &= ~((0x7 << 16) | (0x7 << 24) | (0x1 << 4));
	tmpval |= ((0x4 << 16) | (0x4 << 24));
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));

	udelay(100);
#endif

#if defined(CONFIG_TARGET_NA51090_A64)
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
			u3_trim_rint_sel = remap_data&0x1F;
			u3_trim_icdr     = (remap_data>>5)&0xF;
		}
	}

	//usb3_validateTrim();
	USB3PHY_SETREG(0x1A0, 0x40+u3_trim_rint_sel);
	USB3PHY_SETREG(0x1A3, 0x60);

	temp = USB3U2PHY_GETREG(0x06);
	temp &= ~(0x7<<1);
	temp |= (u3_trim_swctrl<<1);
	USB3U2PHY_SETREG(0x06, temp);

	temp = USB3U2PHY_GETREG(0x05);
	temp &= ~(0x7<<2);
	temp |= (u3_trim_sqsel<<2);
	USB3U2PHY_SETREG(0x05, temp);

	/* asd_mode=1 (toggle rate) */
	USB3PHY_SETREG(0x198, 0x04);
	/* RX_ICTRL's offset = 0 */
	USB3PHY_SETREG(0x1BF, 0x40);
	/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
	USB3PHY_SETREG(0x014, 0x8a);
	/* TX_LFPS_AMP_CTL = 1 */
	USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
	/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */

	USB3PHY_SETREG(0x114, 0x0B);
	USB3PHY_SETREG(0x152, 0x2E);
	USB3PHY_SETREG(0x153, 0x01);
	USB3PHY_SETREG(0x1B0, 0xC0);
	USB3PHY_SETREG(0x1B1, 0x91);
	USB3PHY_SETREG(0x1B2, 0x00);
	USB3PHY_SETREG(0x135, 0x88);
	USB3PHY_SETREG(0x12A, 0x50);
	USB3PHY_SETREG(0x1F0, 0x80);
	USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
	USB3PHY_SETREG(0x105, 0x01);
	USB3PHY_SETREG(0x056, 0x74);//disconnect level to 0x3
	udelay(2);

	USB3PHY_SETREG(0x102, 0x20);
	udelay(10);
	USB3PHY_SETREG(0x102, 0x00);
	udelay(300);

	USB3PHY_SETREG(0x103, 0x01);
	udelay(100);

	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if(1) {
		temp = USB3PHY_GETREG(0x011);
		temp |= 0x1;
		USB3PHY_SETREG(0x011, temp);

		temp = USB3PHY_GETREG(0x031);
		temp |= 0x1;
		USB3PHY_SETREG(0x031, temp);
	}

	USB3PHY_SETREG(0x029, 0x1);
#elif defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
	na51102_phyinit();
#endif
	hccr = (struct xhci_hccr *)IOADDR_USB3_REG_BASE;
	len = HC_LENGTH(xhci_readl(&hccr->cr_capbase));
	hcor = (struct xhci_hcor *)((unsigned long)hccr + len);

	printk("XHCI init hccr 0x%lx and hcor 0x%lx hc_length 0x%x ver %s\n",
		(unsigned long)hccr, (unsigned long)hcor, len, DRV_VERSION);

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	return 0;
}

static int xhci_usb_probe(struct udevice *dev)
{
	struct uboot_xhci_priv *ctx = dev_get_priv(dev);
	int ret = 0;

	ret = dm_xhci_hcd_init(0 , &ctx->hccr, &ctx->hcor);
	if (ret) {
		puts("XHCI: failed to initialize controller\n");
		return -EINVAL;
	}
#if defined(CONFIG_TARGET_NA51090_A64)
	nvtim_set_gpio_power();
#endif
	debug("%s hccr 0x%lx and hcor 0x%lx hc_length %ld\n", __func__,
			(unsigned long)ctx->hccr, (unsigned long)ctx->hcor,
			(unsigned long)HC_LENGTH(xhci_readl(&ctx->hccr->cr_capbase)));

	return xhci_register(dev, ctx->hccr, ctx->hcor);
}

static int xhci_usb_remove(struct udevice *dev)
{
	int ret;

	ret = xhci_deregister(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id xhci_usb_ids[] = {
	{ .compatible = "nvt,nvt_usb3xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name   = "xhci_nvtivot",
	.id     = UCLASS_USB,
	.of_match = xhci_usb_ids,
	.probe = xhci_usb_probe,
	.remove = xhci_usb_remove,
	.ops    = &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct uboot_xhci_priv),
	.flags  = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
