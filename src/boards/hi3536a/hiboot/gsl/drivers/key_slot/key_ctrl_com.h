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

#ifndef __COMMON_KS_H__
#define __COMMON_KS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define KC_BASE             	0x10112000

#define KC_TEE_LOCK_CMD         0xB00
#define KC_REE_LOCK_CMD         0xB04
#define KC_TEE_FLUSH_BUSY       0xB10
#define KC_REE_FLUSH_BUSY       0xB14
#define KC_SEND_DBG             0xB20
#define KC_ROB_ALARM            0xB24
#define KC_RD_SLOT_NUM          0xB30
#define KC_RD_LOCK_STATUS       0xB34

typedef enum {
	KC_SLOT_STAT_UN_LOCK     = 0x0,
	KC_SLOT_STAT_REE_LOCK    = 0x1,
	KC_SLOT_STAT_TEE_LOCK    = 0x2,
	KC_SLOT_STAT_MAX
} kc_slot_stat_e;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __common_ks_h__ */

