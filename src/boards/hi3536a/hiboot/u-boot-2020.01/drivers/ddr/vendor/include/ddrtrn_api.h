// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DDR_API_H
#define DDR_API_H

enum ddrtrn_training_item {
	DDR_HW_TRAINING = 0,
	DDR_SW_TRAINING,
	DDR_PCODE_TRAINING,
	DDR_AC_TRAINING,
	DDR_ADJUST_TRAINING,
	DDR_DATAEYE_TRAINING,
	DDR_DCC_TRAINING,
	DDR_GATE_TRAINING,
	DDR_MPR_TRAINING,
	DDR_LPCA_TRAINING,
	DDR_VREF_TRAINING,
	DDR_WL_TRAINING,
	DDR_MAX_TRAINING
};

int bsp_ddrtrn_init(void);
int bsp_ddrtrn_training(void);
int bsp_ddrtrn_resume(void);
int bsp_ddrtrn_suspend(void);
int bsp_ddrtrn_cmd_help(void);
int bsp_ddrtrn_cmd_exec(char *str);
int bsp_ddrtrn_training_item(enum ddrtrn_training_item item);
void bsp_ddrtrn_dmc_auto_power_down_cfg(void);
void ddrtrn_retrain_enable(void);
#endif
