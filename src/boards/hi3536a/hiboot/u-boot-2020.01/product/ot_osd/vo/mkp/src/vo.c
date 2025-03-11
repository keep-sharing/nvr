// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "vo.h"
#include "mkp_vo_dev.h"
#include "mkp_vo_video.h"
#include "mkp_vo_comm.h"
#include "mkp_vo_gfx.h"
#include "mkp_vo_init.h"
#include "mkp_vo_user.h"
#include "ot_math.h"
#include <common.h>
#include <command.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <config.h>
#include <cpu_func.h>
#include "securec.h"

#include "drv_vo.h"
#include "drv_vo_dev.h"
#include "drv_vo_gfx.h"

td_void vo_dcache_range(td_phys_addr_t start_addr, td_u64 size)
{
    if (dcache_status()) {
        flush_dcache_range(start_addr, (td_u64)start_addr + size);
    }
}

int set_vobg(unsigned int dev, unsigned int rgb)
{
    vo_check_dev_id_return(dev);
    vo_dev_set_bg_color(dev, rgb);
    return TD_SUCCESS;
}

static td_void vo_construct_pub_attr(unsigned int dev,
    unsigned int type, unsigned int sync, ot_vo_pub_attr *pub_attr)
{
    const ot_vo_sync_info *sync_info = TD_NULL;

    pub_attr->bg_color = vo_dev_get_bg_color(dev);
    pub_attr->intf_sync = sync;
    pub_attr->intf_type = type;
    if (vo_is_user_intf_sync(sync) == TD_TRUE) {
        sync_info = vo_get_dev_user_sync_timing(dev);
        if (sync_info != TD_NULL) {
            (td_void)memcpy_s(&(pub_attr->sync_info), sizeof(ot_vo_sync_info), sync_info, sizeof(ot_vo_sync_info));
        }
    }
}

static td_s32 vo_set_user_sync_clk(unsigned int dev, unsigned int sync)
{
    td_s32 ret;
    const ot_vo_user_sync_info *user_sync = TD_NULL;

    if (vo_is_user_intf_sync(sync) == TD_TRUE) {
        /* set the user sync info: clk source, clk value */
        user_sync = vo_get_dev_user_sync_info(dev);
        if (user_sync == TD_NULL) {
            return TD_FAILURE;
        }
        ret = vo_set_user_sync_info(dev, user_sync);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }
    return TD_SUCCESS;
}

int start_vo(unsigned int dev, unsigned int type, unsigned int sync)
{
    td_s32 ret;
    ot_vo_pub_attr pub_attr = {0};

    vo_init();

    vo_construct_pub_attr(dev, type, sync, &pub_attr);

    ret = vo_set_pub_attr(dev, &pub_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = vo_set_user_sync_clk(dev, sync);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = vo_enable(dev);
    return ret;
}

int stop_vo(unsigned int dev)
{
    vo_check_dev_id_return(dev);
    return vo_disable(dev);
}

static td_s32 vo_construct_gfx_attr(unsigned long addr, unsigned int stride,
    ot_rect gfx_rect, unsigned int type, ot_vo_gfx_attr *gfx_attr)
{
    ot_vo_gfx_attr gfx_attr_tmp = {0};

    gfx_attr_tmp.stride = stride;
    gfx_attr_tmp.address = addr;
    gfx_attr_tmp.type = type;
    (td_void)memcpy_s(&gfx_attr_tmp.display_rect, sizeof(ot_rect), &gfx_rect, sizeof(ot_rect));

    return vo_drv_gfx_convert_gfx_attr(&gfx_attr_tmp, gfx_attr);
}

int start_gx(unsigned int layer, unsigned long addr, unsigned int strd, ot_rect gx_rect, unsigned int type)
{
    td_s32 ret;
    ot_vo_gfx_attr gfx_attr = {0};

    ret = vo_construct_gfx_attr(addr, strd, gx_rect, type, &gfx_attr);
    if (ret != TD_SUCCESS) {
        vo_err_trace("convert gfx attr failed.\n");
        return ret;
    }

    ret = vo_set_gfx_attr(layer, &gfx_attr);
    if (ret != TD_SUCCESS) {
        printf("invalid parameter!\n");
        return ret;
    }
    ret = vo_enable_gfx_layer(layer, &gfx_attr);

    return ret;
}

int stop_gx(unsigned int layer)
{
    td_s32 ret;
    ret = vo_disable_gfx_layer(layer);
    if (ret != TD_SUCCESS) {
        printf("invalid parameter!\n");
        return ret;
    }

    return TD_SUCCESS;
}

static td_void vo_construct_video_layer_attr(unsigned long addr, unsigned int strd,
    ot_rect layer_rect, ot_vo_video_layer_attr *video_attr)
{
    video_attr->stride = strd;
    video_attr->address = addr;
    (td_void)memcpy_s(&video_attr->display_rect, sizeof(ot_rect), &layer_rect, sizeof(ot_rect));
    video_attr->pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
}

int start_videolayer(unsigned int layer, unsigned long addr, unsigned int strd, ot_rect layer_rect)
{
    td_s32 ret;
    ot_vo_video_layer_attr video_attr = {0};

    vo_construct_video_layer_attr(addr, strd, layer_rect, &video_attr);
    ret = vo_set_video_layer_attr(layer, &video_attr);
    if (ret != TD_SUCCESS) {
        printf("invalid parameter!\n");
        return ret;
    }

    vo_dcache_range(addr, strd * layer_rect.height * 3 / 2); /* 3 / 2 times */

    ret = vo_enable_video_layer(layer, &video_attr);

    return ret;
}

int stop_videolayer(unsigned int layer)
{
    td_s32 ret;
    ret = vo_disable_video_layer(layer);
    if (ret != TD_SUCCESS) {
        printf("invalid parameter!\n");
        return ret;
    }

    return TD_SUCCESS;
}
