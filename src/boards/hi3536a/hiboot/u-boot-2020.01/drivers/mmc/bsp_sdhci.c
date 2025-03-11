// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include "mmc_private.h"

#define MIN_FREQ	400000
#define NOT_FOUND	(-1)
#define PHASE_SCALE	32
#define EDGE_TUNING_PHASE_STEP	4

static void enable_sample(struct sdhci_host *host);
static void set_drv_phase(struct sdhci_host *host,	unsigned int phase);
static void set_sampl_phase(struct sdhci_host *host, unsigned int phase);
static void wait_sampl_dll_slave_ready(struct sdhci_host *host);
static void enable_card_clk(struct sdhci_host *host);
static void disable_card_clk(struct sdhci_host *host);
static void enable_internal_clk(struct sdhci_host *host);
static void disable_internal_clk(struct sdhci_host *host);

#ifdef CONFIG_TARGET_SS626V100
#include <asm/arch-ss626v100/platform.h>
#include "bsp_ss626v100.c"
#endif
static void enable_card_clk(struct sdhci_host *host)
{
	u16 clk;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}

static void disable_card_clk(struct sdhci_host *host)
{
	u16 clk;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}

static void enable_internal_clk(struct sdhci_host *host)
{
	u16 clk;
	u16 timeout = 20;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk |= SDHCI_CLOCK_INT_EN | SDHCI_CLOCK_PLL_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	while (!(clk & SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			printf("%s: Internal clock never stabilised.\n",
					__func__);
			return;
		}
		timeout--;
		udelay(1000); /* delay 1000us */
		clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	}
}

static void __maybe_unused disable_internal_clk(struct sdhci_host *host)
{
	u16 clk;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}

static void set_drv_phase(struct sdhci_host *host, unsigned int phase)
{
	uintptr_t crg_addr;
	unsigned int reg;

	crg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_DRV_DLL_CTRL : REG_SDIO0_DRV_DLL_CTRL;
	reg = readl(crg_addr);
	reg &= ~SDIO_DRV_PHASE_SEL_MASK;
	reg |= sdio_drv_sel(phase);
	writel(reg, crg_addr);
}

static void enable_sample(struct sdhci_host *host)
{
	unsigned int reg;

	reg = sdhci_readl(host, SDHCI_AT_CTRL);
	reg |= SDHCI_SAMPLE_EN;
	sdhci_writel(host, reg, SDHCI_AT_CTRL);
}

static void set_sampl_phase(struct sdhci_host *host, unsigned int phase)
{
	unsigned int reg;

	reg = sdhci_readl(host, SDHCI_AT_STAT);
	reg &= ~SDHCI_PHASE_SEL_MASK;
	reg |= phase;
	sdhci_writel(host, reg, SDHCI_AT_STAT);
}

static void wait_sampl_dll_slave_ready(struct sdhci_host *host)
{
	unsigned int reg;
	unsigned int timeout = 20;
	uintptr_t reg_addr;

	reg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_SAMPL_DLL_STATUS : REG_SDIO0_SAMPL_DLL_STATUS;
	do {
		reg = readl(reg_addr);
		if (reg & SDIO_SAMPL_DLL_SLAVE_READY)
			return;

		udelay(1000); /* delay 1000us */
		timeout--;
	} while (timeout > 0);

	printf("sdhci-bsp: SAMPL DLL slave not ready.\n");
}

static void enable_edge_tuning(struct sdhci_host *host)
{
	unsigned int reg;

	reg = readl(REG_EMMC_SAMPLB_DLL_CTRL);
	reg &= ~SDIO_SAMPLB_DLL_CLK_MASK;
	reg |= sdio_samplb_sel(8); /* sel 8 */
	writel(reg, REG_EMMC_SAMPLB_DLL_CTRL);

	reg = sdhci_readl(host, SDHCI_MULTI_CYCLE);
	reg |= SDHCI_EDGE_DETECT_EN;
	sdhci_writel(host, reg, SDHCI_MULTI_CYCLE);
}

static void disable_edge_tuning(struct sdhci_host *host)
{
	unsigned int reg;

	reg = sdhci_readl(host, SDHCI_MULTI_CYCLE);
	reg &= ~SDHCI_EDGE_DETECT_EN;
	sdhci_writel(host, reg, SDHCI_MULTI_CYCLE);
}

static void select_sampl_phase(struct sdhci_host *host, unsigned int phase)
{
	disable_card_clk(host);
	set_sampl_phase(host, phase);
	wait_sampl_dll_slave_ready(host);
	enable_card_clk(host);
	udelay(1);
}

static int send_tuning(struct sdhci_host *host, u32 opcode, int* cmd_error)
{
	int count, err;
	const int tuning_num = 1;

	count = 0;
	do {
		err = mmc_send_tuning(host->mmc, opcode, NULL);
		if (err)
			break;

		count++;
	} while (count < tuning_num);

	return err;
}

static int bsp_mmc_exec_tuning(struct sdhci_host *host, unsigned int opcode)
{
	unsigned int index, val;
	unsigned int edge_p2f, edge_f2p, start, end, phase;
	unsigned int fall, rise, fall_updat_flag;
	unsigned int found;
	unsigned int prev_found = 0;
	int err;
	int prev_err = 0;
	unsigned short ctrl;

	wait_drv_dll_lock(host);
	enable_sampl_dll_slave(host);
	enable_sample(host);
	enable_edge_tuning(host);
	host->is_tuning = 1;

	start = 0;
	end = PHASE_SCALE / EDGE_TUNING_PHASE_STEP;

	edge_p2f = start;
	edge_f2p = end;
	for (index = 0; index <= end; index++) {
		select_sampl_phase(host, index * EDGE_TUNING_PHASE_STEP);

		err = mmc_send_tuning(host->mmc, opcode, NULL);
		if (!err) {
			val = sdhci_readl(host, SDHCI_MULTI_CYCLE);
			found = val & SDHCI_FOUND_EDGE;
		} else {
			found = 1;
		}

		if (prev_found && !found)
			edge_f2p = index;
		else if (!prev_found && found)
			edge_p2f = index;

		if ((edge_p2f != start) && (edge_f2p != end))
			break;

		prev_found = found;
	}

	if ((edge_p2f == start) && (edge_f2p == end)) {
		printf("sdhci-bsp: tuning failed! can not found edge!\n");
		return -1;
	}

	disable_edge_tuning(host);

	start = edge_p2f * EDGE_TUNING_PHASE_STEP;
	end = edge_f2p * EDGE_TUNING_PHASE_STEP;
	if (end <= start)
		end += PHASE_SCALE;

	fall = start;
	rise = end;
	fall_updat_flag = 0;
	for (index = start; index <= end; index++) {
		select_sampl_phase(host, index % PHASE_SCALE);

		err = send_tuning(host, opcode, NULL);
		if (err)
			debug("sdhci-bsp: send tuning CMD%u fail! phase:%u err:%d\n",
				opcode, index, err);

		if (err && index == start) {
			if (!fall_updat_flag) {
				fall_updat_flag = 1;
				fall = start;
			}
		} else {
			if (!prev_err && err) {
				if (!fall_updat_flag) {
					fall_updat_flag = 1;
					fall = index;
				}
			}
		}

		if (prev_err && !err)
			rise = index;

		if (err && index == end)
			rise = end;

		prev_err = err;
	}

	phase = ((fall + rise) / 2 + PHASE_SCALE / 2) % PHASE_SCALE; /* 2 for cal average */

	printf("sdhci-bsp: tuning done! valid phase shift [%u, %u] Final Phase:%u\n",
			rise % PHASE_SCALE, fall % PHASE_SCALE, phase);

	host->tuning_phase = phase;
	select_sampl_phase(host, phase);

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_TUNED_CLK;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
	host->is_tuning = 0;

	return 0;
}

void sdhci_set_host_caps(struct sdhci_host *host)
{
	host->cfg.host_caps =
		MMC_CAP(MMC_LEGACY) | MMC_CAP(SD_LEGACY) |
		MMC_MODE_HS |
		MMC_MODE_HS_52MHz |
		MMC_MODE_HS200 |
		MMC_MODE_1BIT |
		MMC_MODE_4BIT |
		0x0;

	if (get_mmc_bus_width() == MMC_BUS_WIDTH_8_BIT)
		host->cfg.host_caps |=
			//MMC_MODE_HS400 |
			MMC_MODE_HS400_ES |
			MMC_MODE_8BIT |
			0x0;
}

int sdhci_add_port(int index, uintptr_t regbase, u32 type)
{
	struct sdhci_host *host = NULL;

	if (type == MMC_TYPE_MMC)
		emmc_hardware_init();
	else
		sd_hardware_init();

	host = calloc(1, sizeof(struct sdhci_host));
	if (host == NULL) {
		puts("sdhci_host malloc fail!\n");
		return -ENOMEM;
	}

	host->name = "bsp-sdhci";
	host->index = index;
	host->type = type;
	host->ioaddr = (void *)regbase;
	host->quirks = 0;
	host->set_clock = bsp_mmc_set_clk;
	host->priv_init = bsp_mmc_priv_init;
	host->set_control_reg = bsp_mmc_set_ioconfig;
#ifdef MMC_SUPPORTS_TUNING
	host->execute_tuning = bsp_mmc_exec_tuning;
#endif
	add_sdhci(host, CONFIG_BSP_SDHCI_MAX_FREQ, MIN_FREQ);
	sdhci_set_host_caps(host);
	return 0;
}

static void print_mmcinfo(struct mmc *mmc)
{
	printf("MMC/SD Card:\n");
	printf("    MID:         0x%x\n", mmc->cid[0] >> 24); /* bit24 - 31 */
	printf("    Read Block:  %d Bytes\n", mmc->read_bl_len);
	printf("    Write Block: %d Bytes\n", mmc->write_bl_len);
	printf("    Chip Size:   %s Bytes (%s)\n",
			ultohstr(mmc->capacity),
			mmc->high_capacity ? "High Capacity" : "Low Capacity");
	printf("    Name:        \"%c%c%c%c%c\"\n",
			mmc->cid[0] & 0xff,           /* bit0   -  7 */
			(mmc->cid[1] >> 24),          /* bit24 - 32 */
			(mmc->cid[1] >> 16) & 0xff,   /* bit16 - 23 */
			(mmc->cid[1] >> 8) & 0xff,    /* bit8   - 15 */
			mmc->cid[1] & 0xff);          /* bit0   -  7 */

	printf("    Chip Type:   %s\n"
			"    Version:     %d.%d\n",
			IS_SD(mmc) ? "SD" : "MMC",
			EXTRACT_SDMMC_MAJOR_VERSION(mmc->version),
			EXTRACT_SDMMC_MINOR_VERSION(mmc->version));

	printf("    Speed:       %sHz\n", ultohstr(mmc->clock));
	printf("    Bus Width:   %dbit\n", mmc->bus_width);
	printf("    Mode:        %s\n", mmc->strobe_enhanced ? "HS400ES" :
			(mmc->selected_mode == MMC_HS_400) ? "HS400" :
			(mmc->selected_mode == MMC_HS_200) ? "HS200" :
			(mmc->selected_mode == MMC_HS) ? "HS" :
			(mmc->selected_mode == MMC_HS_52) ? "HS52" :
			(mmc->selected_mode == MMC_LEGACY) ? "LEGACY" :
			(mmc->selected_mode == SD_LEGACY) ? "SD_LEGACY" :"OTHER");
}

int bsp_mmc_init(int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int ret;

	if (mmc == NULL) {
		printf("mmc device not found!!\n");
		return -EINVAL;
	}

	ret = mmc_init(mmc);
	if (ret)
		return ret;

	if (!IS_SD(mmc)) {
		print_mmcinfo(mmc);
		return mmc_set_boot_config(mmc);
	}

	return 0;
}

void printf_mmc(int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);

	if (mmc != NULL)
		print_mmcinfo(mmc);
}
