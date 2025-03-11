// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hdmi_reg_video_path.h"
#include "hdmi_product_define.h"

volatile video_path_regs *g_video_path_all_reg[OT_HDMI_ID_MAX] = {TD_NULL};

td_s32 hdmi_videopath_regs_init(td_u32 id)
{
    td_u32 addr;

    addr = (id == 0) ? HDMI_TX_BASE_ADDR : HDMI1_TX_BASE_ADDR;
    g_video_path_all_reg[id] = (volatile video_path_regs *)(uintptr_t)(addr + HDMI_TX_BASE_ADDR_VIDEO);

    return TD_SUCCESS;
}

td_void hdmi_videopath_regs_deinit(td_u32 id)
{
    if (g_video_path_all_reg[id] != TD_NULL) {
        g_video_path_all_reg[id] = TD_NULL;
    }
    return;
}

td_u32 hdmi_reg_csc_mode_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    multi_csc_ctrl csc_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->csc_ctrl.u32);
    csc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return csc_ctrl.bits.reg_csc_mode;
}

td_void hdmi_dither_mode_set(td_u32 id, td_u32 dither_mode)
{
    td_u32 *reg_addr = TD_NULL;
    dither_config dither_cfg;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->dither_cfg.u32);
    dither_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    dither_cfg.bits.dither_mode = dither_mode;
    hdmi_tx_reg_write(reg_addr, dither_cfg.u32);

    return;
}

td_void hdmi_dither_rnd_byp_set(td_u32 id, td_u32 dither_rnd_byp)
{
    td_u32 *reg_addr = TD_NULL;
    dither_config dither_cfg;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->dither_cfg.u32);
    dither_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    dither_cfg.bits.dither_rnd_byp = dither_rnd_byp;
    hdmi_tx_reg_write(reg_addr, dither_cfg.u32);

    return;
}

td_void hdmi_reg_csc_mode_set(td_u32 id, td_u32 reg_csc_mode)
{
    td_u32 *reg_addr = TD_NULL;
    multi_csc_ctrl csc_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->csc_ctrl.u32);
    csc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    csc_ctrl.bits.reg_csc_mode = reg_csc_mode;
    hdmi_tx_reg_write(reg_addr, csc_ctrl.u32);

    return;
}

td_void hdmi_reg_csc_saturate_en_set(td_u32 id, td_u32 reg_csc_saturate_en)
{
    td_u32 *reg_addr = TD_NULL;
    multi_csc_ctrl csc_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->csc_ctrl.u32);
    csc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    csc_ctrl.bits.reg_csc_saturate_en = reg_csc_saturate_en;
    hdmi_tx_reg_write(reg_addr, csc_ctrl.u32);

    return;
}

td_void hdmi_reg_csc_en_set(td_u32 id, td_u32 reg_csc_en)
{
    td_u32 *reg_addr = TD_NULL;
    multi_csc_ctrl csc_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->csc_ctrl.u32);
    csc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    csc_ctrl.bits.reg_csc_en = reg_csc_en;
    hdmi_tx_reg_write(reg_addr, csc_ctrl.u32);

    return;
}

td_void hdmi_reg_dwsm_vert_byp_set(td_u32 id, td_u32 reg_dwsm_vert_byp)
{
    td_u32 *reg_addr = TD_NULL;
    video_dwsm_ctrl dwsm_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dwsm_ctrl.u32);
    dwsm_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dwsm_ctrl.bits.reg_dwsm_vert_byp = reg_dwsm_vert_byp;
    hdmi_tx_reg_write(reg_addr, dwsm_ctrl.u32);

    return;
}

td_void hdmi_reg_dwsm_vert_en_set(td_u32 id, td_u32 reg_dwsm_vert_en)
{
    td_u32 *reg_addr = TD_NULL;
    video_dwsm_ctrl dwsm_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dwsm_ctrl.u32);
    dwsm_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dwsm_ctrl.bits.reg_dwsm_vert_en = reg_dwsm_vert_en;
    hdmi_tx_reg_write(reg_addr, dwsm_ctrl.u32);

    return;
}

td_void hdmi_reg_hori_filter_en_set(td_u32 id, td_u32 reg_hori_filter_en)
{
    td_u32 *reg_addr = TD_NULL;
    video_dwsm_ctrl dwsm_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dwsm_ctrl.u32);
    dwsm_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dwsm_ctrl.bits.reg_hori_filter_en = reg_hori_filter_en;
    hdmi_tx_reg_write(reg_addr, dwsm_ctrl.u32);

    return;
}

td_void hdmi_reg_dwsm_hori_en_set(td_u32 id, td_u32 reg_dwsm_hori_en)
{
    td_u32 *reg_addr = TD_NULL;
    video_dwsm_ctrl dwsm_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dwsm_ctrl.u32);
    dwsm_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dwsm_ctrl.bits.reg_dwsm_hori_en = reg_dwsm_hori_en;
    hdmi_tx_reg_write(reg_addr, dwsm_ctrl.u32);

    return;
}

td_void hdmi_reg_pxl_div_en_set(td_u32 id, td_u32 reg_pxl_div_en)
{
    td_u32 *reg_addr = TD_NULL;
    data_align_ctrl align_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->align_ctrl.u32);
    align_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    align_ctrl.bits.reg_pxl_div_en = reg_pxl_div_en;
    hdmi_tx_reg_write(reg_addr, align_ctrl.u32);

    return;
}

td_void hdmi_reg_demux_420_en_set(td_u32 id, td_u32 reg_demux_420_en)
{
    td_u32 *reg_addr = TD_NULL;
    data_align_ctrl align_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->align_ctrl.u32);
    align_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    align_ctrl.bits.reg_demux_420_en = reg_demux_420_en;
    hdmi_tx_reg_write(reg_addr, align_ctrl.u32);

    return;
}

td_void hdmi_reg_inver_sync_set(td_u32 id, td_u32 reg_inver_sync)
{
    td_u32 *reg_addr = TD_NULL;
    video_dmux_ctrl dmux_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dmux_ctrl.u32);
    dmux_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dmux_ctrl.bits.reg_inver_sync = reg_inver_sync;
    hdmi_tx_reg_write(reg_addr, dmux_ctrl.u32);

    return;
}

td_void hdmi_reg_syncmask_en_set(td_u32 id, td_u32 reg_syncmask_en)
{
    td_u32 *reg_addr = TD_NULL;
    video_dmux_ctrl dmux_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dmux_ctrl.u32);
    dmux_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dmux_ctrl.bits.reg_syncmask_en = reg_syncmask_en;
    hdmi_tx_reg_write(reg_addr, dmux_ctrl.u32);

    return;
}

td_void hdmi_reg_vmux_cr_sel_set(td_u32 id, td_u32 reg_vmux_cr_sel)
{
    td_u32 *reg_addr = TD_NULL;
    video_dmux_ctrl dmux_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dmux_ctrl.u32);
    dmux_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dmux_ctrl.bits.reg_vmux_cr_sel = reg_vmux_cr_sel;
    hdmi_tx_reg_write(reg_addr, dmux_ctrl.u32);

    return;
}

td_void hdmi_reg_vmux_cb_sel_set(td_u32 id, td_u32 reg_vmux_cb_sel)
{
    td_u32 *reg_addr = TD_NULL;
    video_dmux_ctrl dmux_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dmux_ctrl.u32);
    dmux_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dmux_ctrl.bits.reg_vmux_cb_sel = reg_vmux_cb_sel;
    hdmi_tx_reg_write(reg_addr, dmux_ctrl.u32);

    return;
}

td_void hdmi_reg_vmux_y_sel_set(td_u32 id, td_u32 reg_vmux_y_sel)
{
    td_u32 *reg_addr = TD_NULL;
    video_dmux_ctrl dmux_ctrl;

    reg_addr = (td_u32 *)&(g_video_path_all_reg[id]->vo_dmux_ctrl.u32);
    dmux_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    dmux_ctrl.bits.reg_vmux_y_sel = reg_vmux_y_sel;
    hdmi_tx_reg_write(reg_addr, dmux_ctrl.u32);

    return;
}

