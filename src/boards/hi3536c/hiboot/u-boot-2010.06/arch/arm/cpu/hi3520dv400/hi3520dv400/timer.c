/*
 * Configuation settings for the hi3520dv400 board.
 *
 * Copyright (c) 2016-2017 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
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

#include <common.h>
#include <asm/proc-armv/ptrace.h>
#include <asm/arch/platform.h>
#include <asm/io.h>
#define TIMER_LOAD_VAL 0xffffffff

static ulong timestamp;
static ulong lastdec;


int timer_init(void)
{
	unsigned int regval;

	/* Select bus clock as the reference clk Timer0. */
	regval = __raw_readl(SYS_CTRL_REG_BASE + REG_SC_CTRL);
	regval &= ~TIMER0_CLK_SEL_MASK;
	regval |= SC_CTRL_TIMER0_CLK_SEL(TIMER_CLK_3M);
	__raw_writel(regval, SYS_CTRL_REG_BASE + REG_SC_CTRL);

	/*
	 * Under uboot, 0xffffffff is set to load register,
	 * timer_clk = BUSCLK / 4 / 256.
	 * e.g. BUSCLK = 50M, it will roll back after
	 * 0xffffffff / timer_clk ~= 87961s ~= 24hours
	 */

	/* Init timer control */
	__raw_writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);

	/* Set to load register initial value 0xffffffff */
	__raw_writel(TIMER_LOAD_VAL, CFG_TIMERBASE + REG_TIMER_RELOAD);

	/* Enable timer, and set default timer configure */
	__raw_writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	/* Init the time stamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER;  /* capure current decrementer value time */
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

ulong get_timer_masked(void)
{
	ulong now = READ_TIMER;		/* current tick value */

	if (lastdec >= now) {
		/* normal mode, not roll back */
		timestamp += lastdec - now;
	} else {
		/* rollback */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}


void set_timer(ulong t)
{
	timestamp = t;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}

/* delay x useconds AND perserve advance timstamp value */
void __udelay(unsigned long usec)
{
	ulong tmo, tmp;

	/* if "big" number, spread normalization to seconds */
	if (usec >= 1000) {
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= CONFIG_SYS_HZ;
		/* finish normalize. */
		tmo /= 1000;
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000 * 1000);
	}

	tmp = get_timer(0);		/* get current timestamp */
	/* if setting this fordward will roll time stamp */
	if ((tmo + tmp + 1) < tmp)
		reset_timer_masked();	/* reset "advancing" timestamp to 0,
					   set lastdec value */
	/* else, set advancing stamp wake up time */
	else
		tmo += tmp;

	/* loop till event */
	while (get_timer_masked() < tmo)
		;
}

/* waits specified delay value and resets timestamp */
void udelay_masked(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	/* if "big" number, spread normalization to seconds */
	if (usec >= 1000) {
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= CONFIG_SYS_HZ;
		/* finish normalize. */
		tmo /= 1000;
	} else {			/* else small number,don't kill
					 * it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000 * 1000);
	}

	endtime = get_timer_masked() + tmo;

	do {
		ulong now = get_timer_masked();

		diff = endtime - now;
	} while (diff >= 0);
}


