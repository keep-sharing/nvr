/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/list.h>
#include <div64.h>
#include "mmc_private.h"

u8 saved_ext_csd[MMC_MAX_BLOCK_LEN];

__weak int board_mmc_getwp(struct mmc *mmc)
{
	return -1;
}

int mmc_getwp(struct mmc *mmc)
{
	int wp;

	wp = board_mmc_getwp(mmc);

	if (wp < 0) {
		if (mmc->cfg->ops->getwp)
			wp = mmc->cfg->ops->getwp(mmc);
		else
			wp = 0;
	}

	return wp;
}

__weak int board_mmc_getcd(struct mmc *mmc)
{
	return -1;
}

#if defined(CONFIG_NVT_MMC_CMD_LOG)

void mmc_cmd_log(struct mmc *mmc, struct mmc_cmd *cmd)
{
    static u8 log_idx;

   log_idx %=20;

   mmc->log_cmd_dump[log_idx].cmdidx = cmd->cmdidx;
   mmc->log_cmd_dump[log_idx].resp_type = cmd->resp_type;
   mmc->log_cmd_dump[log_idx].cmdarg = cmd->cmdarg;
   memcpy(&(mmc->log_cmd_dump[log_idx].response), &(cmd->response),sizeof(uint)*4);
   mmc->last_log_idx = log_idx;
   log_idx++;

}

void mmc_cmd_log_dump(struct mmc *mmc)
{
	int i,j;
	u8 *ptr;

	printf("\n/**********eMMC CMD log dump(last cmd idx = %d)**********/\n",mmc->last_log_idx);

	for(j = 0; j< 20; j++)
	{
	printf("CMD_SEND:%d\n", mmc->log_cmd_dump[j].cmdidx);
	printf("\t\tARG\t\t\t 0x%08X\n", mmc->log_cmd_dump[j].cmdarg);
	switch (mmc->log_cmd_dump[j].resp_type) {
		case MMC_RSP_NONE:
			printf("\t\tMMC_RSP_NONE\n");
			break;
		case MMC_RSP_R1:
			printf("\t\tMMC_RSP_R1,5,6,7 \t 0x%08X \n",
				mmc->log_cmd_dump[j].response[0]);
			break;
		case MMC_RSP_R1b:
			printf("\t\tMMC_RSP_R1b\t\t 0x%08X \n",
				mmc->log_cmd_dump[j].response[0]);
			break;
		case MMC_RSP_R2:
			printf("\t\tMMC_RSP_R2\t\t 0x%08X \n",
				mmc->log_cmd_dump[j].response[0]);
			printf("\t\t          \t\t 0x%08X \n",
				mmc->log_cmd_dump[j].response[1]);
			printf("\t\t          \t\t 0x%08X \n",
				mmc->log_cmd_dump[j].response[2]);
			printf("\t\t          \t\t 0x%08X \n",
				mmc->log_cmd_dump[j].response[3]);
			printf("\n");
			printf("\t\t\t\t\tDUMPING DATA\n");
			for (i = 0; i < 4; i++) {
				int j;
				printf("\t\t\t\t\t%03d - ", i*4);
				ptr = (u8 *)&mmc->log_cmd_dump[j].response[i];
				ptr += 3;
				for (j = 0; j < 4; j++)
					printf("%02X ", *ptr--);
				printf("\n");
			}
			break;
		case MMC_RSP_R3:
			printf("\t\tMMC_RSP_R3,4\t\t 0x%08X \n",
				mmc->log_cmd_dump[j].response[0]);
			break;
		default:
			printf("\t\tERROR MMC rsp not supported\n");
			break;
	}
	}
	printf("\n/****************eMMC CMD log dump**********/\n");
}
#endif
int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret;

#ifdef CONFIG_MMC_TRACE
	int i;
	u8 *ptr;

	printf("CMD_SEND:%d\n", cmd->cmdidx);
	printf("\t\tARG\t\t\t 0x%08X\n", cmd->cmdarg);
	ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
	if (ret) {
		printf("\t\tRET\t\t\t %d\n", ret);
	} else {
		switch (cmd->resp_type) {
		case MMC_RSP_NONE:
			printf("\t\tMMC_RSP_NONE\n");
			break;
		case MMC_RSP_R1:
			printf("\t\tMMC_RSP_R1,5,6,7 \t 0x%08X \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R1b:
			printf("\t\tMMC_RSP_R1b\t\t 0x%08X \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R2:
			printf("\t\tMMC_RSP_R2\t\t 0x%08X \n",
				cmd->response[0]);
			printf("\t\t          \t\t 0x%08X \n",
				cmd->response[1]);
			printf("\t\t          \t\t 0x%08X \n",
				cmd->response[2]);
			printf("\t\t          \t\t 0x%08X \n",
				cmd->response[3]);
			printf("\n");
			printf("\t\t\t\t\tDUMPING DATA\n");
			for (i = 0; i < 4; i++) {
				int j;
				printf("\t\t\t\t\t%03d - ", i*4);
				ptr = (u8 *)&cmd->response[i];
				ptr += 3;
				for (j = 0; j < 4; j++)
					printf("%02X ", *ptr--);
				printf("\n");
			}
			break;
		case MMC_RSP_R3:
			printf("\t\tMMC_RSP_R3,4\t\t 0x%08X \n",
				cmd->response[0]);
			break;
		default:
			printf("\t\tERROR MMC rsp not supported\n");
			break;
		}
	}
#else
	ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
#endif
#if defined(CONFIG_NVT_MMC_CMD_LOG)
	mmc_cmd_log(mmc, cmd);
	if (ret) {
		printf("mmc cmd failed\n");
		mmc_cmd_log_dump (mmc);
		return ret;
	}
#endif
	return ret;
}

int mmc_send_status(struct mmc *mmc, int timeout)
{
	struct mmc_cmd cmd;
	int err, retries = 5;
#ifdef CONFIG_MMC_TRACE
	int status;
#endif

	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	if (!mmc_host_is_spi(mmc))
		cmd.cmdarg = mmc->rca << 16;

	while (1) {
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (!err) {
			if ((cmd.response[0] & MMC_STATUS_RDY_FOR_DATA) &&
			    (cmd.response[0] & MMC_STATUS_CURR_STATE) !=
			     MMC_STATE_PRG)
				break;
			else if (cmd.response[0] & MMC_STATUS_MASK) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
				printf("Status Error: 0x%08X\n",
					cmd.response[0]);
#endif
				return COMM_ERR;
			}
		} else if (--retries < 0)
			return err;

		if (timeout-- <= 0)
			break;
#if 1/*wait timer tick todo*/
		udelay(1000);
#endif
	}

#ifdef CONFIG_MMC_TRACE
	status = (cmd.response[0] & MMC_STATUS_CURR_STATE) >> 9;
	printf("CURR STATE:%d\n", status);
#endif
	if (timeout <= 0) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("Timeout waiting card ready\n");
#endif
		return TIMEOUT;
	}

	return 0;
}
#if 1
int mmc_wait_device_in_trans(struct mmc *mmc, int timeout)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	if (!mmc_host_is_spi(mmc))
		cmd.cmdarg = mmc->rca << 16;

	do {
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err) {
			printf("cmd13 failed  err=%d ",err);
			return err;
		}

		if((cmd.response[0] & MMC_STATUS_CURR_STATE) == MMC_STATUS_TRANS_ST)
			break;

	} while (timeout--);

	return 0;
}
#endif

int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;

	if (mmc->ddr_mode)
		return 0;

	cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = len;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

static int mmc_read_blocks(struct mmc *mmc, void *dst, lbaint_t start,
			   lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->read_bl_len;

	cmd.resp_type = MMC_RSP_R1;

	data.dest = dst;
	data.blocks = blkcnt;
	data.blocksize = mmc->read_bl_len;
	data.flags = MMC_DATA_READ;

	if (mmc_send_cmd(mmc, &cmd, &data))
		return 0;

	if (blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
			printf("mmc fail to send stop cmd\n");
#endif
			return 0;
		}
	}

	return blkcnt;
}

#ifdef CONFIG_BLK
static ulong mmc_bread(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		       void *dst)
#else
static ulong mmc_bread(struct blk_desc *block_dev, lbaint_t start,
		       lbaint_t blkcnt, void *dst)
#endif
{
#ifdef CONFIG_BLK
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
#endif
	int dev_num = block_dev->devnum;
	int err;
	lbaint_t cur, blocks_todo = blkcnt;

	if (blkcnt == 0)
		return 0;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
		return 0;

	err = blk_dselect_hwpart(block_dev, block_dev->hwpart);
	if (err < 0)
		return 0;

	if ((start + blkcnt) > block_dev->lba) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
			start + blkcnt, block_dev->lba);
#endif
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->read_bl_len)) {
		debug("%s: Failed to set blocklen\n", __func__);
		return 0;
	}

	do {
		cur = (blocks_todo > mmc->cfg->b_max) ?
			mmc->cfg->b_max : blocks_todo;
		if (mmc_read_blocks(mmc, dst, start, cur) != cur) {
			debug("%s: Failed to read blocks\n", __func__);
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		dst += cur * mmc->read_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

static int mmc_go_idle(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;
#if 1/*wait timer tick todo*/
	udelay(1000);
#endif
	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;
#if 1/*wait timer tick todo*/
	udelay(2000);
#endif
	return 0;
}

#ifndef CONFIG_NVT_EMMC_ONLY_HOST
static int sd_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	while (1) {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;

		/*
		 * Most cards do not answer if some reserved bits
		 * in the ocr are set. However, Some controller
		 * can set bit 7 (reserved for low voltages), but
		 * how to manage low voltages SD card is not yet
		 * specified.
		 */
		cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 :
			(mmc->cfg->voltages & 0xff8000);

		if (mmc->version == SD_VERSION_2)
			cmd.cmdarg |= OCR_HCS;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		if (cmd.response[0] & OCR_BUSY)
			break;

		if (timeout-- <= 0)
			return UNUSABLE_ERR;
#if 1/*wait timer tick todo*/
		udelay(1000);
#endif
	}

	if (mmc->version != SD_VERSION_2)
		mmc->version = SD_VERSION_1_0;

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}

	mmc->ocr = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 0;

	return 0;
}
#endif
static int mmc_send_op_cond_iter(struct mmc *mmc, int use_arg)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = MMC_CMD_SEND_OP_COND;
	cmd.resp_type = MMC_RSP_R3;
	cmd.cmdarg = 0;
	if (use_arg && !mmc_host_is_spi(mmc))
		cmd.cmdarg = OCR_HCS |
			(mmc->cfg->voltages &
			(mmc->ocr & OCR_VOLTAGE_MASK)) |
			(mmc->ocr & OCR_ACCESS_MODE);

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		return err;
	mmc->ocr = cmd.response[0];
	return 0;
}

static int mmc_send_op_cond(struct mmc *mmc)
{
	int err, i;

	/* Some cards seem to need this */
	mmc_go_idle(mmc);

 	/* Asking to the card its capabilities */
	for (i = 0; i < 2; i++) {
		err = mmc_send_op_cond_iter(mmc, i != 0);
		if (err)
			return err;

		/* exit if not busy (flag seems to be inverted) */
		if (mmc->ocr & OCR_BUSY)
			break;
	}
	mmc->op_cond_pending = 1;
	return 0;
}

static int mmc_complete_op_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	uint start;
	int err;

	mmc->op_cond_pending = 0;
	if (!(mmc->ocr & OCR_BUSY)) {
		start = get_timer(0);
		while (1) {
			err = mmc_send_op_cond_iter(mmc, 1);
			if (err)
				return err;
			if (mmc->ocr & OCR_BUSY)
				break;
			if (get_timer(start) > timeout)
				return UNUSABLE_ERR;
#if 1/*wait timer tick todo*/
			udelay(100);
#endif
		}
	}

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		mmc->ocr = cmd.response[0];
	}

	mmc->version = MMC_VERSION_UNKNOWN;

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 1;

	return 0;
}


static int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	/* Get the Card Status Register */
	cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	data.dest = (char *)ext_csd;
	data.blocks = 1;
	data.blocksize = MMC_MAX_BLOCK_LEN;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	return err;
}


static int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	int ret;

	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				 (index << 16) |
				 (value << 8);

	ret = mmc_send_cmd(mmc, &cmd, NULL);

	/* Waiting for the ready status */
	if (!ret)
		ret = mmc_send_status(mmc, timeout);

	return ret;

}

static int mmc_change_freq(struct mmc *mmc)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
	char cardtype;
	int err;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Only version 4 supports high-speed */
	if (mmc->version < MMC_VERSION_4)
		return 0;

	mmc->card_caps |= MMC_MODE_4BIT | MMC_MODE_8BIT;

	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err)
		return err;

	cardtype = ext_csd[EXT_CSD_CARD_TYPE] & 0xf;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

	if (err)
		return err;

	/* Now check to see that it worked */
	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err)
		return err;

	/* No high-speed support */
	if (!ext_csd[EXT_CSD_HS_TIMING])
		return 0;

	/* High Speed is set, there are two types: 52MHz and 26MHz */
	if (cardtype & EXT_CSD_CARD_TYPE_52) {
		if (cardtype & EXT_CSD_CARD_TYPE_DDR_1_8V)
			mmc->card_caps |= MMC_MODE_DDR_52MHz;
		mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	} else {
		mmc->card_caps |= MMC_MODE_HS;
	}

	return 0;
}

static int mmc_set_capacity(struct mmc *mmc, int part_num)
{
	switch (part_num) {
	case 0:
		mmc->capacity = mmc->capacity_user;
		break;
	case 1:
	case 2:
		mmc->capacity = mmc->capacity_boot;
		break;
	case 3:
		mmc->capacity = mmc->capacity_rpmb;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		mmc->capacity = mmc->capacity_gp[part_num - 4];
		break;
	default:
		return -1;
	}

	mmc_get_blk_desc(mmc)->lba = lldiv(mmc->capacity, mmc->read_bl_len);

	return 0;
}

static int mmc_switch_part(struct mmc *mmc, unsigned int part_num)
{
	int ret;

	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			 (mmc->part_config & ~PART_ACCESS_MASK)
			 | (part_num & PART_ACCESS_MASK));

	/*
	 * Set the capacity if the switch succeeded or was intended
	 * to return to representing the raw device.
	 */
	if ((ret == 0) || ((ret == -ENODEV) && (part_num == 0))) {
		ret = mmc_set_capacity(mmc, part_num);
		mmc_get_blk_desc(mmc)->hwpart = part_num;
	}

	return ret;
}

#ifdef CONFIG_BLK
static int mmc_select_hwpart(struct udevice *bdev, int hwpart)
{
	struct udevice *mmc_dev = dev_get_parent(bdev);
	struct mmc *mmc = mmc_get_mmc_dev(mmc_dev);
	struct blk_desc *desc = dev_get_uclass_platdata(bdev);
	int ret;

	if (desc->hwpart == hwpart)
		return 0;

	if (mmc->part_config == MMCPART_NOAVAILABLE)
		return -EMEDIUMTYPE;

	ret = mmc_switch_part(mmc, hwpart);
	if (ret)
		return ret;

	return 0;
}
#else
static int mmc_select_hwpartp(struct blk_desc *desc, int hwpart)
{
	struct mmc *mmc = find_mmc_device(desc->devnum);
	int ret;

	if (!mmc)
		return -ENODEV;

	if (mmc->block_dev.hwpart == hwpart)
		return 0;

	if (mmc->part_config == MMCPART_NOAVAILABLE)
		return -EMEDIUMTYPE;

	ret = mmc_switch_part(mmc, hwpart);
	if (ret)
		return ret;

	return 0;
}
#endif

int mmc_hwpart_config(struct mmc *mmc,
		      const struct mmc_hwpart_conf *conf,
		      enum mmc_hwpart_conf_mode mode)
{
	u8 part_attrs = 0;
	u32 enh_size_mult;
	u32 enh_start_addr;
	u32 gp_size_mult[4];
	u32 max_enh_size_mult;
	u32 tot_enh_size_mult = 0;
	u8 wr_rel_set;
	int i, pidx, err;
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);

	if (mode < MMC_HWPART_CONF_CHECK || mode > MMC_HWPART_CONF_COMPLETE)
		return -EINVAL;

	if (IS_SD(mmc) || (mmc->version < MMC_VERSION_4_41)) {
		printf("eMMC >= 4.4 required for enhanced user data area\n");
		return -EMEDIUMTYPE;
	}

	if (!(mmc->part_support & PART_SUPPORT)) {
		printf("Card does not support partitioning\n");
		return -EMEDIUMTYPE;
	}

	if (!mmc->hc_wp_grp_size) {
		printf("Card does not define HC WP group size\n");
		return -EMEDIUMTYPE;
	}

	/* check partition alignment and total enhanced size */
	if (conf->user.enh_size) {
		if (conf->user.enh_size % mmc->hc_wp_grp_size ||
		    conf->user.enh_start % mmc->hc_wp_grp_size) {
			printf("User data enhanced area not HC WP group "
			       "size aligned\n");
			return -EINVAL;
		}
		part_attrs |= EXT_CSD_ENH_USR;
		enh_size_mult = conf->user.enh_size / mmc->hc_wp_grp_size;
		if (mmc->high_capacity) {
			enh_start_addr = conf->user.enh_start;
		} else {
			enh_start_addr = (conf->user.enh_start << 9);
		}
	} else {
		enh_size_mult = 0;
		enh_start_addr = 0;
	}
	tot_enh_size_mult += enh_size_mult;

	for (pidx = 0; pidx < 4; pidx++) {
		if (conf->gp_part[pidx].size % mmc->hc_wp_grp_size) {
			printf("GP%i partition not HC WP group size "
			       "aligned\n", pidx+1);
			return -EINVAL;
		}
		gp_size_mult[pidx] = conf->gp_part[pidx].size / mmc->hc_wp_grp_size;
		if (conf->gp_part[pidx].size && conf->gp_part[pidx].enhanced) {
			part_attrs |= EXT_CSD_ENH_GP(pidx);
			tot_enh_size_mult += gp_size_mult[pidx];
		}
	}

	if (part_attrs && ! (mmc->part_support & ENHNCD_SUPPORT)) {
		printf("Card does not support enhanced attribute\n");
		return -EMEDIUMTYPE;
	}

	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err)
		return err;

	max_enh_size_mult =
		(ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT+2] << 16) +
		(ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT+1] << 8) +
		ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT];
	if (tot_enh_size_mult > max_enh_size_mult) {
		printf("Total enhanced size exceeds maximum (%u > %u)\n",
		       tot_enh_size_mult, max_enh_size_mult);
		return -EMEDIUMTYPE;
	}

	/* The default value of EXT_CSD_WR_REL_SET is device
	 * dependent, the values can only be changed if the
	 * EXT_CSD_HS_CTRL_REL bit is set. The values can be
	 * changed only once and before partitioning is completed. */
	wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];
	if (conf->user.wr_rel_change) {
		if (conf->user.wr_rel_set)
			wr_rel_set |= EXT_CSD_WR_DATA_REL_USR;
		else
			wr_rel_set &= ~EXT_CSD_WR_DATA_REL_USR;
	}
	for (pidx = 0; pidx < 4; pidx++) {
		if (conf->gp_part[pidx].wr_rel_change) {
			if (conf->gp_part[pidx].wr_rel_set)
				wr_rel_set |= EXT_CSD_WR_DATA_REL_GP(pidx);
			else
				wr_rel_set &= ~EXT_CSD_WR_DATA_REL_GP(pidx);
		}
	}

	if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET] &&
	    !(ext_csd[EXT_CSD_WR_REL_PARAM] & EXT_CSD_HS_CTRL_REL)) {
		puts("Card does not support host controlled partition write "
		     "reliability settings\n");
		return -EMEDIUMTYPE;
	}

	if (ext_csd[EXT_CSD_PARTITION_SETTING] &
	    EXT_CSD_PARTITION_SETTING_COMPLETED) {
		printf("Card already partitioned\n");
		return -EPERM;
	}

	if (mode == MMC_HWPART_CONF_CHECK)
		return 0;

	/* Partitioning requires high-capacity size definitions */
	if (!(ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01)) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ERASE_GROUP_DEF, 1);

		if (err)
			return err;

		ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;

		/* update erase group size to be high-capacity */
		mmc->erase_grp_size =
			ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;

	}

	/* all OK, write the configuration */
	for (i = 0; i < 4; i++) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ENH_START_ADDR+i,
				 (enh_start_addr >> (i*8)) & 0xFF);
		if (err)
			return err;
	}
	for (i = 0; i < 3; i++) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ENH_SIZE_MULT+i,
				 (enh_size_mult >> (i*8)) & 0xFF);
		if (err)
			return err;
	}
	for (pidx = 0; pidx < 4; pidx++) {
		for (i = 0; i < 3; i++) {
			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
					 EXT_CSD_GP_SIZE_MULT+pidx*3+i,
					 (gp_size_mult[pidx] >> (i*8)) & 0xFF);
			if (err)
				return err;
		}
	}
	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PARTITIONS_ATTRIBUTE, part_attrs);
	if (err)
		return err;

	if (mode == MMC_HWPART_CONF_SET)
		return 0;

	/* The WR_REL_SET is a write-once register but shall be
	 * written before setting PART_SETTING_COMPLETED. As it is
	 * write-once we can only write it when completing the
	 * partitioning. */
	if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET]) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_WR_REL_SET, wr_rel_set);
		if (err)
			return err;
	}

	/* Setting PART_SETTING_COMPLETED confirms the partition
	 * configuration but it only becomes effective after power
	 * cycle, so we do not adjust the partition related settings
	 * in the mmc struct. */

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PARTITION_SETTING,
			 EXT_CSD_PARTITION_SETTING_COMPLETED);
	if (err)
		return err;

	return 0;
}

int mmc_getcd(struct mmc *mmc)
{
	int cd;

	cd = board_mmc_getcd(mmc);

	if (cd < 0) {
		if (mmc->cfg->ops->getcd)
			cd = mmc->cfg->ops->getcd(mmc);
		else
			cd = 1;
	}

	return cd;
}

static int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	/* Switch the frequency */
	cmd.cmdidx = SD_CMD_SWITCH_FUNC;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = (mode << 31) | 0xffffff;
	cmd.cmdarg &= ~(0xf << (group * 4));
	cmd.cmdarg |= value << (group * 4);

	data.dest = (char *)resp;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	return mmc_send_cmd(mmc, &cmd, &data);
}


static int sd_change_freq(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	ALLOC_CACHE_ALIGN_BUFFER(uint, scr, 2);
	ALLOC_CACHE_ALIGN_BUFFER(uint, switch_status, 16);
	struct mmc_data data;
	int timeout;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;
	/* Read the SCR to find out if this card supports higher speeds */
	cmd.cmdidx = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	cmd.cmdidx = SD_CMD_APP_SEND_SCR;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	timeout = 3;

retry_scr:
	data.dest = (char *)scr;
	data.blocksize = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	if (err) {
		if (timeout--)
			goto retry_scr;

		return err;
	}

	mmc->scr[0] = __be32_to_cpu(scr[0]);
	mmc->scr[1] = __be32_to_cpu(scr[1]);

	switch ((mmc->scr[0] >> 24) & 0xf) {
	case 0:
		mmc->version = SD_VERSION_1_0;
		break;
	case 1:
		mmc->version = SD_VERSION_1_10;
		break;
	case 2:
		mmc->version = SD_VERSION_2;
		if ((mmc->scr[0] >> 15) & 0x1)
			mmc->version = SD_VERSION_3;
		break;
	default:
		mmc->version = SD_VERSION_1_0;
		break;
	}

	if (mmc->scr[0] & SD_DATA_4BIT)
		mmc->card_caps |= MMC_MODE_4BIT;

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0)
		return 0;

	timeout = 4;
	while (timeout--) {
		err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
				(u8 *)switch_status);

		if (err)
			return err;

		/* The high-speed function is busy.  Try again */
		if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
			break;
	}

	/* If high-speed isn't supported, we return */
	if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
		return 0;

	/*
	 * If the host doesn't support SD_HIGHSPEED, do not switch card to
	 * HIGHSPEED mode even if the card support SD_HIGHSPPED.
	 * This can avoid furthur problem when the card runs in different
	 * mode between the host.
	 */
	if (!((mmc->cfg->host_caps & MMC_MODE_HS_52MHz) &&
		(mmc->cfg->host_caps & MMC_MODE_HS)))
		return 0;

	err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *)switch_status);

	if (err)
		return err;

	if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000)
		mmc->card_caps |= MMC_MODE_HS;

	return 0;
}

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
	10000,
	100000,
	1000000,
	10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const u8 multipliers[] = {
	0,	/* reserved */
	10,
	12,
	13,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
	60,
	70,
	80,
};

static void mmc_set_ios(struct mmc *mmc)
{
	if (mmc->cfg->ops->set_ios)
		mmc->cfg->ops->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, uint clock)
{
	if (clock > mmc->cfg->f_max)
		clock = mmc->cfg->f_max;

	if (clock < mmc->cfg->f_min)
		clock = mmc->cfg->f_min;

	mmc->clock = clock;

	mmc_set_ios(mmc);
}

static void mmc_set_bus_width(struct mmc *mmc, uint width)
{
	mmc->bus_width = width;

	mmc_set_ios(mmc);
}

static int mmc_startup(struct mmc *mmc)
{
	int err, i;
	uint mult, freq;
	u64 cmult, csize, capacity;
	struct mmc_cmd cmd;
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
	ALLOC_CACHE_ALIGN_BUFFER(u8, test_csd, MMC_MAX_BLOCK_LEN);
	int timeout = 1000;
	bool has_parts = false;
	bool part_completed;
	struct blk_desc *bdesc;

#ifdef CONFIG_MMC_SPI_CRC_ON
	if (mmc_host_is_spi(mmc)) { /* enable CRC check for spi */
		cmd.cmdidx = MMC_CMD_SPI_CRC_ON_OFF;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 1;
		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}
#endif

	/* Put the Card in Identify Mode */
	cmd.cmdidx = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID :
		MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	memcpy(mmc->cid, cmd.response, 16);

	/*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 * This also puts the cards into Standby State
	 */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
		cmd.cmdarg = mmc->rca << 16;
		cmd.resp_type = MMC_RSP_R6;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		if (IS_SD(mmc))
			mmc->rca = (cmd.response[0] >> 16) & 0xffff;
	}

	/* Get the Card-Specific Data */
	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	/* Waiting for the ready status */
	mmc_send_status(mmc, timeout);

	if (err)
		return err;

	mmc->csd[0] = cmd.response[0];
	mmc->csd[1] = cmd.response[1];
	mmc->csd[2] = cmd.response[2];
	mmc->csd[3] = cmd.response[3];

	if (mmc->version == MMC_VERSION_UNKNOWN) {
		int version = (cmd.response[0] >> 26) & 0xf;

		switch (version) {
		case 0:
			mmc->version = MMC_VERSION_1_2;
			break;
		case 1:
			mmc->version = MMC_VERSION_1_4;
			break;
		case 2:
			mmc->version = MMC_VERSION_2_2;
			break;
		case 3:
			mmc->version = MMC_VERSION_3;
			break;
		case 4:
			mmc->version = MMC_VERSION_4;
			break;
		default:
			mmc->version = MMC_VERSION_1_2;
			break;
		}
	}

	/* divide frequency by 10, since the mults are 10x bigger */
	freq = fbase[(cmd.response[0] & 0x7)];
	mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

	mmc->tran_speed = freq * mult;

	mmc->dsr_imp = ((cmd.response[1] >> 12) & 0x1);
	mmc->read_bl_len = 1 << ((cmd.response[1] >> 16) & 0xf);

	if (IS_SD(mmc))
		mmc->write_bl_len = mmc->read_bl_len;
	else
		mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);

	if (mmc->high_capacity) {
		csize = (mmc->csd[1] & 0x3f) << 16
			| (mmc->csd[2] & 0xffff0000) >> 16;
		cmult = 8;
	} else {
		csize = (mmc->csd[1] & 0x3ff) << 2
			| (mmc->csd[2] & 0xc0000000) >> 30;
		cmult = (mmc->csd[2] & 0x00038000) >> 15;
	}

	mmc->capacity_user = (csize + 1) << (cmult + 2);
	mmc->capacity_user *= mmc->read_bl_len;
	mmc->capacity_boot = 0;
	mmc->capacity_rpmb = 0;
	for (i = 0; i < 4; i++)
		mmc->capacity_gp[i] = 0;

	if (mmc->read_bl_len > MMC_MAX_BLOCK_LEN)
		mmc->read_bl_len = MMC_MAX_BLOCK_LEN;

	if (mmc->write_bl_len > MMC_MAX_BLOCK_LEN)
		mmc->write_bl_len = MMC_MAX_BLOCK_LEN;

	if ((mmc->dsr_imp) && (0xffffffff != mmc->dsr)) {
		cmd.cmdidx = MMC_CMD_SET_DSR;
		cmd.cmdarg = (mmc->dsr & 0xffff) << 16;
		cmd.resp_type = MMC_RSP_NONE;
		if (mmc_send_cmd(mmc, &cmd, NULL))
			printf("MMC: SET_DSR failed\n");
	}

	/* Select the card, and put it into Transfer Mode */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = MMC_CMD_SELECT_CARD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = mmc->rca << 16;
		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}

	/*
	 * For SD, its erase group is always one sector
	 */
	mmc->erase_grp_size = 1;
	mmc->part_config = MMCPART_NOAVAILABLE;
	if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4)) {
		/* check  ext_csd version and capacity */
		err = mmc_send_ext_csd(mmc, ext_csd);
		if (err)
			return err;
		if (ext_csd[EXT_CSD_REV] >= 2) {
			/*
			 * According to the JEDEC Standard, the value of
			 * ext_csd's capacity is valid if the value is more
			 * than 2GB
			 */
			capacity = ext_csd[EXT_CSD_SEC_CNT] << 0
					| ext_csd[EXT_CSD_SEC_CNT + 1] << 8
					| ext_csd[EXT_CSD_SEC_CNT + 2] << 16
					| ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
			capacity *= MMC_MAX_BLOCK_LEN;
			if ((capacity >> 20) > 2 * 1024)
				mmc->capacity_user = capacity;
		}

		switch (ext_csd[EXT_CSD_REV]) {
		case 1:
			mmc->version = MMC_VERSION_4_1;
			break;
		case 2:
			mmc->version = MMC_VERSION_4_2;
			break;
		case 3:
			mmc->version = MMC_VERSION_4_3;
			break;
		case 5:
			mmc->version = MMC_VERSION_4_41;
			break;
		case 6:
			mmc->version = MMC_VERSION_4_5;
			break;
		case 7:
			mmc->version = MMC_VERSION_5_0;
			break;
		case 8:
			mmc->version = MMC_VERSION_5_1;
			break;
		}

		/* The partition data may be non-zero but it is only
		 * effective if PARTITION_SETTING_COMPLETED is set in
		 * EXT_CSD, so ignore any data if this bit is not set,
		 * except for enabling the high-capacity group size
		 * definition (see below). */
		part_completed = !!(ext_csd[EXT_CSD_PARTITION_SETTING] &
				    EXT_CSD_PARTITION_SETTING_COMPLETED);

		/* store the partition info of emmc */
		mmc->part_support = ext_csd[EXT_CSD_PARTITIONING_SUPPORT];
		if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
		    ext_csd[EXT_CSD_BOOT_MULT])
			mmc->part_config = ext_csd[EXT_CSD_PART_CONF];
		if (part_completed &&
		    (ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & ENHNCD_SUPPORT))
			mmc->part_attr = ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE];

		mmc->capacity_boot = ext_csd[EXT_CSD_BOOT_MULT] << 17;

		mmc->capacity_rpmb = ext_csd[EXT_CSD_RPMB_MULT] << 17;

		for (i = 0; i < 4; i++) {
			int idx = EXT_CSD_GP_SIZE_MULT + i * 3;
			uint mult = (ext_csd[idx + 2] << 16) +
				(ext_csd[idx + 1] << 8) + ext_csd[idx];
			if (mult)
				has_parts = true;
			if (!part_completed)
				continue;
			mmc->capacity_gp[i] = mult;
			mmc->capacity_gp[i] *=
				ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
			mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
			mmc->capacity_gp[i] <<= 19;
		}

		if (part_completed) {
			mmc->enh_user_size =
				(ext_csd[EXT_CSD_ENH_SIZE_MULT+2] << 16) +
				(ext_csd[EXT_CSD_ENH_SIZE_MULT+1] << 8) +
				ext_csd[EXT_CSD_ENH_SIZE_MULT];
			mmc->enh_user_size *= ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
			mmc->enh_user_size *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
			mmc->enh_user_size <<= 19;
			mmc->enh_user_start =
				(ext_csd[EXT_CSD_ENH_START_ADDR+3] << 24) +
				(ext_csd[EXT_CSD_ENH_START_ADDR+2] << 16) +
				(ext_csd[EXT_CSD_ENH_START_ADDR+1] << 8) +
				ext_csd[EXT_CSD_ENH_START_ADDR];
			if (mmc->high_capacity)
				mmc->enh_user_start <<= 9;
		}

		/*
		 * Host needs to enable ERASE_GRP_DEF bit if device is
		 * partitioned. This bit will be lost every time after a reset
		 * or power off. This will affect erase size.
		 */
#if 0
		printf("ext_csd[EXT_CSD_PARTITIONING_SUPPORT] = %d\n", ext_csd[EXT_CSD_PARTITIONING_SUPPORT]);
		printf("ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE] = %d\n", ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE]);
		printf("ext_csd[PARTITION_SETTING_COMPLETED ] = %d\n", ext_csd[155]);
		printf("ext_csd[ PRE_EOL_INFO] = %d\n", ext_csd[267]);
		printf("ext_csd[DEVICE_LIFE_TIME_EST_TYP_A] = %d\n", ext_csd[268]);
		printf("ext_csd[DEVICE_LIFE_TIME_EST_TYP_B] = %d\n", ext_csd[269]);
#endif
		if (part_completed)
			has_parts = true;
#if defined(CONFIG_NVT_ERASE_TOSHIBA_TEST_PATCH)
		if (((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) &&
		    (ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE] & PART_ENH_ATTRIB)) || mmc->en_egroup) 
#else
		if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) &&
		    (ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE] & PART_ENH_ATTRIB))
#endif
			has_parts = true;
		if (has_parts) {
			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_ERASE_GROUP_DEF, 1);

			if (err)
				return err;
			else
				ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;
		}

		if (ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01) {
			/* Read out group size from ext_csd */
			mmc->erase_grp_size =
				ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;
			/*
			 * if high capacity and partition setting completed
			 * SEC_COUNT is valid even if it is smaller than 2 GiB
			 * JEDEC Standard JESD84-B45, 6.2.4
			 */
			if (mmc->high_capacity && part_completed) {
				capacity = (ext_csd[EXT_CSD_SEC_CNT]) |
					(ext_csd[EXT_CSD_SEC_CNT + 1] << 8) |
					(ext_csd[EXT_CSD_SEC_CNT + 2] << 16) |
					(ext_csd[EXT_CSD_SEC_CNT + 3] << 24);
				capacity *= MMC_MAX_BLOCK_LEN;
				mmc->capacity_user = capacity;
			}
		} else {
#if defined(CONFIG_NVT_ERASE_TOSHIBA_TEST_PATCH)
		    if (!mmc->en_egroup) {

			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_ERASE_GROUP_DEF, 0);

			if (err)
			    return err;
		    }
#endif
			/* Calculate the group size from the csd value. */
			int erase_gsz, erase_gmul;
			erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
			erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
			mmc->erase_grp_size = (erase_gsz + 1)
				* (erase_gmul + 1);
		}

		mmc->hc_wp_grp_size = 1024
			* ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
			* ext_csd[EXT_CSD_HC_WP_GRP_SIZE];

		mmc->wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];
	}

	err = mmc_set_capacity(mmc, mmc_get_blk_desc(mmc)->hwpart);
	if (err)
		return err;

	if (IS_SD(mmc))
		err = sd_change_freq(mmc);
	else
		err = mmc_change_freq(mmc);

	if (err)
		return err;

	/* Restrict card's capabilities by what the host can do */
	mmc->card_caps &= mmc->cfg->host_caps;

	if (IS_SD(mmc)) {
		if (mmc->card_caps & MMC_MODE_4BIT) {
			cmd.cmdidx = MMC_CMD_APP_CMD;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = mmc->rca << 16;

			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (err)
				return err;

			cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = 2;
			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (err)
				return err;

			mmc_set_bus_width(mmc, 4);
		}

		if (mmc->card_caps & MMC_MODE_HS)
			mmc->tran_speed = 50000000;
		else
			mmc->tran_speed = 25000000;
	} else if (mmc->version >= MMC_VERSION_4) {
		/* Only version 4 of MMC supports wider bus widths */
		int idx;

		/* An array of possible bus widths in order of preference */
		static unsigned ext_csd_bits[] = {
			EXT_CSD_DDR_BUS_WIDTH_8,
			EXT_CSD_DDR_BUS_WIDTH_4,
			EXT_CSD_BUS_WIDTH_8,
			EXT_CSD_BUS_WIDTH_4,
			EXT_CSD_BUS_WIDTH_1,
		};

		/* An array to map CSD bus widths to host cap bits */
		static unsigned ext_to_hostcaps[] = {
			[EXT_CSD_DDR_BUS_WIDTH_4] =
				MMC_MODE_DDR_52MHz | MMC_MODE_4BIT,
			[EXT_CSD_DDR_BUS_WIDTH_8] =
				MMC_MODE_DDR_52MHz | MMC_MODE_8BIT,
			[EXT_CSD_BUS_WIDTH_4] = MMC_MODE_4BIT,
			[EXT_CSD_BUS_WIDTH_8] = MMC_MODE_8BIT,
		};

		/* An array to map chosen bus width to an integer */
		static unsigned widths[] = {
			8, 4, 8, 4, 1,
		};

		for (idx=0; idx < ARRAY_SIZE(ext_csd_bits); idx++) {
			unsigned int extw = ext_csd_bits[idx];
			unsigned int caps = ext_to_hostcaps[extw];

			/*
			 * If the bus width is still not changed,
			 * don't try to set the default again.
			 * Otherwise, recover from switch attempts
			 * by switching to 1-bit bus width.
			 */
			if (extw == EXT_CSD_BUS_WIDTH_1 &&
					mmc->bus_width == 1) {
				err = 0;
				break;
			}

			/*
			 * Check to make sure the card and controller support
			 * these capabilities
			 */
			if ((mmc->card_caps & caps) != caps)
				continue;

			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_BUS_WIDTH, extw);

			if (err)
				continue;

			mmc->ddr_mode = (caps & MMC_MODE_DDR_52MHz) ? 1 : 0;
			mmc_set_bus_width(mmc, widths[idx]);

			err = mmc_send_ext_csd(mmc, test_csd);

			if (err)
				continue;

			/* Only compare read only fields */
			if (ext_csd[EXT_CSD_PARTITIONING_SUPPORT]
				== test_csd[EXT_CSD_PARTITIONING_SUPPORT] &&
			    ext_csd[EXT_CSD_HC_WP_GRP_SIZE]
				== test_csd[EXT_CSD_HC_WP_GRP_SIZE] &&
			    ext_csd[EXT_CSD_REV]
				== test_csd[EXT_CSD_REV] &&
			    ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
				== test_csd[EXT_CSD_HC_ERASE_GRP_SIZE] &&
			    memcmp(&ext_csd[EXT_CSD_SEC_CNT],
				   &test_csd[EXT_CSD_SEC_CNT], 4) == 0)
				break;
			else
				err = SWITCH_ERR;
		}

		if (err)
			return err;

		if (mmc->card_caps & MMC_MODE_HS) {
			if (mmc->card_caps & MMC_MODE_HS_52MHz)
				mmc->tran_speed = 52000000;
			else
				mmc->tran_speed = 26000000;
		}
	}

	mmc_set_clock(mmc, mmc->tran_speed);

	/* Fix the block length for DDR mode */
	if (mmc->ddr_mode) {
		mmc->read_bl_len = MMC_MAX_BLOCK_LEN;
		mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
	}

	/* fill in device description */
	bdesc = mmc_get_blk_desc(mmc);
	bdesc->lun = 0;
	bdesc->hwpart = 0;
	bdesc->type = 0;
	bdesc->blksz = mmc->read_bl_len;
	bdesc->log2blksz = LOG2(bdesc->blksz);
	bdesc->lba = lldiv(mmc->capacity, mmc->read_bl_len);
#if !defined(CONFIG_SPL_BUILD) || \
		(defined(CONFIG_SPL_LIBCOMMON_SUPPORT) && \
		!defined(CONFIG_USE_TINY_PRINTF))
	sprintf(bdesc->vendor, "Man %06x Snr %04x%04x",
		mmc->cid[0] >> 24, (mmc->cid[2] & 0xffff),
		(mmc->cid[3] >> 16) & 0xffff);
	sprintf(bdesc->product, "%c%c%c%c%c%c", mmc->cid[0] & 0xff,
		(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
		(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
		(mmc->cid[2] >> 24) & 0xff);
	sprintf(bdesc->revision, "%d.%d", (mmc->cid[2] >> 20) & 0xf,
		(mmc->cid[2] >> 16) & 0xf);
#else
	bdesc->vendor[0] = 0;
	bdesc->product[0] = 0;
	bdesc->revision[0] = 0;
#endif
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBDISK_SUPPORT)
	part_init(bdesc);
#endif

	memcpy(saved_ext_csd, ext_csd, MMC_MAX_BLOCK_LEN);

	return 0;
}

#ifndef CONFIG_NVT_EMMC_ONLY_HOST
static int mmc_send_if_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = ((mmc->cfg->voltages & 0xff8000) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	if ((cmd.response[0] & 0xff) != 0xaa)
		return UNUSABLE_ERR;
	else
		mmc->version = SD_VERSION_2;

	return 0;
}
#endif

#ifdef CONFIG_BLK
int mmc_bind(struct udevice *dev, struct mmc *mmc, const struct mmc_config *cfg)
{
	struct blk_desc *bdesc;
	struct udevice *bdev;
	int ret;

	ret = blk_create_devicef(dev, "mmc_blk", "blk", IF_TYPE_MMC, -1, 512,
				 0, &bdev);
	if (ret) {
		debug("Cannot create block device\n");
		return ret;
	}
	bdesc = dev_get_uclass_platdata(bdev);
	mmc->cfg = cfg;
	mmc->priv = dev;

	/* the following chunk was from mmc_register() */

	/* Setup dsr related values */
	mmc->dsr_imp = 0;
	mmc->dsr = 0xffffffff;
	/* Setup the universal parts of the block interface just once */
	bdesc->removable = 1;

	/* setup initial part type */
	bdesc->part_type = cfg->part_type;
	mmc->dev = dev;

	return 0;
}

int mmc_unbind(struct udevice *dev)
{
	struct udevice *bdev;

	device_find_first_child(dev, &bdev);
	if (bdev) {
		device_remove(bdev);
		device_unbind(bdev);
	}

	return 0;
}

#else
struct mmc *mmc_create(const struct mmc_config *cfg, void *priv)
{
	struct blk_desc *bdesc;
	struct mmc *mmc;

	/* quick validation */
	if (cfg == NULL || cfg->ops == NULL || cfg->ops->send_cmd == NULL ||
			cfg->f_min == 0 || cfg->f_max == 0 || cfg->b_max == 0)
		return NULL;

	mmc = calloc(1, sizeof(*mmc));
	if (mmc == NULL)
		return NULL;

	mmc->cfg = cfg;
	mmc->priv = priv;

	/* the following chunk was mmc_register() */

	/* Setup dsr related values */
	mmc->dsr_imp = 0;
	mmc->dsr = 0xffffffff;
	/* Setup the universal parts of the block interface just once */
	bdesc = mmc_get_blk_desc(mmc);
	bdesc->if_type = IF_TYPE_MMC;
	bdesc->removable = 1;
	bdesc->devnum = mmc_get_next_devnum();
	bdesc->block_read = mmc_bread;
	bdesc->block_write = mmc_bwrite;
	bdesc->block_erase = mmc_berase;

	/* setup initial part type */
	bdesc->part_type = mmc->cfg->part_type;
	mmc_list_add(mmc);

	return mmc;
}

void mmc_destroy(struct mmc *mmc)
{
	/* only freeing memory for now */
	free(mmc);
}
#endif

#ifndef CONFIG_BLK
static int mmc_get_dev(int dev, struct blk_desc **descp)
{
	struct mmc *mmc = find_mmc_device(dev);
	int ret;

	if (!mmc)
		return -ENODEV;
	ret = mmc_init(mmc);
	if (ret)
		return ret;

	*descp = &mmc->block_dev;

	return 0;
}
#endif

/* board-specific MMC power initializations. */
__weak void board_mmc_power_init(void)
{
}

int mmc_start_init(struct mmc *mmc)
{
	int err;

	/* we pretend there's no card when init is NULL */
	if (mmc_getcd(mmc) == 0 || mmc->cfg->ops->init == NULL) {
		mmc->has_init = 0;
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("MMC: no card present\n");
#endif
		return NO_CARD_ERR;
	}

	if (mmc->has_init)
		return 0;

#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
	mmc_adapter_card_type_ident();
#endif
	board_mmc_power_init();

	/* made sure it's not NULL earlier */
	err = mmc->cfg->ops->init(mmc);

	if (err)
		return err;

	mmc->ddr_mode = 0;
	mmc_set_bus_width(mmc, 1);
	mmc_set_clock(mmc, 1);

	/* Reset the Card */
	err = mmc_go_idle(mmc);

	if (err)
		return err;

	/* The internal partition reset to user partition(0) at every CMD0*/
	mmc_get_blk_desc(mmc)->hwpart = 0;
#ifndef CONFIG_NVT_EMMC_ONLY_HOST
	err = TIMEOUT;
	/* Test for SD version 2 */
	err = mmc_send_if_cond(mmc);

	/* Now try to get the SD card's operating condition */
	err = sd_send_op_cond(mmc);
	/* If the command timed out, we check for an MMC card */
	if (err == TIMEOUT)
#endif
        {
		err = mmc_send_op_cond(mmc);

		if (err) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
			printf("Card did not respond to voltage select!\n");
#endif
			return UNUSABLE_ERR;
		}
	}

	if (!err)
		mmc->init_in_progress = 1;

	return err;
}

static int mmc_complete_init(struct mmc *mmc)
{
	int err = 0;

	mmc->init_in_progress = 0;
	if (mmc->op_cond_pending)
		err = mmc_complete_op_cond(mmc);

	if (!err)
		err = mmc_startup(mmc);
	if (err)
		mmc->has_init = 0;
	else
		mmc->has_init = 1;
	return err;
}

int mmc_init(struct mmc *mmc)
{
	int err = 0;
	unsigned start;
#ifdef CONFIG_DM_MMC
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(mmc->dev);

	upriv->mmc = mmc;
#endif
	if (mmc->has_init)
		return 0;

	start = get_timer(0);

	if (!mmc->init_in_progress)
		err = mmc_start_init(mmc);

	if (!err)
		err = mmc_complete_init(mmc);
	debug("%s: %d, time %lu\n", __func__, err, get_timer(start));
	return err;
}

int mmc_set_dsr(struct mmc *mmc, u16 val)
{
	mmc->dsr = val;
	return 0;
}

/* CPU-specific MMC initializations */
__weak int cpu_mmc_init(bd_t *bis)
{
	return -1;
}

/* board-specific MMC initializations. */
__weak int board_mmc_init(bd_t *bis)
{
	return -1;
}

void mmc_set_preinit(struct mmc *mmc, int preinit)
{
	mmc->preinit = preinit;
}

#if defined(CONFIG_DM_MMC) && defined(CONFIG_SPL_BUILD)
static int mmc_probe(bd_t *bis)
{
	return 0;
}
#elif defined(CONFIG_DM_MMC)
static int mmc_probe(bd_t *bis)
{
	int ret, i;
	struct uclass *uc;
	struct udevice *dev;

	ret = uclass_get(UCLASS_MMC, &uc);
	if (ret)
		return ret;

	/*
	 * Try to add them in sequence order. Really with driver model we
	 * should allow holes, but the current MMC list does not allow that.
	 * So if we request 0, 1, 3 we will get 0, 1, 2.
	 */
	for (i = 0; ; i++) {
		ret = uclass_get_device_by_seq(UCLASS_MMC, i, &dev);
		if (ret == -ENODEV)
			break;
	}
	uclass_foreach_dev(dev, uc) {
		ret = device_probe(dev);
		if (ret)
			printf("%s - probe failed: %d\n", dev->name, ret);
	}

	return 0;
}
#else
static int mmc_probe(bd_t *bis)
{
	if (board_mmc_init(bis) < 0)
		cpu_mmc_init(bis);

	return 0;
}
#endif

int mmc_initialize(bd_t *bis)
{
	static int initialized = 0;
	int ret;
	if (initialized)	/* Avoid initializing mmc multiple times */
		return 0;
	initialized = 1;

#ifndef CONFIG_BLK
	mmc_list_init();
#endif
	ret = mmc_probe(bis);
	if (ret)
		return ret;

#ifndef CONFIG_SPL_BUILD
	print_mmc_devices(',');
#endif

	mmc_do_preinit();
	return 0;
}

#ifdef CONFIG_SUPPORT_EMMC_BOOT
/*
 * This function changes the size of boot partition and the size of rpmb
 * partition present on EMMC devices.
 *
 * Input Parameters:
 * struct *mmc: pointer for the mmc device strcuture
 * bootsize: size of boot partition
 * rpmbsize: size of rpmb partition
 *
 * Returns 0 on success.
 */

int mmc_boot_partition_size_change(struct mmc *mmc, unsigned long bootsize,
				unsigned long rpmbsize)
{
	int err;
	struct mmc_cmd cmd;

	/* Only use this command for raw EMMC moviNAND. Enter backdoor mode */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = MMC_CMD62_ARG1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error1 = %d\n", err);
		return err;
	}

	/* Boot partition changing mode */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = MMC_CMD62_ARG2;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error2 = %d\n", err);
		return err;
	}
	/* boot partition size is multiple of 128KB */
	bootsize = (bootsize * 1024) / 128;

	/* Arg: boot partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = bootsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error3 = %d\n", err);
		return err;
	}
	/* RPMB partition size is multiple of 128KB */
	rpmbsize = (rpmbsize * 1024) / 128;
	/* Arg: RPMB partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = rpmbsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error4 = %d\n", err);
		return err;
	}
	return 0;
}

/*
 * Modify EXT_CSD[177] which is BOOT_BUS_WIDTH
 * based on the passed in values for BOOT_BUS_WIDTH, RESET_BOOT_BUS_WIDTH
 * and BOOT_MODE.
 *
 * Returns 0 on success.
 */
int mmc_set_boot_bus_width(struct mmc *mmc, u8 width, u8 reset, u8 mode)
{
	int err;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BOOT_BUS_WIDTH,
			 EXT_CSD_BOOT_BUS_WIDTH_MODE(mode) |
			 EXT_CSD_BOOT_BUS_WIDTH_RESET(reset) |
			 EXT_CSD_BOOT_BUS_WIDTH_WIDTH(width));

	if (err)
		return err;
	return 0;
}

int mmc_set_enhance_user_attribute(struct mmc *mmc)
{
	int err;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_ERASE_GROUP_DEF, 1);
	if (err)
		return err;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PARTITIONS_ATTRIBUTE,
			 EXT_CSD_PART_ATTR_ENH_USER);

	if (err)
		return err;
	return 0;

}

int mmc_clear_enhance_user_attribute(struct mmc *mmc)
{
	int err;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_ERASE_GROUP_DEF, 1);
	if (err)
		return err;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PARTITIONS_ATTRIBUTE,
			 0);

	if (err)
		return err;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_ERASE_GROUP_DEF, 0);
	if (err)
		return err;
	return 0;

}


/*
 * Modify EXT_CSD[179] which is PARTITION_CONFIG (formerly BOOT_CONFIG)
 * based on the passed in values for BOOT_ACK, BOOT_PARTITION_ENABLE and
 * PARTITION_ACCESS.
 *
 * Returns 0 on success.
 */
int mmc_set_part_conf(struct mmc *mmc, u8 ack, u8 part_num, u8 access)
{
	int err;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			 EXT_CSD_BOOT_ACK(ack) |
			 EXT_CSD_BOOT_PART_NUM(part_num) |
			 EXT_CSD_PARTITION_ACCESS(access));

	if (err)
		return err;
	return 0;
}

/*
 * Modify EXT_CSD[162] which is RST_n_FUNCTION based on the given value
 * for enable.  Note that this is a write-once field for non-zero values.
 *
 * Returns 0 on success.
 */
int mmc_set_rst_n_function(struct mmc *mmc, u8 enable)
{
	return mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_RST_N_FUNCTION,
			  enable);
}
#endif
int mmc_dump_boot_info(struct mmc *mmc)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, 512);
	int err = 0;
	u8 bus = 0;
	u8 mode = 0;

	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err != 0) {
		printf("mmc send ext_csd failed\n");
		return err;
	}

	bus = ext_csd[EXT_CSD_BOOT_BUS_WIDTH];
	//kwinyee debug
	printf("177 bus = 0x%x\n", bus);
	printf("boot size = %llu\n", mmc->capacity_boot);

	switch(((bus>>3) & 3)) {
		case 0:
			printf("SDR + backward compatiible timings in boot mode\n");
			break;

		case 1:
			printf("SDR + high speed timings in boot mode\n");
			break;

		case 2:
			printf("DDR in boot mode\n");
			break;
	}

	switch((bus & 3)) {
		case 0:
			printf("x1(sdr) or x4(ddr) bus width in boot mode\n");
			break;

		case 1:
			printf("x4(sdr/ddr) bus width in boot mode\n");
			break;

		case 2:
			printf("x8(sdr/ddr) bus width in boot mode\n");
			break;
	}

	mode = ext_csd[EXT_CSD_PART_CONF];
	//kwinyee debug
	printf("179 mode = 0x%x\n", mode);

	if((mode & (1 << 6)))
		printf("Boot ack sent during boot operation\n");
	else
		printf("No boot ack sent\n");

	switch(((mode >> 3)&7)) {
		case 0:
			printf("device no boot enable\n");
			break;

		case 1:
			printf("Boot partition 1 enabled for boot\n");
			break;

		case 2:
			printf("Boot partition 2 enable for boot\n");
			break;

		case 7:
			printf("User area enabled for boot\n");
			break;
	}

	switch((mode&7)) {
		case 0:
			printf("No access to boot partition\n");
			break;

		case 1:
			printf("R/W boot partition 1\n");
			break;

		case 2:
			printf("R/W boot partition 2\n");
			break;

		case 3:
			printf("R/W RPMB\n");
			break;

		case 4:
			printf("Access to General Purpose partition 1\n");
			break;

		case 5:
			printf("Access to General Purpose partition 2\n");
			break;

		case 6:
			printf("Access to General Purpose partition 3\n");
			break;

		case 7:
			printf("Access to General Purpose partition 4\n");
			break;
	}


	mode = ext_csd[EXT_CSD_BOOT_WP];
        printf("boot wp %x\n",mode);
	mode = ext_csd[EXT_CSD_BOOT_WP_STATUS];
        printf("boot wp status %x\n",mode);
	switch((mode&(3))) {
		case 0:
			printf("Boot Area 1 is not protected\n");
			break;

		case 1:
			printf("Boot Area 1 is Power on protected \n");
			break;

		case 2:
			printf("Boot Area 1 is Permanently Protected \n");
			break;

		case 3:
			printf("reserved\n");
			break;

	}
	switch(((mode>>2)&(3))) {
		case 0:
			printf("Boot Area 2 is not protected\n");
			break;

		case 1:
			printf("Boot Area 2 is Power on protected \n");
			break;

		case 2:
			printf("Boot Area 2 is Permanently Protected \n");
			break;

		case 3:
			printf("reserved\n");
			break;

	}
	return err;

}

#if defined(CONFIG_EMMC_WRITE_RELIABILITY_TEST)

int check_write_reliability_setting(void)
{

	if (saved_ext_csd[EXT_CSD_WR_REL_SET] == 0x1f) {
		printf("Done setting for Write Reliability \n");
		return 1;
	}

	printf("setting OTP PARTITION_SETTING_COMPLETED!\n");
	if (saved_ext_csd[EXT_CSD_PARTITION_SETTING] &
	    EXT_CSD_PARTITION_SETTING_COMPLETED) {
		printf("Card already partitioned\n");
                if (saved_ext_csd[EXT_CSD_WR_REL_SET] == 0x0)
                    printf("This eMMC isn't supported for Write Reliability \n");
		return 1;
	}

	return 0;
}
#endif

#ifdef CONFIG_BLK
static const struct blk_ops mmc_blk_ops = {
	.read	= mmc_bread,
	.write	= mmc_bwrite,
	.select_hwpart	= mmc_select_hwpart,
};

U_BOOT_DRIVER(mmc_blk) = {
	.name		= "mmc_blk",
	.id		= UCLASS_BLK,
	.ops		= &mmc_blk_ops,
};
#else
U_BOOT_LEGACY_BLK(mmc) = {
	.if_typename	= "mmc",
	.if_type	= IF_TYPE_MMC,
	.max_devs	= -1,
	.get_dev	= mmc_get_dev,
	.select_hwpart	= mmc_select_hwpartp,
};
#endif
