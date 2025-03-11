// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __OTP_REG_BASE_H__
#define __OTP_REG_BASE_H__

#define OTP_REG_BASE_ADDR_PHY       0x10122000
#define OTP_REG_BASE_ADDR_SIZE      0x2000
#define OTP_TEE_REG_OFFSET          0x1000

extern td_u32 g_tee_reg_offset;

#define OTP_USER_WORK_MODE          (g_tee_reg_offset + 0x0000)
#define OTP_USER_REV_ADDR           (g_tee_reg_offset + 0x0038)
#define OTP_USER_REV_WDATA          (g_tee_reg_offset + 0x003c)
#define OTP_USER_REV_RDATA          (g_tee_reg_offset + 0x0040)
#define OTP_USER_LOCK_STA_ADDR      (g_tee_reg_offset + 0x0044)
#define OTP_USER_LOCK_STA_DATA      (g_tee_reg_offset + 0x0048)
#define OTP_USER_CTRL_STA           (g_tee_reg_offset + 0x004c)

#define OTP_USER_OP_START           0x0004
#define OTP_USER_KEY_INDEX          0x0008
#define OTP_USER_KEY_DATA0          0x000c
#define OTP_USER_KEY_DATA1          0x0010
#define OTP_USER_KEY_DATA2          0x0014
#define OTP_USER_KEY_DATA3          0x0018
#define OTP_USER_KEY_DATA4          0x001c
#define OTP_USER_KEY_DATA5          0x0020
#define OTP_USER_KEY_DATA6          0x0024
#define OTP_USER_KEY_DATA7          0x0028
#define OTP_USER_KEY_DATA8          0x002c
#define OTP_USER_FLAG_VALUE         0x0030
#define OTP_USER_FLAG_INDEX         0x0034

#define OTP_OP_START_VAL            0x1acce551

#endif /* __OTP_REG_BASE_H__ */
