/*
* Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#define base                    (0x10180000)
#define IO_REG_USB3_CTRL        (0x12010000 + 0xb8)
#define USB3_VCC_SRST_REQ2     (1<<0)

#define GTXTHRCFG             0xc108
#define GRXTHRCFG             0xc10c
#define REG_GCTL              0xc110

#define REG_GUSB2PHYCFG0         0xC200
#define BIT_UTMI_ULPI         (0x1 << 4)
#define BIT_UTMI_8_16         (0x1 << 3)

#define REG_GUSB3PIPECTL0	0xc2c0
#define PCS_SSP_SOFT_RESET	(0x1 << 31)

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	*hccr = (struct xhci_hccr *)(base);
	*hcor = (struct xhci_hcor *)((uint32_t) *hccr
			+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

void hiusb_start_hcd(void)
{
	unsigned long flags;
	unsigned int reg;

	local_irq_save(flags);

	writel(0x300, (0x120100ac));
	wait_ms(10);
	/* de-assert usb3_vcc_srst_req */
	reg = readl(IO_REG_USB3_CTRL);
	reg &= ~(USB3_VCC_SRST_REQ2);
	writel(reg, IO_REG_USB3_CTRL);
	wait_ms(100);
	reg = readl(base + REG_GUSB3PIPECTL0);
	reg |= PCS_SSP_SOFT_RESET;
	writel(reg, base + REG_GUSB3PIPECTL0);

	/*step 3: USB2 PHY chose ulpi 8bit interface */
	reg = readl(base + REG_GUSB2PHYCFG0);
	reg &= ~BIT_UTMI_ULPI;
	reg &= ~(BIT_UTMI_8_16);
	writel(reg, base + REG_GUSB2PHYCFG0);
	wait_ms(20);
	reg = readl(base + REG_GCTL);
	reg &= ~(0x3<<12);
	reg |= (0x1<<12); /*[13:12] 01: Host; 10: Device; 11: OTG*/
	writel(reg, base + REG_GCTL);
	wait_ms(20);

	reg = readl(base + REG_GUSB3PIPECTL0);
	reg &= ~PCS_SSP_SOFT_RESET;
	reg &= ~(1<<17);       /* disable suspend */
	writel(reg, base + REG_GUSB3PIPECTL0);
	wait_ms(100);

	writel(0x23100000, base + GTXTHRCFG);
	writel(0x23100000, base + GRXTHRCFG);
	wait_ms(20);

	local_irq_restore(flags);
}

void hiusb_stop_hcd(void)
{
	unsigned long flags;
	unsigned int reg;

	local_irq_save(flags);

	reg = readl(IO_REG_USB3_CTRL);
	writel(reg | (USB3_VCC_SRST_REQ2), IO_REG_USB3_CTRL);
	wait_ms(500);
	local_irq_restore(flags);
}
