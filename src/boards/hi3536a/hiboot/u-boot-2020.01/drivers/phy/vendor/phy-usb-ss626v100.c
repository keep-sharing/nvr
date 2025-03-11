/*
 * Copyright (c) 2017 Shenshu Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* Attention: For ss626v100 USB3_CTRL_REG_BASE is the 1-port USB3.0 controler
 * with base address: 0x10300000 ; USB2_CTRL_REG_BASE is the 2-port USB2.0 controler
 * with base address: 0x10340000
 */
#include <asm/arch-ss626v100/platform.h>
#include <dm.h>
#include <usb.h>
#include <usb/xhci.h>
#include "phy-usb.h"

/* offset 0x38c0 */
#define USB2_SRST_REQ       (0x1 << 0)
#define USB2_BUS_CKEN       (0x1 << 4)
#define USB2_REF_CKEN       (0x1 << 5)
#define USB2_1_UTMI_CKEN    (0x1 << 8)
#define USB2_0_UTMI_CKEN    (0x1 << 9)
#define USB2_1_UTMI_SELECT  (0x1 << 16)

/* offset 0x38d0 */
#define USB2_PHY1_REQ       (0x1 << 0)
#define USB2_PHY1_TREQ      (0x1 << 1)
#define USB2_PHY1_APB_SRST_REQ  (0x1 << 2)
#define USB2_PHY1_XTAL_CKEN (0x1 << 4)
#define USB2_PHY1_APB_CKEN	(0x1 << 6)

/* offset 0x38d4 */
#define USB2_PHY2_REQ       (0x1 << 0)
#define USB2_PHY2_TREQ      (0x1 << 1)
#define USB2_PHY2_APB_SRST_REQ  (0x1 << 2)
#define USB2_PHY2_XTAL_CKEN (0x1 << 4)
#define USB2_PHY2_APB_CKEN	(0x1 << 6)

/* offset 0x3940 */
#define USB3_SRST_REQ       (0x1 << 0)
#define USB3_BUS_CKEN       (0x1 << 4)
#define USB3_REF_CKEN       (0x1 << 5)
#define USB3_SUSPEND_CKEN   (0x1 << 6)
#define USB3_UTMI_CKEN      (0x1 << 8)
#define USB3_PIPE_CKEN      (0x1 << 12)

/* offset 0x3960 */
#define USB2_PHY0_REQ       (0x1 << 0)
#define USB2_PHY0_TREQ      (0x1 << 1)
#define USB2_PHY0_APB_SRST_REQ  (0x1 << 2)
#define USB2_PHY0_XTAL_CKEN (0x1 << 4)
#define USB2_PHY0_APB_CKEN	(0x1 << 6)

/* offset 0x3970 */
#define USB3PHY0_SRST_REQ   (0x1 << 0)
#define USB3PHY0_REF_CKEN   (0x1 << 4)

#define GUSB2PHYCFG         0xc204
#define U2_FREECLK_EXISTS   (0x1 << 30)

#define PHY_PLL_ENABLE      (0x3 << 0)
#define PHY_PLL_OFFSET      0x14

#define AHB_USB_CTRL1		0x44
#define U3_PORT_DISABLE		(0x1 << 9)

#define PERI_CRG3632_DEF	0x30001
#define PERI_CRG3636_DEF	0x107
#define PERI_CRG3637_DEF	0x107
#define PERI_CRG3664_DEF	0x10001
#define PERI_CRG3672_DEF	0x107
#define PERI_CRG3676_DEF	0x3

/* phy eye releated */
#define USB2_PHY_ANA_CFG0        (0x0U)
#define USB2_PHY_ANA_CFG2        (0x8U)
#define RG_HSTX_DE_MASK          (0xfU << 8)
#define RG_HSTX_DE_VAL           (0xcU << 8)
#define RG_HSTX_DEEN_MASK        (0x1U << 5)
#define RG_HSTX_DEEN_VAL         (0x1U << 5)
#define RG_HSTX_MBIAS_MASK       (0xfU)
#define RG_HSTX_MBIAS_VAL        (0xbU)

#define RG_TEST_TX_MASK          (0xffU << 20)
#define RG_TEST_TX_VAL           (0x2U << 20)
#define RG_VDISCREF_SEL_MASK     (0x7U << 16)
#define RG_VDISCREF_SEL_VAL      (0x6U << 16)

#define OTP_TRIM_ADDR            (0x11020444)
#define OTP_TRIM_MASK            (0x1fU)
#define TRIM_MAX_VALUE           (0x1dU)
#define TRIM_MIN_VALUE           (0x09U)
#define U2_PHY_TRIM_BIT          0x8
#define RG_RT_TRIM_MASK          (0x1fU << U2_PHY_TRIM_BIT)

static uintptr_t xhci_base = 0;

int xhci_hcd_init(int index, struct xhci_hccr **ret_hccr,
					struct xhci_hcor **ret_hcor)
{
	if ((ret_hccr == NULL) || (ret_hcor == NULL))
		return -EINVAL;

	if (index == 0)
		xhci_base = USB3_CTRL_REG_BASE;
	if (index == 1)
		xhci_base = USB2_CTRL_REG_BASE;

	*ret_hccr = (struct xhci_hccr *)(xhci_base);
	*ret_hcor = (struct xhci_hcor *)((uintptr_t)*ret_hccr +
				HC_LENGTH(xhci_readl(&(*ret_hccr)->cr_capbase)));

	return 0;
}

static void usb2_phy0_config(void)
{
	unsigned int reg;

	/* usb2 phy0 pll enable */
	reg = readl(USB2_PHY0_BASE + PHY_PLL_OFFSET);
	reg |= PHY_PLL_ENABLE;
	writel(reg, USB2_PHY0_BASE + PHY_PLL_OFFSET);
	udelay(U_LEVEL5);
}

static void usb2_phy1_config(void)
{
	unsigned int reg;

	/* usb2 phy1 pll enable */
	reg = readl(USB2_PHY1_BASE + PHY_PLL_OFFSET);
	reg |= PHY_PLL_ENABLE;
	writel(reg, USB2_PHY1_BASE + PHY_PLL_OFFSET);
	udelay(U_LEVEL5);
}

static void usb2_phy2_config(void)
{
	unsigned int reg;

	/* usb2 phy2 pll enable */
	reg = readl(USB2_PHY2_BASE + PHY_PLL_OFFSET);
	reg |= PHY_PLL_ENABLE;
	writel(reg, USB2_PHY2_BASE + PHY_PLL_OFFSET);
	udelay(U_LEVEL5);
}

static void usb2_ctrl_config(void)
{
	unsigned int reg;

	/* port1 freeclk no exist */
	reg = readl(USB2_CTRL_REG_BASE + GUSB2PHYCFG);
	reg &= ~U2_FREECLK_EXISTS;
	writel(reg, USB2_CTRL_REG_BASE + GUSB2PHYCFG);
	udelay(U_LEVEL5);
}

static void usb2_config(void)
{
	unsigned int reg;

	/* USB2 CRG default val config */
	writel(PERI_CRG3632_DEF, PERI_CRG3632);
	writel(PERI_CRG3636_DEF, PERI_CRG3636);
	writel(PERI_CRG3637_DEF, PERI_CRG3637);
	udelay(U_LEVEL6);

	/* open usb2 phy1 clk */
	reg = readl(PERI_CRG3636);
	reg |= (USB2_PHY1_XTAL_CKEN | USB2_PHY1_APB_CKEN);
	writel(reg, PERI_CRG3636);
	udelay(U_LEVEL6);

	/* open usb2 phy2 clk */
	reg = readl(PERI_CRG3637);
	reg |= (USB2_PHY2_XTAL_CKEN | USB2_PHY2_APB_CKEN);
	writel(reg, PERI_CRG3637);
	udelay(U_LEVEL6);

	/* cancel usb2 phy1 rst */
	reg = readl(PERI_CRG3636);
	reg &= ~(USB2_PHY1_REQ | USB2_PHY1_TREQ | USB2_PHY1_APB_SRST_REQ);
	writel(reg, PERI_CRG3636);
	udelay(U_LEVEL10);

	/* cancel usb2 phy2 rst */
	reg = readl(PERI_CRG3637);
	reg &= ~(USB2_PHY2_REQ | USB2_PHY2_TREQ | USB2_PHY2_APB_SRST_REQ);
	writel(reg, PERI_CRG3637);
	udelay(U_LEVEL10);

	usb2_phy1_config();

	usb2_phy2_config();

	/* open utmi/ref/bus clk */
	reg = readl(PERI_CRG3632);
	reg |= (USB2_BUS_CKEN | USB2_REF_CKEN);
	reg |= (USB2_0_UTMI_CKEN | USB2_1_UTMI_CKEN);
	reg &= ~(USB2_1_UTMI_SELECT);
	writel(reg, PERI_CRG3632);
	udelay(U_LEVEL6);

	/* cancel ctrl1 rst */
	reg = readl(PERI_CRG3632);
	reg &= ~USB2_SRST_REQ;
	writel(reg, PERI_CRG3632);
	udelay(U_LEVEL6);

	usb2_ctrl_config();
	udelay(U_LEVEL6);
}

static void usb3_port_disable(void)
{
	unsigned int reg;

	/* USB3 disable */
	reg = readl(AHB_SUBSYS_MISC_REG + AHB_USB_CTRL1);
	reg |= U3_PORT_DISABLE;
	writel(reg, AHB_SUBSYS_MISC_REG + AHB_USB_CTRL1);
	udelay(U_LEVEL5);
}

static void usb3_config(void)
{
	unsigned int reg;

	/* USB3 CRG default val config */
	writel(PERI_CRG3664_DEF, PERI_CRG3664);
	writel(PERI_CRG3672_DEF, PERI_CRG3672);
	writel(PERI_CRG3676_DEF, PERI_CRG3676);
	udelay(U_LEVEL6);

	usb3_port_disable();

	/* open usb2 phy0 clk */
	reg = readl(PERI_CRG3672);
	reg |= (USB2_PHY0_XTAL_CKEN | USB2_PHY0_APB_CKEN);
	writel(reg, PERI_CRG3672);
	udelay(U_LEVEL6);

	/* cancel usb2 phy0 rst */
	reg = readl(PERI_CRG3672);
	reg &= ~(USB2_PHY0_REQ | USB2_PHY0_TREQ | USB2_PHY0_APB_SRST_REQ);
	writel(reg, PERI_CRG3672);
	udelay(U_LEVEL10);

	/* open combphy0 clk */
	reg = readl(PERI_CRG3676);
	reg |= USB3PHY0_REF_CKEN;
	writel(reg, PERI_CRG3676);
	udelay(U_LEVEL6);

	/* cancel combphy0 rst */
	reg = readl(PERI_CRG3676);
	reg &= ~USB3PHY0_SRST_REQ;
	writel(reg, PERI_CRG3676);
	udelay(U_LEVEL5);

	usb2_phy0_config();

	/* open pipe/suspend/ref/bus clk */
	reg = readl(PERI_CRG3664);
	reg |= (USB3_BUS_CKEN | USB3_REF_CKEN | USB3_SUSPEND_CKEN);
	reg |= (USB3_UTMI_CKEN | USB3_PIPE_CKEN);
	writel(reg, PERI_CRG3664);
	udelay(U_LEVEL6);

	/* cancel ctrl0 rst */
	reg = readl(PERI_CRG3664);
	reg &= ~USB3_SRST_REQ;
	writel(reg, PERI_CRG3664);
	udelay(U_LEVEL7);
}

static void usb_eye_config_u2phy(uintptr_t phy_base)
{
	unsigned int reg, trim_val;
	reg = readl(phy_base + USB2_PHY_ANA_CFG0);
	reg &= ~(RG_HSTX_DE_MASK | RG_HSTX_DEEN_MASK | RG_HSTX_MBIAS_MASK);
	reg |= (RG_HSTX_DE_VAL | RG_HSTX_DEEN_VAL | RG_HSTX_MBIAS_VAL);
	writel(reg, phy_base + USB2_PHY_ANA_CFG0);

	reg = readl(phy_base + USB2_PHY_ANA_CFG2);
	reg &= ~(RG_TEST_TX_MASK | RG_VDISCREF_SEL_MASK);
	reg |= (RG_TEST_TX_VAL | RG_VDISCREF_SEL_VAL);
	writel(reg, phy_base + USB2_PHY_ANA_CFG2);

	trim_val = readl(OTP_TRIM_ADDR);
	trim_val &= OTP_TRIM_MASK;
	if ((trim_val >= TRIM_MIN_VALUE) && (trim_val <= TRIM_MAX_VALUE)) {
		reg = readl(phy_base + USB2_PHY_ANA_CFG2);
		reg &= ~(RG_RT_TRIM_MASK);
		reg |= (trim_val << U2_PHY_TRIM_BIT);
		writel(reg, phy_base + USB2_PHY_ANA_CFG2);
	}
}
void phy_usb_init(int index)
{
	if (index) {
		usb2_config();
		usb_eye_config_u2phy(USB2_PHY1_BASE);
		usb_eye_config_u2phy(USB2_PHY2_BASE);
	} else {
		usb3_config();
		usb_eye_config_u2phy(USB2_PHY0_BASE);
	}
}
EXPORT_SYMBOL(phy_usb_init);

void xhci_hcd_stop(int index)
{
	if (index == 0) {
		/* USB3 CRG default val config */
		writel(PERI_CRG3664_DEF, PERI_CRG3664);
		writel(PERI_CRG3672_DEF, PERI_CRG3672);
		writel(PERI_CRG3676_DEF, PERI_CRG3676);
		udelay(U_LEVEL6);
	}

	if (index == 1) {
		/* USB2 CRG default val config */
		writel(PERI_CRG3632_DEF, PERI_CRG3632);
		writel(PERI_CRG3636_DEF, PERI_CRG3636);
		writel(PERI_CRG3637_DEF, PERI_CRG3637);
		udelay(U_LEVEL6);
	}
}
EXPORT_SYMBOL(xhci_hcd_stop);
