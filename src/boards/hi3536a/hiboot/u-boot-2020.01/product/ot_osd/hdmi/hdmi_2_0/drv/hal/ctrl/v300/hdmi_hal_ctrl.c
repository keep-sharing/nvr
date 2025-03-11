// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "hdmi_hal_ctrl.h"
#include "hdmi_reg_tx.h"
#include "hdmi_reg_video_path.h"
#include "hdmi_reg_aon.h"
#include "hdmi_reg_ctrl.h"
#include "hdmi_hal_ddc.h"
#include "hdmi_hal_intf.h"
#include "hdmi_reg_crg.h"
#include "hdmi_product_define.h"
#include "securec.h"

#define CTRL_CHANNEL0_Y          0x0
#define CTRL_CHANNEL0_Y422       0x3
#define CTRL_CHANNEL1_CB         0x1
#define CTRL_CHANNEL1_Y422       0x4
#define CTRL_CHANNEL2_CR         0x2
#define CTRL_CHANNEL2_Y422       0x3
#define CTRL_COLORMETRY_OUT_MASK 0xfc
#define CTRL_COLORMETRY_OUT_BIT  0    /* out colormetry offset in reg_csc_mode */
#define CTRL_COLORMETRY_IN_MASK  0xcf
#define CTRL_COLORMETRY_IN_BIT   0x4  /* in colormetry offset in reg_csc_mode */
#define CTRL_COLORMETRY_MASK     0x3
#define CTRL_RGB_OUT_BIT         0x3  /* out color space offset in reg_csc_mode */
#define CTRL_RGB_IN_BIT          0x7  /* in color space offset in reg_csc_mode */
#define CTRL_QUANTIZAION_OUT_BIT 0x2  /* out quantization offset in reg_csc_mode */
#define CTRL_QUANTIZAION_IN_BIT  0x6  /* in quantization offset in reg_csc_mode */
#define CTRL_SYCN_POL_V_BIT      0    /* vsync offset in reg_inver_sync */
#define CTRL_SYCN_POL_H_BIT      1    /* hsync offset in reg_inver_sync */
#define CTRL_SYCN_POL_DE_BIT     0x3  /* DE offset in reg_inver_sync */
#define CTRL_DATA_RESET_DELAY    5

typedef struct {
    td_bool in_rgb;
    td_bool out_rgb;
    td_bool csc_enable;
    td_bool y422_enable;
    td_bool y420_enable;
    td_bool dwsm_hori_enable;
    td_bool dwsm_vert_enable;
} ctrl_colorspace_en;

static hdmi_ctrl_info g_ctrl_info[HDMI_DEVICE_ID_BUTT];

static hdmi_ctrl_info *ctrl_info_get(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        return &g_ctrl_info[hdmi_id];
    }
    return TD_NULL;
}

static td_void ctrl_avmute_set(td_u32 id, const hdmi_avmute_cfg *avmute_cfg)
{
    hdmi_cea_cp_en_set(id, TD_FALSE);
    /* set GCP set mute & clr mute */
    hdmi_cp_set_avmute_set(id, avmute_cfg->mute_set);
    hdmi_cp_clr_avmute_set(id, avmute_cfg->mute_clr);
    /* set GCP avmute funtion enable, sent loop */
    hdmi_cea_cp_rpt_cnt_set(id, avmute_cfg->rpt_cnt);
    hdmi_cea_cp_rpt_en_set(id, avmute_cfg->mute_rpt_en);
    hdmi_cea_cp_en_set(id, avmute_cfg->mute_pkg_en);

    return;
}

static td_void ctrl_videopath_deep_clr_set(td_u32 id, td_bool enalbe_gcp, hdmi_deep_color deep_color_bit)
{
    hdmi_tx_tmds_pack_mode_set(id, deep_color_bit);
    hdmi_avmixer_config_dc_pkt_en_set(id, enalbe_gcp);
    return;
}

static td_void ctrl_videopath_polarity_set(td_u32 id, const hdmi_sync_pol *out_sync_pol)
{
    td_u32 sync_pol_cfg = 0;

    if (out_sync_pol->h_pol_invert) {
        hdmi_set_bit(sync_pol_cfg, CTRL_SYCN_POL_H_BIT);
    } else {
        hdmi_clr_bit(sync_pol_cfg, CTRL_SYCN_POL_H_BIT);
    }
    if (out_sync_pol->v_pol_invert) {
        hdmi_set_bit(sync_pol_cfg, CTRL_SYCN_POL_V_BIT);
    } else {
        hdmi_clr_bit(sync_pol_cfg, CTRL_SYCN_POL_V_BIT);
    }
    if (out_sync_pol->de_invert) {
        hdmi_set_bit(sync_pol_cfg, CTRL_SYCN_POL_DE_BIT);
    } else {
        hdmi_clr_bit(sync_pol_cfg, CTRL_SYCN_POL_DE_BIT);
    }
    hdmi_reg_inver_sync_set(id, sync_pol_cfg);
    hdmi_reg_syncmask_en_set(id, TD_FALSE);

    return;
}

static td_void ctrl_videopath_dither_set(td_u32 id, td_bool enable, hdmi_video_dither dither_mode)
{
    hdmi_dither_rnd_byp_set(id, !enable);
    hdmi_dither_mode_set(id, dither_mode);
    return;
}

static td_void ctrl_video_color_rgb_set(td_u32 id, td_bool rgb_in, td_bool rgb_out)
{
    td_u32 reg_value;

    reg_value = hdmi_reg_csc_mode_get(id);
    if (rgb_in) {
        hdmi_set_bit(reg_value, CTRL_RGB_IN_BIT);
    } else {
        hdmi_clr_bit(reg_value, CTRL_RGB_IN_BIT);
    }
    if (rgb_out) {
        hdmi_set_bit(reg_value, CTRL_RGB_OUT_BIT);
    } else {
        hdmi_clr_bit(reg_value, CTRL_RGB_OUT_BIT);
    }
    hdmi_reg_csc_mode_set(id, reg_value);

    return;
}

static td_void ctrl_video_color_csc_set(td_u32 id, td_bool csc_enable)
{
    hdmi_reg_csc_en_set(id, csc_enable);
    return;
}

static td_void ctrl_video_color_ycbcr422_set(td_u32 id, td_bool ycbcr422)
{
    if (ycbcr422) {
        hdmi_reg_vmux_y_sel_set(id, CTRL_CHANNEL0_Y422);
        hdmi_reg_vmux_cb_sel_set(id, CTRL_CHANNEL1_Y422);
        hdmi_reg_vmux_cr_sel_set(id, CTRL_CHANNEL2_Y422);
    } else {
        hdmi_reg_vmux_y_sel_set(id, CTRL_CHANNEL0_Y);
        hdmi_reg_vmux_cb_sel_set(id, CTRL_CHANNEL1_CB);
        hdmi_reg_vmux_cr_sel_set(id, CTRL_CHANNEL2_CR);
    }

    return;
}

static td_void ctrl_video_color_ycbcr420_set(td_u32 id, td_bool ycbcr420)
{
    hdmi_reg_demux_420_en_set(id, ycbcr420);
    hdmi_reg_pxl_div_en_set(id, ycbcr420);
    return;
}

static td_void ctrl_video_color_dwsm_vert_set(td_u32 id, td_bool dwsm_vert)
{
    hdmi_reg_dwsm_vert_byp_set(id, !dwsm_vert);
    hdmi_reg_dwsm_vert_en_set(id, dwsm_vert);
    return;
}

static td_void ctrl_video_color_dwsm_hori_set(td_u32 id, td_bool dwsm_hori)
{
    hdmi_reg_hori_filter_en_set(id, dwsm_hori);
    hdmi_reg_dwsm_hori_en_set(id, dwsm_hori);
    return;
}

static td_void ctrl_videopath_colormetry_set(td_u32 id, hdmi_colormetry in, hdmi_colormetry out)
{
    td_u32 csc_mode;

    csc_mode = hdmi_reg_csc_mode_get(id);
    out = (td_u32)out & CTRL_COLORMETRY_MASK;
    csc_mode &= CTRL_COLORMETRY_OUT_MASK;
    csc_mode |= (td_u32)out << CTRL_COLORMETRY_OUT_BIT;
    in = (td_u32)in & CTRL_COLORMETRY_MASK;
    csc_mode &= CTRL_COLORMETRY_IN_MASK;
    csc_mode |= (td_u32)in << CTRL_COLORMETRY_IN_BIT;
    hdmi_reg_csc_mode_set(id, csc_mode);

    return;
}

static td_void videopath_colorspace_param_get(hdmi_colorspace in, hdmi_colorspace out, ctrl_colorspace_en *en)
{
    if (in == HDMI_COLORSPACE_YCBCR444) {
        switch (out) {
            case HDMI_COLORSPACE_YCBCR422:
                en->y422_enable = TD_TRUE;
                en->dwsm_hori_enable = TD_TRUE;
                break;
            case HDMI_COLORSPACE_YCBCR420:
                en->dwsm_hori_enable = TD_TRUE;
                en->y420_enable      = TD_TRUE;
                en->dwsm_vert_enable = TD_TRUE;
                break;
            case HDMI_COLORSPACE_RGB:
                en->out_rgb    = TD_TRUE;
                en->csc_enable = TD_TRUE;
                break;
            default:
                break;
        }
    } else if (in == HDMI_COLORSPACE_RGB) {
        switch (out) {
            case HDMI_COLORSPACE_YCBCR444:
                en->in_rgb     = TD_TRUE;
                en->csc_enable = TD_TRUE;
                break;
            case HDMI_COLORSPACE_YCBCR422:
                en->in_rgb           = TD_TRUE;
                en->csc_enable       = TD_TRUE;
                en->y422_enable      = TD_TRUE;
                en->dwsm_hori_enable = TD_TRUE;
                break;
            case HDMI_COLORSPACE_YCBCR420:
                en->in_rgb           = TD_TRUE;
                en->csc_enable       = TD_TRUE;
                en->dwsm_hori_enable = TD_TRUE;
                en->y420_enable      = TD_TRUE;
                en->dwsm_vert_enable = TD_TRUE;
                break;
            default:
                en->in_rgb  = TD_TRUE;
                en->out_rgb = TD_TRUE;
                break;
        }
    } else {
        hdmi_warn("invalid expected colorspace=%u\n", in);
    }
    return;
}

static td_void ctrl_videopath_colorspace_set(td_u32 id, hdmi_colorspace in, hdmi_colorspace out)
{
    ctrl_colorspace_en colorspace_en = {0};

    videopath_colorspace_param_get(in, out, &colorspace_en);
    hdmi_info("csc set:\n"
              "in_rgb=%u, out_rgb=%u, csc_enable=%u \n"
              "y422_enable=%u, dwsm_hori_enable=%u \n"
              "y420_enable=%u, dwsm_vert_enable=%u \n",
              colorspace_en.in_rgb, colorspace_en.out_rgb, colorspace_en.csc_enable,
              colorspace_en.y422_enable, colorspace_en.dwsm_hori_enable,
              colorspace_en.y420_enable, colorspace_en.dwsm_vert_enable);

    ctrl_video_color_rgb_set(id, colorspace_en.in_rgb, colorspace_en.out_rgb);
    ctrl_video_color_ycbcr422_set(id, colorspace_en.y422_enable);
    ctrl_video_color_dwsm_hori_set(id, colorspace_en.dwsm_hori_enable);
    ctrl_video_color_ycbcr420_set(id, colorspace_en.y420_enable);
    ctrl_video_color_dwsm_vert_set(id, colorspace_en.dwsm_vert_enable);
    ctrl_video_color_csc_set(id, colorspace_en.csc_enable);

    return;
}

static td_void ctrl_videopath_quantization_set(td_u32 id, hdmi_quantization in, hdmi_quantization out)
{
    td_u32 csc_mode;

    csc_mode = hdmi_reg_csc_mode_get(id);
    if (in != HDMI_QUANTIZATION_RANGE_LIMITED) {
        hdmi_set_bit(csc_mode, CTRL_QUANTIZAION_IN_BIT);
    } else {
        hdmi_clr_bit(csc_mode, CTRL_QUANTIZAION_IN_BIT);
    }
    if (out != HDMI_QUANTIZATION_RANGE_LIMITED) {
        hdmi_set_bit(csc_mode, CTRL_QUANTIZAION_OUT_BIT);
    } else {
        hdmi_clr_bit(csc_mode, CTRL_QUANTIZAION_OUT_BIT);
    }
    hdmi_reg_csc_mode_set(id, csc_mode);
    hdmi_reg_csc_saturate_en_set(id, TD_TRUE);

    return;
}

static td_void ctrl_vendor_infoframe_en_set(td_u32 id, td_bool enable)
{
    hdmi_cea_vsif_rpt_en_set(id, enable);
    hdmi_cea_vsif_en_set(id, enable);
    return;
}

static td_void ctrl_vendor_infoframe_data_set(td_u32 id, const td_u8 *if_data)
{
    /* HB config */
    hdmi_vsif_pkt_header_hb_set(id, if_data[VENDOR_OFFSET_TYPE], if_data[VENDOR_OFFSET_VERSION],
                                if_data[VENDOR_OFFSET_LENGTH]);
    /* pkt0 config */
    hdmi_l_vsif_pkt0_pb_set(id, if_data[VENDOR_OFFSET_CHECSUM], if_data[VENDOR_OFFSET_IEEE_LOWER],
                            if_data[VENDOR_OFFSET_IEEE_UPPER], if_data[VENDOR_OFFSET_IEEE]);
    hdmi_h_vsif_pkt0_pb_set(id, if_data[VENDOR_OFFSET_FMT],
        if_data[VENDOR_OFFSET_VIC], if_data[VENDOR_OFFSET_3D_STRUCT]);
    /* pkt1 config */
    hdmi_l_vsif_pkt1_pb_set(id, if_data[VENDOR_OFFSET_EXT_DATA], if_data[VENDOR_OFFSET_PB7],
                            if_data[VENDOR_OFFSET_PB8], if_data[VENDOR_OFFSET_PB9]);
    hdmi_h_vsif_pkt1_pb_set(id, if_data[VENDOR_OFFSET_PB10], if_data[VENDOR_OFFSET_PB11], if_data[VENDOR_OFFSET_PB12]);
    /* pkt2 config */
    hdmi_l_vsif_pkt2_pb_set(id, if_data[VENDOR_OFFSET_PB13], if_data[VENDOR_OFFSET_PB14], if_data[VENDOR_OFFSET_PB15],
                            if_data[VENDOR_OFFSET_PB16]);
    hdmi_h_vsif_pkt2_pb_set(id, if_data[VENDOR_OFFSET_PB17], if_data[VENDOR_OFFSET_PB18], if_data[VENDOR_OFFSET_PB19]);
    /* pkt3 config */
    hdmi_l_vsif_pkt3_pb_set(id, if_data[VENDOR_OFFSET_PB20], if_data[VENDOR_OFFSET_PB21], if_data[VENDOR_OFFSET_PB22],
                            if_data[VENDOR_OFFSET_PB23]);
    hdmi_h_vsif_pkt3_pb_set(id, if_data[VENDOR_OFFSET_PB24], if_data[VENDOR_OFFSET_PB25], if_data[VENDOR_OFFSET_PB26]);

    return;
}

static td_void ctrl_avi_infoframe_en_set(td_u32 id, td_bool enable)
{
    hdmi_cea_avi_rpt_en_set(id, enable);
    hdmi_cea_avi_en_set(id, enable);
    return;
}

static td_void ctrl_avi_infoframe_data_set(td_u32 id, const td_u8 *if_data)
{
    hdmi_avi_pkt_header_hb_set(id, if_data[AVI_OFFSET_TYPE], if_data[AVI_OFFSET_VERSION], if_data[AVI_OFFSET_LENGTH]);
    hdmi_l_avi_pkt0_pb_set(id, if_data[AVI_OFFSET_CHECKSUM], if_data[AVI_OFFSET_PB1], if_data[AVI_OFFSET_PB2],
                           if_data[AVI_OFFSET_PB3]);
    hdmi_h_avi_pkt0_pb_set(id, if_data[AVI_OFFSET_VIC], if_data[AVI_OFFSET_PB5], if_data[AVI_OFFSET_TOP_BAR_LOWER]);
    hdmi_l_avi_pkt1_pb_set(id, if_data[AVI_OFFSET_TOP_BAR_UPPER], if_data[AVI_OFFSET_BOTTOM_BAR_LOWER],
                           if_data[AVI_OFFSET_BOTTOM_BAR_UPPER], if_data[AVI_OFFSET_LEFT_BAR_LOWER]);
    hdmi_h_avi_pkt1_pb_set(id, if_data[AVI_OFFSET_LEFT_BAR_UPPER], if_data[AVI_OFFSET_RIGHT_BAR_LOWER],
                           if_data[AVI_OFFSET_RIGHT_BAR_UPPER]);
    hdmi_l_avi_pkt2_pb_set(id, if_data[AVI_OFFSET_PB14], if_data[AVI_OFFSET_PB15], if_data[AVI_OFFSET_PB16],
                           if_data[AVI_OFFSET_PB17]);
    hdmi_h_avi_pkt2_pb_set(id, if_data[AVI_OFFSET_PB18], if_data[AVI_OFFSET_PB19], if_data[AVI_OFFSET_PB20]);
    hdmi_l_avi_pkt3_pb_set(id, if_data[AVI_OFFSET_PB21], if_data[AVI_OFFSET_PB22], if_data[AVI_OFFSET_PB23],
                           if_data[AVI_OFFSET_PB24]);
    hdmi_h_avi_pkt3_pb_set(id, if_data[AVI_OFFSET_PB25], if_data[AVI_OFFSET_PB26], if_data[AVI_OFFSET_PB27]);

    return;
}

static td_s32 ctrl_tmds_mode_set(td_u32 id, hdmi_tmds_mode tmds_mode)
{
    switch (tmds_mode) {
        case HDMI_TMDS_MODE_DVI:
            hdmi_avmixer_config_hdmi_mode_set(id, TD_FALSE);
            break;
        case HDMI_TMDS_MODE_HDMI_1_4:
            hdmi_avmixer_config_hdmi_mode_set(id, TD_TRUE);
            hdmi_enc_hdmi2_on_set(id, TD_FALSE);
            break;
        case HDMI_TMDS_MODE_HDMI_2_0:
            hdmi_avmixer_config_hdmi_mode_set(id, TD_TRUE);
            hdmi_enc_hdmi2_on_set(id, TD_TRUE);
            hdmi_enc_bypass_set(id, TD_FALSE);
            break;
        default:
            hdmi_warn("hdmi%u :un-known tmds mode:%u\n", id, tmds_mode);
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void ctrl_null_packet_set(td_u32 id, td_bool enable)
{
    hdmi_avmixer_config_null_pkt_en_set(id, enable);
    return;
}

td_s32 hal_hdmi_ctrl_init(hdmi_device_id hdmi_id, const hdmi_hal_init *hal_init)
{
    td_s32 ret = TD_SUCCESS;
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);

    hdmi_if_null_return(ctrl_info, TD_FAILURE);
    if (!ctrl_info->init) {
        hdmi_reg_crg_init(hdmi_id);
        ret += hdmi_tx_aon_regs_init(hdmi_id);
        ret += hdmi_tx_ctrl_regs_init(hdmi_id);
        ret += hdmi_tx_hdmi_regs_init(hdmi_id);
        ret += hdmi_videopath_regs_init(hdmi_id);
        ctrl_info->init = TD_TRUE;
        drv_hdmi_prod_crg_gate_set(hdmi_id, TD_TRUE);
    }

    return (ret != TD_SUCCESS) ? TD_FAILURE : TD_SUCCESS;
}

td_void hal_hdmi_ctrl_deinit(hdmi_device_id hdmi_id)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);

    hdmi_if_null_return_void(ctrl_info);
    hdmi_if_false_return_void(ctrl_info->init);

    hdmi_tx_aon_regs_deinit(hdmi_id);
    hdmi_tx_ctrl_regs_deinit(hdmi_id);
    hdmi_tx_hdmi_regs_deinit(hdmi_id);
    hdmi_videopath_regs_deinit(hdmi_id);
    (td_void)memset_s(ctrl_info, sizeof(hdmi_ctrl_info), 0, sizeof(hdmi_ctrl_info));
    ctrl_info->init = TD_FALSE;

    return;
}

td_void hal_hdmi_ctrl_avmute_set(hdmi_device_id hdmi_id, const hdmi_avmute_cfg *avmute_cfg)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);

    hdmi_if_null_return_void(avmute_cfg);
    hdmi_if_null_return_void(ctrl_info);
    ctrl_avmute_set(hdmi_id, avmute_cfg);

    return;
}

td_void hal_hdmi_ctrl_csc_set(hdmi_device_id hdmi_id, const hdmi_video_config *video_cfg)
{
    hdmi_if_null_return_void(video_cfg);

    ctrl_videopath_colormetry_set(hdmi_id, video_cfg->conv_std, video_cfg->conv_std);
    ctrl_videopath_quantization_set(hdmi_id, video_cfg->out_csc_quantization, video_cfg->out_csc_quantization);
    ctrl_videopath_colorspace_set(hdmi_id, video_cfg->in_colorspace, video_cfg->out_colorspace);

    return;
}

static td_void ctrl_video_dither_set(td_u32 id, hdmi_video_path *path)
{
    td_bool enable_gcp = TD_FALSE;
    td_bool enable_dither = TD_FALSE;
    hdmi_video_dither dither_mode = HDMI_VIDEO_DITHER_DISABLE;

    if (path->out_deep_color != HDMI_DEEP_COLOR_30BIT && path->out_deep_color != HDMI_DEEP_COLOR_36BIT) {
        path->out_deep_color = HDMI_DEEP_COLOR_24BIT;
    }
    /* dither */
    switch (path->out_deep_color) {
        case HDMI_DEEP_COLOR_24BIT:
            enable_gcp = TD_FALSE;
            switch (path->in_deep_color) {
                case HDMI_DEEP_COLOR_30BIT:
                    enable_dither = TD_TRUE;
                    dither_mode = HDMI_VIDEO_DITHER_10_8;
                    break;
                case HDMI_DEEP_COLOR_36BIT:
                    enable_dither = TD_TRUE;
                    dither_mode = HDMI_VIDEO_DITHER_12_8;
                    break;
                default:
                    enable_dither = TD_FALSE;
                    break;
            }
            break;
        case HDMI_DEEP_COLOR_30BIT:
            enable_gcp = TD_TRUE;
            if (path->in_deep_color == HDMI_DEEP_COLOR_36BIT) {
                enable_dither = TD_TRUE;
                dither_mode = HDMI_VIDEO_DITHER_12_10;
            } else {
                enable_dither = TD_FALSE;
            }
            break;
        case HDMI_DEEP_COLOR_36BIT:
            enable_gcp = TD_TRUE;
            enable_dither = TD_FALSE;
            break;
        default:
            break;
    }
    if (path->timing == HDMI_VIDEO_TIMING_1440X480I_60000 || path->timing == HDMI_VIDEO_TIMING_1440X576I_50000 ||
        path->out_colorspace == HDMI_COLORSPACE_YCBCR422) {
        enable_dither = TD_FALSE;
        dither_mode = HDMI_VIDEO_DITHER_DISABLE;
    }
    ctrl_videopath_dither_set(id, enable_dither, dither_mode);
    ctrl_videopath_deep_clr_set(id, enable_gcp, path->out_deep_color);

    return;
}

td_void hal_hdmi_ctrl_videopath_set(hdmi_device_id hdmi_id, hdmi_video_path *video_path)
{
    errno_t ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);

    hdmi_if_null_return_void(video_path);
    hdmi_if_null_return_void(ctrl_info);

    ret = memcpy_s(&ctrl_info->video_path, sizeof(ctrl_info->video_path), video_path, sizeof(hdmi_video_path));
    hdmi_unequal_eok(ret);

    ctrl_video_dither_set(hdmi_id, video_path);
    ctrl_videopath_polarity_set(hdmi_id, &video_path->out_hv_sync_pol);
    ctrl_videopath_colormetry_set(hdmi_id, video_path->in_colormetry, video_path->out_colormetry);
    ctrl_videopath_quantization_set(hdmi_id, video_path->in_quantization, video_path->out_quantization);
    ctrl_videopath_colorspace_set(hdmi_id, video_path->in_colorspace, video_path->out_colorspace);

    return;
}

td_s32 hal_hdmi_ctrl_tmds_mode_set(hdmi_device_id hdmi, hdmi_tmds_mode tmds_mode)
{
    td_s32 ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi);

    hdmi_if_null_return(ctrl_info, TD_FAILURE);
    hdmi_if_false_return(ctrl_info->init, TD_FAILURE);
    ret = ctrl_tmds_mode_set(hdmi, tmds_mode);

    return ret;
}

td_s32 hal_hdmi_ctrl_infoframe_en_set(hdmi_device_id hdmi_id, hdmi_infoframe_id infoframe_id, td_bool enable)
{
    td_s32 ret = TD_SUCCESS;
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);

    hdmi_if_null_return(ctrl_info, TD_FAILURE);
    hdmi_if_false_return(ctrl_info->init, TD_FAILURE);

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_NULL:
            ctrl_null_packet_set(hdmi_id, enable);
            break;
        case HDMI_INFOFRAME_TYPE_VENDOR:
            ctrl_vendor_infoframe_en_set(hdmi_id, enable);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            ctrl_avi_infoframe_en_set(hdmi_id, enable);
            break;
        default:
            hdmi_warn("invalid support infoframe type:%u!\n", infoframe_id);
            ret = TD_FAILURE;
            break;
    }

    return ret;
}

td_s32 hal_hdmi_ctrl_infoframe_data_set(hdmi_device_id hdmi_id, hdmi_infoframe_id infoframe_id, const td_u8 *if_data)
{
    td_s32 ret = TD_SUCCESS;
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);

    hdmi_if_null_return(ctrl_info, TD_FAILURE);
    hdmi_if_null_return(if_data, TD_FAILURE);
    hdmi_if_false_return(ctrl_info->init, TD_FAILURE);

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            ctrl_vendor_infoframe_data_set(hdmi_id, if_data);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            ctrl_avi_infoframe_data_set(hdmi_id, if_data);
            break;
        default:
            hdmi_warn("un-support infoframe type:%u!\n", infoframe_id);
            ret = TD_FAILURE;
            break;
    }

    return ret;
}

td_void hal_hdmi_ctrl_data_reset(hdmi_device_id hdmi_id, td_bool debug_mode, td_u32 delay_ms)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);
    static td_u32 delay = 0;

    hdmi_if_null_return_void(ctrl_info);
    hdmi_if_false_return_void(ctrl_info->init);

    hdmi_pwd_tx_acr_srst_req_set(hdmi_id, TD_TRUE);
    hdmi_pwd_tx_afifo_srst_req_set(hdmi_id, TD_TRUE);
    hdmi_pwd_tx_aud_srst_req_set(hdmi_id, TD_TRUE);
    hdmi_pwd_tx_hdmi_srst_req_set(hdmi_id, TD_TRUE);
    udelay(CTRL_DATA_RESET_DELAY);
    hdmi_pwd_tx_acr_srst_req_set(hdmi_id, TD_FALSE);
    hdmi_pwd_tx_afifo_srst_req_set(hdmi_id, TD_FALSE);
    hdmi_pwd_tx_aud_srst_req_set(hdmi_id, TD_FALSE);
    hdmi_pwd_tx_hdmi_srst_req_set(hdmi_id, TD_FALSE);
    /* at least udelay(80) */
    if (debug_mode) {
        delay = delay_ms;
    }
    if (delay) {
        mdelay(delay);
        hdmi_info("dataRst %u ms\n", delay);
    }

    return;
}

td_void hal_hdmi_ctrl_tmds_stable_get(hdmi_device_id hdmi_id, td_bool *stable)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_get(hdmi_id);

    hdmi_if_null_return_void(stable);
    hdmi_if_null_return_void(ctrl_info);
    hdmi_if_false_return_void(ctrl_info->init);
    *stable = hdmi_tx_pclk2tclk_stable_get(hdmi_id) ? TD_TRUE : TD_FALSE;
    return;
}

