// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef MKP_VO_USER_H
#define MKP_VO_USER_H

#include "ot_common_vo.h"
#include "ot_common.h"

const ot_vo_sync_info *vo_get_dev_user_sync_timing(ot_vo_dev dev);
const ot_vo_user_sync_info *vo_get_dev_user_sync_info(ot_vo_dev dev);

#endif /* end of MKP_VO_USER_H */
