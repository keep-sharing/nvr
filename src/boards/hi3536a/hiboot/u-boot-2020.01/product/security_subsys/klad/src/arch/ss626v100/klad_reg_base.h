// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __KLAD_REG_BASE_H__
#define __KLAD_REG_BASE_H__

/* rkp register base addr and offset */
#define RKP_REG_BASE_ADDR_PHY       0x10111000
#define RKP_REG_BASE_ADDR_SIZE      0x1000
#define RKP_DBG_STAT                0x4
#define RKP_INT_EN                  0x50
#define RKP_INT_RAW                 0x54
#define RKP_INT                     0x58
#define RKP_SEC_CFG                 0x74
#define RKP_SW_REG                  0x78
#define RKP_CALC_CTRL               0x7C
#define RKP_ERROR                   0x80

/* klad register base addr and offset */
#define HKL_REG_BASE_ADDR_PHY       0x10110000
#define HKL_REG_BASE_ADDR_SIZE      0x1000
#define KL_DATA_IN_0                0x00
#define KL_DATA_IN_1                0x04
#define KL_DATA_IN_2                0x08
#define KL_DATA_IN_3                0x0C
#define KL_KEY_ADDR                 0x10
#define KL_KEY_CFG                  0x14
#define KL_KEY_SEC_CFG              0x18
#define KL_STATE                    0x30
#define KL_ERROR                    0x38
#define KC_ERROR                    0x3C
#define KL_INT_EN                   0x40
#define KL_INT_RAW                  0x44
#define KL_INT                      0x48
#define KL_LOCK_CTRL                0x74
#define KL_UNLOCK_CTRL              0x78
#define KL_COM_LOCK_INFO            0x7C
#define KL_COM_LOCK_STATUS          0x80
#define KL_COM_CTRL                 0x84
#define KL_CLR_CTRL                 0x438

#endif
