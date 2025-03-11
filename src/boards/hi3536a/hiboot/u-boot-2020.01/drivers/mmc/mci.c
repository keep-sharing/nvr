// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <config.h>
#include <common.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <mci_reg.h>

#include "mmc_private.h"
#include "mci.h"

#define MMC_CS_ERROR_MASK   0xFDFFA080
#define MCI_WAIT_CMD_DELAY 100

#if MCI_DEBUG
int debug_type = MCI_DEBUG_TYPE;

char *get_debug_type_string(int type)
{
	if (type & MCI_DEBUG_TYPE_REG)
		return "REG";
	else if (type & MCI_DEBUG_TYPE_FUN)
		return "FUN";
	else if (type & MCI_DEBUG_TYPE_CMD)
		return "CMD";
	else if (type & MCI_DEBUG_TYPE_INFO)
		return "INFO";
	else if (type & MCI_DEBUG_TYPE_ERR)
		return "ERR";
	else
		return "UNKNOWN";
}
#endif

static unsigned int retry_count = MAX_RETRY_COUNT;

static struct mci_dma_des dma_des[MAX_DMA_DES]
__attribute__((aligned(512)));

#define MCI_SYS_RESET_TIME_OUT 1000000

/* reset MMC host controler */
static void mci_sys_reset(const struct mci_host *host)
{
	unsigned int tmp_reg;
	unsigned int time_out;

	mci_debug_fun("Function Call");
	mci_assert(host);

	tmp_reg = mci_readl(host->base + MCI_BMOD);
	tmp_reg |= BMOD_SWR;
	mci_writel(tmp_reg, host->base + MCI_BMOD);

	time_out = MCI_SYS_RESET_TIME_OUT;
	do {
		tmp_reg = mci_readl(host->base + MCI_BMOD);
	} while ((tmp_reg & BMOD_SWR) && time_out--);

	tmp_reg = mci_readl(host->base + MCI_BMOD);
	tmp_reg |= BURST_16 | BURST_INCR;
	mci_writel(tmp_reg, host->base + MCI_BMOD);

	tmp_reg = mci_readl(host->base + MCI_CTRL);
	tmp_reg |=  CTRL_RESET | FIFO_RESET | DMA_RESET;
	mci_writel(tmp_reg, host->base + MCI_CTRL);
}

static void mci_ctrl_power(const struct mci_host *host, int flag)
{
	int reg_value;
	mci_debug_fun("Function Call: flag %d", flag);
	mci_assert(host);

	if (flag)
		reg_value = 1 << host->port;
	else
		reg_value = 0;

	mci_writel(reg_value, host->base + MCI_PWREN);
}

static int tuning_reset_flag = 0;
#define MCI_WAIT_CMD_RETRY_TIMES 500
static int mci_wait_cmd(const struct mci_host *host)
{
	unsigned int tmp_reg;
	unsigned int wait_retry_count = 0;

	mci_debug_fun("Function Call");
	mci_assert(host);

	do {
		tmp_reg = mci_readl(host->base + MCI_CMD);
		if ((tmp_reg & START_CMD) == 0)
			return 0;
		wait_retry_count++;
	} while (wait_retry_count < retry_count * 10000); /* 10000: increase retry count */
	if (host->is_tuning)
		tuning_reset_flag = 1;
	mci_debug_info("CMD send timeout");

	return -1;
}

static void mci_control_cclk(struct mci_host *host, unsigned int flag)
{
	unsigned int reg;
	cmd_arg_s cmd_reg;

	mci_debug_fun("Function Call: flag %d", flag);
	mci_assert(host);

	reg = mci_readl(host->base + MCI_CLKENA);
	if (flag == ENABLE)
		reg |= CCLK_ENABLE << host->port;
	else
		reg &= ~(CCLK_ENABLE << host->port);
	mci_writel(reg, host->base + MCI_CLKENA);

	cmd_reg.cmd_arg = mci_readl(host->base + MCI_CMD);
	cmd_reg.cmd_arg &= ~MCI_CMD_MASK;
	cmd_reg.bits.start_cmd = 1;
	cmd_reg.bits.card_number = host->port;
	cmd_reg.bits.update_clk_reg_only = 1;
	mci_writel(cmd_reg.cmd_arg, host->base + MCI_CMD);

	if (mci_wait_cmd(host) != 0)
		mci_debug_err("Disable or enable CLK is timeout");
}

static void mci_set_cclk(struct mci_host *host, unsigned int cclk)
{
	unsigned int reg_value;
	cmd_arg_s cmd_reg;

	mci_debug_fun("Function Call: cclk %d", cclk);
	mci_assert(host);
	mci_assert(cclk);

	/*set card clk divider value, clk_divider = Fmmcclk/(Fmmc_cclk * 2) */
	reg_value = 0;
	if (cclk < MMC_CLK) {
		reg_value = MMC_CLK / (cclk * 2);
		if (MMC_CLK % (cclk * 2))
			reg_value++;
		if (reg_value > 0xFF)
			reg_value = 0xFF;
	}
	mci_writel(reg_value, host->base + MCI_CLKDIV);

	cmd_reg.cmd_arg = mci_readl(host->base + MCI_CMD);
	cmd_reg.cmd_arg &= ~MCI_CMD_MASK;
	cmd_reg.bits.start_cmd = 1;
	cmd_reg.bits.card_number = host->port;
	cmd_reg.bits.update_clk_reg_only = 1;
	mci_writel(cmd_reg.cmd_arg, host->base + MCI_CMD);

	if (mci_wait_cmd(host) != 0)
		mci_debug_err("Set card CLK divider is failed");
}
/**********************************************
 *1: card off
 *0: card on
 ***********************************************/
static unsigned int mci_sys_card_detect(const struct mci_host *host)
{
	unsigned int card_status;

	card_status = readl((uintptr_t)(host->base + MCI_CDETECT));
	card_status &= (MCI_CARD0 << host->port);

	return card_status >> host->port;
}

#define MCI_UM_IN_MM 1000
static int mci_is_emmc(const struct mci_host *host)
{
	return (int)(host->base == EMMC_REG_BASE);
}

static void mci_init_card(struct mci_host *host)
{
	unsigned int tmp_reg;

	mci_debug_fun("Function Call");
	mci_assert(host);

	mci_sys_reset(host);

	/* card reset */
	mci_writel(~(1 << host->port), host->base + MCI_RESET_N);
	__udelay(CONFIG_MMC_RESET_LOW_TIMEOUT);

	/* card power off and power on */
	mci_ctrl_power(host, POWER_OFF);
	if (!mci_is_emmc(host))
		__udelay(CONFIG_MMC_POWER_OFF_TIMEOUT * MCI_UM_IN_MM);
	mci_ctrl_power(host, POWER_ON);
	if (!mci_is_emmc(host))
		__udelay(CONFIG_MMC_POWER_ON_TIMEROUT * MCI_UM_IN_MM);

	/* card reset cancel */
	mci_writel(1 << host->port, host->base + MCI_RESET_N);
	__udelay(CONFIG_MMC_RESET_HIGH_TIMEROUT);

	/* set drv/smpl phase shift */
	tmp_reg = mci_readl(host->base + MCI_UHS_REG_EXT);
	tmp_reg &= ~(DRV_PHASE_MASK | SMPL_PHASE_MASK);
	tmp_reg |= DRV_PHASE_SHIFT | SMPL_PHASE_SHIFT;
	mci_writel(tmp_reg, host->base + MCI_UHS_REG_EXT);

	/* clear MMC host intr */
	mci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);

	/*read write threshold*/
	mci_writel(RW_THRESHOLD_SIZE, host->base + MMC_CARDTHRCTL);

	/* MASK MMC host intr */
	tmp_reg = mci_readl(host->base + MCI_INTMASK);
	tmp_reg &= ~ALL_INT_MASK;
	mci_writel(tmp_reg, host->base + MCI_INTMASK);

	/* enable inner DMA mode and close intr of MMC host controler */
	tmp_reg = mci_readl(host->base + MCI_CTRL);
	tmp_reg &= ~INTR_EN;
	tmp_reg |= USE_INTERNAL_DMA;
	mci_writel(tmp_reg, host->base + MCI_CTRL);

	/* enable dma intr */
	tmp_reg = TI | RI | NI;
	mci_writel(tmp_reg, host->base + MCI_IDINTEN);

	/* set timeout param */
	mci_writel(DATA_TIMEOUT | RESPONSE_TIMEOUT, host->base + MCI_TIMEOUT);

	/* set FIFO param */
	mci_writel(BURST_SIZE | RX_WMARK | TX_WMARK, host->base + MCI_FIFOTH);

	/* set dto fix bypass */
	tmp_reg = mci_readl(host->base + MCI_GPIO);
	tmp_reg |= DTO_FIX_BYPASS;
	mci_writel(tmp_reg, host->base + MCI_GPIO);
}

static void mci_idma_start(const struct mci_host *host, const struct mmc_data *data)
{
	unsigned int tmp_reg;

	mci_debug_fun("Function Call");
	mci_assert(host);
	mci_assert(data);

	mci_writel((unsigned long)(uintptr_t)host->dma_des, host->base + MCI_DBADDR);
	mci_debug_info("host->dma_des is 0x%x", (unsigned int)host->dma_des);

	tmp_reg = mci_readl(host->base + MCI_BMOD);
	tmp_reg |= BMOD_DMA_EN;
	mci_writel(tmp_reg, host->base + MCI_BMOD);
}

static void mci_idma_stop(const struct mci_host *host)
{
	unsigned int tmp_reg;

	mci_debug_fun("Function Call");
	mci_assert(host);

	tmp_reg = mci_readl(host->base + MCI_BMOD);
	tmp_reg &= ~BMOD_DMA_EN;
	mci_writel(tmp_reg, host->base + MCI_BMOD);
	mci_writel(0, host->base + MCI_BYTCNT);
	mci_writel(0, host->base + MCI_BLKSIZ);
	mci_writel(0, host->base + MCI_DBADDR);
}

static void mci_idma_reset(const struct mci_host *host)
{
	unsigned int regval;

	regval = mci_readl(host->base + MCI_BMOD);
	regval |= BMOD_SWR;
	mci_writel(regval, host->base + MCI_BMOD);

	regval = mci_readl(host->base + MCI_CTRL);
	regval |= CTRL_RESET | FIFO_RESET | DMA_RESET;
	mci_writel(regval, host->base + MCI_CTRL);

	udelay(1);
	mci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);
}

static unsigned long mci_setup_idma(const struct mci_host *host, const struct mmc_data *data)
{
	unsigned long data_size = data->blocks * data->blocksize;
	unsigned long des_cnt = 0;
	struct mci_dma_des *des = host->dma_des;
	unsigned long src = (uintptr_t)data->src;
	while (data_size) {
		des[des_cnt].idmac_des_ctrl = DMA_DES_OWN | DMA_DES_NEXT_DES;
		des[des_cnt].idmac_des_buf_addr = src;
		des[des_cnt].idmac_des_next_addr = (uintptr_t)(des + des_cnt + 1);

		if (data_size >= 0x1000) {
			des[des_cnt].idmac_des_buf_size = 0x1000;
			data_size -= 0x1000;
			src += 0x1000;
		} else {
			des[des_cnt].idmac_des_buf_size = data_size;
			data_size = 0;
		}

		mci_debug_info("des[%ld] vaddr is 0x%X", des_cnt, (unsigned int)&des[des_cnt]);
		mci_debug_info("des[%ld].idmac_des_ctrl is 0x%X", des_cnt, (unsigned int)des[des_cnt].idmac_des_ctrl);
		mci_debug_info("des[%ld].idmac_des_buf_size is 0x%X", des_cnt, (unsigned int)des[des_cnt].idmac_des_buf_size);
		mci_debug_info("des[%ld].idmac_des_buf_addr 0x%X", des_cnt, (unsigned int)des[des_cnt].idmac_des_buf_addr);
		mci_debug_info("des[%ld].idmac_des_next_addr is 0x%X", des_cnt, (unsigned int)des[des_cnt].idmac_des_next_addr);
		des_cnt++;
	}

	des[0].idmac_des_ctrl |= DMA_DES_FIRST_DES;
	des[des_cnt - 1].idmac_des_ctrl |= DMA_DES_LAST_DES;
	des[des_cnt - 1].idmac_des_next_addr = 0;
	return des_cnt;
}

static int mci_isnot_valid_mmc_data(const struct mmc_data *data)
{
	unsigned long data_size = data->blocks * data->blocksize;
	if (((data_size + 0x1000 - 1) / 0x1000) > MAX_DMA_DES) {
		mci_error("Data size outside the limit of DMA des, "
			    "data size: 0x%08lx, limit of DMA des: 0x%08x",
			    data_size, 0x1000 * MAX_DMA_DES);
		return -1;
	}
	return 0;
}

static void mci_flush_idma_cache(unsigned long des_cnt, const struct mmc_data *data, const struct mci_host *host)
{
	unsigned long data_size = data->blocks * data->blocksize;
	data_size = ALIGN(data_size, CONFIG_SYS_CACHELINE_SIZE);
	des_cnt = ALIGN(des_cnt, CONFIG_SYS_CACHELINE_SIZE);
	if (data->flags != MMC_DATA_READ)
		flush_cache((uintptr_t)data->src, data_size);
	else
		invalidate_dcache_range((uintptr_t)data->dest, (uintptr_t)data->dest + data_size);

	flush_cache((uintptr_t)host->dma_des, sizeof(struct mci_dma_des) * (unsigned long)des_cnt);
}

static int mci_setup_data(const struct mci_host *host, const struct mmc_data *data)
{
	unsigned long des_cnt;

	mci_debug_fun("Function Call");
	mci_assert(host);
	mci_assert(data);

	if (mci_isnot_valid_mmc_data(data))
		return -1;

	des_cnt = mci_setup_idma(host, data);
	mci_flush_idma_cache(des_cnt, data, host);
	return 0;
}
static int mci_cmd_reg_set_resp_type(volatile cmd_arg_s *cmd_reg, const struct mmc_cmd *cmd)
{
	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		cmd_reg->bits.response_expect = 0;
		cmd_reg->bits.response_length = 0;
		cmd_reg->bits.check_response_crc = 0;
		break;
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
		cmd_reg->bits.response_expect = 1;
		cmd_reg->bits.response_length = 0;
		cmd_reg->bits.check_response_crc = 1;
		break;
	case MMC_RSP_R2:
		cmd_reg->bits.response_expect = 1;
		cmd_reg->bits.response_length = 1;
		cmd_reg->bits.check_response_crc = 1;
		break;
	case MMC_RSP_R3:
		cmd_reg->bits.response_expect = 1;
		cmd_reg->bits.response_length = 0;
		cmd_reg->bits.check_response_crc = 0;
		break;
	default:
		mci_error("unhandled response type %02x", cmd->resp_type);
		return -1;
	}
	return 0;
}

static void mci_set_cmd_reg_with_data(volatile cmd_arg_s *cmd_reg, const struct mmc_data *data)
{
	cmd_reg->bits.data_transfer_expected = 1;
	if (data->flags & (MMC_DATA_WRITE | MMC_DATA_READ))
		cmd_reg->bits.transfer_mode = 0;

	if (data->flags & MMC_DATA_WRITE)
		cmd_reg->bits.read_write = 1;
	else if (data->flags & MMC_DATA_READ)
		cmd_reg->bits.read_write = 0;
}

static void mci_set_cmd_reg_with_out_data(volatile cmd_arg_s *cmd_reg)
{
	cmd_reg->bits.data_transfer_expected = 0;
	cmd_reg->bits.transfer_mode = 0;
	cmd_reg->bits.read_write = 0;
}

static int mci_exec_cmd(const struct mci_host *host, struct mmc_cmd *cmd,
			   const struct mmc_data *data)
{
	volatile cmd_arg_s cmd_reg;

	mci_debug_fun("Function Call");
	mci_assert(host);
	mci_assert(cmd);

	mci_writel(cmd->cmdarg, host->base + MCI_CMDARG);

	cmd_reg.cmd_arg = mci_readl(host->base + MCI_CMD);
	cmd_reg.cmd_arg &= ~MCI_CMD_MASK;
	if (data) {
		mci_set_cmd_reg_with_data(&cmd_reg, data);
	} else {
		mci_set_cmd_reg_with_out_data(&cmd_reg);
	}

	cmd_reg.bits.wait_prvdata_complete = 1;
	cmd_reg.bits.send_auto_stop = 0;

	if (!(host->is_tuning))
		cmd_reg.bits.send_auto_stop = 1;

	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION) {
		cmd_reg.bits.stop_abort_cmd = 1;
		cmd_reg.bits.wait_prvdata_complete = 0;
	} else {
		cmd_reg.bits.stop_abort_cmd = 0;
		cmd_reg.bits.wait_prvdata_complete = 1;
	}

	if (mci_cmd_reg_set_resp_type(&cmd_reg, cmd))
		return -1;

	mci_debug_info("Send cmd of card is CMD %d", cmd->cmdidx);

	if (cmd->cmdidx == MMC_CMD_GO_IDLE_STATE)
		cmd_reg.bits.send_initialization = 1;
	else
		cmd_reg.bits.send_initialization = 0;

	cmd_reg.bits.card_number = host->port;
	cmd_reg.bits.cmd_index = cmd->cmdidx;
	cmd_reg.bits.send_auto_stop = 0;
	cmd_reg.bits.start_cmd = 1;
	cmd_reg.bits.update_clk_reg_only = 0;
	mci_writel(cmd_reg.cmd_arg, host->base + MCI_CMD);

	if (mci_wait_cmd(host) != 0) {
		mci_debug_err("Send card cmd is failed");
		return -1;
	}
	return 0;
}

static int mci_handle_response(struct mmc_cmd *cmd, const struct mci_host *host)
{
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = mci_readl(host->base + MCI_RESP3);
		cmd->response[1] = mci_readl(host->base + MCI_RESP2);
		cmd->response[2] = mci_readl(host->base + MCI_RESP1);
		cmd->response[3] = mci_readl(host->base + MCI_RESP0);
		mci_debug_info("CMD Response of card is "
				"%08x %08x %08x %08x",
				cmd->response[0], cmd->response[1],
				cmd->response[2], cmd->response[3]);
	} else {
		cmd->response[0] = mci_readl(host->base + MCI_RESP0);
		mci_debug_info("CMD Response of card is %08x",
				cmd->response[0]);
	}
	if (host->mmc->version && !IS_SD(host->mmc)) {
		if ((cmd->resp_type == MMC_RSP_R1)
				|| (cmd->resp_type == MMC_RSP_R1b)) {
			if (cmd->response[0] & MMC_CS_ERROR_MASK) {
				mci_debug_err("Card status"
						" stat = 0x%x"
						" is card error!",
						cmd->response[0]);
				return -1;
			}
		}
	}
	return 0;
}

static int mci_response_present(const struct mmc_cmd *cmd)
{
	return cmd->resp_type & MMC_RSP_PRESENT;
}

static int mci_cmd_done(struct mci_host *host, unsigned int stat)
{
	struct mmc_cmd *cmd = host->cmd;

	mci_debug_fun("Function Call: stat 0x%08x", stat);
	mci_assert(host);
	mci_assert(cmd);

	host->cmd = NULL;

	if (mci_response_present(cmd)) {
		if (mci_handle_response(cmd, host))
			return -1;
	}

	if (stat & RTO_INT_STATUS) {
		mci_debug_err("CMD status stat = 0x%x is timeout error!",
				stat);
		return -ETIMEDOUT;
	} else if (stat & (RCRC_INT_STATUS | RE_INT_STATUS)) {
		mci_debug_err("CMD status stat = 0x%x is response error!",
				stat);
		return -1;
	}
	return 0;
}

static void mci_data_done(const struct mci_host *host, unsigned int stat)
{
	mci_debug_fun("Function Call: stat 0x%08x", stat);
	mci_assert(host);

	if (stat & (HTO_INT_STATUS | DRTO_INT_STATUS)) {
		mci_debug_err("Data status stat = 0x%x is timeout error!",
				stat);
	} else if (stat & (EBE_INT_STATUS | SBE_INT_STATUS | FRUN_INT_STATUS
			   | DCRC_INT_STATUS)) {
		mci_debug_err("Data status stat = 0x%x is data error!", stat);
	}
}


static int mci_wait_cmd_complete(struct mci_host *host)
{
	unsigned int wait_retry_count = 0;
	unsigned int reg_data;
	int ret;

	mci_debug_fun("Function Call");
	mci_assert(host);

	do {
		reg_data = mci_readl(host->base + MCI_RINTSTS);
		if (reg_data & CD_INT_STATUS) {
			mci_writel(CD_INT_STATUS | RTO_INT_STATUS |
				     RCRC_INT_STATUS | RE_INT_STATUS,
				     host->base + MCI_RINTSTS);
			ret = mci_cmd_done(host, reg_data);
			return ret;
		}
		wait_retry_count++;
	} while (wait_retry_count < retry_count * 10000); /* 10000: increase retry count */

	mci_debug_err("Wait cmd complete error! irq status is 0x%x",
			reg_data);

	return -1;
}

static int mci_wait_data_complete(const struct mci_host *host)
{
	unsigned int wait_retry_count = 0;
	unsigned int reg_data;

	mci_debug_fun("Function Call");
	mci_assert(host);

	do {
		reg_data = mci_readl(host->base + MCI_RINTSTS);
		if (reg_data & DTO_INT_STATUS) {
			mci_idma_stop(host);
			mci_data_done(host, reg_data);
			return 0;
		}

		if (host->is_tuning) {
			if (reg_data & (HTO_INT_STATUS | DRTO_INT_STATUS |
					EBE_INT_STATUS | SBE_INT_STATUS |
					FRUN_INT_STATUS | DCRC_INT_STATUS)) {
				mci_idma_stop(host);
				mci_data_done(host, reg_data);
				return -1;
			}
		}
		wait_retry_count++;
	} while (wait_retry_count < retry_count * 10000); /* 10000: increase retry count */

	mci_debug_err("Wait data complete error! irq status is 0x%x",
			reg_data);

	return -1;
}

static int mci_wait_card_complete(const struct mci_host *host)
{
	unsigned int wait_retry_count = 0;
	unsigned int reg_data;

	mci_debug_fun("Function Call");
	mci_assert(host);

	do {
		reg_data = mci_readl(host->base + MCI_STATUS);
		if (!(reg_data & DATA_BUSY))
			return 0;
		wait_retry_count++;
	} while (wait_retry_count < retry_count * 10000); /* 10000: increase retry count */

	mci_debug_err("Wait card complete error! status is 0x%x", reg_data);

	return -1;
}

static int mci_request_with_data(const struct mmc *mmc, const struct mmc_data *data)
{
	struct mci_host *host = mmc->priv;
	unsigned int blk_size, tmp_reg;
	unsigned int fifo_count = 0;

	if (mci_setup_data(host, data)) {
		mci_error("Data setup is error!");
		return -1;
	}

	blk_size = data->blocks * data->blocksize;

	mci_writel(blk_size, host->base + MCI_BYTCNT);
	mci_writel(data->blocksize, host->base + MCI_BLKSIZ);

	tmp_reg = mci_readl(host->base + MCI_CTRL);
	tmp_reg |= FIFO_RESET;
	mci_writel(tmp_reg, host->base + MCI_CTRL);

	do {
		tmp_reg = mci_readl(host->base + MCI_CTRL);
		if (fifo_count > retry_count) {
			mci_error("FIFO reset error!");
			break;
		}
		fifo_count++;
	} while (tmp_reg & FIFO_RESET);

	/* start DMA */
	mci_idma_start(host, data);
	return 0;
}

static void mci_request_with_out_data(const struct mmc *mmc)
{
	struct mci_host *host = mmc->priv;
	mci_writel(0, host->base + MCI_BYTCNT);
	mci_writel(0, host->base + MCI_BLKSIZ);
}

static int mci_request(struct mmc *mmc, struct mmc_cmd *cmd,
			  struct mmc_data *data)
{
	struct mci_host *host = mmc->priv;
	int ret;

	mci_debug_fun("Function Call");
	mci_assert(mmc);
	mci_assert(host);
	mci_assert(cmd);

	host->cmd = cmd;

	mci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);

	/* prepare data */
	if (data) {
		ret = mci_request_with_data(mmc, data);
		if (ret)
			goto request_end;
	} else {
		mci_request_with_out_data(mmc);
	}

	/* send command */
	ret = mci_exec_cmd(host, cmd, data);
	if (ret) {
		mci_error("CMD execute is error!");
		goto request_end;
	}

	/* wait command send complete */
	ret = mci_wait_cmd_complete(host);
	if (ret)
		goto request_end;

	/* start data transfer */
	if (data) {
		/* wait data transfer complete */
		ret = mci_wait_data_complete(host);
		if (ret)
			goto request_end;

		/* wait card complete */
		ret = mci_wait_card_complete(host);
		if (ret)
			goto request_end;
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		/* wait card complete */
		ret = mci_wait_card_complete(host);
		if (ret)
			goto request_end;
	}
request_end:
	/* clear MMC host intr */
	mci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);
	return ret;
}

#define MCI_8BIT 8
#define MCI_4BIT 4

static int mci_set_ios(struct mmc *mmc)
{
	struct mci_host *host = mmc->priv;
	unsigned int tmp_reg;

	mci_debug_fun("Function Call");
	mci_assert(mmc);
	mci_assert(host);

	if (mmc->clock) {
		mci_control_cclk(host, DISABLE);
		mci_set_cclk(host, mmc->clock);
		mci_control_cclk(host, ENABLE);
	} else {
		mci_control_cclk(host, DISABLE);
	}

	/* set bus_width */
	mci_debug_info("ios->bus_width = %d", mmc->bus_width);

	tmp_reg = mci_readl(host->base + MCI_CTYPE);
	tmp_reg &= ~CARD_WIDTH_MASK;

	if (mmc->bus_width == MCI_8BIT)
		tmp_reg |= (CARD_WIDTH_8BIT << host->port);
	else if (mmc->bus_width == MCI_4BIT)
		tmp_reg |= (CARD_WIDTH_4BIT << host->port);
	else
		tmp_reg |= (CARD_WIDTH_1BIT << host->port);

	mci_writel(tmp_reg, host->base + MCI_CTYPE);

	mci_set_drv_cap(mmc, 0);
	mci_set_default_phase(mmc);

	return 0;
}

static int mci_init(struct mmc *mmc)
{
	struct mci_host *host = mmc->priv;

	mci_debug_fun("Function Call");

	mci_init_card(host);

	return 0;
}

static int mci_card_busy(struct mmc *mmc)
{
	struct mci_host *host = mmc->priv;
	unsigned int regval;

	mci_debug_fun("Function Call");
	mci_assert(mmc);
	mci_assert(host);

	regval = mci_readl(host->base + MCI_STATUS);
	regval &= DATA_BUSY;

	return regval;
}

#define MCI_REG_PERI_CRG83 0x1201014c
#define MCI_SAP_DLL_DLLMODE_OFFSET 16
#define MCI_SAP_DLL_SLAVE_EN 0x80000
#define MCI_DLL_CKEN 1

static void mci_edge_tuning_enable(const struct mci_host *host)
{
	unsigned int val;

	mci_writel(MCI_SAP_DLL_SLAVE_EN | MCI_DLL_CKEN, MCI_REG_PERI_CRG83);

	val = mci_readl(host->base + MCI_TUNING_CTRL);
	val |= HW_TUNING_EN;
	mci_writel(val, host->base + MCI_TUNING_CTRL);
}

static void mci_edge_tuning_disable(const struct mci_host *host)
{
	unsigned int val;

	val = mci_readl(MCI_REG_PERI_CRG83);
	val |= (1 << MCI_SAP_DLL_DLLMODE_OFFSET);
	mci_writel(val, MCI_REG_PERI_CRG83);

	val = mci_readl(host->base + MCI_TUNING_CTRL);
	val &= ~HW_TUNING_EN;
	mci_writel(val, host->base + MCI_TUNING_CTRL);
}

static void mci_set_sap_phase(const struct mci_host *host, unsigned int phase)
{
	unsigned int reg_value;

	reg_value = mci_readl(host->base + MCI_UHS_REG_EXT);
	reg_value &= ~CLK_SMPL_PHS_MASK;
	reg_value |= (phase << CLK_SMPL_PHS_SHIFT);
	mci_writel(reg_value, host->base + MCI_UHS_REG_EXT);
}

static int mci_send_stop(struct mmc *mmc)
{
	struct mmc_cmd cmd = {0};
	int err;

	cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
	cmd.resp_type = MMC_RSP_R1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	return err;
}

static int mci_send_tuning(struct mmc *mmc, unsigned int opcode)
{
	int err;
	unsigned int status = 1000;
	struct mci_host *host = mmc->priv;
	/* fix a problem that When the I/O voltage is increased to 1.89 V or 1.91V
	 * at high and low temperatures, the system is suspended during the reboot test.
	 */
	unsigned cmd_count = 1000;

	mci_control_cclk(host, DISABLE);
tuning_retry:
	mci_idma_reset(host);
	mci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);
	mci_control_cclk(host, ENABLE);
	if (tuning_reset_flag == 1) {
		tuning_reset_flag = 0;
		cmd_count--;
		if (cmd_count == 0) {
			printf("BUG_ON:controller reset is failed!!!\n");
			return -EINVAL;
		}
		goto tuning_retry;
	}
	err = mmc_send_tuning(mmc, opcode, NULL);
	mci_send_stop(mmc);
	mmc_send_status(mmc, &status);
	return err;
}

static unsigned int mci_get_sap_dll_taps(void)
{
	unsigned int regval;

	regval = mci_readl(0x12010150);

	return (regval & 0xff);
}
#define MCI_DLL_ELEMENT_OFFSET 8
static void mci_set_dll_element(unsigned int element)
{
	unsigned int regval;

	regval = mci_readl(MCI_REG_PERI_CRG83);
	regval &= ~(0xFF << MCI_DLL_ELEMENT_OFFSET);
	regval |= (element << MCI_DLL_ELEMENT_OFFSET);
	mci_writel(regval, MCI_REG_PERI_CRG83);
}

static void mci_tuning_feedback(const struct mmc *mmc)
{
	struct mci_host *host = mmc->priv;

	mci_control_cclk(host, DISABLE);
	mdelay(1);
	mci_sys_reset(host);
	mdelay(1);
	mci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);
	mci_control_cclk(host, ENABLE);
	mdelay(1);
}

/* ********************************************
 *********************************************
 EdgeMode A:
 |<---- totalphases(ele) ---->|
 _____________
 ______|||||||||||||||_______
 edge_p2f       edge_f2p
 (endp)         (startp)

 EdgeMode B:
 |<---- totalphases(ele) ---->|
 ________           _________
 ||||||||||_________|||||||||||
 edge_f2p     edge_p2f
 (startp)     (endp)

BestPhase:
if(endp < startp)
endp = endp + totalphases;
Best = ((startp + endp) / 2) % totalphases
 **********************************************
 ********************************************* */
static int mci_edgedll_mode_tuning(struct mmc *mmc, unsigned int opcode, int edge_p2f,
				      int edge_f2p)
{
	unsigned int found = 0;
	unsigned int index, startp, endp, startp_init, endp_init, phaseoffset, totalphases;
	unsigned short ele, start_ele, phase_dll_elements;
	unsigned char mdly_tap_flag;
	int prev_err = 0;
	int err;
	struct mci_host *host = mmc->priv;
	unsigned int phase_num = MCI_PHASE_SCALE;

	mdly_tap_flag = mci_get_sap_dll_taps();
	phase_dll_elements = mdly_tap_flag / MCI_PHASE_SCALE;
	totalphases = phase_dll_elements * phase_num;
	startp_init = edge_f2p * phase_dll_elements;
	endp_init = edge_p2f * phase_dll_elements;
	startp = startp_init;
	endp = endp_init;

	start_ele = 2;

	/*Note: edgedll tuning must from edge_p2f to edge_f2p*/
	if (edge_f2p >=  edge_p2f) {
		phaseoffset = edge_p2f * phase_dll_elements;
		for (index = edge_p2f; index < edge_f2p; index++) {
			/* set phase shift */
			mci_set_sap_phase(host, index);

			for (ele = start_ele; ele <= phase_dll_elements ; ele++) {
				mci_set_dll_element(ele);
				err = mci_send_tuning(mmc, opcode);
				if (!err)
					found = 1;

				if (!prev_err && err && (endp == endp_init))
					endp = phaseoffset + ele;

				if (err)
					startp = phaseoffset + ele;

#ifdef TUNING_PROC_DEBUG
				printf("\tphase:%01u ele:%02u st:%03u end:%03u error:%d\n", index, ele, startp, endp, err);
#endif
				prev_err = err;
				err = 0;
			}
			phaseoffset += phase_dll_elements;
		}
	} else {
		phaseoffset = edge_p2f * phase_dll_elements;
		for (index = edge_p2f; index < phase_num; index++) {
			/* set phase shift */
			mci_set_sap_phase(host, index);
			for (ele = start_ele; ele <= phase_dll_elements ; ele++) {
				mci_set_dll_element(ele);
				err = mci_send_tuning(mmc, opcode);
				if (!err)
					found = 1;

				if (!prev_err && err && (endp == endp_init))
					endp = phaseoffset + ele;

				if (err)
					startp = phaseoffset + ele;

#ifdef TUNING_PROC_DEBUG
				printf("\tphase:%02u ele:%02u st:%03u end:%03u error:%d\n", index, ele, startp, endp, err);
#endif

				prev_err = err;
				err = 0;
			}
			phaseoffset += phase_dll_elements;
		}

		phaseoffset = 0;
		for (index = 0; index < edge_f2p; index++) {
			/* set phase shift */
			mci_set_sap_phase(host, index);
			for (ele = start_ele; ele <= phase_dll_elements ; ele++) {
				mci_set_dll_element(ele);

				err = mci_send_tuning(mmc, opcode);
				if (!err)
					found = 1;

				if (!prev_err && err && (endp == endp_init))
					endp = phaseoffset + ele;

				if (err)
					startp = phaseoffset + ele;

#ifdef TUNING_PROC_DEBUG
				printf("\tphase:%02u ele:%02u st:%03u end:%03u error:%d\n", index, ele, startp, endp, err);
#endif

				prev_err = err;
				err = 0;
			}
			phaseoffset += phase_dll_elements;
		}
	}

	if (found) {
		printf("scan elemnts: startp:%u endp:%u\n", startp, endp);

		if (endp <= startp)
			endp += totalphases;

		phaseoffset = ((startp + endp) / 2) % totalphases;
		index = (phaseoffset / phase_dll_elements);
		ele = (phaseoffset % phase_dll_elements);
		ele = ((ele > start_ele) ? ele : start_ele);

		mci_set_sap_phase(host, index);
		mci_set_dll_element(ele);

		printf("Tuning SampleClock. mix set phase:[%02u/%02u] ele:[%0ud/%02u]\n",
		       index, (phase_num - 1), ele, phase_dll_elements);
		mci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);
		return 0;
	}
	printf("No valid phase shift! use default\n");
	return -1;
}

static int mci_execute_mix_mode_tuning(struct mmc *mmc, unsigned int opcode)
{
	struct mci_host *host = mmc->priv;
	unsigned int index, regval;
	unsigned int found;
	unsigned int prefound = 0;
	unsigned int edge_p2f, edge_f2p;
	unsigned int edge_num = 0;
	int err;

	mci_edge_tuning_enable(host);

	edge_p2f = 0;
	edge_f2p = MCI_PHASE_SCALE;

	for (index = 0; index < MCI_PHASE_SCALE; index++) {
		/* set phase shift */
		mci_set_sap_phase(host, index);
		err = mci_send_tuning(mmc, opcode);
		if (!err) {
			regval = mci_readl(host->base + MCI_TUNING_CTRL);
			found = ((regval & FOUND_EDGE) == FOUND_EDGE);
		} else {
			found = 1;
		}

		if (found)
			edge_num++;

		if (prefound && !found)
			edge_f2p = index;
		else if (!prefound && found)
			edge_p2f = index;

#ifdef TUNING_PROC_DEBUG
		printf("\tphase:%02u found:%02u p2f:%u f2p:%u error:%d\n",
		       index, found, edge_p2f, edge_f2p, err);
#endif

		if ((edge_p2f != 0) && (edge_f2p != MCI_PHASE_SCALE))
			break;

		prefound = found;
		found = 0;
	}

	if ((edge_p2f == 0) && (edge_f2p == MCI_PHASE_SCALE)) {
		printf("unfound correct edge! check your config is correct!!\n");
		return -1;
	}

	printf("scan edges:%u p2f:%u f2p:%u\n", edge_num, edge_p2f, edge_f2p);

	if (edge_f2p < edge_p2f)
		index = ((edge_f2p + edge_p2f) / 2) % MCI_PHASE_SCALE;
	else
		index = ((edge_f2p + MCI_PHASE_SCALE + edge_p2f) / 2) % MCI_PHASE_SCALE;
	printf("mix set temp-phase %u\n", index);
	mci_set_sap_phase(host, index);
	err = mci_send_tuning(mmc, opcode);

	mci_edge_tuning_disable(host);
	err = mci_edgedll_mode_tuning(mmc, opcode, edge_p2f, edge_f2p);

	return err;
}
static int mci_execute_tuning(struct mmc *mmc, unsigned int opcode)
{
	struct mci_host *host = mmc->priv;
	int err;

	host->is_tuning = 1;
	err = mci_execute_mix_mode_tuning(mmc, opcode);
	mci_tuning_feedback(mmc);
	if (!err)
		err = mci_send_tuning(mmc, opcode);

	host->is_tuning = 0;
	return err;
}

#define MCI_SHUTDOWN_DELAY 100

static void mci_shutdown(void)
{
	unsigned long base_addr;
	unsigned int value;

	base_addr = EMMC_REG_BASE;

	value = readl((uintptr_t)(base_addr + MCI_CTRL));
	value |= CTRL_RESET | FIFO_RESET | DMA_RESET;

	writel(value, (uintptr_t)(base_addr + MCI_CTRL));
	writel(POWER_OFF, (uintptr_t)(base_addr + MCI_PWREN));

	/* Delay 100ms, waiting for the eMMC device power off*/
	udelay(MCI_SHUTDOWN_DELAY * MCI_UM_IN_MM);
}

#define MCI_EXT_CSD_NUM 512

void print_ext_csd(struct mmc *mmc)
{
	unsigned char ext_csd[MCI_EXT_CSD_NUM];
	unsigned int tmp;

	mci_debug_fun("Function Call");

	int err = mmc_send_ext_csd(mmc, ext_csd);
	if (err) {
		mci_error("Check est_csd error!");
		return;
	}

	mci_debug_info("Extended CSD register:");
	for (tmp = 0; tmp < MCI_EXT_CSD_NUM; tmp += 8)
		mci_debug_info("%03d: %02x %02x %02x %02x %02x %02x %02x %02x",
		 tmp, ext_csd[tmp], ext_csd[tmp + 1], ext_csd[tmp + 2],
		 ext_csd[tmp + 3], ext_csd[tmp + 4], ext_csd[tmp + 5],
		 ext_csd[tmp + 6], ext_csd[tmp + 7]);
}

#define MCI_MID_OFFSET 24
#define MCI_VERSION_OFFSET 4

static void print_mmcinfo(const struct mmc *mmc)
{
	mci_debug_fun("Function Call");

	printf("MMC/SD Card:\n");
	printf("    MID:         0x%x\n", mmc->cid[0] >> MCI_MID_OFFSET);
	printf("    Read Block:  %d Bytes\n", mmc->read_bl_len);
	printf("    Write Block: %d Bytes\n", mmc->write_bl_len);
	printf("    Chip Size:   %s Bytes (%s)\n",
	       ultohstr(mmc->capacity),
	       mmc->high_capacity ? "High Capacity" : "Low Capacity");
	printf("    Name:        \"%c%c%c%c%c\"\n",
	       mmc->cid[0] & 0xff, (mmc->cid[1] >> MCI_MID_OFFSET),
	       (mmc->cid[1] >> 16) & 0xff, (mmc->cid[1] >> 8) & 0xff,
	       mmc->cid[1] & 0xff);

	printf("    Chip Type:   %s\n"
	       "    Version:     %d.%d\n",
	       IS_SD(mmc) ? "SD" : "MMC",
	       (mmc->version >> MCI_VERSION_OFFSET) & 0xf, mmc->version & 0xf);

	printf("    Speed:       %sHz\n", ultohstr(mmc->clock));
	printf("    Bus Width:   %dbit\n", mmc->bus_width);
}

int mci_probe(int dev_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int err;

	puts("\nEMMC/MMC/SD controller initialization.\n");

	if (!mmc) {
		printf("mmc device not found!!\n");
		return -1;
	}

	err = mmc_init(mmc);
	if (err) {
		printf("mmc_init failed! err:%d\n", err);
		return err;
	}

	print_mmcinfo(mmc);
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	if (!IS_SD(mmc))
		return mmc_set_boot_config(mmc);
#endif
	return 0;
}

static const struct mmc_ops mci_ops = {
	.send_cmd   = mci_request,
	.set_ios    = mci_set_ios,
	.init       = mci_init,
	.execute_tuning = mci_execute_tuning,
	.card_busy = mci_card_busy,
};

static void mci_setup_cfg(struct mmc_config *cfg, const struct mci_host *host,
			    unsigned int max_clk, unsigned int min_clk)
{
	cfg->name = host->name;
	cfg->f_min = min_clk;
	cfg->f_max = max_clk;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;

	cfg->host_caps |= MMC_MODE_4BIT;
	cfg->host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_HS200;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	cfg->ops = &mci_ops;
}

static int add_mci(struct mci_host *host, unsigned int max_clk, unsigned int min_clk)
{
	mci_setup_cfg(&host->cfg, host, max_clk, min_clk);

	host->mmc = mmc_create(&host->cfg, host);
	if (host->mmc == NULL)
		return -1;

	add_shutdown(mci_shutdown);
	return 0;
}

int mci_add_port(int index, unsigned int reg_base, unsigned int freq)
{
	struct mci_host *host = NULL;
	unsigned int regval;

	mci_debug_fun("Function Call");
	mci_sys_init(index, freq);

	/* check controller version. */
	regval = mci_readl(reg_base + MCI_VERID);
	if ((regval != MCI_VERID_VALUE) && (regval != MCI_VERID_VALUE2) &&
	    (regval != MCI_VERID_VALUE3)) {
		printf("MMC/SD/EMMC controller version incorrect.\n");
		return -ENODEV;
	}

	host = calloc(1, sizeof(struct mci_host));
	if (!host) {
		puts("host malloc fail!\n");
		return -ENOMEM;
	}

	host->name = "mci";
	host->dev_id = index;
	host->base = (unsigned long)reg_base;
	host->dma_des = dma_des;
	host->card_status = mci_sys_card_detect(host);
	host->port = 0;
	host->is_tuning = 0;
	return add_mci(host, freq, MMC_CCLK_MIN);
}
