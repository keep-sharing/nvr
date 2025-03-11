// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <stdarg.h>
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

/* ddr training cmd result */
struct ddrtrn_training_result g_ddrt_result_sram;
static unsigned int g_ddr_training_addr_start;
static unsigned int g_ddr_training_addr_end;
static unsigned int g_ddr_training_cmd;
static int g_ddr_print_level;

#ifdef DDR_TRAINING_LOG_CONFIG
/* Log ddr training info. */
void ddrtrn_training_log(const char *func, int level, const char *fmt, ...)
{
	va_list args;
	g_ddr_print_level = DDR_LOG_ERROR;

	if (g_ddr_print_level > level)
		return;

	DDR_PUTC('[');
	switch (level) {
	case DDR_LOG_INFO:
		DDR_PUTS("INFO");
		break;
	case DDR_LOG_DEBUG:
		DDR_PUTS("DEBUG");
		break;
	case DDR_LOG_WARNING:
		DDR_PUTS("WARNING");
		break;
	case DDR_LOG_ERROR:
		DDR_PUTS("ERROR");
		break;
	case DDR_LOG_FATAL:
		DDR_PUTS("FATAL");
		break;
	default:
		break;
	}
	DDR_PUTC(']');
	DDR_PUTC('[');
	DDR_PUTS(func);
	DDR_PUTC(']');

	va_start(args, fmt);
	while (*fmt != '\0') {
		if (*fmt != '%') {
			DDR_PUTC(*fmt);
		} else {
			fmt++;
			switch (*fmt) {
			case 'x':
			case 'X':
				DDR_PUTS("0x");
				DDR_PUT_HEX(va_arg(args, int));
				break;
			default:
				DDR_PUTC('%');
				DDR_PUTC(*fmt);
				break;
			} /* switch */
		}
		fmt++;
	} /* while */
	va_end(args);
	DDR_PUTS("\r\n");
}

/* Nothing to do in DDR command when defined DDR_TRAINING_LOG_CONFIG. */
void ddrtrn_training_error(unsigned int mask, unsigned int phy, int byte, int dq)
{
	return;
}
#else
/* Display DDR training error. */
void ddrtrn_training_error(unsigned int mask, unsigned int phy, int byte, int dq)
{
	switch (mask) {
	case DDR_ERR_WL:
		DDR_PUTS("WL");
		break;
	case DDR_ERR_HW_GATING:
		DDR_PUTS("HW Gate");
		break;
	case DDR_ERR_GATING:
		DDR_PUTS("Gate");
		break;
	case DDR_ERR_DDRT_TIME_OUT:
		DDR_PUTS("DDRT");
		break;
	case DDR_ERR_HW_RD_DATAEYE:
		DDR_PUTS("HW Dataeye");
		break;
	case DDR_ERR_MPR:
		DDR_PUTS("MPR");
		break;
	case DDR_ERR_DATAEYE:
		DDR_PUTS("Dataeye");
		break;
	case DDR_ERR_LPCA:
		DDR_PUTS("LPCA");
		break;
	default:
		break;
	}

	DDR_PUTS(" Err:");

	if (phy != 0) {
		DDR_PUTS(" Phy:");
		DDR_PUT_HEX(phy);
	}

	if (byte != -1) {
		DDR_PUTS(" Byte:");
		DDR_PUT_HEX(byte);
	}

	if (dq != -1) {
		DDR_PUTS(" DQ:");
		DDR_PUT_HEX(dq);
	}

	DDR_PUTS("\r\n");
}
#endif

/* Inint ddr training cmd result */
static void ddrtrn_training_result_init(struct ddrtrn_training_result *ddrtr_res)
{
	unsigned int i, j;

	ddrtr_set_data(ddrtr_res, 0, sizeof(struct ddrtrn_training_result));
	ddrtr_res->phy_num = ddrtrn_hal_get_phy_num();
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtr_res->phy[i].rank_num = ddrtrn_hal_get_phy_rank_num(i);

		for (j = 0; j < ddrtrn_hal_get_phy_rank_num(i); j++) {
			ddrtr_res->phy[i].rank[j].item = ddrtrn_hal_get_rank_item(i, j);
			ddrtr_res->phy[i].rank[j].ddrtr_data.base_phy = ddrtrn_hal_get_phy_addr(i);
			ddrtr_res->phy[i].rank[j].ddrtr_data.byte_num = ddrtrn_hal_get_phy_total_byte_num(i);
			ddrtr_res->phy[i].rank[j].ddrtr_data.rank_idx = j;
		}
	}
}

/* Save ddr training cmd result */
void ddrtrn_result_data_save(const struct ddrtrn_training_result_data *training)
{
	unsigned int i;
	unsigned int offset;
	struct ddrtrn_training_result_data *dest = NULL;
	struct ddrtrn_training_result *ddrtr_res = &g_ddrt_result_sram;

	if (ddrtrn_hal_get_res_flag() == RES_DISABLE || training == NULL)
		return;

	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ)
		dest = &ddrtr_res->phy[ddrtrn_hal_get_phy_id()].rank[ddrtrn_hal_get_rank_id()].ddrtr_data.read;
	else
		dest = &ddrtr_res->phy[ddrtrn_hal_get_phy_id()].rank[ddrtrn_hal_get_rank_id()].ddrtr_data.write;
	if (ddrtrn_hal_get_cur_phy_dmc_num() == 1) {
		ddrtr_copy_data(dest, training, sizeof(struct ddrtrn_training_result_data));
	} else {
		/* dmc[0] + dmc[1] */
		offset = ddrtrn_hal_get_dmc_id() << 4; /* Shift left 4:16 */
		for (i = 0; i < (ddrtrn_hal_get_byte_num() << DDR_BYTE_DQ); i++) {
			dest->ddrtrn_bit_result[i + offset] = training->ddrtrn_bit_result[i + offset];
			dest->ddrtrn_bit_best[i + offset] = training->ddrtrn_bit_best[i + offset];
		}
		dest->ddrtrn_win_sum += training->ddrtrn_win_sum;
	}
}

/* Save lpca training data */
void ddrtrn_lpca_data_save(const struct ddrtrn_ca_data *data)
{
	unsigned int index;
	struct ddrtrn_training_result *ddrtr_res = &g_ddrt_result_sram;
	struct ddrtrn_training_data *tr_data = NULL;

	if (ddrtrn_hal_get_res_flag() == RES_DISABLE || data == NULL)
		return;
	tr_data = &ddrtr_res->phy[ddrtrn_hal_get_phy_id()].rank[ddrtrn_hal_get_rank_id()].ddrtr_data;

	for (index = 0; index < DDR_PHY_CA_MAX; index++)
		tr_data->ca_addr[index] =
			((unsigned int)data->left[index] << DDR_DATAEYE_RESULT_BIT) | (unsigned int)data->right[index];
}

/* Nothing to do in DDR command */
void ddrtrn_training_suc(void)
{
	return;
}

/* Nothing to do in DDR command */
void ddrtrn_training_start(void)
{
	return;
}

/* Get DDRT test addrress */
unsigned int ddrtrn_ddrt_get_test_addr(void)
{
	if (g_ddr_training_addr_start <= DDRT_CFG_TEST_ADDR_CMD &&
		g_ddr_training_addr_end >= DDRT_CFG_TEST_ADDR_CMD) {
		return DDRT_CFG_TEST_ADDR_CMD;
	} else {
		ddrtrn_warning("DDRT test address[%x] out of range[%x, %x]",
			DDRT_CFG_TEST_ADDR_CMD, g_ddr_training_addr_start,
			g_ddr_training_addr_end);
		return g_ddr_training_addr_start;
	}
}

static void dump_result(const struct ddrtrn_training_data *ddrtr_data)
{
	unsigned int i;
	unsigned int base_phy = ddrtr_data->base_phy;
	unsigned int byte_num = ddrtr_data->byte_num;
	unsigned int rank = ddrtr_data->rank_idx;
	unsigned int acphyctl7;

	/* Some registers need to be read multiple times to obtain correct values */
	acphyctl7 = ddrtrn_hal_read_repeatedly(base_phy, DDR_PHY_ACPHYCTL7);

	for (i = 0; i < (ddrtr_data->byte_num << DDR_BYTE_DQ); i++)
		ddrtrn_info("Byte[%x] Write[%x][%x] Read[%x][%x]", i,
			ddrtr_data->write.ddrtrn_bit_result[i],
			ddrtr_data->write.ddrtrn_bit_best[i],
			ddrtr_data->read.ddrtrn_bit_result[i],
			ddrtr_data->read.ddrtrn_bit_best[i]);

	for (i = 0; i < DDR_PHY_CA_MAX; i++) {
		if (ddrtr_data->ca_addr[i] != 0)
			ddrtrn_info("CA[%x] Range[%x]", i, ddrtr_data->ca_addr[i]);
	}

	/* WDQS */
	ddrtrn_hal_phy_wdqs_display_cmd(base_phy, rank, byte_num);

	/* WDQ Phase */
	ddrtrn_hal_phy_wdqphase_display_cmd(base_phy, rank, byte_num);

	/* WDQ BDL */
	ddrtrn_hal_phy_wdqbdl_display_cmd(base_phy, rank, byte_num);

	/* WDM */
	ddrtrn_hal_phy_wdm_display_cmd(base_phy, rank, byte_num);

	/* Write DO/DOS OE */
	ddrtrn_hal_phy_wdq_wdqs_oe_display_cmd(base_phy, rank, byte_num);

	/* RDQS */
	ddrtrn_hal_phy_rdqs_display_cmd(base_phy, byte_num);

	/* RDQ BDL */
	ddrtrn_hal_phy_rdqbdl_display_cmd(base_phy, rank, byte_num);

	/* Gate */
	ddrtrn_hal_phy_gate_display_cmd(base_phy, rank, byte_num);

	/* CS */
	ddrtrn_hal_phy_cs_display_cmd(base_phy);

	/* CLK */
	ddrtrn_hal_phy_clk_display_cmd(base_phy, acphyctl7);

	ddrtrn_hal_phy_switch_rank(base_phy, rank);

	/* HOST Vref */
	ddrtrn_hal_phy_vref_host_display_cmd(base_phy, rank, byte_num);

	/* DRAM Vref */
	ddrtrn_hal_phy_vref_dram_display_cmd(base_phy, byte_num);

	/* DPMC */
	ddrtrn_hal_phy_dx_dpmc_display_cmd(base_phy, byte_num);

	/* Addr Phase */
	ddrtrn_hal_phy_addrph_display_cmd(base_phy);

	/* Addr BDL */
	ddrtrn_hal_phy_addrbdl_display_cmd(base_phy);

	/* DCC */
	ddrtrn_hal_phy_dcc_display_cmd(base_phy);

	ddrtrn_hal_phy_switch_rank(base_phy, 0);
}

static void dump_result_by_rank(const struct ddrtrn_training_result *ddrtr_result,
	unsigned int phy_index, unsigned int rank_index)
{
	unsigned int mask = 1 << phy_index; /* DDR_BYPASS_PHY0_MASK DDR_BYPASS_PHY1_MASK */
	const struct ddrtrn_rank_data *rank = &ddrtr_result->phy[phy_index].rank[rank_index];

	if (rank->item & mask)
		return;

	ddrtrn_info("PHY[%x] RANK[%x]:", phy_index, rank_index);
	dump_result(&rank->ddrtr_data);
}

static void dump_result_by_phy(const struct ddrtrn_training_result *ddrtr_result, unsigned int phy_index)
{
	int i;
	const struct ddrtrn_phy_data *phy = &ddrtr_result->phy[phy_index];

	for (i = 0; i < (int)phy->rank_num; i++)
		dump_result_by_rank(ddrtr_result, phy_index, i);
}

/* Display ddr training result before return to DDR */
static void dump_result_all(const struct ddrtrn_training_result *ddrtr_result)
{
	int i;

	for (i = 0; i < (int)ddrtr_result->phy_num; i++)
		dump_result_by_phy(ddrtr_result, i);
}

int ddrtrn_training_cmd_func(void)
{
	int result;
	unsigned int item = ddrtrn_hal_get_cur_item();

	ddrtrn_debug("DDR training cmd[%x]", g_ddr_training_cmd);

	switch (g_ddr_training_cmd) {
	case DDR_TRAINING_CMD_SW:
		result = ddrtrn_dataeye_training_func();
		result += ddrtrn_vref_training_func();
		break;
	case DDR_TRAINING_CMD_DATAEYE:
		result = ddrtrn_dataeye_training_func();
		break;
	case DDR_TRAINING_CMD_HW:
		result = ddrtrn_hw_training_if();
		break;
#ifdef DDR_MPR_TRAINING_CONFIG
	case DDR_TRAINING_CMD_MPR:
		result = ddrtrn_mpr_training_func();
		break;
#endif
	case DDR_TRAINING_CMD_WL:
		result = ddrtrn_wl_func();
		break;
	case DDR_TRAINING_CMD_GATE:
		result = ddrtrn_gating_func();
		break;
	case DDR_TRAINING_CMD_VREF:
		result = ddrtrn_vref_training_func();
		break;
	case DDR_TRAINING_CMD_AC:
		result = ddrtrn_ac_training_func();
		break;
#ifdef DDR_LPCA_TRAINING_CONFIG
	case DDR_TRAINING_CMD_LPCA:
		result = ddrtrn_lpca_training_func();
		break;
#endif
	case DDR_TRAINING_CMD_SW_NO_WL:
		/* wl bypass */
		ddrtrn_hal_set_sysctrl_cfg(item | DDR_BYPASS_WL_MASK);
		result = ddrtrn_dataeye_training_func();
		result += ddrtrn_vref_training_func();
		/* restore cfg */
		ddrtrn_hal_set_sysctrl_cfg(item);
		break;
	case DDR_TRAINING_CMD_CONSOLE:
		result = ddrtrn_training_console_if();
		break;
	default:
		result = -1;
		break;
	}

	return result;
}

/* DDR training command entry. Call by ddrtrn_cmd_handle(). */
struct ddrtrn_training_result *ddrtrn_training_cmd_entry(struct ddrtrn_cmd *cmd)
{
	int result;
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	uintptr_t ctx_addr = (uintptr_t)&ctx;
	uintptr_t phy_all_addr = (uintptr_t)&phy_all;

	ddrtrn_hal_set_cfg_addr(ctx_addr, phy_all_addr);

	g_ddr_training_addr_start = cmd->start;
	g_ddr_training_addr_end = cmd->start + cmd->length;
	g_ddr_print_level = cmd->level;
	g_ddr_training_cmd = cmd->cmd;

	ddrtrn_info("DDR Training Version: " DDR_TRAINING_VER);
	ddrtrn_debug("DDR training command entry. Sysctl[%x = %x]",
		(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG),
		ddrtrn_hal_get_sysctrl_cfg(SYSCTRL_DDR_TRAINING_CFG));

#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	ddrtrn_debug("Rank1 Sysctl[%x = %x]",
		(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC),
		ddrtrn_hal_get_sysctrl_cfg(SYSCTRL_DDR_TRAINING_CFG_SEC));
#endif

	ddrtrn_hal_cfg_init();
	ddrtrn_hal_cmd_flag_enable();
	ddrtrn_hal_res_flag_enable();

	ddrtrn_training_result_init(&g_ddrt_result_sram);

	if (cmd->cmd == DDR_TRAINING_CMD_HW)
		result = ddrtrn_hw_training();
	else if (cmd->cmd == DDR_TRAINING_CMD_PCODE)
		result = ddrtrn_pcode_training();
	else if (cmd->cmd == DDR_TRAINING_CMD_DCC)
		result = ddrtrn_dcc_training_func();
	else if (cmd->cmd == DDR_TRAINING_CMD_CONSOLE)
		result = ddrtrn_training_console_if();
	else
		result = ddrtrn_training_all();
	dump_result_all(&g_ddrt_result_sram);

	if (result == 0) {
		return &g_ddrt_result_sram;
	} else {
		ddrtrn_debug("DDR training result[%x]", result);
		return 0;
	}
}
