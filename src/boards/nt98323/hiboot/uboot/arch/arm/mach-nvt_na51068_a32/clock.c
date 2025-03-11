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
#include <asm/arch/efuse_protected.h>
#include <asm/nvt-common/nvt_common.h>

#define CG_CPU_CKCTRL_REG_OFFSET 0x100
#define CPU_CLK_SEL_CPU_APLL  0x80000000
#define CPU_CLOCK_RATE_RATIO0_OFS 0x107E0
#define CPU_CLOCK_RATE_RATIO1_OFS 0x107E4
#define CPU_CLOCK_RATE_RATIO2_OFS 0x107E8

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
	}
	else if (PLL_PAGE_B == page) {
		writel(APLL_PAGE_B_EN, (APLL_PAGE_EN_ADDR));
	}
	else {
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
	}
	else if (PLL_PAGE_B == page) {
		writel(MPLL_PAGE_B_EN, (MPLL_PAGE_EN_ADDR));
	}
	else {
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

unsigned int pmu_readpll(u32 offset)
{
	u32 value = 0, data = 0, tmp = 0;

	value = readl(IOADDR_CG_REG_BASE + 0x10000 + offset+ 0x20);
	data = readl(IOADDR_CG_REG_BASE + 0x10000 + offset + 0x24);
	value |= (data & 0xFF) << 8;
	data = readl(IOADDR_CG_REG_BASE + 0x10000 + offset + 0x28);
	value |= (data & 0xFF) << 16;

	data = (value * 12) >> 17;
	tmp = data << 17;

	if(offset != 0x4C0) {
		if(tmp < value * 12) /* rounding issue */
			data++;
	}
	return data;
}

unsigned long get_cpu_clk(void)
{
#ifdef CONFIG_NVT_FPGA_EMULATION_CA9
	return 24000000;
#else
	return pmu_readpll(0x7C0) * 8 * 1000;
#endif
}

int prepare_power_trim(int mode, int freq)
{
    BOOL      is_found;
    int       code;
    unsigned int    power_trim, remap_data;
    char cmd[50];

    code = otp_key_manager(7);

    if(code == -33) {
        printf("Read power trim error\r\n");
        return -1;
    } else {
        power_trim = 0;
        is_found = extract_trim_valid(code, (UINT32 *)&power_trim);
        if(is_found) {
            printf("Read power trim = 0x%08x\r\n", power_trim);
        } else {
            printf("Read power trim = 0x%08x(NULL)\r\n", power_trim);
        }

        if (mode == PWM0_CORE_POWER) {
            remap_data = extract_trim_to_pwm_duty(PWM0_CORE_POWER, power_trim, freq);
            printf("ddr %d remap_data %d\n", freq, remap_data);
            sprintf(cmd, "power_ctl %d", remap_data);
        } else {
            remap_data = extract_trim_to_pwm_duty(PWM1_CPU_POWER, power_trim, freq);
            printf("cpu %d remap_data %d\n", freq, remap_data);
            sprintf(cmd, "cpu_power_ctl %d", remap_data);
        }
        run_command(cmd, 0);
    }
    return 0;
}

int do_nvt_cpu_freq(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u32 value = simple_strtoul(argv[1], NULL, 10);

    switch (argc) {
    case 2:
            switch (value) {
            case 800:
                    if (prepare_power_trim(PWM1_CPU_POWER, 800))
                        return -1;

                    //CPU 800 MHz -> CPU-MPLL-100Mhz = 0x10AAAA
                    writel(0xAA, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x20)); // B0
                    writel(0xAA, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x24)); // B1
                    writel(0x10, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x28)); // B2
                    printf("Set CPU clk 800MHz\n");
                    break;
            case 1000:
                    if (prepare_power_trim(PWM1_CPU_POWER, 1000))
                        return -1;

                    //CPU 1000 MHz -> CPU-MPLL-125Mhz = 0x14D555
                    writel(0x55, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x20)); // B0
                    writel(0xD5, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x24)); // B1
                    writel(0x14, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x28)); // B2
                    printf("Set CPU clk 1000MHz\n");
                    break;
            case 1200:
                    if (prepare_power_trim(PWM1_CPU_POWER, 1200))
                        return -1;

                    //CPU 1200 MHz -> CPU-MPLL-150Mhz = 0x190000
                    writel(0x00, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x20)); // B0
                    writel(0x00, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x24)); // B1
                    writel(0x19, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x28)); // B2
                    printf("Set CPU clk 1200MHz\n");
                    break;
            case 1400:
                    if (prepare_power_trim(PWM1_CPU_POWER, 1400))
                        return -1;

                    //CPU 1400 MHz -> CPU-MPLL-175Mhz = 0x1D2AAA
                    writel(0xAA, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x20)); // B0
                    writel(0x2A, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x24)); // B1
                    writel(0x1D, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x28)); // B2
                    printf("Set CPU clk 1400MHz\n");
                    break;
            default:
                    printf("Not define this CPU Freq %d, use defualt value 1400!\n", value);
                    if (prepare_power_trim(PWM1_CPU_POWER, 1400))
                        return -1;

                    //CPU 1400 MHz -> CPU-MPLL-175Mhz = 0x1D2AAA
                    writel(0xAA, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x20)); // B0
                    writel(0x2A, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x24)); // B1
                    writel(0x1D, IOADDR_CG_REG_BASE + 0x10000 + (0x7C0+0x28)); // B2
                    printf("Set CPU clk 1400MHz\n");
                    break;
            }
            break;
    default:
            printf("CPU:%d DRAM:%d AXI0:%d AXI1:%d AXI2:%d\n", pmu_readpll(0x7C0) * 8,
					pmu_readpll(0x4C0) * 8, pmu_readpll(0x400), pmu_readpll(0x440), pmu_readpll(0x700));
            return CMD_RET_USAGE;
    }
    return 0;
}

U_BOOT_CMD(
	nvt_cpu_freq, 2,	1,	do_nvt_cpu_freq,
	"change cpu freq",
	"[MHz]\n"
	"nvt_cpu_freq 800\n"
	"nvt_cpu_freq 1000\n"
	"nvt_cpu_freq 1200\n"
	"nvt_cpu_freq 1400"
);

u32 nvt_get_ddr_type(u64 *freq, u32 *multiple)
{
	*freq = pmu_readpll(0x4C0) * 8;
	*multiple = (*(u32*) 0xFC400008 & 0x40000000) ? 2 : 1;

	return (*freq) * (*multiple);
}

int do_nvt_ddr_freq(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u64 value = 0x0;
	u32 multiple = 0x0;

	nvt_get_ddr_type(&value, &multiple);

	printf("DDR type %lldx%d\n", value, multiple);

	return 0;
}

U_BOOT_CMD(
	nvt_ddr_freq, 2,      1,     do_nvt_ddr_freq,
	"get ddr freq/type\n",
	"\n"
);

int do_nvt_ddr_power(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u64 value = 0x0;
	u32 multiple = 0x0, ddr_freq = 0;

    switch (argc) {
    case 1:
        ddr_freq = nvt_get_ddr_type(&value, &multiple);
        if (prepare_power_trim(PWM0_CORE_POWER, ddr_freq))
            return -1;
        break;

    default:
        return CMD_RET_USAGE;
    }
    return 0;
}

U_BOOT_CMD(
	nvt_ddr_power, 1,      1,     do_nvt_ddr_power,
	"set ddr power\n",
	"\n"
);