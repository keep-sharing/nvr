// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_KLAD_INTF_H
#define DRV_KLAD_INTF_H

#include "ot_type.h"

td_s32 intf_klad_ioctl(td_u32 cmd, td_void *param);

td_s32 klad_entry(td_void);

td_void klad_exit(td_void);

#endif /* DRV_KLAD_INTF_H */
