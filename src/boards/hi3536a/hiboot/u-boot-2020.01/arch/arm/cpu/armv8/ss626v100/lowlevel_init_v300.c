// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <asm/arch/platform.h>
#include <config.h>
#include "ddrtrn_interface.h"
#include "ddrtrn_api.h"

static inline void delay(unsigned int num)
{
	volatile unsigned int i;

	for (i = 0; i < (100 * num); i++)  /* 100: Cycle */
		__asm__ __volatile__("nop");
}

static inline void dwb(void) /* drain write buffer */
{
}

static inline unsigned int readl(unsigned addr)
{
	unsigned int val;

	val = (*(volatile unsigned int *)(long)(addr));
	return val;
}

static inline void writel(unsigned val, unsigned addr)
{
	dwb();
	(*(volatile unsigned *)(long)(addr)) = (val);
	dwb();
}

#define REG_BASE_MISC			0x11024000

#ifdef DDR_SCRAMB_ENABLE

#undef reg_get
#define reg_get(addr) (*(volatile unsigned int *)((long)addr))

#undef reg_set
#define reg_set(addr, val) (*(volatile unsigned int *)((long)addr) = (val))

/* RTNG */
#define REG_BASE_RNG_GEN		0x10114000
#define TRNG_DSTA_FIFO_DATA_REG		(REG_BASE_RNG_GEN + 0x204)
#define TRNG_DATA_ST_REG		(REG_BASE_RNG_GEN + 0x208)
#define TRNG_FIFO_DATA_CNT_SHIFT	0x8
#define TRNG_FIFO_DATA_CNT_MASK		0xf

/* DDRC */
#define REG_BASE_DDRC			0x11140000

#define DMC0_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0x8000 + 0x0)
#define DMC1_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0x9000 + 0x0)
#define DMC2_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0xa000 + 0x0)
#define DMC3_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0xb000 + 0x0)

#define DMC0_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0x8000 + 0x50)
#define DMC1_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0x9000 + 0x50)
#define DMC2_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0xa000 + 0x50)
#define DMC3_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0xb000 + 0x50)

#define DMC0_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0x8000 + 0x294)
#define DMC1_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0x9000 + 0x294)
#define DMC2_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0xa000 + 0x294)
#define DMC3_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0xb000 + 0x294)

#define DDRC_SREF_DONE			(0x1 << 1)
#define DDRC_SREF_REQ			(0x1 << 0)

/* MSIC DDRCA */
#define DDRCA_REE_RANDOM_L_REG		(REG_BASE_MISC + 0x40)
#define DDRCA_REE_RANDOM_H_REG		(REG_BASE_MISC + 0x44)
#define DDRCA_TEE_RANDOM_L_REG		(REG_BASE_MISC + 0x48)
#define DDRCA_TEE_RANDOM_H_REG		(REG_BASE_MISC + 0x4C)
#define DDRCA_EN_REG			(REG_BASE_MISC + 0x50)
#define DDRCA_REE_UPDATE_REG		(REG_BASE_MISC + 0x54)
#define DDRCA_TEE_UPDATE_REG		(REG_BASE_MISC + 0x58)
#define DDRCA_LOCK_REG			(REG_BASE_MISC + 0x5c)
#define SCRAMB_BYPASS_DIS		0x5
#define SCRAMB_ENABLE			0x1
#define SCRAMB_LOCK			0x1

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

/*-----------------------------------------------------------------------------------
 *  SVB
 *-----------------------------------------------------------------------------------*/
#define SYSCTRL_REG		0x11020000
#define HPM_CORE_VOL_REG	(SYSCTRL_REG + 0x9004)
#define HPM_CPU_VOL_REG		(SYSCTRL_REG + 0x9104)
#define CYCLE_NUM           5
#define HPM_CPU_REG0		0xb008
#define HPM_CPU_REG1		0xb00c
#define HPM_CORE_REG0		0xb018
#define HPM_CORE_REG1		0xb01c

#define CORE_HPM_MAX		500
#define CORE_HPM_MIN		300
#define CPU_HPM_MAX		    540
#define CPU_HPM_MIN		    340
#define CORE_HPM_TEMP_MIN	360
#define CPU_HPM_TEMP_MIN	395
/* tsensor */
#define REG_TSENSOR_CTRL	0x1102a000
#define TSENSOR_CTRL0		0x0
#define TSENSOR_CTRL0_CFG	0x80f00000
#define TSENSOR_CTRL1		0x4
#define TSENSOR_CTRL1_CFG	0x0
#define TSENSOR_STATUS0     0x8
#define TSENSOR_MAX_CHN     2
#define TSENSOR_OFFSET      0x100
#define TSENSOR_MASK        0x3ff
#define TSENSOR_CURVE_A		146
#define TSENSOR_CURVE_B		718
#define TSENSOR_CURVE_C		165
#define TSENSOR_CURVE_D		40


#define SYSCTRL_BASE_REG	        0x11020000
#define HPM_CORE_STORAGE_REG	    0x340
#define HPM_CPU_STORAGE_REG	        0x348
#define TEMPERATURE_STORAGE_REG     0x34c
#define DELTA_V_STORAGE_REG         0x350
#define CORE_DUTY_STORAGE_REG       0x354
#define CPU_DUTY_STORAGE_REG        0x35c


/* physical max/min */
#define CORE_VOLT_MAX		920
#define CORE_VOLT_MIN		600

#define CPU_VOLT_MAX		1050
#define CPU_VOLT_MIN		600


/* curve max/min; voltage curve:  */
#define CORE_CURVE_VLOT_MAX		842
#define CORE_CURVE_VLOT_MIN		766
#define CORE_CURVE_B			11937422
#define CORE_CURVE_A			9762

#define CPU_CURVE_VLOT_MAX	    960
#define CPU_CURVE_VLOT_MIN	    845
#define CPU_CURVE_B			    15120000
#define CPU_CURVE_A			    13595
/* curve max/min; CORE HPM curve: */
#define HPM_CORE_90_CURVE_A	            1722
#define HPM_CORE_90_CURVE_B	            575000
#define HPM_CORE_70_CURVE_A	            728
#define HPM_CORE_70_CURVE_B	            233000
#define HPM_CORE_50_CURVE_A	            560
#define HPM_CORE_50_CURVE_B	            195000
#define HPM_CORE_38_SUBZERO_CURVE_A	    523
#define HPM_CORE_38_SUBZERO_CURVE_B	    119000
#define HPM_CORE_24_SUBZERO_CURVE_A	    510
#define HPM_CORE_24_SUBZERO_CURVE_B	    132000

/* curve max/min; CPU HPM curve:   */

#define HPM_CPU_90_CURVE_A	            1085
#define HPM_CPU_90_CURVE_B	            318000
#define HPM_CPU_70_CURVE_A	            645
#define HPM_CPU_70_CURVE_B	            183000
#define HPM_CPU_50_CURVE_A	            308
#define HPM_CPU_50_CURVE_B	            90000
#define HPM_CPU_38_SUBZERO_CURVE_A	    889
#define HPM_CPU_38_SUBZERO_CURVE_B	    261000
#define HPM_CPU_24_SUBZERO_CURVE_A	    823
#define HPM_CPU_24_SUBZERO_CURVE_B	    259000

#define HPM_0_SCARE	            10000
#define TEMP_SCARE	            1024

#define HPM_HIGH_MASK	        0x3ff
#define HPM_HIGH_16	            16
#define HPM_CYCLE	            2
#define DELTA_MASK	            0x7
#define DELTA_SCARE	            10
#define DELTA_OFST	            8

#define SVB_VER_REG		        0x11020168
#define SVB_VER			        0x10
#define OTP_SHPM_MDA_OFFSET	    0x153c

#define temperature_formula(val)  (((((val) - 116) * 165) / 806) - 40)
#define duty_formula(max, min, val) ((((max) - (val)) * 416 + ((max) - (min) + 1) / 2) / ((max) - (min)) - 1)
#define volt_regval_formula(val) (((val) << 16) + ((416 - 1) << 4) + 0x5)

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
		delay(5);
		/* cpu */
		temp = readl(SYSCTRL_REG + HPM_CPU_REG0);
		cpu_value[1] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;
		cpu_value[0] += temp & HPM_HIGH_MASK;
		temp = readl(SYSCTRL_REG + HPM_CPU_REG1);
		cpu_value[3] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;
		cpu_value[2] += temp & HPM_HIGH_MASK;

		/* core */
		temp = readl(SYSCTRL_REG + HPM_CORE_REG0);
		core_value[1] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;
		core_value[0] += temp & HPM_HIGH_MASK;
		temp = readl(SYSCTRL_REG + HPM_CORE_REG1);
		core_value[3] += (temp >> HPM_HIGH_16) & HPM_HIGH_MASK;
		core_value[2] += temp & HPM_HIGH_MASK;
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

	delay(550);
}

static void get_temperature(int *temperature)
{
	unsigned int value = 0;
	int tsensor_chn;
	int tmp = 0;
	int temperature_temp[TSENSOR_MAX_CHN] = {0};
	delay(1000);
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
	if (temperature >= 90) /* 90: temperature */
		*hpm_core += (((HPM_CORE_90_CURVE_A * (*hpm_core) - HPM_CORE_90_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 70) /* 70: temperature */
		*hpm_core += (((HPM_CORE_70_CURVE_A * (*hpm_core) - HPM_CORE_70_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 50) /* 50: temperature */
		*hpm_core += (((HPM_CORE_50_CURVE_A * (*hpm_core) - HPM_CORE_50_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 0)
		*hpm_core = *hpm_core;
	else if (temperature >= -24) /* -24: temperature */
		*hpm_core -= ((HPM_CORE_24_SUBZERO_CURVE_A * (*hpm_core) - HPM_CORE_24_SUBZERO_CURVE_B) / HPM_0_SCARE);
	else
		*hpm_core -= ((HPM_CORE_38_SUBZERO_CURVE_A * (*hpm_core) - HPM_CORE_38_SUBZERO_CURVE_B) / HPM_0_SCARE);
}

static void adjust_hpm_cpu(unsigned int *hpm_cpu, int temperature)
{
	if (temperature >= 90) /* 90: temperature */
		*hpm_cpu += (((HPM_CPU_90_CURVE_A * (*hpm_cpu) - HPM_CPU_90_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 70) /* 70: temperature */
		*hpm_cpu += (((HPM_CPU_70_CURVE_A * (*hpm_cpu) - HPM_CPU_70_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 50) /* 50: temperature */
		*hpm_cpu += (((HPM_CPU_50_CURVE_A * (*hpm_cpu) - HPM_CPU_50_CURVE_B) / HPM_0_SCARE) - 1);
	else if (temperature >= 0)
		*hpm_cpu = *hpm_cpu;
	else if (temperature >= -24) /* -24: temperature */
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

static void start_svb(void)
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

void start_ddr_training(unsigned int base)
{
	start_svb();

	/* ddr hw training */
	bsp_ddrtrn_training_item(DDR_HW_TRAINING);
	/* ddr sw training */
	bsp_ddrtrn_training_item(DDR_SW_TRAINING);

	/* disable it when sample back */
	ddrtrn_retrain_enable();

	/* ddr DMC auto power down config */
	bsp_ddrtrn_dmc_auto_power_down_cfg();

#ifdef DDR_SCRAMB_ENABLE
	/* enable ddr scramble */
	ddr_scrambling();
#endif
}

#define REG_OTP_PO_VDAC_INFO	(SYSCTRL_REG + 0x1500)
#define OTP_DYING_GASP_OFST	6
#define OTP_DYING_GASP_MASK	(0x3f << OTP_DYING_GASP_OFST)

#define REG_DYING_GASP_CFG0	(REG_BASE_MISC + 0x20)
#define DYING_GASP_OFST		20
#define DYING_GASP_MASK		(0x3f << DYING_GASP_OFST)

void setup_dying_gasp_trim(void)
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
