/*
* Copyright (c) 2015 HiSilicon Technologies Co., Ltd.
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
#define HIUSB_OHCI_DEV_NAME     "hiusb-ohci"

#define IO_REG_USB2_CTRL		(0x12010000 + 0xb4)
#define USB_OTG_PHY_SRST_REQ    (1 << 8)
#define USB_CKEN		(1 << 7)
#define USB_CTRL_UTMI0_REG	(1 << 5)
#define USB_CTRL_HUB_REQ	(1 << 4)
#define USBPHY_PORT0_TREQ	(1 << 2)
#define USBPHY_REQ		(1 << 1)
#define USB_AHB_SRST_REQ	(1 << 0)
#define PERI_USB		(0x12030000 + 0x78)
#define WORDINTERFACE		(1 << 0)
#define ULPI_BYPASS_EN		(1 << 3)
#define SS_BURST4_EN		(1 << 7)
#define SS_BURST8_EN		(1 << 8)
#define SS_BURST16_EN		(1 << 9)
#define USBOVR_P_CTRL		(1 << 17)

static void hiusb_ohci_enable_clk(void)
{
	int reg;

	/* enable clock to EHCI block and HS PHY PLL*/
	reg = readl(IO_REG_USB2_CTRL);
	reg |= USB_CKEN;
	reg &= ~(USB_CTRL_UTMI0_REG);
	reg &= ~(USBPHY_PORT0_TREQ);
	reg &= ~(USBPHY_REQ);
	reg &= ~(USB_AHB_SRST_REQ);
	reg &= ~(USB_CTRL_HUB_REQ);
	writel(reg, IO_REG_USB2_CTRL);
	udelay(100);

	/* enable phy */
	reg = readl(PERI_USB);
	reg &= ~ULPI_BYPASS_EN;
	reg &= ~(WORDINTERFACE);
	/* disable ehci burst16 mode*/
	reg &= ~(SS_BURST16_EN);
	reg |= USBOVR_P_CTRL;
	writel(reg, PERI_USB);
	udelay(100);
	writel(0x800000, (0x12030000 + 0x80));
	mdelay(100);
	writel(0xa0060c, (0x12030000 + 0x80));
	mdelay(100);

	writel(0x0, 0x120401ac);
	udelay(100);
	writel(0x8, 0x1214d400);
	udelay(100);
	writel(0x8, 0x1214d020);
	udelay(500);



}

static void hiusb_ohci_disable_clk(void)
{
	int reg;

	/* Disable EHCI clock.
	   If the HS PHY is unused disable it too. */

	reg = readl(IO_REG_USB2_CTRL);
	reg &= ~(USB_CKEN);
	reg |= (USB_CTRL_UTMI0_REG);
	reg |= (USBPHY_PORT0_TREQ);
	reg |= (USBPHY_REQ);
	reg |= (USB_AHB_SRST_REQ);
	reg |= (USB_CTRL_HUB_REQ);
	writel(reg, IO_REG_USB2_CTRL);
	udelay(100);
}
