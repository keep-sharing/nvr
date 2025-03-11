/*
 *
 * Copyright (c) 2020-2021 Shenshu Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <types.h>
#include <common.h>
#include <lib.h>
#include <platform.h>
#include <flash_map.h>
#include <checkup.h>
#include <atf_common.h>
#include <securecutil.h>
#include "../drivers/otp/otp.h"
#include "../drivers/ddr_init/init_regs.h"
#include "../drivers/ddr_init/include/ddrtrn_interface.h"
#include "../drivers/ddr_init/include/ddrtrn_api.h"
#include "../drivers/pke/drv_pke.h"
#include "../drivers/pke/ot_drv_pke.h"
#include "../drivers/cipher/cipher.h"
#include "../drivers/uart/uart.h"
#include "../drivers/emmc/emmc.h"

#define EMMC_READ_BLOCK_MAX 10

unsigned long jump_addr;
unsigned long __stack_chk_guard;
backup_image_params_s g_backup_params;

static inline void delay(unsigned int num)
{
	volatile unsigned int i;

	for (i = 0; i < (100 * num); i++)  /* 100: Cycle */
		__asm__ __volatile__("nop");
}


void call_reset(void)
{
	mdelay(500);  /* 500: delay time */
	timer_deinit();
	reg_set((uint32_t *)(REG_BASE_SCTL + REG_SC_SYSRES), 0x1);
}

void err_print(uint8_t err_type, uint8_t err_idx)
{
	if (is_scs_dbg_enable() != AUTH_SUCCESS)
		return;

	if (uart_inited == 0) {
		uart_init();
		uart_reset();
		mdelay(10);  /* 10: delay time */
	}
	serial_putc('\n');
	serial_putc('G');
	serial_putc(err_type);
	serial_putc('S');
	serial_putc(err_idx);
	mdelay(10);  /* 10: delay time */
}

static void check_and_set_backup_image_flag(uint32_t offset_times)
{
	uint32_t val;
	uint32_t backup_image_enable_flag;

	if (is_backup_image_enable() != AUTH_SUCCESS)
		return;

	val = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_FLG_REG);
	backup_image_enable_flag = get_backup_image_flag(val);
	if (backup_image_enable_flag == BACKUP_IMAGE_ENABLE) {
		val = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_OFF_REG);
		val &= ~BACKUP_IMAGE_OFF_MASK;
		val |= (offset_times << BACKUP_IMAGE_OFF_BIT);
		reg_set(REG_BASE_SCTL + BACKUP_IMAGE_OFF_REG, val);
	} else {
		val = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_FLG_REG);
		val &= ~BACKUP_IMAGE_FLG_MASK;
		val |= (BACKUP_IMAGE_ENABLE << BACKUP_IMAGE_FLG_BIT);
		reg_set(REG_BASE_SCTL + BACKUP_IMAGE_FLG_REG, val);

		val = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_OFF_REG);
		val &= ~BACKUP_IMAGE_OFF_MASK;
		val |= (0x1 << BACKUP_IMAGE_OFF_BIT);
		reg_set(REG_BASE_SCTL + BACKUP_IMAGE_OFF_REG, val);
	}
}

void failure_process(void)
{
	check_and_set_backup_image_flag(g_backup_params.offset_times);

	if ((is_func_jtag_enable() == AUTH_FAILURE) ||
	    ((is_backup_image_enable() == AUTH_SUCCESS)))
		call_reset();

	extern void _secure_failure_process(void);
	_secure_failure_process();

	return;
}

void tee_img_verify_failure()
{
	uint32_t failure_count;
	err_print('T', '3');
	/* add TEE_Launch_Failure register with 1 */
	failure_count = reg_get(REG_TEE_LAUNCH_FAILURE);
	failure_count++;
	reg_set(REG_TEE_LAUNCH_FAILURE, failure_count);
	/* soft reset */
	call_reset();
}

static void configure_trng_parameters(void)
{
}

static void start_flow_prepare(void)
{
	malloc_init();
	uart_port_init();
	configure_trng_parameters();
	return;
}

static void setup_dying_gasp_trim(void)
{
	unsigned int reg_val;
	unsigned int dying_gasp;

	writel(0x0, REG_DYING_GASP_CFG0);

	reg_val = readl(REG_OTP_PO_VDAC_INFO);
	dying_gasp = (reg_val & OTP_DYING_GASP_MASK) >> OTP_DYING_GASP_OFST;

	reg_val = readl(REG_DYING_GASP_CFG0);
	reg_val &= ~DYING_GASP_MASK;
	reg_val |= dying_gasp << DYING_GASP_OFST;
	writel(reg_val, REG_DYING_GASP_CFG0);
}

static void configure_ddr_parameters(void)
{
	uint32_t tables_base;
	uint32_t gsl_code_area_len;

	gsl_code_area_len = get_gsl_code_area_len();
	tables_base = (uint32_t)(VENDOR_ROOT_PUBLIC_KEY_ADDR + BOOTLOADER_CFG_PARAMS_OFFSET +
				 gsl_code_area_len);
	init_registers(tables_base, 0);
}

static unsigned hpm_value_avg(const unsigned int *val, unsigned int num)
{
	unsigned int i;
	unsigned tmp = 0;

	for (i = 0; i < num; i++) /* 4: Cycle */
		tmp += val[i];

	return (tmp / CYCLE_NUM) >> HPM_CYCLE;
}


static void get_hpm_value(unsigned int *hpm_core, unsigned int *hpm_cpu)
{
	int i;
	unsigned int temp;
	unsigned int core_value[4] = {0, 0, 0, 0};
	unsigned int cpu_value[4] = {0, 0, 0, 0};

	for (i = 0; i < CYCLE_NUM; i++) {
		delay(5);  /* 5: delay time */
		/* cpu */
		temp = readl(SYSCTRL_REG + HPM_CPU_REG0);
		cpu_value[1] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;
		cpu_value[0] += temp & HPM_HIGH_MASK;
		temp = readl(SYSCTRL_REG + HPM_CPU_REG1);
		cpu_value[3] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;  /* 3: Array size */
		cpu_value[2] += temp & HPM_HIGH_MASK;  /* 2: Array size */

		/* core */
		temp = readl(SYSCTRL_REG + HPM_CORE_REG0);
		core_value[1] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;
		core_value[0] += temp & HPM_HIGH_MASK;
		temp = readl(SYSCTRL_REG + HPM_CORE_REG1);
		core_value[3] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;  /* 3: Array size */
		core_value[2] += temp & HPM_HIGH_MASK;  /* 2: Array size */
	}
	*hpm_core = hpm_value_avg(core_value, 4); /* 4 : Array size */
	*hpm_cpu = hpm_value_avg(cpu_value, 4); /* 4 : Array size */
}


static void save_hpm(unsigned int hpm_core, unsigned int hpm_cpu)
{
	writel(hpm_cpu, SYSCTRL_BASE_REG + HPM_CPU_STORAGE_REG);
	writel(hpm_core, SYSCTRL_BASE_REG + HPM_CORE_STORAGE_REG);
}
static unsigned int calc_volt_regval(unsigned int volt_val, unsigned int volt_max,
				     unsigned int volt_min)
{
	unsigned int duty;

	if (volt_val >= volt_max)
		volt_val = volt_max - 1;
	if (volt_val <= volt_min)
		volt_val = volt_min + 1;
	duty =  duty_formula(volt_max, volt_min, volt_val);

	return duty;
}

static void set_hpm_core_volt(unsigned int hpm_core_value, int delta_v)
{
	unsigned int volt_val;
	unsigned int reg_val;

	volt_val = (CORE_CURVE_B - CORE_CURVE_A * hpm_core_value) / HPM_0_SCARE;

	if (volt_val > CORE_CURVE_VLOT_MAX)
		volt_val = CORE_CURVE_VLOT_MAX;
	else if (volt_val < CORE_CURVE_VLOT_MIN)
		volt_val = CORE_CURVE_VLOT_MIN;

	volt_val += delta_v;
	reg_val = calc_volt_regval(volt_val, CORE_VOLT_MAX, CORE_VOLT_MIN);
	writel(reg_val, SYSCTRL_BASE_REG + CORE_DUTY_STORAGE_REG);
	writel(reg_val, HPM_CORE_VOL_REG);
}

static void set_hpm_cpu_volt(unsigned int hpm_cpu_value, int delta_v)
{
	unsigned int volt_val;
	unsigned int reg_val;

	volt_val = (CPU_CURVE_B - CPU_CURVE_A * hpm_cpu_value) / HPM_0_SCARE;
	if (volt_val > CPU_CURVE_VLOT_MAX)
		volt_val = CPU_CURVE_VLOT_MAX;
	else if (volt_val < CPU_CURVE_VLOT_MIN)
		volt_val = CPU_CURVE_VLOT_MIN;

	volt_val += delta_v;
	reg_val = calc_volt_regval(volt_val, CPU_VOLT_MAX, CPU_VOLT_MIN);
	writel(reg_val, SYSCTRL_BASE_REG + CPU_DUTY_STORAGE_REG);

	writel(reg_val, HPM_CPU_VOL_REG);
}
static void get_delta_v(int *core_delta_v, int *cpu_delta_v)
{
	unsigned int value = readl(SYSCTRL_REG + OTP_SHPM_MDA_OFFSET);
	writel(value, SYSCTRL_BASE_REG + DELTA_V_STORAGE_REG);
	/* core:bit 11-8,
	 * bit11 equal to 1 means negative, equal to 0 means positive,
	 * bit 8-10 is the absolute delta_v
	 */
	int flag = (value & 0x800) ? -1 : 1;
	*core_delta_v = flag * (int)((value >> DELTA_OFST) & DELTA_MASK) * DELTA_SCARE;

	/* cpu:bit 3-0,
	 * bit3 equal to 1 means negative, equal to 0 means positive,
	 * bit 0-2 is the absolute delta_v
	 */
	flag = (value & 0x8) ? -1 : 1;
	*cpu_delta_v = flag * (int)(value & DELTA_MASK)  * DELTA_SCARE;
}

static void set_volt(unsigned int hpm_core, unsigned int hpm_cpu)
{
	int  core_delta_v = 0;
	int  cpu_delta_v = 0;
	get_delta_v(&core_delta_v, &cpu_delta_v);

	set_hpm_core_volt(hpm_core, core_delta_v);
	set_hpm_cpu_volt(hpm_cpu, cpu_delta_v);
	delay(550);  /* 550: delay time */
}

static void get_temperature(int *temperature)
{
	unsigned int value = 0;
	int tsensor_chn;
	int tmp = 0;
	int temperature_temp[TSENSOR_MAX_CHN] = {0};
	delay(1000);  /* 1000: delay time */
	for (tsensor_chn = 0; tsensor_chn < TSENSOR_MAX_CHN; tsensor_chn++) {
		value = readl(REG_TSENSOR_CTRL + TSENSOR_STATUS0 + TSENSOR_OFFSET * tsensor_chn);
		value = value & TSENSOR_MASK;
		tmp = value - TSENSOR_CURVE_A;
		tmp = (tmp * TEMP_SCARE) / TSENSOR_CURVE_B;
		tmp = ((tmp * TSENSOR_CURVE_C) / TEMP_SCARE) - TSENSOR_CURVE_D;
		temperature_temp[tsensor_chn] = tmp;
	}
	*temperature = (int)((temperature_temp[0] + temperature_temp[1]) / TSENSOR_MAX_CHN);
	writel((unsigned int)*temperature, SYSCTRL_REG + TEMPERATURE_STORAGE_REG);
}

static void adjust_hpm_core(unsigned int *hpm_core, int temperature)
{
	if (temperature >= 90)  /* 90: temperaturee */
		*hpm_core += (((HPM_CORE_90_CURVE_A * (*hpm_core) - HPM_CORE_90_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 70)  /* 70: temperaturee */
		*hpm_core += (((HPM_CORE_70_CURVE_A * (*hpm_core) - HPM_CORE_70_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 50)  /* 50: temperaturee */
		*hpm_core += (((HPM_CORE_50_CURVE_A * (*hpm_core) - HPM_CORE_50_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 0)
		*hpm_core = *hpm_core;
	else if (temperature >= -24)  /* 24: temperaturee */
		*hpm_core -= ((HPM_CORE_24_SUBZERO_CURVE_A * (*hpm_core) - HPM_CORE_24_SUBZERO_CURVE_B) / HPM_0_SCARE);
	else
		*hpm_core -= ((HPM_CORE_38_SUBZERO_CURVE_A * (*hpm_core) - HPM_CORE_38_SUBZERO_CURVE_B) / HPM_0_SCARE);
}

static void adjust_hpm_cpu(unsigned int *hpm_cpu, int temperature)
{
	if (temperature >= 90)  /* 90: temperaturee */
		*hpm_cpu += (((HPM_CPU_90_CURVE_A * (*hpm_cpu) - HPM_CPU_90_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 70)  /* 70: temperaturee */
		*hpm_cpu += (((HPM_CPU_70_CURVE_A * (*hpm_cpu) - HPM_CPU_70_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 50)  /* 50: temperaturee */
		*hpm_cpu += (((HPM_CPU_50_CURVE_A * (*hpm_cpu) - HPM_CPU_50_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 0)
		*hpm_cpu = *hpm_cpu;
	else if (temperature >= -24)  /* 24: temperaturee */
		*hpm_cpu -= ((HPM_CPU_24_SUBZERO_CURVE_A * (*hpm_cpu) - HPM_CPU_24_SUBZERO_CURVE_B) / HPM_0_SCARE);
	else
		*hpm_cpu -= ((HPM_CPU_38_SUBZERO_CURVE_A * (*hpm_cpu) - HPM_CPU_38_SUBZERO_CURVE_B) / HPM_0_SCARE);
}

static void safe_hpm(unsigned int *hpm_core, unsigned int *hpm_cpu, int temperature)
{
	if (*hpm_core > CORE_HPM_TEMP_MIN)
	   	adjust_hpm_core(hpm_core, temperature);
	if (*hpm_cpu > CPU_HPM_TEMP_MIN)
	   	adjust_hpm_cpu(hpm_cpu, temperature);

	if (*hpm_core >= CORE_HPM_MAX)
		*hpm_core = CORE_HPM_MAX - 1;
	else if (*hpm_core <= CORE_HPM_MIN)
		*hpm_core = CORE_HPM_MIN + 1;

	if (*hpm_cpu >= CPU_HPM_MAX)
		*hpm_cpu = CPU_HPM_MAX - 1;
	else if (*hpm_cpu <= CPU_HPM_MIN)
		*hpm_cpu = CPU_HPM_MIN + 1;
}
static void svb_voltage_change(void)
{
	unsigned int hpm_core;
	unsigned int hpm_cpu;
	unsigned int ver_tmp;
	int temperature;

	get_hpm_value(&hpm_core, &hpm_cpu);
	get_temperature(&temperature);
	safe_hpm(&hpm_core, &hpm_cpu, temperature);
	set_volt(hpm_core, hpm_cpu);
	save_hpm(hpm_core, hpm_cpu);

	/* add SVB VER */
	ver_tmp = readl(SVB_VER_REG);
	writel(((ver_tmp & 0xffff0000) | (SVB_VER & 0xffff)), SVB_VER_REG);
}

static void ddr_training(void)
{
	/* ddr hw training */
	bsp_ddrtrn_training_item(DDR_HW_TRAINING);

	/* ddr sw training */
	bsp_ddrtrn_training_item(DDR_SW_TRAINING);

	/* disable it when sample back */
	ddrtrn_retrain_enable();

	/* ddr DMC auto power down config */
	bsp_ddrtrn_dmc_auto_power_down_cfg();
}

#ifdef DDR_SCRAMB_ENABLE
unsigned int get_random_num(void)
{
	unsigned int reg_val;
	unsigned int data_cnt;

	do {
		reg_val = reg_get(TRNG_DATA_ST_REG);
		data_cnt = (reg_val >> TRNG_FIFO_DATA_CNT_SHIFT) & TRNG_FIFO_DATA_CNT_MASK;
	} while (data_cnt == 0);

	reg_val = reg_get(TRNG_DSTA_FIFO_DATA_REG);
	return reg_val;
}

static void ddr_scramb_start(void)
{
	reg_set(DDRCA_REE_RANDOM_L_REG, get_random_num());
	reg_set(DDRCA_REE_RANDOM_H_REG, get_random_num());
	reg_set(DDRCA_TEE_RANDOM_L_REG, get_random_num());
	reg_set(DDRCA_TEE_RANDOM_H_REG, get_random_num());
	reg_set(DDRCA_EN_REG, SCRAMB_BYPASS_DIS);
	reg_set(DDRCA_REE_UPDATE_REG, SCRAMB_ENABLE);
	reg_set(DDRCA_TEE_UPDATE_REG, SCRAMB_ENABLE);
	reg_set(DDRCA_LOCK_REG, SCRAMB_LOCK);

	/* clear the old random value */
	get_random_num();
}

static void ddr_scrambling(void)
{
	unsigned int status;
	unsigned int dmc0_isvalid = 0;
	unsigned int dmc1_isvalid = 0;
	unsigned int dmc2_isvalid = 0;
	unsigned int dmc3_isvalid = 0;

	/*
	 * read ddrc_cfg_ddrmode register,
	 * if bit[3:0] is not 0x0 ,
	 * the channel is valid.
	 */
	dmc0_isvalid = (reg_get(DMC0_DDRC_CFG_DDRMODE_REG) & 0xf) != 0;
	dmc1_isvalid = (reg_get(DMC1_DDRC_CFG_DDRMODE_REG) & 0xf) != 0;
	dmc2_isvalid = (reg_get(DMC2_DDRC_CFG_DDRMODE_REG) & 0xf) != 0;
	dmc3_isvalid = (reg_get(DMC3_DDRC_CFG_DDRMODE_REG) & 0xf) != 0;

	/* set ddrc to enter self-refresh */
	if (dmc0_isvalid)
		reg_set(DMC0_DDRC_CTRL_SREF_REG, DDRC_SREF_REQ);
	if (dmc1_isvalid)
		reg_set(DMC1_DDRC_CTRL_SREF_REG, DDRC_SREF_REQ);
	if (dmc2_isvalid)
		reg_set(DMC2_DDRC_CTRL_SREF_REG, DDRC_SREF_REQ);
	if (dmc3_isvalid)
		reg_set(DMC3_DDRC_CTRL_SREF_REG, DDRC_SREF_REQ);

	/* wait the status of ddrc to be self-refresh (status == 1). */
	do {
		status = 1;
		status &= dmc0_isvalid ? (reg_get(DMC0_DDRC_CURR_FUNC_REG) & 0x1) : 1;
		status &= dmc1_isvalid ? (reg_get(DMC1_DDRC_CURR_FUNC_REG) & 0x1) : 1;
		status &= dmc2_isvalid ? (reg_get(DMC2_DDRC_CURR_FUNC_REG) & 0x1) : 1;
		status &= dmc3_isvalid ? (reg_get(DMC3_DDRC_CURR_FUNC_REG) & 0x1) : 1;
	} while (status != 1);

	/* start ddr scrambling */
	ddr_scramb_start();

	/* set ddrc to exit self-refresh */
	if (dmc0_isvalid)
		reg_set(DMC0_DDRC_CTRL_SREF_REG, DDRC_SREF_DONE);
	if (dmc1_isvalid)
		reg_set(DMC1_DDRC_CTRL_SREF_REG, DDRC_SREF_DONE);
	if (dmc2_isvalid)
		reg_set(DMC2_DDRC_CTRL_SREF_REG, DDRC_SREF_DONE);
	if (dmc3_isvalid)
		reg_set(DMC3_DDRC_CTRL_SREF_REG, DDRC_SREF_DONE);

	/* wait the status of ddrc to be normal (status == 0). */
	do {
		status = 0;
		status |= dmc0_isvalid ? (reg_get(DMC0_DDRC_CURR_FUNC_REG) & 0x1) : 0;
		status |= dmc1_isvalid ? (reg_get(DMC1_DDRC_CURR_FUNC_REG) & 0x1) : 0;
		status |= dmc2_isvalid ? (reg_get(DMC2_DDRC_CURR_FUNC_REG) & 0x1) : 0;
		status |= dmc3_isvalid ? (reg_get(DMC3_DDRC_CURR_FUNC_REG) & 0x1) : 0;
	} while (status != 0);

	return;
}
#endif

static void wait_pcie_slave_flag(int print_enable, uint32_t flag_val)
{
	uint32_t cnt = 0;
	uint32_t reg;

	do {
		if (cnt % PCIE_SLAVE_PRINT_INTERVAL == 0) {
			if (print_enable)
				serial_putc('.');
		}

		cnt++;

		reg = reg_get(REG_BASE_SCTL + PCIE_SLAVE_BOOT_CTL_REG);
	} while (reg != flag_val);

	if (print_enable) {
		serial_putc('O');
		serial_putc('K');
		serial_putc('\n');
	}
}

static void pcie_slave_set_ddr_init_done_flag(int print_enable)
{
	reg_set(REG_BASE_SCTL + PCIE_SLAVE_BOOT_CTL_REG,
		DDR_INIT_EXCUTE_OK_FLAG);

	if (print_enable) {
		serial_putc('\n');
		serial_putc('P');
		serial_putc('C');
		serial_putc('I');
		serial_putc('E');
		serial_putc(':');
		serial_putc('D');
		serial_putc('D');
		serial_putc('R');
		serial_putc('I');
		serial_putc('N');
		serial_putc('I');
		serial_putc('T');
		serial_putc(' ');
		serial_putc('D');
		serial_putc('O');
		serial_putc('N');
		serial_putc('E');
		serial_putc('\n');
	}
}

static int wait_pcie_slave_bootloader_download_ok(int print_enable)
{
	if (print_enable) {
		serial_putc('\n');
		serial_putc('P');
		serial_putc('C');
		serial_putc('I');
		serial_putc('E');
		serial_putc(':');
		serial_putc('D');
		serial_putc('O');
		serial_putc('W');
		serial_putc('N');
		serial_putc('L');
		serial_putc('O');
		serial_putc('A');
		serial_putc('D');
		serial_putc(' ');
		serial_putc('B');
		serial_putc('O');
		serial_putc('O');
		serial_putc('T');
		serial_putc('L');
		serial_putc('O');
		serial_putc('A');
		serial_putc('D');
		serial_putc('E');
		serial_putc('R');
		serial_putc(' ');
	}

	wait_pcie_slave_flag(print_enable, UBOOT_DOWNLOAD_OK_FLAG);

	return TD_SUCCESS;
}

static void clear_backup_image_flag_and_offset_times(void)
{
	uint32_t val;
	errno_t err;

	/* Clear backup image boot flag */
	val = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_FLG_REG);
	val &= ~(0xffffff << 0x4);
	reg_set(REG_BASE_SCTL + BACKUP_IMAGE_FLG_REG, val);

	/* Clear bakup image offset times */
	val = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_OFF_REG);
	val &= ~(0xfff << 0x4);
	reg_set(REG_BASE_SCTL + BACKUP_IMAGE_OFF_REG, val);

	err = memset_s(&g_backup_params, sizeof(backup_image_params_s), 0, sizeof(backup_image_params_s));
	if (err != EOK)
		return;
}

static int system_init()
{
	setup_dying_gasp_trim();

	configure_ddr_parameters();

	svb_voltage_change();

	ddr_training();

#ifdef DDR_SCRAMB_ENABLE
	ddr_scrambling();
#endif
	return TD_SUCCESS;
}

static uint32_t get_data_channel_type(void)
{
	uint32_t channel_type;

	channel_type = reg_get(REG_BASE_SCTL + DATA_CHANNEL_TYPE_REG);
	switch (channel_type) {
	case BOOT_SEL_PCIE:
	case BOOT_SEL_UART:
	case BOOT_SEL_FLASH:
	case BOOT_SEL_EMMC:
		break;
	default:
		err_print('3', '1');
		channel_type = BOOT_SEL_UNKNOW;
		break;
	}

	return channel_type;
}

static void get_image_backup_params(uint32_t channel_type, backup_image_params_s *backup_params)
{
	uint32_t backup_image_enable_flag;
	uint32_t val;

	val = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_FLG_REG);
	backup_image_enable_flag = get_backup_image_flag(val);
	if ((is_backup_image_enable() != AUTH_SUCCESS) ||
	    (backup_image_enable_flag != BACKUP_IMAGE_ENABLE))
		return;

	backup_params->offset_times = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_TIMES_REG);
	backup_params->offset_addr = reg_get(REG_BASE_SCTL + BACKUP_IMAGE_ADDR_REG);
	backup_params->enable = 1;
}

static int get_head_area_data_from_flash(backup_image_params_s *backup_params)
{
	uint32_t bootloader_key_area_addr;
	uint32_t gsl_code_area_len;
	uint32_t spacc_channel_number = 1;
	spacc_decrypt_params decrypt_params = {0};
	int ret;

	gsl_code_area_len = get_gsl_code_area_len();
	bootloader_key_area_addr = (uint32_t)(VENDOR_ROOT_PUBLIC_KEY_ADDR +
					      BOOTLOADER_KEY_AREA_ADDR_OFFSET + gsl_code_area_len);
	get_image_backup_params(BOOT_SEL_FLASH, backup_params);
	decrypt_params.chn = spacc_channel_number;
	decrypt_params.dst_addr = bootloader_key_area_addr;
	decrypt_params.src_addr = (uint32_t)(SPI_BASE_ADDR +  backup_params->offset_addr +
					     BOOTLOADER_KEY_AREA_ADDR_OFFSET + gsl_code_area_len);
	decrypt_params.length = BOOTLOADER_KEY_PARAMS_AREA_SIZE;
	decrypt_params.iv_addr = 0;
	decrypt_params.iv_length = 0;
	decrypt_params.alg = SYMC_ALG_DMA;
	decrypt_params.mode = SYMC_MODE_CBC;

	/* Not an actual decryption operation, just get data from flash by cipher dma */
	ret = drv_spacc_decrypt(decrypt_params);
	if (ret != TD_SUCCESS) {
		err_print('4', '7');
		return TD_FAILURE;
	}

	return TD_SUCCESS;
}

static int get_head_area_data_from_emmc(backup_image_params_s *backup_params)
{
	uint32_t bootloader_key_area_addr;
	uint32_t gsl_code_area_len;
	uint32_t align_addr;
	uint32_t align_len;
	uint32_t align_params_size;
	uint32_t tmp_addr;
	uint32_t data_addr = 0;
	uint32_t data_len;
	int ret;

	if (mmc_init() != 0) {
		err_print('4', '8');
		return TD_FAILURE;
	}

	gsl_code_area_len = get_gsl_code_area_len();
	bootloader_key_area_addr = (uint32_t)(VENDOR_ROOT_PUBLIC_KEY_ADDR +
					      BOOTLOADER_KEY_AREA_ADDR_OFFSET + gsl_code_area_len);

	get_image_backup_params(BOOT_SEL_EMMC, backup_params);
	/* The eMMC read data must be 512-byte aligned. */
	align_addr = (backup_params->offset_addr + BOOTLOADER_KEY_AREA_ADDR_OFFSET +
		      gsl_code_area_len) / EMMC_BLOCK_SIZE * EMMC_BLOCK_SIZE;
	align_len = backup_params->offset_addr + BOOTLOADER_KEY_AREA_ADDR_OFFSET +
		    gsl_code_area_len - align_addr;
	align_params_size = align_len + BOOTLOADER_KEY_PARAMS_AREA_SIZE;

	/* Here tmp_addr is used as the temporary storage address. */
	tmp_addr = bootloader_key_area_addr;

	/* The eMMC must read data from address 0. So Redundant data is discarded. */
	for (int i = 0; i < align_addr / EMMC_BLOCK_SIZE;) {
		if ((align_addr / EMMC_BLOCK_SIZE - i) >= EMMC_READ_BLOCK_MAX) {
			data_len = EMMC_READ_BLOCK_MAX * EMMC_BLOCK_SIZE;
			i += EMMC_READ_BLOCK_MAX;
		} else {
			data_len = EMMC_BLOCK_SIZE;
			i++;
		}

		/* get data from emmc by cipher dma */
		ret = mmc_read((void *)(uintptr_t)tmp_addr, data_addr, data_len, READ_DATA_BY_DMA);
		if (ret != TD_SUCCESS) {
			err_print('4', '9');
			return TD_FAILURE;
		}
		data_addr += data_len;
	}

	ret = mmc_read((void *)(uintptr_t)tmp_addr, data_addr, align_params_size, READ_DATA_BY_DMA);
	if (ret != TD_SUCCESS) {
		err_print('4', 'a');
		return TD_FAILURE;
	}
	ret = memmove_s((void *)(uintptr_t)bootloader_key_area_addr, BOOTLOADER_KEY_PARAMS_AREA_SIZE,
			(void *)(uintptr_t)(tmp_addr + align_len), BOOTLOADER_KEY_PARAMS_AREA_SIZE);
	if (ret != EOK) {
		err_print('4', 'b');
		return TD_FAILURE;
	}

	return TD_SUCCESS;
}

static int get_head_area_data(uint32_t channel_type, backup_image_params_s *backup_params)
{
	int ret;

	switch (channel_type) {
	case BOOT_SEL_PCIE:
	case BOOT_SEL_UART:
		break;

	case BOOT_SEL_FLASH:
		ret = get_head_area_data_from_flash(backup_params);
		if (ret != TD_SUCCESS) {
			err_print('4', '5');
			return TD_FAILURE;
		}

		break;

	case BOOT_SEL_EMMC:
		ret = get_head_area_data_from_emmc(backup_params);
		if (ret != TD_SUCCESS) {
			err_print('4', '6');
			return TD_FAILURE;
		}

		break;

	default:
		break;
	}

	return TD_SUCCESS;
}

static int get_bootloader_code_area_from_pcie()
{
	int ret;
	int print_enable = 0;

	if (is_boot_info_lv_debug_enable() == AUTH_SUCCESS)
		print_enable = uart_boot_flag(reg_get(REG_BASE_SCTL + REG_SC_SYSSTAT));

	pcie_slave_set_ddr_init_done_flag(print_enable);
	ret = wait_pcie_slave_bootloader_download_ok(print_enable);

	return ret;
}

static int get_bootloader_code_area_from_uart()
{
	uint32_t image_int_ddr_addr;
	uint32_t image_total_len;
	int ret;

	image_int_ddr_addr = get_boot_image_int_ddr_addr();
	image_total_len = get_boot_image_total_len();

	ret = copy_from_uart((void *)(uintptr_t)image_int_ddr_addr, image_total_len);
	if (ret != TD_SUCCESS)
		return TD_FAILURE;

	return TD_SUCCESS;
}

static int get_bootloader_code_area_from_flash(const backup_image_params_s *backup_params)
{
	uint32_t image_int_ddr_addr;
	uint32_t image_total_len;
	uint32_t spacc_channel_number = 1;
	int ret;
	spacc_decrypt_params decrypt_params = {0};

	image_int_ddr_addr = get_boot_image_int_ddr_addr();
	image_total_len = get_boot_image_total_len();

	decrypt_params.chn = spacc_channel_number;
	decrypt_params.dst_addr = image_int_ddr_addr;
	decrypt_params.src_addr = (uint32_t)(SPI_BASE_ADDR + backup_params->offset_addr);
	decrypt_params.length = image_total_len;
	decrypt_params.alg = SYMC_ALG_DMA;
	decrypt_params.mode = SYMC_MODE_CBC;
	/* Not an actual decryption operation, just get data from flash by cipher dma */
	ret = drv_spacc_decrypt(decrypt_params);

	return ret;
}

static int get_bootloader_code_area_from_emmc(const backup_image_params_s *backup_params)
{
	uint32_t image_int_ddr_addr;
	uint32_t image_total_len;
	uint32_t tmp_addr;
	int 	 ret;

	image_int_ddr_addr = get_boot_image_int_ddr_addr();
	image_total_len = get_boot_image_total_len();

	mmc_deinit();
	if (mmc_init() != 0) {
		err_print('8', '7');
		return TD_FAILURE;
	}

	/* NOte: The start address for U-Boot backup must be 512-byte (BLOCK) aligned. */
	/* Redundant data is Discard during U-Boot backup. */
	if (backup_params->offset_addr != 0) {
		/* Here tmp_addr is used as the temporary storage address. */
		tmp_addr = image_int_ddr_addr;
		ret = mmc_read((void *)(uintptr_t)tmp_addr, 0,
			       backup_params->offset_addr, READ_DATA_BY_DMA);
		if (ret != TD_SUCCESS) {
			err_print('8', '8');
			return TD_FAILURE;
		}
	}

	/* get data from emmc by cipher dma */
	ret = mmc_read((void *)(uintptr_t)image_int_ddr_addr, backup_params->offset_addr,
		       image_total_len, READ_DATA_BY_DMA);
	if (ret != TD_SUCCESS) {
		err_print('8', '9');
		return TD_FAILURE;
	}

	return TD_SUCCESS;
}

static int get_bootloader_code_area_data(uint32_t channel_type,
		const backup_image_params_s *backup_params)
{
	int ret;

	switch (channel_type) {
	case BOOT_SEL_PCIE:
		ret = get_bootloader_code_area_from_pcie();
		if (ret != TD_SUCCESS) {
			err_print('8', '1');
			return TD_FAILURE;
		}
		break;

	case BOOT_SEL_UART:
		ret = get_bootloader_code_area_from_uart();
		if (ret != TD_SUCCESS) {
			err_print('8', '2');
			return TD_FAILURE;
		}
		break;

	case BOOT_SEL_FLASH:
		ret = get_bootloader_code_area_from_flash(backup_params);
		if (ret != TD_SUCCESS) {
			err_print('8', '5');
			return TD_FAILURE;
		}
		break;

	case BOOT_SEL_EMMC:
		ret = get_bootloader_code_area_from_emmc(backup_params);
		if (ret != TD_SUCCESS) {
			err_print('8', '6');
			return TD_FAILURE;
		}
		break;

	default:
		break;
	}

	return TD_SUCCESS;
}

static ALWAYS_INLINE void store_booting_param()
{
	uint32_t decrypt_bootloader_addr;
	uint32_t boot_code_addr;
	uint32_t gsl_code_area_len;
	uint32_t unchecked_area_len;

	if (g_backup_params.enable)
		clear_backup_image_flag_and_offset_times();

	gsl_code_area_len = get_gsl_code_area_len();
	unchecked_area_len = get_bootloader_unchecked_area_len();
	decrypt_bootloader_addr = get_ddr_decrypt_bootloader_addr();
	boot_code_addr = (uint32_t)(decrypt_bootloader_addr + BOOTLOADER_BOOT_CODE_OFFSET +
				    gsl_code_area_len + unchecked_area_len);

	/* The DDR is not initialized in the U-boot */
	reg_set(REG_BASE_SCTL + REG_SC_GEN4, 0);
	jump_addr = boot_code_addr;
}

static int sec_module_init(char err_code)
{
	uint32_t key_slot = 0;

	if (set_key_slot(key_slot) != TD_SUCCESS) {
		err_print(err_code, '1');
		goto err_exit;
	}

	if (drv_pke_init() != TD_SUCCESS) {
		err_print(err_code, '2');
		goto err_exit;
	}

	if (spacc_init(key_slot) != TD_SUCCESS) {
		err_print(err_code, '3');
		goto err_exit;
	}
	return TD_SUCCESS;
err_exit:
	return TD_FAILURE;
}

static void sec_module_deinit(void)
{
	spacc_deinit();
	drv_pke_deinit();
	unset_key_slot();
}

void main_entry(void)
{
	int ret;
	uint32_t channel_type;

	start_flow_prepare();

	ret = sec_module_init('2');
	if (ret != TD_SUCCESS)
		failure_process();

	channel_type = get_data_channel_type();
	if (channel_type == BOOT_SEL_UNKNOW)
		failure_process();

	ret = get_head_area_data(channel_type, &g_backup_params);
	if (ret != TD_SUCCESS)
		failure_process();

	ret = handle_bootloader_key_area();
	if (ret != TD_SUCCESS)
		failure_process();

	ret = handle_bootloader_params_area();
	if (ret != TD_SUCCESS)
		failure_process();

	ret = system_init();
	if (ret != TD_SUCCESS)
		failure_process();

	ret = get_bootloader_code_area_data(channel_type, &g_backup_params);
	if (ret != TD_SUCCESS)
		failure_process();

	ret = handle_bootloader_code_area();
	if (ret != TD_SUCCESS)
		failure_process();

	store_booting_param();

	sec_module_deinit();
}

static int load_secure_os_fip(void)
{
	entry_point_info_t *bl32_ep = (entry_point_info_t *)(uintptr_t)BL32_EP_INFO_ADDR;
	entry_point_info_t *bl33_ep = (entry_point_info_t *)(uintptr_t)BL33_EP_INFO_ADDR;
	bl31_params_t *bl31_p = (bl31_params_t *)(uintptr_t)BL31_PARAMS_ADDR;
	uint64_t bl31_pc;
	errno_t err;

	/* to do: add  return value judgment  */
	err = memset_s(bl32_ep, sizeof(entry_point_info_t), 0, sizeof(entry_point_info_t));
	if (err != EOK)
		return -1;

	err = memset_s(bl31_p, sizeof(bl31_params_t), 0, sizeof(bl31_params_t));
	if (err != EOK)
		return -1;

	bl31_p->bl32_ep_info = (uint64_t)(uintptr_t)bl32_ep;
	bl31_p->bl33_ep_info = (uint64_t)(uintptr_t)bl33_ep;

	bl31_pc = BL31_BASE;
	bl32_ep->pc = (uint64_t)(uintptr_t)BL32_BASE;
	bl32_ep->args.arg0 = 0; /* no dtb */

	(*(volatile uint64_t *)OS_SYS_CTRL_REG2) = (uint64_t)(uintptr_t)bl31_p;
	(*(volatile uint64_t *)OS_SYS_CTRL_REG4) = (uint64_t)NULL;

	void (*atf_entry)(void) = (void(*)(void))(uintptr_t)(bl31_pc);
	atf_entry();

	return 0;
}

void secure_os_entry(void)
{
	int ret;

	ret = sec_module_init('i');
	if (ret != TD_SUCCESS)
		tee_img_verify_failure();

	/* configure ree DDR for tee image */
	config_tee_img_non_sec_ddr();
	/* configure secure DDR for tee key area */
	config_key_area_sec_ddr();

	/* disable secure DRR bypass */
	disable_sec_ddr_bypass();

	ret = handle_tee_key_area();
	if (ret != TD_SUCCESS)
		tee_img_verify_failure();

	/* configure secure DDR for tee image */
	config_tee_img_sec_ddr();

	ret = handle_atf_area();
	if (ret != TD_SUCCESS)
		tee_img_verify_failure();

	ret = handle_tee_code_area();
	if (ret != TD_SUCCESS)
		tee_img_verify_failure();

	sec_module_deinit();

	config_non_sec_ddr();

	load_secure_os_fip();
}

#pragma GCC push_options
#pragma GCC optimize ("-fno-stack-protector")
void __stack_chk_fail(void)
{
	err_print('0', '1');
	call_reset();
}
void stack_chk_guard_setup()
{
	unsigned random = 0;

	random = get_random_num();
	__stack_chk_guard = random;
	__stack_chk_guard <<= 32; /* 32:Move to the first 32 bits of the unsigned long. */
	random = get_random_num();
	__stack_chk_guard |= random;
}
#pragma GCC pop_options
