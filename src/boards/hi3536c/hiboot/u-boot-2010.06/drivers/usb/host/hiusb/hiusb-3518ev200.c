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
#define HIUSB_OHCI_BASE		0x100a0000
#define HIUSB_OHCI_DEV_NAME	"hiusb-ohci"

#define USB_CKEN		(1 << 7)
#define USB_CTRL_UTMI0_REG	(1 << 5)
#define USB_CTRL_HUB_REG	(1 << 4)
#define USBPHY_PORT0_TREQ	(1 << 2)
#define USBPHY_REQ		(1 << 1)
#define USB_AHB_SRST_REQ	(1 << 0)

#define MISC_CTRL		0x20120000
#define PERI_USB		(MISC_CTRL + 0x78)
#define WORDINTERFACE		(1 << 0)
#define SS_BURST4_EN		(1 << 7)
#define SS_BURST8_EN		(1 << 8)
#define SS_BURST16_EN		(1 << 9)
#define USBOVR_P_CTRL		(1 << 17)
#define MISC_USB                (MISC_CTRL + 0x80)
static void hiusb_ohci_enable_clk(void)
{
	int reg;

	/* enable phy ref clk to enable phy */
	reg = readl(CRG_REG_BASE + REG_CRG46);
	reg |= USB_CKEN;
	writel(reg, CRG_REG_BASE + REG_CRG46);
	udelay(100);

	/* config controller */
	reg = readl(PERI_USB);
	reg &= ~(WORDINTERFACE); /* 8bit */
	/* disable ehci burst16 mode */
	reg &= ~(SS_BURST16_EN);
	reg |= USBOVR_P_CTRL;
	writel(reg, PERI_USB);
	udelay(100);

	/* de-assert phy port */
	reg = readl(CRG_REG_BASE + REG_CRG46);
	reg &= ~(USBPHY_REQ);
	writel(reg, CRG_REG_BASE + REG_CRG46);
	udelay(100);

	/* open phy clk */
	writel(0xc06, MISC_USB);
	udelay(10);
	writel(0xc26, MISC_USB);
	mdelay(5);

	/* usb2.0 phy eye pattern */
	/* open USB2 pre-emphasis */
	writel(0x0, MISC_USB);
	udelay(10);
	writel(0x1c20, MISC_USB);
	mdelay(5);
#if 0
	/* usb2.0 phy eye pattern */
	writel(0x1c00, MISC_USB);
	udelay(10);
	writel(0x1c20, MISC_USB);
	mdelay(5);

	writel(0x0c09, MISC_USB);
	udelay(10);
	writel(0x0c29, MISC_USB);
	mdelay(5);

	writel(0x1a0a, MISC_USB);
	udelay(10);
	writel(0x1a2a, MISC_USB);
	mdelay(5);
#endif

	/* cancel phy utmi reset */
	reg = readl(CRG_REG_BASE + REG_CRG46);
	reg &= ~(USBPHY_PORT0_TREQ);
	writel(reg, CRG_REG_BASE + REG_CRG46);
	udelay(300);

	/* de-assert all the rsts of ctrl */
	reg = readl(CRG_REG_BASE + REG_CRG46);
	reg &= ~(USB_CTRL_UTMI0_REG);
	reg &= ~(USB_CTRL_HUB_REG);
	reg &= ~(USB_AHB_SRST_REQ);
	writel(reg, CRG_REG_BASE + REG_CRG46);
	udelay(200);

#if 0
	/* decrease the threshold value from 650 to 550*/
	writel(0xa, MISC_USB);
	udelay(10);
	writel(0x092a, MISC_USB);
	mdelay(5);
#endif

}

static void hiusb_ohci_disable_clk(void)
{
	int reg;

	/* Disable EHCI clock.
	   If the HS PHY is unused disable it too. */

	reg = readl(CRG_REG_BASE + REG_CRG46);
	reg &= ~(USB_CKEN);
	reg |= (USB_CTRL_UTMI0_REG);
	reg |= (USB_CTRL_HUB_REG);
	reg |= (USBPHY_PORT0_TREQ);
	reg |= (USBPHY_REQ);
	reg |= (USB_AHB_SRST_REQ);
	writel(reg, CRG_REG_BASE + REG_CRG46);
	udelay(100);

	/* enable phy */
	reg = readl(PERI_USB);
	reg |= (WORDINTERFACE);
	reg |= (SS_BURST16_EN);
	reg |= (USBOVR_P_CTRL);
	writel(reg, PERI_USB);
	udelay(100);
}

