// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_RKP_H
#define HAL_RKP_H

#include "ot_common_klad.h"

td_s32 hal_rkp_init(td_void);

td_void hal_rkp_deinit(td_void);

td_s32 hal_rkp_startup(const ot_klad_rootkey_attr *key_attr);

#endif /* HAL_RKP_H */
