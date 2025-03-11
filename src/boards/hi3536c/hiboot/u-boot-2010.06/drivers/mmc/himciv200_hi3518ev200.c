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

extern void udelay(unsigned long usec);

static void hi_mci_sys_init(unsigned int dev_num)
{
	unsigned int tmp_reg;
	HIMCI_DEBUG_FUN("Function Call");

	if (dev_num == 0) {
		/* SDIO0 clock phase */
		tmp_reg = himci_readl(CRG_REG_BASE + REG_CRG49);
		tmp_reg &= ~SDIO0_CLK_SEL_MASK;
		tmp_reg |= SDIO0_CLK_SEL_49_5M;
		himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);

		/* SDIO0 soft reset */
		tmp_reg |= SDIO0_SRST_REQ;
		himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
		udelay(100);
		tmp_reg &= ~SDIO0_SRST_REQ;
		tmp_reg |= SDIO0_CKEN;
		himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
	} else if (dev_num == 1) {
		/* SDIO1 clock phase */
		tmp_reg = himci_readl(CRG_REG_BASE + REG_CRG49);
		tmp_reg &= ~SDIO1_CLK_SEL_MASK;
		tmp_reg |= SDIO1_CLK_SEL_49_5M;
		himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);

		/* SDIO1 soft reset */
		tmp_reg |= SDIO1_SRST_REQ;
		himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
		udelay(100);
		tmp_reg &= ~SDIO1_SRST_REQ;
		tmp_reg |= SDIO1_CKEN;
		himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
	}
}
