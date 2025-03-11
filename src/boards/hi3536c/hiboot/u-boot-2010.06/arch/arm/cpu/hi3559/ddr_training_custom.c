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
#include <config.h>
#include "ddr_training_impl.h"

/**
 * ddr_cmd_prepare_copy
 * @void
 *
 * Do some prepare before copy code from DDR to SRAM.
 * Keep empty when nothing to do.
 */
void ddr_cmd_prepare_copy(void) { return; }


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

#define DDRC_CTRL_REG	(0x12030100)
#define DDR_BYPASS	(0x5)

static inline unsigned int readl(unsigned addr)
{
	unsigned int val;

	val = (*(volatile unsigned int *)(addr));
	return val;
}

static inline void writel(unsigned val, unsigned addr)
{
	(*(volatile unsigned *) (addr)) = (val);
}

void ddr_training_save_reg_custom(void *reg, unsigned int mask)
{
	struct tr_relate_reg *relate_reg = (struct tr_relate_reg *)reg;
    unsigned int reg_val = 0;

	relate_reg->custom.scramb = readl(DDRC_CTRL_REG);
	/* disable scramb */
	writel(DDR_BYPASS, DDRC_CTRL_REG);
	DDR_DEBUG("Disable scramb [0x%x] = 0x%x.", DDRC_CTRL_REG, DDR_BYPASS);

    reg_val = readl(CRG_REG_BASE + REG_PERI_CRG54);
	reg_val = reg_val | DDRT_CLKEN;
	writel(reg_val, CRG_REG_BASE + REG_PERI_CRG54);
}

void ddr_training_restore_reg_custom(void *reg)
{
	struct tr_relate_reg *relate_reg = (struct tr_relate_reg *)reg;
    unsigned int reg_val = 0;

    /* restore scramb */
	writel(relate_reg->custom.scramb, DDRC_CTRL_REG);
	DDR_DEBUG("Restore scramb[0x%x] = 0x%x.", DDRC_CTRL_REG, relate_reg->custom.scramb);

    reg_val = readl(CRG_REG_BASE + REG_PERI_CRG54);
	reg_val = reg_val & (~DDRT_CLKEN);
	writel(reg_val, CRG_REG_BASE + REG_PERI_CRG54);
}
