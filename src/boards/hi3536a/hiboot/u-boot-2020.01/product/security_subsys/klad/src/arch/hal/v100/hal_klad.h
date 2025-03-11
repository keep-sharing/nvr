// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_KLAD_H
#define HAL_KLAD_H

#include "drv_klad_sw_utils.h"

td_s32 hal_klad_init(td_void);

td_void hal_klad_deinit(td_void);

td_s32 hal_klad_common_route_startup(td_u32 slot_num, const klad_common_slot *common_slot);

td_s32 hal_klad_clear_route_startup(td_u32 slot_num, const klad_clear_slot *key);

#endif /* HAL_KLAD_H */

