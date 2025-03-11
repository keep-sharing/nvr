// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DDR_WAKEUP_H
#define DDR_WAKEUP_H

#define DATA_PHY_BASE_ADDR 0x844000

#define lpds_ddrtrn_hal_phy_parab(y)     (0x1000 + ((y) << 2)) /* used to store DDR PHY parameters */
#define lpds_hpp_reseved_areac(y)        (0x1800 + ((y) << 2)) /* used to store DDR PHY parameters */

#define DDR_REG_OFFSET             0x4 /* register offset */
#define DDR_WDQPHASE_ADJUST_OFFSET (-8)

/* register mask */
#define SYS_DDRPHY_LP_EN_MASK3  0x3 /* bit[1:0]sys_ddrphy_lp_en */

/* register bit */
#define SYS_DDRPHY_LP_EN_BIT0  0 /* bit[1:0]sys_ddrphy_lp_en */
struct ddrtrn_hal_phy_param_s {
	unsigned int reg_addr;
	unsigned int reg_num;
	unsigned int save_addr;
};
void ddrtrn_hal_wakeup_lp_en_ctrl(void);
#endif /* DDR_WAKEUP_H */