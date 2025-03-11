// SPDX-License-Identifier: GPL-2.0
/*
 * Watchdog driver for Novatek IVOT SoCs
 *
 * Copyright (C) 2020 Novatek Inc.
 * Author: Howard Chang
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>

#define WDT_REG_ADDR(ofs)       (IOADDR_WDT_REG_BASE+(ofs))
#define WDT_GETREG(ofs)         INW(WDT_REG_ADDR(ofs))
#define WDT_SETREG(ofs,value)   OUTW(WDT_REG_ADDR(ofs), (value))

#define CG_REG_ADDR(ofs)       (IOADDR_CG_REG_BASE+(ofs))
#define CG_GETREG(ofs)         INW(CG_REG_ADDR(ofs))
#define CG_SETREG(ofs,value)   OUTW(CG_REG_ADDR(ofs), (value))

#define CG_ENABLE_OFS 0x78
#define CG_RESET_OFS 0x5C
#define WDT_RST_POS (1 << 28)
#define WDT_EN_POS (1 << 15)

#define WDT_CTRL_OFS 0x0
#define WDT_TRIG_OFS 0x8

static void nvtviot_wdt_init(void)
{
	u32 reg_value;
	u32 timeout, timeout_unit = CONFIG_NVT_WDT_TIMEOUT;

	reg_value = CG_GETREG(CG_ENABLE_OFS);
	CG_SETREG(CG_ENABLE_OFS, reg_value & ~WDT_EN_POS);

	reg_value = CG_GETREG(CG_RESET_OFS);
	CG_SETREG(CG_RESET_OFS, reg_value | WDT_RST_POS);

	timeout = timeout_unit * 3;
	if ((timeout > 0xFF) || !timeout)
		timeout = 0xFF;

	reg_value = (0x5A960012 | (timeout << 8));

	WDT_SETREG(WDT_CTRL_OFS, reg_value);

	udelay(80);

	WDT_SETREG(WDT_CTRL_OFS, reg_value | 0x1);
}

static void nvtviot_wdt_reset(void)
{
#ifndef CONFIG_WATCHDOG_RESET_DISABLE
	/*Kick the dog*/
	WDT_SETREG(WDT_TRIG_OFS, 0x1);
#endif
}

#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	nvtviot_wdt_reset();
}

void hw_watchdog_init(void)
{
	nvtviot_wdt_init();
}
#endif