/*
 * Copyright (c) 2010 HiSilicon Technologies Co., Ltd.
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

#include <asm/arch/platform.h>

static void hi_mci_sys_init(void)
{
	unsigned int tmp_reg;

	HIMCI_DEBUG_FUN("Function Call");

	/* enable SDIO clock and clock 50MHz */
	tmp_reg = himci_readl(REG_BASE_CRG + REG_PERI_CRG46);
	tmp_reg &= ~(SDIO_SRST_REQ | SDIO_CKEN | SDIO_HCLKEN
			 | SDIO_CLK_50M | SDIO_CLK_PCTRL);
	tmp_reg |= SDIO_CKEN | SDIO_HCLKEN | SDIO_CLK_50M;
	himci_writel(tmp_reg, REG_BASE_CRG + REG_PERI_CRG46);

	/* SDIO soft reset */
	tmp_reg |= SDIO_SRST_REQ;
	himci_writel(tmp_reg, REG_BASE_CRG + REG_PERI_CRG46);
	udelay(1000);
	tmp_reg &= ~SDIO_SRST_REQ;
	himci_writel(tmp_reg, REG_BASE_CRG + REG_PERI_CRG46);
}

