// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"

#if PHY_DQ_BDL_LEVEL == 32
static void print_dataeye_win(unsigned int dq_num, unsigned int range,
	unsigned int dqs, unsigned int dq, unsigned int win)
{
	unsigned int k;

	printf("%-4u", dq_num);
	for (k = 0; k < PHY_DQ_BDL_LEVEL; k++) {
		if (k >= (range >> DDR_DATAEYE_RESULT_BIT) && k <= (range & DDR_DATAEYE_RESULT_MASK))
			printf("%-3s", "-");
		else
			printf("%-3s", "X");
	}
	printf(" 0x%08x  0x%-4x%-4u%-4u\n", range, dqs, dq, win);
}

void print_dataeye_title(const char *phase)
{
	unsigned int k;

	printf("%-4s", "DQ");
	for (k = 0; k < PHY_DQ_BDL_LEVEL; k++)
		printf("%-3u", k);
	printf(" %-10s  %-6s%-4s%-4s\n", "RANGE", phase, "DQ", "WIN");
}
#else
static void print_dataeye_win(unsigned int dq_num, unsigned int range,
	unsigned int dqs, unsigned int dq, unsigned int win)
{
	unsigned int k;

	printf("%-4u", dq_num);
	for (k = 0; k < PHY_DQ_BDL_LEVEL; k++) {
		if (k >= (range >> DDR_DATAEYE_RESULT_BIT) && k <= (range & DDR_DATAEYE_RESULT_MASK))
			printf("%-1s", "-");
		else
			printf("%-1s", "X");
	}
	printf(" 0x%08x  0x%-4x%-4u%-4u\n", range, dqs, dq, win);
}

static void print_dataeye_title(const char *phase)
{
	unsigned int k;

	printf("%-4s", "DQ");
	for (k = 0; k < PHY_DQ_BDL_LEVEL; k++) {
		if (k % 4 == 0) /* Print out the CA number which is a multiple of 4 */
			printf("%-4u", k);
	}
	printf(" %-10s  %-6s%-4s%-4s\n", "RANGE", phase, "DQ", "WIN");
}
#endif

struct ddrtrn_reg_val training_reg_val_rank0[] = {
	/* rank, byte, offset, value, name */
	{0, 0, ddrtrn_hal_phy_dxwdqsdly(0, 0), 0, "WDQS Byte0"},
	{0, 1, ddrtrn_hal_phy_dxwdqsdly(0, 1), 0, "WDQS Byte1"},
	{0, 2, ddrtrn_hal_phy_dxwdqsdly(0, 2), 0, "WDQS Byte2"},
	{0, 3, ddrtrn_hal_phy_dxwdqsdly(0, 3), 0, "WDQS Byte3"},
	{0, 0, ddrtrn_hal_phy_dxnwdqdly(0, 0), 0, "WDQ Phase Byte0"},
	{0, 1, ddrtrn_hal_phy_dxnwdqdly(0, 1), 0, "WDQ Phase Byte1"},
	{0, 2, ddrtrn_hal_phy_dxnwdqdly(0, 2), 0, "WDQ Phase Byte2"},
	{0, 3, ddrtrn_hal_phy_dxnwdqdly(0, 3), 0, "WDQ Phase Byte3"},
	{0, 0, ddrtrn_hal_phy_dxnwdqnbdl0(0, 0), 0, "WDQ BDL DQ0-DQ3"},
	{0, 0, ddrtrn_hal_phy_dxnwdqnbdl1(0, 0), 0, "WDQ BDL DQ4-DQ7"},
	{0, 1, ddrtrn_hal_phy_dxnwdqnbdl0(0, 1), 0, "WDQ BDL DQ8-DQ11"},
	{0, 1, ddrtrn_hal_phy_dxnwdqnbdl1(0, 1), 0, "WDQ BDL DQ12-DQ15"},
	{0, 2, ddrtrn_hal_phy_dxnwdqnbdl0(0, 2), 0, "WDQ BDL DQ16-DQ19"},
	{0, 2, ddrtrn_hal_phy_dxnwdqnbdl1(0, 2), 0, "WDQ BDL DQ20-DQ23"},
	{0, 3, ddrtrn_hal_phy_dxnwdqnbdl0(0, 3), 0, "WDQ BDL DQ24-DQ27"},
	{0, 3, ddrtrn_hal_phy_dxnwdqnbdl1(0, 3), 0, "WDQ BDL DQ28-DQ31"},
	{0, 0, ddrtrn_hal_phy_dxnwdqnbdl2(0, 0), 0, "WDM Byte0"},
	{0, 1, ddrtrn_hal_phy_dxnwdqnbdl2(0, 1), 0, "WDM Byte1"},
	{0, 2, ddrtrn_hal_phy_dxnwdqnbdl2(0, 2), 0, "WDM Byte2"},
	{0, 3, ddrtrn_hal_phy_dxnwdqnbdl2(0, 3), 0, "WDM Byte3"},
	{0, 0, ddrtrn_hal_phy_dxnoebdl(0, 0), 0, "Write DQ/DQS OE Byte0"},
	{0, 1, ddrtrn_hal_phy_dxnoebdl(0, 1), 0, "Write DQ/DQS OE Byte1"},
	{0, 2, ddrtrn_hal_phy_dxnoebdl(0, 2), 0, "Write DQ/DQS OE Byte2"},
	{0, 3, ddrtrn_hal_phy_dxnoebdl(0, 3), 0, "Write DQ/DQS OE Byte3"},
	{0, 0, ddrtrn_hal_phy_dxnrdqsdly(0), 0, "RDQS Byte0"},
	{0, 1, ddrtrn_hal_phy_dxnrdqsdly(1), 0, "RDQS Byte1"},
	{0, 2, ddrtrn_hal_phy_dxnrdqsdly(2), 0, "RDQS Byte2"},
	{0, 3, ddrtrn_hal_phy_dxnrdqsdly(3), 0, "RDQS Byte3"},
	{0, 0, ddrtrn_hal_phy_dxnrdqnbdl0(0, 0), 0, "RDQ BDL DQ0-DQ3"},
	{0, 0, ddrtrn_hal_phy_dxnrdqnbdl1(0, 0), 0, "RDQ BDL DQ4-DQ7"},
	{0, 1, ddrtrn_hal_phy_dxnrdqnbdl0(0, 1), 0, "RDQ BDL DQ8-DQ11"},
	{0, 1, ddrtrn_hal_phy_dxnrdqnbdl1(0, 1), 0, "RDQ BDL DQ12-DQ15"},
	{0, 2, ddrtrn_hal_phy_dxnrdqnbdl0(0, 2), 0, "RDQ BDL DQ16-DQ19"},
	{0, 2, ddrtrn_hal_phy_dxnrdqnbdl1(0, 2), 0, "RDQ BDL DQ20-DQ23"},
	{0, 3, ddrtrn_hal_phy_dxnrdqnbdl0(0, 3), 0, "RDQ BDL DQ24-DQ27"},
	{0, 3, ddrtrn_hal_phy_dxnrdqnbdl1(0, 3), 0, "RDQ BDL DQ28-DQ31"},
	{0, 0, ddrtrn_hal_phy_dxnrdqsgdly(0, 0), 0, "Gate Byte0"},
	{0, 1, ddrtrn_hal_phy_dxnrdqsgdly(0, 1), 0, "Gate Byte1"},
	{0, 2, ddrtrn_hal_phy_dxnrdqsgdly(0, 2), 0, "Gate Byte2"},
	{0, 3, ddrtrn_hal_phy_dxnrdqsgdly(0, 3), 0, "Gate Byte3"},
	{0, 0, DDR_PHY_ACCMDBDL2, 0, "CS"},
	{0, 0, DDR_PHY_ACPHYCTL7, 0, "CLK"},
	DDR_PHY_VREF_HOST_DISPLAY
	DDR_PHY_VREF_DRAM_DISPLAY
	DDR_DX_DPMC_DISPLAY
	DDR_PHY_ADDRPH_DISPLAY
	DDR_PHY_ADDRBDL_DISPLAY
	DDR_PHY_DCC_DISPLAY
};

/* rank 1 */
struct ddrtrn_reg_val training_reg_val_rank1[] = {
	{1, 0, ddrtrn_hal_phy_dxwdqsdly(1, 0), 0, "WDQS Byte0"},
	{1, 1, ddrtrn_hal_phy_dxwdqsdly(1, 1), 0, "WDQS Byte1"},
	{1, 2, ddrtrn_hal_phy_dxwdqsdly(1, 2), 0, "WDQS Byte2"},
	{1, 3, ddrtrn_hal_phy_dxwdqsdly(1, 3), 0, "WDQS Byte3"},
	{1, 0, ddrtrn_hal_phy_dxnwdqdly(1, 0), 0, "WDQ Phase Byte0"},
	{1, 1, ddrtrn_hal_phy_dxnwdqdly(1, 1), 0, "WDQ Phase Byte1"},
	{1, 2, ddrtrn_hal_phy_dxnwdqdly(1, 2), 0, "WDQ Phase Byte2"},
	{1, 3, ddrtrn_hal_phy_dxnwdqdly(1, 3), 0, "WDQ Phase Byte3"},
	{1, 0, ddrtrn_hal_phy_dxnwdqnbdl0(1, 0), 0, "WDQ BDL DQ0-DQ3"},
	{1, 0, ddrtrn_hal_phy_dxnwdqnbdl1(1, 0), 0, "WDQ BDL DQ4-DQ7"},
	{1, 1, ddrtrn_hal_phy_dxnwdqnbdl0(1, 1), 0, "WDQ BDL DQ8-DQ11"},
	{1, 1, ddrtrn_hal_phy_dxnwdqnbdl1(1, 1), 0, "WDQ BDL DQ12-DQ15"},
	{1, 2, ddrtrn_hal_phy_dxnwdqnbdl0(1, 2), 0, "WDQ BDL DQ16-DQ19"},
	{1, 2, ddrtrn_hal_phy_dxnwdqnbdl1(1, 2), 0, "WDQ BDL DQ20-DQ23"},
	{1, 3, ddrtrn_hal_phy_dxnwdqnbdl0(1, 3), 0, "WDQ BDL DQ24-DQ27"},
	{1, 3, ddrtrn_hal_phy_dxnwdqnbdl1(1, 3), 0, "WDQ BDL DQ28-DQ31"},
	{1, 0, ddrtrn_hal_phy_dxnwdqnbdl2(1, 0), 0, "WDM Byte0"},
	{1, 1, ddrtrn_hal_phy_dxnwdqnbdl2(1, 1), 0, "WDM Byte1"},
	{1, 2, ddrtrn_hal_phy_dxnwdqnbdl2(1, 2), 0, "WDM Byte2"},
	{1, 3, ddrtrn_hal_phy_dxnwdqnbdl2(1, 3), 0, "WDM Byte3"},
	{1, 0, ddrtrn_hal_phy_dxnoebdl(1, 0), 0, "Write DQ/DQS OE Byte0"},
	{1, 1, ddrtrn_hal_phy_dxnoebdl(1, 1), 0, "Write DQ/DQS OE Byte1"},
	{1, 2, ddrtrn_hal_phy_dxnoebdl(1, 2), 0, "Write DQ/DQS OE Byte2"},
	{1, 3, ddrtrn_hal_phy_dxnoebdl(1, 3), 0, "Write DQ/DQS OE Byte3"},

	{1, 0, ddrtrn_hal_phy_dxnrdqsdly(0), 0, "RDQS Byte0"},
	{1, 1, ddrtrn_hal_phy_dxnrdqsdly(1), 0, "RDQS Byte1"},
	{1, 2, ddrtrn_hal_phy_dxnrdqsdly(2), 0, "RDQS Byte2"},
	{1, 3, ddrtrn_hal_phy_dxnrdqsdly(3), 0, "RDQS Byte3"},

	{1, 0, ddrtrn_hal_phy_dxnrdqnbdl0(1, 0), 0, "RDQ BDL DQ0-DQ3"},
	{1, 0, ddrtrn_hal_phy_dxnrdqnbdl1(1, 0), 0, "RDQ BDL DQ4-DQ7"},
	{1, 1, ddrtrn_hal_phy_dxnrdqnbdl0(1, 1), 0, "RDQ BDL DQ8-DQ11"},
	{1, 1, ddrtrn_hal_phy_dxnrdqnbdl1(1, 1), 0, "RDQ BDL DQ12-DQ15"},
	{1, 2, ddrtrn_hal_phy_dxnrdqnbdl0(1, 2), 0, "RDQ BDL DQ16-DQ19"},
	{1, 2, ddrtrn_hal_phy_dxnrdqnbdl1(1, 2), 0, "RDQ BDL DQ20-DQ23"},
	{1, 3, ddrtrn_hal_phy_dxnrdqnbdl0(1, 3), 0, "RDQ BDL DQ24-DQ27"},
	{1, 3, ddrtrn_hal_phy_dxnrdqnbdl1(1, 3), 0, "RDQ BDL DQ28-DQ31"},
	{1, 0, ddrtrn_hal_phy_dxnrdqsgdly(1, 0), 0, "Gate Byte0"},
	{1, 1, ddrtrn_hal_phy_dxnrdqsgdly(1, 1), 0, "Gate Byte1"},
	{1, 2, ddrtrn_hal_phy_dxnrdqsgdly(1, 2), 0, "Gate Byte2"},
	{1, 3, ddrtrn_hal_phy_dxnrdqsgdly(1, 3), 0, "Gate Byte3"},
	{1, 0, DDR_PHY_ACCMDBDL2, 0, "CS"},
	{1, 0, DDR_PHY_ACPHYCTL7, 0, "CLK"},

	DDR_PHY_VREF_HOST_DISPLAY_RANK1
	DDR_PHY_VREF_DRAM_DISPLAY
	DDR_DX_DPMC_DISPLAY
	DDR_PHY_ADDRPH_DISPLAY
	DDR_PHY_ADDRBDL_DISPLAY
	DDR_PHY_DCC_DISPLAY
};

static void ddrtrn_cmd_result_print_write_dataeye(const struct ddrtrn_training_data *ddrtr_data, unsigned int win_min,
	unsigned int win_max, unsigned int win_sum)
{
	unsigned int i, j;
	unsigned int dq_num, dqs, dq, win;

	printf("Write window of prebit-deskew:\n");
	printf("--------------------------------------------------------\n");
	print_dataeye_title("DQPH");
	if (ddrtr_data->byte_num > DDR_PHY_BYTE_MAX) {
		printf("byte num error, byte_num = %x", ddrtr_data->byte_num);
		return;
	}
	for (j = 0; j < ddrtr_data->byte_num; j++) {
		dqs = (ddrtrn_reg_read(ddrtr_data->base_phy +
			ddrtrn_hal_phy_dxnwdqdly(ddrtr_data->rank_idx, j)) >>
			PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK;
		for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
			dq_num = (j << 3) + i; /* shift left 3: 8 bit */
			win = ddrtr_data->write.ddrtrn_bit_best[dq_num] >> DDR_DATAEYE_RESULT_BIT;
			if (win < win_min)
				win_min = win;

			if (win > win_max)
				win_max = win;

			win_sum += win;
			dq = ddrtr_data->write.ddrtrn_bit_best[dq_num] & DDR_DATAEYE_RESULT_MASK;
			print_dataeye_win(dq_num, ddrtr_data->write.ddrtrn_bit_result[dq_num],
				dqs, dq, win);
		}
	}
	printf("--------------------------------------------------------\n");
	printf("Sum WIN: %u. Avg WIN: %u\n", win_sum,
		win_sum / (ddrtr_data->byte_num * DDR_PHY_BIT_NUM));
	printf("Min WIN: %u. DQ Index: ", win_min);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->write.ddrtrn_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_min)
			printf("%u ", i);
	}
	printf("\nMax WIN: %u. DQ Index: ", win_max);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->write.ddrtrn_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_max)
			printf("%u ", i);
	}
	printf("\n\n");
}

static void ddrtrn_cmd_result_print_read_dataeye(const struct ddrtrn_training_data *ddrtr_data, unsigned int win_min,
	unsigned int win_max, unsigned int win_sum)
{
	unsigned int i, j;
	unsigned int dq_num, dqs, dq, win;

	if (ddrtr_data->byte_num > DDR_PHY_BYTE_MAX) {
		printf("get byte num fail, byte_num = %x", ddrtr_data->byte_num);
		return;
	}
	printf("Read window of prebit-deskew:\n");
	printf("--------------------------------------------------------\n");
	print_dataeye_title("DQS");
	for (j = 0; j < ddrtr_data->byte_num; j++) {
		dqs = ddrtrn_reg_read(ddrtr_data->base_phy + ddrtrn_hal_phy_dxnrdqsdly(j)) &
			PHY_RDQS_BDL_MASK;
		for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
			dq_num = (j << 3) + i; /* shift left 3: 8 bit */
			win =
				ddrtr_data->read.ddrtrn_bit_best[dq_num] >> DDR_DATAEYE_RESULT_BIT;
			if (win < win_min)
				win_min = win;

			if (win > win_max)
				win_max = win;

			win_sum += win;
			dq =
				ddrtr_data->read.ddrtrn_bit_best[dq_num] & DDR_DATAEYE_RESULT_MASK;
			print_dataeye_win(dq_num, ddrtr_data->read.ddrtrn_bit_result[dq_num],
				dqs, dq, win);
		}
	}
	printf("--------------------------------------------------------\n");
	printf("Sum WIN: %u. Avg WIN: %u\n", win_sum,
		win_sum / (ddrtr_data->byte_num * DDR_PHY_BIT_NUM));
	printf("Min WIN: %u. DQ Index: ", win_min);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->read.ddrtrn_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_min)
			printf("%u ", i);
	}
	printf("\nMax WIN: %u. DQ Index: ", win_max);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->read.ddrtrn_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_max)
			printf("%u ", i);
	}
	printf("\n\n");
}

static void ddrtrn_cmd_result_print_dataeye(const struct ddrtrn_training_data *ddrtr_data)
{
	/* Write window of prebit-deskew */
	ddrtrn_cmd_result_print_write_dataeye(ddrtr_data, PHY_DQ_BDL_LEVEL, 0, 0);

	/* Read window of prebit-deskew */
	ddrtrn_cmd_result_print_read_dataeye(ddrtr_data, PHY_DQ_BDL_LEVEL, 0, 0);
}

static void ddrtrn_cmd_result_print_ca_data(const struct ddrtrn_training_data *ddrtr_data, unsigned int min,
	unsigned int max, unsigned int sum)
{
	unsigned int i, j;
	unsigned int left, right, mid, win;

	/* data */
	for (i = 0; i < DDR_PHY_CA_MAX; i++) {
		left = ddrtr_data->ca_addr[i] >> DDR_DATAEYE_RESULT_BIT;
		right = ddrtr_data->ca_addr[i] & DDR_DATAEYE_RESULT_MASK;
		mid = (left + right) >> 1;
		win = right - left + 1;

		printf("%-4u", i);
		for (j = 0; j < PHY_DQ_BDL_LEVEL; j++) {
			if (j >= left && j <= right)
				printf("%-1s", "-");
			else
				printf("%-1s", "X");
		}
		printf(" 0x%08x  %-6u%-4u\n", ddrtr_data->ca_addr[i], mid, win);

		if (win < min)
			min = win;

		if (win > max)
			max = win;

		sum += win;
	}
	printf("--------------------------------------------------------\n");

	printf("Sum WIN: %u. Avg WIN: %u\n", sum, sum / DDR_PHY_CA_MAX);
	printf("Min WIN: %u. CA Index: ", min);
	for (i = 0; i < DDR_PHY_CA_MAX; i++) {
		win = (ddrtr_data->ca_addr[i] & DDR_DATAEYE_RESULT_MASK) -
			(ddrtr_data->ca_addr[i] >> DDR_DATAEYE_RESULT_BIT) + 1;
		if (win == min)
			printf("%u ", i);
	}
	printf("\nMax WIN: %u. CA Index: ", max);
	for (i = 0; i < DDR_PHY_CA_MAX; i++) {
		win = (ddrtr_data->ca_addr[i] & DDR_DATAEYE_RESULT_MASK) -
			(ddrtr_data->ca_addr[i] >> DDR_DATAEYE_RESULT_BIT) + 1;
		if (win == max)
			printf("%u ", i);
	}
}

static void ddrtrn_cmd_result_print_ca(const struct ddrtrn_training_data *ddrtr_data)
{
	unsigned int i;

	for (i = 0; i < DDR_PHY_CA_MAX; i++) {
		if (ddrtr_data->ca_addr[i] != 0)
			break;

		if (i == (DDR_PHY_CA_MAX - 1))
			return; /* no result to print */
	}

	printf("Command address window:\n");
	printf("--------------------------------------------------------\n");

	/* title: CA   0   4...124 */
	printf("%-4s", "CA");
	for (i = 0; i < PHY_DQ_BDL_LEVEL; i++) {
		if (i % 4 == 0) /* Print out the CA number which is a multiple of 4 */
			printf("%-4u", i);
	}
	printf(" %-10s  %-6s%-4s\n", "RANGE", "BDL", "WIN");

	/* data */
	ddrtrn_cmd_result_print_ca_data(ddrtr_data, PHY_DQ_BDL_LEVEL, 0, 0);
	printf("\n\n");
}

static void ddrtrn_cmd_result_print_by_rank(const struct ddrtrn_training_result *ddrtr_result, unsigned int cmd,
	unsigned int phy_index, unsigned int rank_index)
{
	/* DDR_BYPASS_PHY0_MASK/DDR_BYPASS_PHY1_MASK/DDR_BYPASS_PHY2_MASK */
	unsigned int mask = 1 << phy_index;
	const struct ddrtrn_rank_data *rank = &ddrtr_result->phy[phy_index].rank[rank_index];

	if (rank->item & mask)
		return;

	printf("\r\n[PHY%u][RANK%u]:\r\n", phy_index, rank_index);
	if (DDR_TRAINING_CMD_DATAEYE & cmd)
		ddrtrn_cmd_result_print_dataeye(&rank->ddrtr_data);

	if (DDR_TRAINING_CMD_LPCA & cmd)
		ddrtrn_cmd_result_print_ca(&rank->ddrtr_data);
}

static void ddrtrn_cmd_result_print_by_phy(const struct ddrtrn_training_result *ddrtr_result, unsigned int cmd,
	unsigned int phy_index)
{
	int i;

	if (ddrtr_result->phy[phy_index].rank_num > DDR_SUPPORT_RANK_MAX) {
		printf("loop upper limit rank number out of range, rank_num = %x",
			ddrtr_result->phy[phy_index].rank_num);
		return;
	}
	for (i = 0; i < ddrtr_result->phy[phy_index].rank_num; i++)
		ddrtrn_cmd_result_print_by_rank(ddrtr_result, cmd, phy_index, i);
}

void ddrtrn_cmd_result_display(const struct ddrtrn_training_result *ddrtr_result,
	unsigned int cmd)
{
	int i;

	if (ddrtr_result->phy_num > DDR_PHY_NUM)
		return;
	for (i = 0; i < ddrtr_result->phy_num; i++)
		ddrtrn_cmd_result_print_by_phy(ddrtr_result, cmd, i);
}

static void ddrtrn_reg_result_display_by_rank(
	const struct ddrtrn_training_result *ddrtr_result, unsigned int phy_index,
	unsigned int rank_index)
{
	int i;
	unsigned int base_phy =
		ddrtr_result->phy[phy_index].rank[rank_index].ddrtr_data.base_phy;
	unsigned int num;
	unsigned int byte_num =
		ddrtr_result->phy[phy_index].rank[rank_index].ddrtr_data.byte_num;
	unsigned int rank_num = ddrtr_result->phy[phy_index].rank_num;
	struct ddrtrn_reg_val *ddrtrn_reg = NULL;

	if (rank_index == 0) {
		num = sizeof(training_reg_val_rank0) / sizeof(struct ddrtrn_reg_val);
		ddrtrn_reg = &training_reg_val_rank0[0];
	} else {
		num = sizeof(training_reg_val_rank1) / sizeof(struct ddrtrn_reg_val);
		ddrtrn_reg = &training_reg_val_rank1[0];
	}

	printf("\r\n[PHY%u][RANK%u]:\r\n", phy_index, rank_index);
	for (i = 0; i < num; i++) {
		if (i != 0)
			ddrtrn_reg++;

		if (ddrtrn_reg->offset == 0)
			continue;

		if (ddrtrn_reg->byte_index >= byte_num)
			continue;

		if (ddrtrn_reg->rank_index >= rank_num)
			continue;

		ddrtrn_reg->val = ddrtrn_reg_read(base_phy + ddrtrn_reg->offset);

		printf("[0x%08x = 0x%08x] %-32s", base_phy + ddrtrn_reg->offset,
			ddrtrn_reg->val, ddrtrn_reg->name);

		if ((i + 1) % 2 == 0) /* 2: Print information for 2 registers per line */
			printf("\r\n");
	}
}

static void ddrtrn_reg_result_display_by_phy(const struct ddrtrn_training_result *ddrtr_result,
	unsigned int phy_index)
{
	unsigned int i;
	unsigned int mask = 1 << phy_index; /* DDR_BYPASS_PHY0_MASK or DDR_BYPASS_PHY1_MASK */
	unsigned int item;
	unsigned int enable = 0;
	unsigned int rank_num = ddrtr_result->phy[phy_index].rank_num;

	/* check rank0 and rank1 training item */
	for (i = 0; i < rank_num; i++) {
		item = ddrtr_result->phy[phy_index].rank[i].item;
		if ((item & mask) == 0)
			enable = 1;
	}

	if (enable == 0)
		return;

	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_phy_switch_rank((unsigned int)ddrtr_result->phy[phy_index].rank[i].ddrtr_data.base_phy, i);
		ddrtrn_reg_result_display_by_rank(ddrtr_result, phy_index, i);
		ddrtrn_hal_phy_switch_rank((unsigned int)ddrtr_result->phy[phy_index].rank[i].ddrtr_data.base_phy, 0);
	}
}

/* Display DDR training register */
void ddrtrn_reg_result_display(const struct ddrtrn_training_result *ddrtr_result)
{
	int i;

	for (i = 0; i < ddrtr_result->phy_num; i++)
		ddrtrn_reg_result_display_by_phy(ddrtr_result, i);

	printf("\r\n");
}

#ifndef CONFIG_MINI_BOOT
static int ddrtrn_cmd_is_disable(void)
{
	unsigned int cfg;
	unsigned int mask;
	unsigned int i;
	int disable = 1;

	cfg = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG);
	for (i = 0; i < DDR_PHY_NUM; i++) {
		mask = 0x1 << i;
		if ((cfg & mask) == 0)
			disable = 0;
	}

#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	cfg = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC);
	for (i = 0; i < DDR_PHY_NUM; i++) {
		mask = 0x1 << i;
		if ((cfg & mask) == 0)
			disable = 0;
	}
#endif

	return disable;
}

/* Get DDR training command function entry address */
static void *ddrtrn_cmd_get_entry(void)
{
	char *src_ptr = 0;
	char *dst_ptr;
	unsigned int length;

	src_ptr = g_ddrtrn_training_cmd_start;
	dst_ptr = (char *)(DDR_TRAINING_RUN_STACK);
	length = (uintptr_t)g_ddrtrn_training_cmd_end - (uintptr_t)src_ptr;

	if ((src_ptr == NULL) || length == 0) {
		printf("DDR training is unsupport.\n");
		return 0;
	}

	printf("DDR training cmd entry[0x%08X] size[%u]byte cfg[0x%08X = 0x%08X]",
		DDR_TRAINING_RUN_STACK, length,
		(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG),
		ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG));

#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	printf("[0x%08X = 0x%08X]", (DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC),
		ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC));
#endif
	printf("\n");

	if (ddrtrn_cmd_is_disable() != 0) {
		printf("Please config DDR training item. Bypass bit:\n"
			"[0]PHY0            : 0x1\n"
			"[1]PHY1            : 0x2\n"
			"[2]PHY2            : 0x4\n"
			"[4]Write Leveling  : 0x10\n"
			"[8]Gate            : 0x100\n"
			"[16]Dataeye        : 0x10000\n"
			"[18]Pcode          : 0x40000\n"
			"[20]HW             : 0x100000\n"
			"[21]MPR            : 0x200000\n"
			"[22]AC             : 0x400000\n"
			"[23]LPCA           : 0x800000\n"
			"[24]Host Vref      : 0x1000000\n"
			"[25]Dram Vref      : 0x2000000\n"
			"[27]DCC            : 0x8000000\n"
			"[28]Dataeye Adjust : 0x10000000\n"
			"[29]WL Write Adjust: 0x20000000\n"
			"[30]HW Read Adjust : 0x40000000\n");
		return 0;
	}

	ddrtrn_hal_cmd_prepare_copy();
	ddrtr_copy_data(dst_ptr, src_ptr, length);
	return (void *)dst_ptr;
}

/* Copy training codes from DDR to SRAM and do ddr training */
struct ddrtrn_training_result *ddrtrn_cmd_training_if(struct ddrtrn_cmd *cmd)
{
	ddrtrn_cmd_entry_func entry;
	struct ddrtrn_training_result *result = NULL;

	entry = (ddrtrn_cmd_entry_func)ddrtrn_cmd_get_entry();
	if (entry == NULL)
		return 0;

#ifdef CONFIG_ARM64
	asm("isb");
	asm("dsb sy");
#else
	/* instruction cache invalidate all to PoU */
	asm("mcr p15, 0, r0, c7, c5, 0");
	/* data synchronization barrier operation */
	asm("mcr p15, 0, r0, c7, c10, 4");
#endif

	/* save site before execute cmd */
	ddrtrn_hal_cmd_site_save();

	/* entry equal ddrtrn_training_cmd_entry() */
	result = entry(cmd);

	/* restore site before execute cmd */
	ddrtrn_hal_cmd_site_restore();

	if (result == NULL) {
		printf("DDR training fail\n");
		return 0;
	}

	return result;
}
#endif
