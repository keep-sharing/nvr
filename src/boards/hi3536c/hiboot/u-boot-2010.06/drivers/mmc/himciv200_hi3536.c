/*
 * Copyright (c) 2013 HiSilicon Technologies Co., Ltd.
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

#define PERI_CRG49			(0x12040000 + 0xC4)
#define SDIO0_CLK_SEL_MASK              (0x3 << 2)
#define SDIO0_CLK_SEL_50M               (0x1 << 2)
#define SDIO0_CLK_SEL_24M               (0x0 << 2)
#define SDIO0_SRST_REQ                  (0x1 << 0)
#define SDIO0_CKEN                      (0x1 << 1)

#define PERI_CRG132			(0x12040000 + 0x210)
#define SDIO1_CLK_SEL_MASK              (0x7 << 4)
#define SDIO1_CLK_SEL_50M               (0x0 << 4)
#define SDIO1_CLK_SEL_25M               (0x2 << 4)
#define SDIO1_SRST_REQ                  (0x1 << 0)
#define SDIO1_CKEN                      (0x1 << 1)

extern void udelay(unsigned long usec);

static void hi_mci_sys_init(unsigned int dev_num)
{
	unsigned int tmp_reg;
	HIMCI_DEBUG_FUN("Function Call");

	if (dev_num == 1) {
		/* SDIO0 clock phase */
		tmp_reg = himci_readl(PERI_CRG49);
		tmp_reg &= ~SDIO0_CLK_SEL_MASK;
		tmp_reg |= SDIO0_CLK_SEL_50M;
		himci_writel(tmp_reg, PERI_CRG49);

		/* SDIO0 soft reset */
		tmp_reg |= SDIO0_SRST_REQ;
		himci_writel(tmp_reg, PERI_CRG49);
		udelay(1000);
		tmp_reg &= ~SDIO0_SRST_REQ;
		tmp_reg |= SDIO0_CKEN;
		himci_writel(tmp_reg, PERI_CRG49);

		udelay(1000);
	} else if (dev_num == 0) {
		/* SDIO1 clock phase */
		tmp_reg = himci_readl(PERI_CRG132);
		tmp_reg &= ~SDIO1_CLK_SEL_MASK;
		tmp_reg |= SDIO1_CLK_SEL_50M;
		himci_writel(tmp_reg, PERI_CRG132);

		/* SDIO1 soft reset */
		tmp_reg |= SDIO1_SRST_REQ;
		himci_writel(tmp_reg, PERI_CRG132);
		udelay(1000);
		tmp_reg &= ~SDIO1_SRST_REQ;
		tmp_reg |= SDIO1_CKEN;
		himci_writel(tmp_reg, PERI_CRG132);

		udelay(1000);
	}
}
