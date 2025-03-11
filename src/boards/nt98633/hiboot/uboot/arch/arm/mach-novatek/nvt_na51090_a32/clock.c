/**
    Clock info

    @file       clock.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/arch/efuse_protected.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>

#define SYSTEM_CLOCK_RATE_OFS           0x10
#define CPU_CLOCK_RATE_RATIO0_OFS       0x4820
#define CPU_CLOCK_RATE_RATIO1_OFS       0x4824
#define CPU_CLOCK_RATE_RATIO2_OFS       0x4828
#define DMA_CLOCK_RATE_RATIO0_OFS       (0x4000 + 0x4C0 + 0x20)
#define DMA_CLOCK_RATE_RATIO1_OFS       (0x4000 + 0x4C0 + 0x24)
#define DMA_CLOCK_RATE_RATIO2_OFS       (0x4000 + 0x4C0 + 0x28)

#define PLL_CLKSEL_CPU					0
#define PLL_CLKSEL_CPU_80         		(0x00 << PLL_CLKSEL_CPU)      	//< Select CPU clock 80MHz
#define PLL_CLKSEL_CPU_ARMPLL      		(0x01 << PLL_CLKSEL_CPU)      	//< Select CPU clock ARMPLL (for CPU)
#define PLL_CLKSEL_CPU_480        		(0x02 << PLL_CLKSEL_CPU)      	//< Select CPU clock 480MHz
#define PLL_CLKSEL_CPUMASK				0x3

#define CPU_NORMAL_MAX_CLOCK_RATE		1200
#define CPU_OV_CLK_MAX_CLOCK_RATE		1350

inline void apll_set_data(u8 offset, u8 value)
{
#if 0
	writel(value, (APLL_BASE_ADDR + (offset * 4)));
#endif
}

inline u32 apll_get_data(u8 offset)
{
	return 0;
}

#if 0
inline void apll_enable(pll_page_t page)
{
	if (PLL_PAGE_0 == page) {
		writel(APLL_PAGE_0_EN, (APLL_PAGE_EN_ADDR));
	} else if (PLL_PAGE_B == page) {
		writel(APLL_PAGE_B_EN, (APLL_PAGE_EN_ADDR));
	} else {
		/* ignore */
	}
}
#endif

inline void mpll_set_data(u8 offset, u8 value)
{
#if 0
	writel(value, (MPLL_BASE_ADDR + (offset * 4)));
#endif
}

inline u32 mpll_get_data(u8 offset)
{
	return 0;
#if 0
	return readl((MPLL_BASE_ADDR + (offset * 4)));
#endif
}

#if 0
inline void mpll_enable(pll_page_t page)
{
	if (PLL_PAGE_0 == page) {
		writel(MPLL_PAGE_0_EN, (MPLL_PAGE_EN_ADDR));
	} else if (PLL_PAGE_B == page) {
		writel(MPLL_PAGE_B_EN, (MPLL_PAGE_EN_ADDR));
	} else {
		/* ignore */
	}
}
#endif

void set_sys_mpll(unsigned long off, unsigned long val)
{
#if 0
	val <<= 17; val  /= 12;

	mpll_enable(PLL_PAGE_B);

	mpll_set_data((off + 0), ((val >> 0) & 0xff));
	mpll_set_data((off + 1), ((val >> 8) & 0xff));
	mpll_set_data((off + 2), ((val >> 16) & 0xff));
#endif
}

unsigned long get_sys_mpll(unsigned long off)
{
#if 0
	unsigned long val;

	mpll_enable(PLL_PAGE_B);

	val  = mpll_get_data(off);
	val |= (mpll_get_data((off + 1)) << 8);
	val |= (mpll_get_data((off + 2)) << 16);

	val *= 12;val += ((1UL << 17) - 1); val >>= 17;

	return val;
#endif
	return 0;
}

void set_cpu_clk(unsigned long freq)
{
	/* no implement */
}

unsigned long get_cpu_clk(void)
{
	unsigned int cpu_clk_sel = 0;
#ifndef CONFIG_NVT_FPGA_EMULATION
	unsigned int cpu_freq_ratio;
	unsigned int cpu_freq_ratio0, cpu_freq_ratio1, cpu_freq_ratio2;
#endif
	cpu_clk_sel = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS) & 0x3;

	if (cpu_clk_sel == 1) {
#ifdef CONFIG_NVT_FPGA_EMULATION
			return 24000;
#else
		cpu_freq_ratio0 = readl(IOADDR_CG_REG_BASE + \
					CPU_CLOCK_RATE_RATIO0_OFS);
		cpu_freq_ratio1 = readl(IOADDR_CG_REG_BASE + \
					CPU_CLOCK_RATE_RATIO1_OFS);
		cpu_freq_ratio2 = readl(IOADDR_CG_REG_BASE + \
					CPU_CLOCK_RATE_RATIO2_OFS);
		cpu_freq_ratio = cpu_freq_ratio0 | (cpu_freq_ratio1 << 8) | \
				(cpu_freq_ratio2 << 16);

		cpu_freq_ratio = (cpu_freq_ratio << 3);

		return (12*cpu_freq_ratio / 131072) * 1000;
#endif
	} else if (cpu_clk_sel == 2)
		return 480000;
	else
		return 80000;
}

#ifdef CONFIG_NVT_FPGA_EMULATION
#define CALCULATE_CPU_FREQ_UNIT_US		100000
#define timer_gettick()	readl(IOADDR_TIMER_REG_BASE + 0x108) * (10)
#else
#define CALCULATE_CPU_FREQ_UNIT_US		100000
#define timer_gettick()	readl(IOADDR_TIMER_REG_BASE + 0x108)
#endif

#define read_PMCR() \
	({ \
		unsigned long cfg; \
		__asm__ __volatile__(\
				"mrc p15, 0, %0, c9, c12, 0\n\t" \
				: "=r"(cfg) \
				);\
		cfg;\
	})

#define write_PMCR(m)\
	({\
		__asm__ __volatile__("mcr p15, 0, %0, c9, c12, 0" : : "r" (m));\
	})

static BOOL cpu_count_open = FALSE;
void timer2_delay(u32 us)
{
	u32 start, end;
	start = timer_gettick();
	/*check timer count to target level*/
	while (1) {
		end = timer_gettick();
		if ((end - start) > us) {
			break;
		}
	}
}

void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider)
{
	/* in general enable all counters (including cycle counter)*/
	int value = 1;

	if (cpu_count_open == TRUE) {
		printf("cpu count start already\r\n");
		return;
	}

	cpu_count_open = TRUE;

	if (do_reset) {
		value |= 2;     /* reset all counters to zero.*/
		value |= 4;     /* reset cycle counter to zero.*/
	}

	if (enable_divider) {
		value |= 8;    /* enable "by 64" divider for CCNT.*/
	}

	value |= 16;

	/* program the performance-counter control-register: */
	asm volatile("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));

	/* enable all counters: */
	asm volatile("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));

	/* clear overflows: */
	asm volatile("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
	return;
}

/**
	CA53 get CPU clock cycle count

	get CPU clock cycle count

	@param[out] type
	@return success or not
		- @b UINT64:   clock cycle of CPU
*/
u32 ca53_get_cycle_count(void)
{
	unsigned int value;
	__asm__ __volatile__("mrc p15, 0, %0, c9, c13, 0\n\t"
						: "=r"(value)
						);
	return value;
}

void ca53_cycle_count_stop(void)
{
	u32 pmcr_reg;

	if (cpu_count_open == FALSE) {
		printf("cpu count not start yet\r\n");
		return;
	}

	cpu_count_open = FALSE;
	pmcr_reg = read_PMCR();
	pmcr_reg &= ~(0x1 << 0); /*E*/
	pmcr_reg &= ~(0x1 << 5); /*DP increase each clock cycle*/

	write_PMCR(pmcr_reg);
	return;
}

int do_nvt_cpu_get_freq(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 /*test, */freq;
	u32 time_1, time_2, temp, time_interval;

	ca53_cycle_count_start(TRUE, FALSE);
	//for (test = 1; test <= 1; test ++) {
	time_1 = ca53_get_cycle_count();
	timer2_delay(CALCULATE_CPU_FREQ_UNIT_US);
	time_2 = ca53_get_cycle_count();

	if (time_2 > time_1) {
		time_interval = (time_2 - time_1);
	} else {
		temp = 0xFFFFFFFF - time_1;
		time_interval = temp + time_2;
	}

	freq = (time_interval) / (CALCULATE_CPU_FREQ_UNIT_US);
	ca53_cycle_count_stop();
	printf("CPU Freq %d MHz\n", freq);

	return 0;
}


U_BOOT_CMD(
	nvt_get_cpu_freq, 2,    1,  do_nvt_cpu_get_freq,
	"get cpu freq",
	"[MHz]\n"
);


int do_nvt_cpu_freq(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u32 value = simple_strtoul(argv[1], NULL, 10);
	u32 uiReg;

	if(is_c_n_ovr_clk()) {
		if (value > CPU_NORMAL_MAX_CLOCK_RATE) {
			value = CPU_NORMAL_MAX_CLOCK_RATE;
			printf("[NM]force CPU clk @%dMHz\n", value);
		}
	} else {
		if (value > CPU_OV_CLK_MAX_CLOCK_RATE) {
			value = CPU_OV_CLK_MAX_CLOCK_RATE;
			printf("[OV]force CPU clk @%dMHz\n", value);
		}
	}
	switch (argc) {
	case 2:
		switch (value) {
		case 480:
    		uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
    		if((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_480) {
				printf("Already 480MHz\n");
				break;
    		} else {
    			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
                uiReg &= ~PLL_CLKSEL_CPUMASK;
                uiReg |= PLL_CLKSEL_CPU_480;
                writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
                writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
                while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
                writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
                printf("Set CPU clk 480MHz\n");
            }
			break;
#ifdef CONFIG_NVT_FPGA_EMULATION
		case 1000:
		case 1200:
		case 1400:
		default:
		uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
		if((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_ARMPLL) {
			printf("Already ARMPLL\n");
		} else {
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
            uiReg &= ~PLL_CLKSEL_CPUMASK;
            uiReg |= PLL_CLKSEL_CPU_ARMPLL;
            writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
            writel(0x34, (IOADDR_CG_REG_BASE + 0xB0));
            while (!(readl(IOADDR_CG_REG_BASE + 0xB0) & (1 << 3)));
            writel(0x28, (IOADDR_CG_REG_BASE + 0xB0));
            printf("Set CPU clk APLLMHz\n");
		}

		break;
#else
		case 1000:
			//CPU 1000 MHz -> CPU-MPLL-125Mhz = 0x14D555
			writel(0x55, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0xD5, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x14, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1000MHz\n");
			break;

		case 1100:
			//CPU 1100 MHz -> CPU-MPLL-137.5Mhz = 0x16EAAA
			writel(0xAA, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0xEA, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x16, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1100MHz\n");
			break;

		case 1200:
			//CPU 1200 MHz -> CPU-MPLL-150Mhz = 0x190000
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x19, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1200MHz\n");
			break;

		case 1300:
			//CPU 1300 MHz -> CPU-MPLL-162.5Mhz = 0x1B1555
			writel(0x55, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x15, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x1B, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1300MHz\n");
			break;

		case 1350:
			//CPU 1350 MHz -> CPU-MPLL-168.75Mhz = 0x1C2000
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x20, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x1C, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1350MHz\n");
			break;

		default:
			printf("Not define this CPU Freq %d, use defualt value 1200!\n", value);
			printf("Support CPU freq=(1000,1100,1200,1300,1350)MHz\n");
			//CPU 1200 MHz -> CPU-MPLL-150Mhz = 0x190000
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x19, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1200MHz\n");
			break;
#endif
		}
		break;
	default:
		printf("CPU:%d => type nvt_cpu_freq <freq>\n", (int)get_cpu_clk());
		return CMD_RET_USAGE;
	}
	return 0;
}


U_BOOT_CMD(
	nvt_cpu_freq, 2,    1,  do_nvt_cpu_freq,
	"change cpu freq",
	"[MHz]\n"
	"nvt_cpu_freq 480\n"
#ifdef CONFIG_NVT_FPGA_EMULATION
	"nvt_cpu_freq !=480(change to ARMPLL@FPGA)\n"
#else
	"nvt_cpu_freq 1000\n"
	"nvt_cpu_freq 1100\n"
	"nvt_cpu_freq 1200\n"
	"nvt_cpu_freq 1300\n"
	"nvt_cpu_freq 1350\n"
#endif
);


int do_nvt_ddr_freq(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int freq_ratio;
	unsigned int freq_ratio0, freq_ratio1, freq_ratio2;

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x20
	freq_ratio0 = readl(IOADDR_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO0_OFS);

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x24
	freq_ratio1 = readl(IOADDR_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO1_OFS);

	//0xF0020000 + (0x4000 + 0x400 + 0x20) = 0xF0024400 + 0x28
	freq_ratio2 = readl(IOADDR_CG_REG_BASE + \
						DMA_CLOCK_RATE_RATIO2_OFS);
	freq_ratio = freq_ratio0 | (freq_ratio1 << 8) | \
				 (freq_ratio2 << 16);

	freq_ratio = (freq_ratio << 3);
	printf("DMA clock rate = %d => [%d]MHz\r\n", (12 * freq_ratio / 131072), ((12 * freq_ratio / 131072) >> 1));
	return 0;
}

U_BOOT_CMD(
	nvt_get_ddr_freq, 2,      1,     do_nvt_ddr_freq,
	"get ddr freq/type\n",
	"\n"
);

