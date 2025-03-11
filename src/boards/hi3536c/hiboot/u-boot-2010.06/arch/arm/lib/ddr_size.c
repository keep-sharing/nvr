/*
 * Copyright (c) 2012 HiSilicon Technologies Co., Ltd.
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


#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <asm/arch/platform.h>
#include <version.h>

#define _16M        0x1000000
/*****************************************************************************/
/* Use this feature, Dcache should be disable */
unsigned int get_ddr_size(void)
{
#define TO_UINT32(_p)   (*(volatile unsigned int *)(_p))
	DECLARE_GLOBAL_DATA_PTR;
	static unsigned int ddr_size;
	volatile unsigned char *memskip;
	volatile unsigned char *membase = (unsigned char *)MEM_BASE_DDR;
	unsigned int orgin = TO_UINT32(membase);
	unsigned int rd_origin = 0, rd_verify = 0;
	unsigned int offset = 0;

	if (ddr_size)
		return ddr_size;

	for (memskip = membase + _16M;
	     memskip <= membase + get_max_ddr_size();
	     memskip += _16M) {
		offset = memskip - membase;
		if (offset == gd->bd->bi_dram[0].size) {
			ddr_size = gd->bd->bi_dram[0].size;
			break;
		}
		TO_UINT32(membase) = 0xA9A9A9A9;
		rd_origin = TO_UINT32(memskip);

		TO_UINT32(membase) = 0x53535352;
		rd_verify = TO_UINT32(memskip);

		if (rd_origin != rd_verify) {
			ddr_size = (unsigned int)(memskip - membase);
			break;
		}
	}

	/* restore membase value. */
	TO_UINT32(membase) = orgin;
	return ddr_size;
}

