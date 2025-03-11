// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "mkp_vo_comm.h"
#include "vo.h"
#include "ot_common_vo.h"
#include "ot_common_video.h"

#include "mkp_vo.h"
#include "sys_ext.h"

#include "ot_board.h"
#include "drv_vo.h"

#if vo_desc("UBOOT_VO")
#if vo_desc("dev check")

td_s32 vo_def_check_dev_id(ot_vo_dev dev)
{
    if ((dev < 0) || (dev >= OT_VO_MAX_PHYS_DEV_NUM)) {
        return OT_ERR_VO_INVALID_DEV_ID;
    }

    return TD_SUCCESS;
}

td_bool vo_def_is_gfx_layer_id(ot_vo_layer layer)
{
    if ((layer >= OT_VO_LAYER_G0) && (layer < VO_LAYER_BUTT)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

#endif

#if vo_desc("check attr pub")

td_s32 vo_check_bool_value(td_bool bool_val)
{
    if ((bool_val != TD_TRUE) && (bool_val != TD_FALSE)) {
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 vo_check_rect_align(const ot_rect *rect)
{
    if ((rect->x % VO_RECT_ALIGN) || (rect->y % VO_RECT_ALIGN) ||
        (rect->width % VO_RECT_ALIGN) || (rect->height % VO_RECT_ALIGN)) {
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}
#endif

#if vo_desc("check dev pub")

td_bool vo_is_phy_dev(ot_vo_dev dev)
{
    return TD_TRUE;
}

td_bool vo_is_typical_intf_sync(ot_vo_intf_sync intf_sync)
{
    if ((intf_sync < OT_VO_OUT_BUTT) && (intf_sync != OT_VO_OUT_USER)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

td_bool vo_is_user_intf_sync(ot_vo_intf_sync intf_sync)
{
    if (intf_sync == OT_VO_OUT_USER) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

#endif
#endif /* #if vo_desc("UBOOT_VO") */
