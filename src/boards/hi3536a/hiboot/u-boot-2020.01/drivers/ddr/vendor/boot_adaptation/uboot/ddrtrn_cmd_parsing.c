// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include <command.h>
#include "ddrtrn_training.h"

#ifndef TEXT_BASE
#define TEXT_BASE (CONFIG_SYS_TEXT_BASE) /* for arm64 u-boot-2016.11 */
#endif

#define DDR_TRAINING_ENV    "ddrtr"
#define DDR_TRAINING_ENV_UN "unddrtr"

#define DDR_TRAINING_DDRT_START_OFFSET 0x400000 /* 4M */
#define DDR_TRAINING_DDRT_LENGTH       0x400000 /* 4M at lease 0x8000 */

#define DDR_CMD_SW_STR       "training"
#define DDR_CMD_TR_STR       "tr"
#define DDR_CMD_HW_STR       "hw"
#define DDR_CMD_MPR_STR      "mpr"
#define DDR_CMD_WL_STR       "wl"
#define DDR_CMD_GATE_STR     "gate"
#define DDR_CMD_DATAEYE_STR  "dataeye"
#define DDR_CMD_VREF_STR     "vref"
#define DDR_CMD_DPMC_STR     "dpmc"
#define DDR_CMD_DCC_STR      "dcc"
#define DDR_CMD_PCODE_STR    "pcode"
#define DDR_CMD_AC_STR       "ac"
#define DDR_CMD_LPCA_STR     "lpca"
#define DDR_CMD_LOG_STR      "log"
#define DDR_CMD_BOOT_STR     "boot"
#define DDR_CMD_CONSOLE_STR  "console"

#ifndef CONFIG_MINI_BOOT
struct ddrtrn_training_result g_ddrtr_result; /* DDR training result */
int g_ddr_log_level; /* DDR training log level */

/*
 * Match string command.
 * NOTE: Write leveling not support run repeatedly,
 * so limit WL only run one time.
 */
static int ddrtrn_cmd_match(const char *str, int *cmd)
{
	static int wl_done; /* Write leveling control */

	if (strncmp(str, DDR_CMD_SW_STR, sizeof(DDR_CMD_SW_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_SW_NO_WL;
	} else if (strncmp(str, DDR_CMD_TR_STR, sizeof(DDR_CMD_TR_STR)) == 0) {
		if (wl_done) {
			*cmd = DDR_TRAINING_CMD_SW_NO_WL;
		} else {
			wl_done++;
			*cmd = DDR_TRAINING_CMD_SW;
		}
	} else if (strncmp(str, DDR_CMD_HW_STR, sizeof(DDR_CMD_HW_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_HW;
	} else if (strncmp(str, DDR_CMD_MPR_STR, sizeof(DDR_CMD_MPR_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_MPR;
	} else if (strncmp(str, DDR_CMD_WL_STR, sizeof(DDR_CMD_WL_STR)) == 0) {
		if (wl_done) {
			printf("WL not support run repeatedly. %s",
				"Already done once, can not do again\n");
			return -1;
		} else {
			*cmd = DDR_TRAINING_CMD_WL;
			wl_done++;
		}
	} else if (strncmp(str, DDR_CMD_GATE_STR, sizeof(DDR_CMD_GATE_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_GATE;
	} else if (strncmp(str, DDR_CMD_DATAEYE_STR, sizeof(DDR_CMD_DATAEYE_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_DATAEYE;
	} else if (strncmp(str, DDR_CMD_VREF_STR, sizeof(DDR_CMD_VREF_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_VREF;
	} else if (strncmp(str, DDR_CMD_DPMC_STR, sizeof(DDR_CMD_DPMC_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_DPMC;
	} else if (strncmp(str, DDR_CMD_AC_STR, sizeof(DDR_CMD_AC_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_AC;
	} else if (strncmp(str, DDR_CMD_LPCA_STR, sizeof(DDR_CMD_LPCA_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_LPCA;
	} else if (strncmp(str, DDR_CMD_DCC_STR, sizeof(DDR_CMD_DCC_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_DCC;
	} else if (strncmp(str, DDR_CMD_PCODE_STR, sizeof(DDR_CMD_PCODE_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_PCODE;
	} else if (strncmp(str, DDR_CMD_CONSOLE_STR, sizeof(DDR_CMD_CONSOLE_STR)) == 0) {
		*cmd = DDR_TRAINING_CMD_CONSOLE;
	} else {
		printf("Command [ddr %s] is unsupport\n", str);
		return -1;
	}

	return 0;
}

/*
 * Handle DDR training.
 * Copy training codes from DDR to SRAM.
 */
static int ddrtrn_cmd_handle(int cmd)
{
	struct ddrtrn_training_result *result = NULL;
	struct ddrtrn_cmd cmd_data;
	g_ddr_log_level = DDR_LOG_ERROR;

	cmd_data.cmd = (unsigned int)cmd;
	cmd_data.level = (unsigned int)g_ddr_log_level;
	cmd_data.start = TEXT_BASE + DDR_TRAINING_DDRT_START_OFFSET;
	cmd_data.length = DDR_TRAINING_DDRT_LENGTH;

	printf("DDR training area: 0x%08X - 0x%08X\n", cmd_data.start,
		cmd_data.start + cmd_data.length);

#ifdef DDR_TRAINING_EXEC_TIME
		ddrtrn_hal_chip_timer_start();
#endif

	result = ddrtrn_cmd_training_if(&cmd_data);

#ifdef DDR_TRAINING_EXEC_TIME
		ddrtrn_hal_chip_timer_stop();
#endif

	if (result == NULL)
		return -1;

	/* copy training result from SRAM to DDR */
	ddrtr_copy_data((void *)&g_ddrtr_result, result, sizeof(struct ddrtrn_training_result));

	printf("DDR training finished\n");

	return 0;
}

/* DDR training cmd dispatch */
static int ddrtrn_cmd_dispatch(int cmd)
{
	int result;

	result = ddrtrn_cmd_handle(cmd);
	switch (cmd) {
	case DDR_TRAINING_CMD_SW:
	case DDR_TRAINING_CMD_SW_NO_WL:
	case DDR_TRAINING_CMD_CONSOLE:
		ddrtrn_cmd_result_display(&g_ddrtr_result,
			DDR_TRAINING_CMD_DATAEYE | DDR_TRAINING_CMD_LPCA);
		break;
	case DDR_TRAINING_CMD_DATAEYE:
	case DDR_TRAINING_CMD_VREF:
		ddrtrn_cmd_result_display(&g_ddrtr_result, DDR_TRAINING_CMD_DATAEYE);
		break;
	case DDR_TRAINING_CMD_WL:
	case DDR_TRAINING_CMD_GATE:
	case DDR_TRAINING_CMD_HW:
	case DDR_TRAINING_CMD_DPMC:
	case DDR_TRAINING_CMD_DCC:
	case DDR_TRAINING_CMD_PCODE:
		break;
	case DDR_TRAINING_CMD_LPCA:
		ddrtrn_cmd_result_display(&g_ddrtr_result, DDR_TRAINING_CMD_LPCA);
		break;
	default:
		break;
	}

	ddrtrn_reg_result_display(&g_ddrtr_result);
	return result;
}

/* Set DDR training log level */
static int ddrtrn_cmd_set_log_level(char * const argv[])
{
	int level;
	const char *str;

	str = argv[1];
	if (strncmp(str, DDR_CMD_LOG_STR, sizeof(DDR_CMD_LOG_STR))) {
		printf("Command [ddr %s] is unsupport\n", str);
		return -1;
	}

	str = argv[2]; /* argv[2]:Third string: ddr log level */
	if (strncmp(str, DDR_LOG_INFO_STR, sizeof(DDR_LOG_INFO_STR)) == 0) {
		level = DDR_LOG_INFO;
	} else if (strncmp(str, DDR_LOG_DEBUG_STR, sizeof(DDR_LOG_DEBUG_STR)) == 0) {
		level = DDR_LOG_DEBUG;
	} else if (strncmp(str, DDR_LOG_WARNING_STR, sizeof(DDR_LOG_WARNING_STR)) == 0) {
		level = DDR_LOG_WARNING;
	} else if (strncmp(str, DDR_LOG_ERROR_STR, sizeof(DDR_LOG_ERROR_STR)) == 0) {
		level = DDR_LOG_ERROR;
	} else if (strncmp(str, DDR_LOG_FATAL_STR, sizeof(DDR_LOG_FATAL_STR)) == 0) {
		level = DDR_LOG_FATAL;
	} else {
		printf("Command [ddr log %s] is unsupport\n", str);
		return -1;
	}

	g_ddr_log_level = level;
	printf("Set DDR training log level [%s] suc\n", str);

	return 0;
}

/*
 * Accept DDR training cmd.
 * Set training result to env without save.
 */
static int do_ddr_training(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *str = NULL;
	int cmd;
	struct ddrtrn_hal_training_custom_reg reg;

	if (argc < 2 || argc > 3) /* cmd string should be 2 or 3 */
		return -1;
	else if (argc == 3) /* 3 cmd string means ddr log level */
		return ddrtrn_cmd_set_log_level(argv);

	str = argv[1];

	if (ddrtrn_cmd_match(str, &cmd))
		return -1;

	ddrtr_set_data(&reg, 0, sizeof(struct ddrtrn_hal_training_custom_reg));
	if (ddrtrn_hal_boot_cmd_save(&reg) != 0) {
		printf("DDR cmd save wait tiemout!\n");
		return -1;
	}

	if (ddrtrn_cmd_dispatch(cmd))
		return -1;

	ddrtrn_hal_boot_cmd_restore(&reg);

	return 0;
}

U_BOOT_CMD(ddr, CONFIG_SYS_MAXARGS, 1, do_ddr_training, "ddr training function",
	"training    - DDR sofeware(Gate/Dataeye/Vref) training.\n"
	"ddr tr          - DDR sofeware(WL/Gate/Dataeye/Vref) training.\n"
	"ddr wl          - DDR Write leveling training.\n"
	"ddr gate        - DDR gate training.\n"
	"ddr dataeye     - DDR dataeye training and display training result.\n"
	"ddr vref        - DDR vref training.\n"
	"ddr hw          - DDR hardware training.\n"
	"ddr mpr         - DDR Multi-Purpose Register training.\n"
	"ddr ac          - DDR address command training.\n"
	"ddr lpca        - LPDDR command address training.\n"
	"ddr dcc         - DDR Duty Correction Control training.\n"
	"ddr pcode       - DDR io pcode training.\n"
	"ddr console     - DDR do training in SRAM.\n"
	"ddr log [level] - DDR log level. [info,debug,warning,error,fatal]\n");
#endif
