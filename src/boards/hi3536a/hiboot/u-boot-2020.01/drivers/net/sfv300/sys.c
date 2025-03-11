// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */


#include <config.h>
#include "bspeth.h"
#include "sys.h"

#if defined(CONFIG_GODBOX) || defined(CONFIG_GODBOX_V1)
#define ETH_CRG_REG (REG_BASE_CRG + REG_PERI_CRG26)
#else
#define ETH_CRG_REG (CRG_REG_BASE + REG_ETH_CRG)
#endif

#define ETH_SOFT_RESET bit(0)
#if defined(CONFIG_GODBOX) || defined(CONFIG_GODBOX_V1)
#define ETH_CLK_ENABLE bit(8)
#else
#define ETH_CLK_ENABLE bit(1)
#endif

#if defined(SFV_RESET_PHY_BY_CRG)
#define ETH_EXTERNAL_PHY_RESET bit(3)
#endif

#define ETH_CORE_CLK_SELECT_54M bit(7)

void set_efuse_unread(void)
{
}

#ifdef INNER_PHY
void set_inner_phy_addr(u32 phy_addr)
{
	u32 reg_value;

	reg_value = readl(ETH_SYSCTL_REG);
	reg_value &= ~0x1f;
	phy_addr &= 0x1f;
	reg_value |= phy_addr;
	writel(reg_value, ETH_SYSCTL_REG);
}
#else
void set_inner_phy_addr(u32 phyaddr)
{
}
#endif

void revise_led_shine(void)
{
}

void set_phy_valtage(void)
{
}

static void bspeth_reset(int rst)
{
	u32 val;

	val = _readl(ETH_CRG_REG);
	if (rst) {
		val |= ETH_SOFT_RESET;
	} else {
		val &= ~ETH_SOFT_RESET;
	}
	_writel(val, ETH_CRG_REG);

	udelay(100); /* delay 100us */
}

static inline void bspeth_clk_ena(void)
{
	u32 val = _readl(ETH_CRG_REG);
	val |= (ETH_CORE_CLK_SELECT_54M | ETH_CLK_ENABLE);
	_writel(val, ETH_CRG_REG);
}

static inline void bspeth_clk_dis(void)
{
	u32 val = _readl(ETH_CRG_REG);
	val &= ~ETH_CLK_ENABLE;
	_writel(val, ETH_CRG_REG);
}

static void bspeth_reset_internal_phy(void)
{
#ifdef INNER_PHY
	u32 rst;

	/* disable MDCK clock to make sure FEPHY reset success */
	bspeth_clk_dis();

	rst = readl(ETH_CRG_INPHY_CLK_REG);
	rst |= ETH_INTERNAL_PHY_CLK_EN;
	/* internal FEPHY only support MII mode */
	writel(rst, ETH_CRG_INPHY_CLK_REG);
	udelay(10); /* delay 10us */

	rst = _readl(ETH_CRG_INPHY_CLK_REG);
	rst |= ETH_INTERNAL_PHY_RESET;
	_writel(rst, ETH_CRG_INPHY_CLK_REG);
	/* delay at least 10ms */
	mdelay(15); /* delay 15ms */

	rst = _readl(ETH_CRG_INPHY_CLK_REG);
	rst &= ~ETH_INTERNAL_PHY_RESET;
	_writel(rst, ETH_CRG_INPHY_CLK_REG);
	/* delay at least 15ms for MDIO operation */
	mdelay(20); /* delay 20ms */

	bspeth_clk_ena();
	/* delay 5ms after enable MDCK to make sure FEPHY trim safe */
	mdelay(5); /* delay 5ms */
	bspeth_fephy_trim();
#endif
}

static void bspeth_reset_external_phy_by_crg(void)
{
#if defined(SFV_RESET_PHY_BY_CRG)
	u32 v;

	/************************************************/
	/* reset external phy with default reset pin */
	v = readl(ETH_CRG_EXTPHY_CLK_REG);
	v |= ETH_EXTERNAL_PHY_RESET;
	writel(v, ETH_CRG_EXTPHY_CLK_REG);

	mdelay(50); /* delay 50ms */

	/* then, cancel reset, and should delay some time */
	v = readl(ETH_CRG_EXTPHY_CLK_REG);
	v &= ~ETH_EXTERNAL_PHY_RESET;
	writel(v, ETH_CRG_EXTPHY_CLK_REG);

	mdelay(50); /* delay 50ms */
#endif
}

static void bspeth_reset_external_phy_by_gpio(void)
{
#ifdef SFV_RESET_GPIO_EN
	unsigned int val;
	/* gpiox[x] set to reset, then delay 200ms */
	val = __raw_readw(SFV_RESET_GPIO_BASE + SFV_RESET_GPIO_DIR);
	val |= (SFV_RESET_GPIO_DIR_OUT << SFV_RESET_GPIO_BIT);
	__raw_writew(val, SFV_RESET_GPIO_BASE + SFV_RESET_GPIO_DIR);
	__raw_writew(SFV_RESET_GPIO_DATA,
				 SFV_RESET_GPIO_BASE +
				 (4 << SFV_RESET_GPIO_BIT)); /* offset addr 4 */

	mdelay(200); /* delay 200ms */

	/* then,cancel reset,and should delay 200ms */
	val = __raw_readw(SFV_RESET_GPIO_BASE + SFV_RESET_GPIO_DIR);
	val |= (SFV_RESET_GPIO_DIR_OUT << SFV_RESET_GPIO_BIT);
	__raw_writew(val, SFV_RESET_GPIO_BASE + SFV_RESET_GPIO_DIR);
	__raw_writew(((!SFV_RESET_GPIO_DATA) << SFV_RESET_GPIO_BIT),
				 SFV_RESET_GPIO_BASE +
				 (4 << SFV_RESET_GPIO_BIT)); /* offset addr 4 */

	mdelay(20); /* delay 20ms */
#endif
}

static void bspeth_phy_reset(void)
{
	bspeth_reset_internal_phy();
	bspeth_reset_external_phy_by_crg();
	bspeth_reset_external_phy_by_gpio();
}

static void bspeth_funsel_config(void)
{
}

static void bspeth_funsel_restore(void)
{
}

void bspeth_sys_startup(void)
{
	bspeth_clk_ena();
	/* undo reset */
	bspeth_reset(0);
}

void bspeth_sys_allstop(void)
{
}

void bspeth_sys_init(void)
{
	bspeth_funsel_config();
	bspeth_sys_allstop();
	bspeth_clk_ena();
	bspeth_reset(1);
	bspeth_reset(0);
	bspeth_phy_reset();
	revise_led_shine();
}

void bspeth_sys_exit(void)
{
	bspeth_funsel_restore();
	bspeth_sys_allstop();
}
