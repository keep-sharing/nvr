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



#include <asm/arch/platform.h>
#include <config.h>

#define REG_BASE_PMC		0x20270000
#define REG_CORE_STATUS		(REG_BASE_PMC + 0x2c)
#define REG_INNER_PWM		REG_BASE_PMC

#ifdef	CONFIG_SVB_ENABLE
static inline unsigned int readl(unsigned int addr)
{
	unsigned int val;

	val = (*(volatile unsigned int *)(addr));
	return val;
}

static inline void writel(unsigned int val, unsigned int addr)
{
	(*(volatile unsigned int *)(addr)) = (val);
}

static inline void delay(unsigned int num)
{
	unsigned int i;

	for (i = 0; i < (100 * num); i++)
		__asm__ __volatile__("nop");
}

static void set_voltage(unsigned int value)
{
	writel(value, REG_INNER_PWM);
}

static unsigned int get_hpm_recorder(void)
{
	unsigned int aver_num = 4;
	unsigned int i;
	unsigned int reg_zone_status;
	unsigned int recorder0 = 0;
	unsigned int recorder1 = 0;
	unsigned int hpm_value0 = 0;
	unsigned int hpm_value1 = 0;
	unsigned int zone_recorder_aver = 0;

	reg_zone_status = REG_CORE_STATUS;

	for (i = 0; i < aver_num; i++) {
		recorder0 = readl(reg_zone_status);
		hpm_value0 += (recorder0 & 0x3ff) + ((recorder0 >> 12) & 0x3ff);

		recorder1 = readl(reg_zone_status + 0x4);
		hpm_value1 += (recorder1 & 0x3ff) + ((recorder1 >> 12) & 0x3ff);

		delay(5);
	}

	zone_recorder_aver = (hpm_value0 + hpm_value1);
	zone_recorder_aver = zone_recorder_aver >> 4; /* aver_num * 4 */

	return zone_recorder_aver;
}

void start_svb(void)
{
	unsigned int core_aver_recorder = 0;
	unsigned int reg_val = 0;

	delay(30000);
	/* CORE Hpm */
	core_aver_recorder = get_hpm_recorder();

	delay(5000);

	reg_val = readl(0x2005014c);

	/* adjusting voltage based on aver record */
	if (core_aver_recorder < reg_val) {
		reg_val = readl(0x20050150);
		set_voltage(reg_val);
	} else {
		reg_val = readl(0x20050154);
		set_voltage(reg_val);
	}
	delay(1000);
}
#endif
