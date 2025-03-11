// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ddrtrn_training.h"

#ifdef DDR_TRAINING_EXEC_TIME
/*
 * Start timer for calculate DDR training execute time.
 * NOTE: Just only for debug.
 */
void ddrtrn_hal_chip_timer_start(void)
{
	/* timer start */
	ddrtrn_reg_write(0, 0xF8002000); /* REG_BASE_TIMER01 + REG_TIMER_RELOAD */
	/* TIMER_EN  | TIMER_MODE |TIMER_PRE | TIMER_SIZE, REG_TIMER_CONTROL */
	ddrtrn_reg_write(0xc2, 0xF8002008);
	ddrtrn_reg_write(0xffffffff, 0xF8002000);
}

/*
 * Stop timer for calculate DDR training execute time.
 * NOTE: Just only for debug.
 */
void ddrtrn_hal_chip_timer_stop(void)
{
	/* timer stop */
	ddrtrn_reg_write(0, 0xF8002008); /* REG_TIMER_CONTROL */
	/* REG_TIMER_VALUE, 24MHz */
	ddrtrn_warning("DDR training execute time: [%d]us\n",
		(0xffffffff - ddrtrn_reg_read(0xF8002004)) / 24); /* 24MHz */
}
#else
void ddrtrn_hal_chip_timer_start(void)
{
}

/*
 * Stop timer for calculate DDR training execute time.
 * NOTE: Just only for debug.
 */
void ddrtrn_hal_chip_timer_stop(void)
{
}
#endif
