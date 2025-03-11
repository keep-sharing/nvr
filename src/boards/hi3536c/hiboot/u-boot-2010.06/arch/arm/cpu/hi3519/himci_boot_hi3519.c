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


#define REG_PERI_CRG49		0xC4
#define EMMC_SRST_REQ           (0x1 << 16)
#define EMMC_CKEN               (0x1 << 17)
static inline void emmc_sys_init(void)
{

	unsigned int tmp_reg;
	/* emmc clock phase */
	tmp_reg = himci_readl(CRG_REG_BASE + REG_PERI_CRG49);

	/* emmc soft reset */
	tmp_reg |= EMMC_SRST_REQ;
	himci_writel(tmp_reg, CRG_REG_BASE + REG_PERI_CRG49);
	delay(1000 * DELAY_US);
	tmp_reg &= ~EMMC_SRST_REQ;
	tmp_reg |= EMMC_CKEN;
	himci_writel(tmp_reg, CRG_REG_BASE + REG_PERI_CRG49);
}
