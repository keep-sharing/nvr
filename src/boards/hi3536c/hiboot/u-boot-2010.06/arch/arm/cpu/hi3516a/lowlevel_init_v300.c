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



#include <asm/arch/platform.h>
#include <config.h>

#define  REG_BASE_MDDRC		0x20111000

#ifdef CONFIG_DDR_DATAEYE_TRAINING_STARTUP
extern int ddr_dataeye_training(void *param);
#endif /* CONFIG_DDR_DATAEYE_TRAINING_STARTUP */

extern void ddr_training_info(int value);
extern int ddrphy_train_route(void);

extern void reset_cpu(unsigned long addr);
extern void ddr_training_print(int type, int num);

static inline unsigned int readl(unsigned addr)
{
	unsigned int val;
	val = (*(volatile unsigned int *)(addr));
	return val;
}

static inline void writel(unsigned val, unsigned addr)
{
	(*(volatile unsigned *) (addr)) = (val);
}

void start_ddr_training(unsigned int base)
{
	int ret = 0;

	/* ddr training function */
	if (!(readl(base + REG_SC_DDRT0) & 0x1)) {
		ret = ddrphy_train_route();
		if (ret) {
			ddr_training_print(ret&0xffff, ret>>16);
			ddr_training_info(ret);
			reset_cpu(0);
		}
	}

#ifdef CONFIG_DDR_DATAEYE_TRAINING_STARTUP
	if (!(readl(base + REG_SC_DDRT0) & (0x1 << 16))) {
		ret = ddr_dataeye_training(0);
		if (ret) {
			ddr_training_info(ret);
			if (!(readl(REG_BASE_SCTL + REG_SC_DDRT0)
						& (0x1 << 31)))
				reset_cpu(0);
		}
	}
#endif /* CONFIG_DDR_DATAEYE_TRAINING_STARTUP */

	/*do sf read dqs gating*/
	ret = readl(REG_BASE_SCTL + REG_SC_DDRT0);
	if ((!(ret & 0x1)) && (ret & 0x100)) {
		writel(0x10, REG_BASE_SCTL + REG_SC_DDRT0);
		ddrphy_train_route();
		writel(ret, REG_BASE_SCTL + REG_SC_DDRT0);
	}

	/*the value should config after trainning, or
	 it will cause chip compatibility problems*/
	writel(0x401, REG_BASE_MDDRC + 0x28);
}

