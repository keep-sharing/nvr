/*
* Copyright (c) 2009-2010 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <malloc.h>
#include <linux/mtd/nand.h>
#include "hinfc504.h"

/*****************************************************************************/

static int hinfc504_toshiba_24nm_set_rr_reg(struct hinfc_host *host, int param)
{
#define TOSHIBA_RR_CMD     0x55

	static char toshiba_rr_param[] = {0x00, 0x04, 0x7c, 0x78, 0x74, 0x08};

	if (!param) {
		host->send_cmd_reset(host, host->chipselect);
		return 0;
	}

	if (param >= 6)
		param = (param % 6);

	hinfc_write(host, 1, HINFC504_DATA_NUM);

	writel(toshiba_rr_param[param], host->chip->IO_ADDR_R);
	hinfc_write(host, 0x4, HINFC504_ADDRL);
	hinfc_write(host, TOSHIBA_RR_CMD, HINFC504_CMD);
	hinfc_write(host, HINFC504_WRITE_1CMD_1ADD_DATA, HINFC504_OP);
	WAIT_CONTROLLER_FINISH();

	writel(toshiba_rr_param[param], host->chip->IO_ADDR_R);
	hinfc_write(host, 0x5, HINFC504_ADDRL);
	hinfc_write(host, TOSHIBA_RR_CMD, HINFC504_CMD);
	hinfc_write(host, HINFC504_WRITE_1CMD_1ADD_DATA, HINFC504_OP);
	WAIT_CONTROLLER_FINISH();

	writel(toshiba_rr_param[param], host->chip->IO_ADDR_R);
	hinfc_write(host, 0x6, HINFC504_ADDRL);
	hinfc_write(host, TOSHIBA_RR_CMD, HINFC504_CMD);
	hinfc_write(host, HINFC504_WRITE_1CMD_1ADD_DATA, HINFC504_OP);
	WAIT_CONTROLLER_FINISH();

	writel(toshiba_rr_param[param], host->chip->IO_ADDR_R);
	hinfc_write(host, 0x7, HINFC504_ADDRL);
	hinfc_write(host, TOSHIBA_RR_CMD, HINFC504_CMD);
	hinfc_write(host, HINFC504_WRITE_1CMD_1ADD_DATA, HINFC504_OP);
	WAIT_CONTROLLER_FINISH();

	return 0;

#undef TOSHIBA_RR_CMD
}
/*****************************************************************************/

static int hinfc504_toshiba_24nm_set_rr_param(struct hinfc_host *host,
	int param)
{
	return hinfc504_toshiba_24nm_set_rr_reg(host, param);
}
/*****************************************************************************/

struct read_retry_t hinfc504_toshiba_24nm_read_retry = {
	.type = NAND_RR_TOSHIBA_24nm,
	.count = 6,
	.set_rr_param = hinfc504_toshiba_24nm_set_rr_param,
	.get_rr_param = NULL,
	.enable_enhanced_slc = NULL,
};
