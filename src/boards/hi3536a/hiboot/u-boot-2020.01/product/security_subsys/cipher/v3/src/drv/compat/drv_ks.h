// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_KS_H
#define DRV_KS_H

#include "ot_type.h"
#include "ot_common_cipher.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

td_s32 drv_kc_slot_init(td_void);

td_void drv_kc_slot_deinit(td_void);

td_s32 drv_kc_slot_lock(const ot_keyslot_attr *attr, td_u32 *slot_num);

td_s32 drv_kc_slot_unlock(td_u32 slot_num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* DRV_KS_H */
