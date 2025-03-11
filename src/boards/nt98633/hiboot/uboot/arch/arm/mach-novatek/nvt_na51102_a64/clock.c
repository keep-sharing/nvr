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
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
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

#define PLL_CPU_NO						16								//No. of CPU's PLL = 8
#define PLL_CLKSEL_CPU                  (0)
#define PLL_CLKSEL_CPU_80               (0x00 << PLL_CLKSEL_CPU)        //< Select CPU clock 80MHz
#define PLL_CLKSEL_CPU_ARMPLL           (0x01 << PLL_CLKSEL_CPU)        //< Select CPU clock ARMPLL (for CPU)
#define PLL_CLKSEL_CPU_480              (0x02 << PLL_CLKSEL_CPU)        //< Select CPU clock 480MHz
#define PLL_CLKSEL_CPUMASK              (0x3)

#ifdef CONFIG_NVT_FPGA_EMULATION
#define CALCULATE_CPU_FREQ_UNIT_US      100000
#define      timer_gettick()            readl(IOADDR_TIMER_REG_BASE + 0x108) * (10)
#define core_timer_gettick()            readl(IOADDR_SYSCNT_READ_BASE +0x00)
#else
#define CALCULATE_CPU_FREQ_UNIT_US      100000
#define      timer_gettick()            readl(IOADDR_TIMER_REG_BASE + 0x108)
#define core_timer_gettick()            readl(IOADDR_SYSCNT_READ_BASE +0x00)
#endif

void core_timer_delay(u32 us);
u64 ca53_get_cycle_count(void);
void ca53_cycle_count_stop(void);
void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider);

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

#ifdef CONFIG_NVT_FPGA_EMULATION
static int nvt_cpu_get_freq_by_core_timer(void)
{
	u32 freq;
	u32 time_1, time_2, temp, time_interval;

	ca53_cycle_count_start(TRUE, FALSE);
	time_1 = ca53_get_cycle_count();
	core_timer_delay(CALCULATE_CPU_FREQ_UNIT_US);
	time_2 = ca53_get_cycle_count();
	if (time_2 > time_1) {
		time_interval = (time_2 - time_1);
	} else {
		temp = 0xFFFFFFFF - time_1;
		time_interval = temp + time_2;
	}
	freq = (time_interval) / (CALCULATE_CPU_FREQ_UNIT_US);
	ca53_cycle_count_stop();
	return (freq * 1000);
}
#endif

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
	val <<= 17;
	val  /= 12;

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

	val *= 12;
	val += ((1UL << 17) - 1);
	val >>= 17;

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
#else
	return  nvt_cpu_get_freq_by_core_timer();
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

		cpu_freq_ratio = (cpu_freq_ratio << 5);

		return (12 * cpu_freq_ratio / 131072) * 1000;
#endif
	} else if (cpu_clk_sel == 2) {
		return 480000;
	} else {
		return 80000;
	}
}


#define PERF_DEF_OPTS           (1 | 16)
#define PERF_OPT_RESET_CYCLES   (2 | 4)
#define PERF_OPT_DIV64          (8)
#define ARMV8_PMCR_MASK         0x3f
#define ARMV8_PMCR_E            (1 << 0) /* Enable all counters */
#define ARMV8_PMCR_P            (1 << 1) /* Reset all counters */
#define ARMV8_PMCR_C            (1 << 2) /* Cycle counter reset */
#define ARMV8_PMCR_D            (1 << 3) /* CCNT counts every 64th cpu cycle */
#define ARMV8_PMCR_X            (1 << 4) /* Export to ETM */
#define ARMV8_PMCR_DP           (1 << 5) /* Disable CCNT if non-invasive debug*/
#define ARMV8_PMCR_LC           (1 << 6) /* Cycle Counter 64bit overflow*/
#define ARMV8_PMCR_N_SHIFT      11       /* Number of counters supported */
#define ARMV8_PMCR_N_MASK       0x1f

#define ARMV8_PMUSERENR_EN_EL0  (1 << 0) /* EL0 access enable */
#define ARMV8_PMUSERENR_CR      (1 << 2) /* Cycle counter read enable */
#define ARMV8_PMUSERENR_ER      (1 << 3) /* Event counter read enable */

static inline u32 armv8pmu_pmcr_read(void)
{
	u64 val = 0;
	asm volatile("mrs %0, pmcr_el0" : "=r"(val));
	return (u32)val;
}
static inline void armv8pmu_pmcr_write(u32 val)
{
	val &= ARMV8_PMCR_MASK;
	isb();
	asm volatile("msr pmcr_el0, %0" : : "r"((u64)val));
}
static inline  long long armv8_read_CNTPCT_EL0(void)
{
	long long val;
	asm volatile("mrs %0, CNTVCT_EL0" : "=r"(val));
	return val;
}

static void enable_cpu_counters(void)
{
	u64 val;
	/* Disable cycle counter overflow interrupt */
	asm volatile("msr pmintenset_el1, %0" : : "r"((u64)(0 << 31)));
	/* Enable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" :: "r" BIT(31));
	/* Enable user-mode access to cycle counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"(BIT(0) | BIT(2)));
	/* Clear cycle counter and start */
	asm volatile("mrs %0, pmcr_el0" : "=r"(val));
	val |= (BIT(0) | BIT(2));
	isb();
	asm volatile("msr pmcr_el0, %0" : : "r"(val));
	val = BIT(27);
	asm volatile("msr pmccfiltr_el0, %0" : : "r"(val));
}

static void disable_cpu_counters(void)
{
	/* Disable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" :: "r"(0 << 31));
	/* Disable user-mode access to counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"((u64)0));
}

static inline u64 arch_counter_get_cntpct(void)
{
	long long val;
	asm volatile("mrs %0, CNTVCT_EL0" : "=r"(val));
	return val;
}
//static BOOL cpu_count_open = FALSE;
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

void core_timer_delay(u32 us)
{
	u32     start, end;
	u32     tick;
	u32     total_tick;

	tick = (1000 / (CONFIG_SYS_HZ_CLOCK / 1000000));
	total_tick = us * 1000 / (u32)tick;
	start = core_timer_gettick();
	/*check timer count to target level*/
	while (1) {
		end = core_timer_gettick();
		if ((end - start) > (u32)total_tick) {
			break;
		}
	}
}

void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider)
{
	enable_cpu_counters();
	return;
}

/**
    CA53 get CPU clock cycle count

    get CPU clock cycle count

    @param[out] type
    @return success or not
        - @b UINT64:   clock cycle of CPU
*/
u64 ca53_get_cycle_count(void)
{
	u64 cval;
	isb();
	asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(cval));
	return cval;
}

void ca53_cycle_count_stop(void)
{
	disable_cpu_counters();
	return;
}

int do_nvt_cpu_get_freq(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u64 time_1, time_2, temp, time_interval;
	u32 freq;
	u32 fpga = 0;

	if (!strncmp(argv[1], "fpga", 4)) {
		fpga = 1;
	} else if (!strncmp(argv[1], "help", 4)) {
		return CMD_RET_USAGE;
	}
	ca53_cycle_count_start(TRUE, FALSE);
	time_1 = ca53_get_cycle_count();
	if (fpga) {
		core_timer_delay(CALCULATE_CPU_FREQ_UNIT_US);
	} else {
		timer2_delay(CALCULATE_CPU_FREQ_UNIT_US);
	}
	time_2 = ca53_get_cycle_count();

	if (time_2 > time_1) {
		time_interval = (time_2 - time_1);
	} else {
		temp = 0xFFFFFFFF - time_1;
		time_interval = temp + time_2;
	}

	freq = (time_interval) / (CALCULATE_CPU_FREQ_UNIT_US);
	ca53_cycle_count_stop();
	printf("CHIP[NA51102] =>");
	printf("CPU Freq %d MHz\n", freq);

	return 0;
}

#ifdef CONFIG_NVT_FPGA_EMULATION
U_BOOT_CMD(
	nvt_get_cpu_freq, 2,    1,  do_nvt_cpu_get_freq,
	"get cpu freq",
	"[Option] \n"
	"	          [fpga]\n"
	"	          [MHz]\n"
);
#else
U_BOOT_CMD(
	nvt_get_cpu_freq, 3,    1,  do_nvt_cpu_get_freq,
	"get cpu freq",
	"[MHz]\n"
);
#endif

int do_nvt_cpu_freq(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u32 value = simple_strtoul(argv[1], NULL, 10);
	u32 uiReg;

	if (value > 1400) {
		value = 1400;
		printf("force CPU clk @1400MHz\n");
	}
	switch (argc) {
	case 2:
		switch (value) {
		case 480:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_480) {
				printf("Already 480MHz)\n");
				break;
			} else {
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_480;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				printf("Set CPU clk 480MHz\n");

				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if((uiReg & (1<<PLL_CPU_NO))==(1<<PLL_CPU_NO)) {
					printf("PLL16 = Enabled => Disable it\n");
					uiReg &= ~(1<<PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					printf("PLL16 = 0x%08x \n", uiReg);
				}
			}
			break;
#ifdef CONFIG_NVT_FPGA_EMULATION
		case 80:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_80) {
				printf("Already 80MHz(24MHz@FPGA)\n");
				break;
			} else {
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_80;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				printf("Set CPU clk to 80MHz(24MHz@FPGA)\n");
			}
			break;

		case 1000:
		case 1200:
		case 1400:
		default:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) == PLL_CLKSEL_CPU_ARMPLL) {
				printf("Already ARMPLL\n");
			} else {
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				printf("Set CPU clk APLLMHz\n");
			}

			break;
#else
		case 1000:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if((uiReg & (1<<PLL_CPU_NO))!=(1<<PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1<<PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1000 MHz -> CPU-MPLL-125Mhz = 0x14D555
			writel(0x85, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x53, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x05, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1000MHz\n");
			break;
		case 1200:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if((uiReg & (1<<PLL_CPU_NO))!=(1<<PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1<<PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1200 MHz -> CPU-MPLL-150Mhz = 0x190000
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x40, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x06, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1200MHz\n");
			break;

		case 1400:
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if((uiReg & (1<<PLL_CPU_NO))!=(1<<PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1<<PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1400 MHz -> CPU-MPLL-150Mhz = 0x1D2AAA
			writel(0xAA, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x4A, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x07, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
			printf("Set CPU clk 1400MHz\n");
			break;
		default:
			printf("Not define this CPU Freq %d, use defualt value 1000!\n", value);
			uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
			if ((uiReg & PLL_CLKSEL_CPUMASK) != PLL_CLKSEL_CPU_ARMPLL) {
				uiReg = readl(IOADDR_STBC_CG_REG_BASE);
				if((uiReg & (1<<PLL_CPU_NO))!=(1<<PLL_CPU_NO)) {
					printf("PLL%d = Disabled => Enable it\n", PLL_CPU_NO);
					uiReg |= (1<<PLL_CPU_NO);
					writel(uiReg, IOADDR_STBC_CG_REG_BASE); // B0
					uiReg = readl(IOADDR_STBC_CG_REG_BASE);
					//Polling PLL16
					while (!(readl(IOADDR_STBC_CG_REG_BASE + 0x04) & (1 << PLL_CPU_NO)));
					printf("PLL%d enable done= 0x%08x 0x%08x\n", PLL_CPU_NO, readl(IOADDR_STBC_CG_REG_BASE + 0x00), readl(IOADDR_STBC_CG_REG_BASE + 0x04));
				}
				uiReg = readl(IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS);
				uiReg &= ~PLL_CLKSEL_CPUMASK;
				uiReg |= PLL_CLKSEL_CPU_ARMPLL;
				writel(uiReg, (IOADDR_CG_REG_BASE + SYSTEM_CLOCK_RATE_OFS));
				writel(0x34, (IOADDR_STBC_CG_REG_BASE + 0xB0));
				while (!(readl(IOADDR_STBC_CG_REG_BASE + 0xB0) & (1 << 3)));
				writel(0x28, (IOADDR_STBC_CG_REG_BASE + 0xB0));
			}
			//CPU 1000 MHz -> CPU-MPLL-125Mhz = 0x14D555
			writel(0x00, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO0_OFS); // B0
			writel(0x40, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO1_OFS); // B1
			writel(0x06, IOADDR_CG_REG_BASE + CPU_CLOCK_RATE_RATIO2_OFS); // B2
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
#ifdef CONFIG_NVT_FPGA_EMULATION
	"nvt_cpu_freq 480(change to 48MHz@FPGA)\n"
	"nvt_cpu_freq  80(change to 24MHz@FPGA)\n"
	"nvt_cpu_freq !=480 && != 80(will change to ARMPLL@FPGA, check excel)\n"
#else
	"nvt_cpu_freq  480\n"
	"nvt_cpu_freq 1000\n"
	"nvt_cpu_freq 1200\n"
	"nvt_cpu_freq 1400\n"
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
