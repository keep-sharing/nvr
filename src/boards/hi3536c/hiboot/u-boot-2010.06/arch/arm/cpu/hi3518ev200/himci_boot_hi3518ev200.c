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


static inline void emmc_sys_init(void)
{
	unsigned int tmp_reg;

	/* SDIO clock phase */
	tmp_reg = himci_readl(CRG_REG_BASE + REG_CRG49);
	tmp_reg &= ~SDIO0_CLK_SEL_MASK;
	tmp_reg |= SDIO0_CLK_SEL_49_5M; /* phasic move to xls table. */
	himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);

	/* SDIO soft reset */
	tmp_reg |= SDIO0_SRST_REQ;
	himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
	delay(100 * DELAY_US);
	tmp_reg &= ~SDIO0_SRST_REQ;
	tmp_reg |= SDIO0_CKEN;
	himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
}

