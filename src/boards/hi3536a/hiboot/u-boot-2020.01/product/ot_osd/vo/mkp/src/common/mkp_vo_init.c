// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "mkp_vo_init.h"
#include "mkp_vo_dev.h"
#include "mkp_vo_gfx.h"
#include "mkp_vo_video.h"
#include "ot_common_vo.h"
#include "ot_common_video.h"
#include "sys_ext.h"

#include "ot_board.h"
#include "drv_vo.h"

#if vo_desc("clk init")
static td_void vo_init_set_sys_clk(td_void)
{
    vo_lpw_bus_reset(TD_FALSE);
}

static td_void vo_init_crg_clk(td_void)
{
    vo_drv_set_all_crg_clk(TD_TRUE);
}

static td_void vo_init_board(td_void)
{
    vo_drv_board_init();
}
#endif

#if vo_desc("init context")

static td_void vo_init_context(td_void)
{
    vo_init_dev_info();

    vo_init_layer_resource();
}
#endif

#if vo_desc("vo init flag")

static td_bool g_is_initialized = TD_FALSE;
td_bool vo_get_vo_init_flag(td_void)
{
    return g_is_initialized;
}

static td_void vo_set_vo_init_flag(td_bool flag)
{
    g_is_initialized = flag;
}
#endif

#if vo_desc("init operation")

td_s32 vo_init(td_void)
{
    ot_vo_dev dev;
    ot_vo_layer layer;
    if (vo_get_vo_init_flag() == TD_FALSE) {
        /* unreset */
        vo_init_set_sys_clk();

        /* clk enable */
        vo_init_crg_clk();

        /* step1: init drv context */
        vo_init_context();

        /* step2: init mkp context */
        for (dev = 0; dev < OT_VO_MAX_PHYS_DEV_NUM; dev++) {
            vo_init_dev_ctx(dev);
        }
        for (layer = 0; layer < OT_VO_MAX_PHYS_VIDEO_LAYER_NUM; layer++) {
            vo_init_layer_ctx(layer);
        }

        /* step3: init mkp gfx context */
        vo_init_gfx();

        /* hardware init */
        vo_init_board();

        vo_set_vo_init_flag(TD_TRUE);
    }

    return TD_SUCCESS;
}
#endif
