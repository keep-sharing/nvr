/*
* Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
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



#include <ddr_interface.h>
#include "ddr_training_impl.h"

#define DDRT_TEST_CLK                  0x200300d8
#define DDRT_ENABLE                    0x20120004
/**
 * ddr_cmd_prepare_copy
 * @void
 *
 * Do some prepare before copy code from DDR to SRAM.
 * Keep empty when nothing to do.
 */
void ddr_cmd_prepare_copy(void) { return; }

/**
 * ddr_ddrt_prepare_custom
 * @void
 *
 * Open DDRT clock and enable DDRT function.
 */
void ddr_ddrt_prepare_custom(void)
{
	REG_WRITE(REG_READ(DDRT_TEST_CLK) | 0x8, DDRT_TEST_CLK);
	REG_WRITE(REG_READ(DDRT_ENABLE) | 0x4, DDRT_ENABLE);
}
/**
 * ddr_cmd_site_save
 * @void
 *
 * Save site before DDR training command execute .
 * Keep empty when nothing to do.
 */
void ddr_cmd_site_save(void) { return; }

/**
 * ddr_cmd_site_restore
 * @void
 *
 * Restore site after DDR training command execute.
 * Keep empty when nothing to do.
 */
void ddr_cmd_site_restore(void) { return; }
