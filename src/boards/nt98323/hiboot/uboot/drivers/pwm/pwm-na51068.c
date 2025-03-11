/*
 * Novatek NA51068 PWM driver.
 *
 * Copyright (C) 2019 Novatek MicroElectronics Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <pwm.h>
#include <asm/io.h>
#include <asm/arch/soc.h>

#define INT_CSTAT       0x00
#define START_CTRL      0x04
#define TMR_REV         0x90
#define INT_PRIO        0x160

#define PWM_CTRL(id)    ((0x10*id)+0x10)
#define PWM_CNTB(id)    ((0x10*id)+0x14)
#define PWM_CMPB(id)    ((0x10*id)+0x18)
#define PWM_CNTO(id)    ((0x10*id)+0x1c)
#define PWM_REP(id)     ((0x04*id)+0xa0)
#define PWM_PAT(id, n)	((0x10*id)+0xc0+(0x04*n))
#define PWM_PATLEN(id)	((0x04*id)+0x140)
#define NUM_PWM 1
#ifdef CONFIG_CMD_FPGA
#define PCLK_FREQ       12000000
#else
#define PCLK_FREQ       12000000
#endif
#define PWM_CTRL_UPDATE 1
#define PWM_CTRL_INV    2
#define PWM_CTRL_MODE   3
#define PWM_CTRL_INTEN  5
#define PWM_CTRL_PWMEN  8

static inline void nvt_pwm_write_reg(int reg, u32 val)
{
	writel(val, CONFIG_PWM_BASE + reg);
}

static inline u32 nvt_pwm_read_reg(int reg)
{
	return readl(CONFIG_PWM_BASE + reg);
}


int pwm_init(int pwm_id, int div, int invert)
{
	u32 ctrl_reg = 0x0;

	/*setup global setting*/
	*(u32*) (CONFIG_SYSC_BASE + 0x5C) |= (0x1 << 29);
	*(u32*) (CONFIG_SYSC_BASE + 0x74) &= ~(0x1 << 23);

	ctrl_reg = nvt_pwm_read_reg(PWM_CTRL(pwm_id));

	if (invert == 1)
		ctrl_reg |= (0x1 << PWM_CTRL_INV);
	else
		ctrl_reg &= ~(0x1 << PWM_CTRL_INV);

	nvt_pwm_write_reg(PWM_CTRL(pwm_id), ctrl_reg);

	return 0;
}

int pwm_config(int pwm_id, int duty_ns, int period_ns)
{
	u64 src_freq = 0;
	u64 period = 0, duty = 0;

	src_freq = PCLK_FREQ;
	period = src_freq * period_ns;
	do_div(period, 1000000000);

	duty = src_freq * duty_ns;
	do_div(duty, 1000000000);
	duty = period - duty;

	nvt_pwm_write_reg(PWM_CNTB(pwm_id), (u32) period);

	nvt_pwm_write_reg(PWM_CMPB(pwm_id), (u32) duty);

	return 0;
}

int pwm_enable(int pwm_id)
{
	u32 start_reg = 0x0, ctrl_reg = 0x0;

	/*Enable Continuous mode*/
	ctrl_reg = nvt_pwm_read_reg(PWM_CTRL(pwm_id));
	ctrl_reg &= ~(0x3 << PWM_CTRL_MODE);
	ctrl_reg |= (0x1 << PWM_CTRL_MODE);
	ctrl_reg |= (0x1 << PWM_CTRL_INTEN);
	ctrl_reg |= (0x1 << PWM_CTRL_UPDATE);
	ctrl_reg |= (0x1 << PWM_CTRL_PWMEN);
	nvt_pwm_write_reg(PWM_CTRL(pwm_id), ctrl_reg);

	start_reg = nvt_pwm_read_reg(START_CTRL);
	start_reg |= (0x1 << pwm_id);
	nvt_pwm_write_reg(START_CTRL, start_reg);

	/*Enable PWM pinmux*/
	switch (pwm_id) {
	case 0:
		*(u32*) (CONFIG_TOP_BASE + 0x30) |= (0x1 << 24);
		break;
	case 1:
		*(u32*) (CONFIG_TOP_BASE + 0x30) |= (0x1 << 28);
		break;
	case 2:
		*(u32*) (CONFIG_TOP_BASE + 0x30) &= ~(0x7 << 8);
		*(u32*) (CONFIG_TOP_BASE + 0x30) |= (0x3 << 8);
		break;
	default:
		break;
	}

	return 0;
}

void pwm_disable(int pwm_id)
{
	u32 start_reg = 0x0;

	start_reg = nvt_pwm_read_reg(START_CTRL);
	start_reg &= ~(0x1 << pwm_id);
	nvt_pwm_write_reg(START_CTRL, start_reg);
}

static int do_pwm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int id, inv = 0;
	u32 period_ns, duty_ns = 0x0;

	if (argc > 3) {
		/* Check argument */
		if (argc < 6)
			return CMD_RET_USAGE;

		id = simple_strtoul(argv[2], NULL, 10);

		inv = simple_strtoul(argv[3], NULL, 10);

		period_ns = simple_strtoul(argv[4], NULL, 10);

		duty_ns = simple_strtoul(argv[5], NULL, 10);

		if ((id > 2) || (inv > 1) || (!period_ns) || (duty_ns > period_ns))
			return CMD_RET_USAGE;

		pwm_init(id, 0, inv);

		pwm_config(id, duty_ns, period_ns);

		return 0;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	if (!strncmp(argv[1], "enable", 6)) {
		id = simple_strtoul(argv[2], NULL, 10);
		pwm_enable(id);
		return 0;
	} else if (!strncmp(argv[1], "disable", 7)) {
		id = simple_strtoul(argv[2], NULL, 10);
		pwm_disable(id);
		return 0;
	} else
		return CMD_RET_USAGE;
}

void pwm_set(int pwm_id, int level)
{
    u32 duty = 0;

	if (level <= 100) {
		pwm_init(pwm_id, 0, 0);

		duty = 84 * level;

		pwm_config(pwm_id, duty, 8400);

		pwm_enable(pwm_id);
	}
}

static int do_power_ctl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 level = 0x0;

	if (argc == 2) {
		level = simple_strtoul(argv[1], NULL, 10);
		pwm_set(0, level);

		return 0;
	}

	return CMD_RET_USAGE;
}

static int do_cpu_power_ctl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 level = 0x0;

	if (argc == 2) {
		level = simple_strtoul(argv[1], NULL, 10);
		pwm_set(1, level);

		return 0;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
        pwm,  6,  0,   do_pwm,
        "nvt pwm operation",
        "config [id] [invert] [period_ns] [duty_ns]\n"
        "pwm enable [id]\n"
        "pwm disable [id]\n"
        "\nVersion:\n"
        "1.1.0\n"
);

U_BOOT_CMD(
        power_ctl,  2,  0,   do_power_ctl,
        "nvt core power control",
        "[level]\n"
);

U_BOOT_CMD(
        cpu_power_ctl,  2,  0,   do_cpu_power_ctl,
        "nvt cpu power control",
        "[level]\n"
);
