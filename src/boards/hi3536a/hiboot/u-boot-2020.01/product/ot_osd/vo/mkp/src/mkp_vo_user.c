// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "mkp_vo_user.h"
#include "drv_vo_dev.h"

#if vo_desc("dev user sync timing")
const ot_vo_sync_info *vo_get_dev_user_sync_timing(ot_vo_dev dev)
{
    return vo_drv_get_dev_user_sync_timing(dev);
}
#endif

#if vo_desc("dev user sync info")
const ot_vo_user_sync_info *vo_get_dev_user_sync_info(ot_vo_dev dev)
{
    return vo_drv_get_dev_user_sync_info(dev);
}
#endif
