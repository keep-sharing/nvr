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

#ifndef __MMC_H__
#define __MMC_H__

#define REG_MMC_BASE    REG_BASE_EMMC  /* EMMC: 0x100e_0000 */

/* CMD */
#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SELECT_CARD         7
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define SD_CMD_APP_SEND_OP_COND		41
#define MMC_CMD_APP_CMD             55
#define SD_CMD_SEND_IF_COND         8
#define SD_CMD_APP_SET_BUS_WIDTH	6

#define OCR_BUSY					0x80000000
#define OCR_HCS						0x40000000

#define CARD_WIDTH_1BIT             (0x00)
#define CARD_WIDTH_4BIT	            (0x01)

#define MMC_VDD_32_33				0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34				0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#endif /* __MMC_H__ */

