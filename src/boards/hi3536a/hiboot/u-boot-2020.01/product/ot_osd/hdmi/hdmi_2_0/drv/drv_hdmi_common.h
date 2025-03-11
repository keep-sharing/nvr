// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __DRV_HDMI_COMMON_H__
#define __DRV_HDMI_COMMON_H__

#include "ot_type.h"
#include "ot_hdmi.h"
#include "drv_hdmi_infoframe.h"

#ifdef HDMI_FPGA_SUPPORT
#define FPGA_SUPPORT TD_TRUE
#else
#define FPGA_SUPPORT TD_FALSE
#endif

/* AVI InfoFrame Packet byte offset define */
#define AVI_OFFSET_TYPE 0
#define AVI_OFFSET_VERSION 1
#define AVI_OFFSET_LENGTH 2
#define AVI_OFFSET_CHECKSUM 3
/*
 * include :
 * color space
 * active information present
 * bar Info data valid
 * scan Information
 */
#define AVI_OFFSET_PB1 4
/*
 * include :
 * colorimetry
 * picture aspect ratio
 * active format aspect ratio
 */
#define AVI_OFFSET_PB2 5
/*
 * include :
 * IT content
 * extended colorimetry
 * quantization range
 * non-uniform picture scaling
 */
#define AVI_OFFSET_PB3 6
#define AVI_OFFSET_VIC 7
/*
 * include :
 * YCC quantization range
 * content type
 * pixel repetition factor
 */
#define AVI_OFFSET_PB5 8
#define AVI_OFFSET_TOP_BAR_LOWER 9
#define AVI_OFFSET_TOP_BAR_UPPER 10
#define AVI_OFFSET_BOTTOM_BAR_LOWER 11
#define AVI_OFFSET_BOTTOM_BAR_UPPER 12
#define AVI_OFFSET_LEFT_BAR_LOWER 13
#define AVI_OFFSET_LEFT_BAR_UPPER 14
#define AVI_OFFSET_RIGHT_BAR_LOWER 15
#define AVI_OFFSET_RIGHT_BAR_UPPER 16
#define AVI_OFFSET_PB14 17
#define AVI_OFFSET_PB15 18
#define AVI_OFFSET_PB16 19
#define AVI_OFFSET_PB17 20
#define AVI_OFFSET_PB18 21
#define AVI_OFFSET_PB19 22
#define AVI_OFFSET_PB20 23
#define AVI_OFFSET_PB21 24
#define AVI_OFFSET_PB22 25
#define AVI_OFFSET_PB23 26
#define AVI_OFFSET_PB24 27
#define AVI_OFFSET_PB25 28
#define AVI_OFFSET_PB26 29
#define AVI_OFFSET_PB27 30
#define AVI_FRAME_COLORIMETRY_MASK 0x3
#define AVI_FRAME_PIC_ASPECT_MASK 0x3
#define AVI_FRAME_ACTIVE_ASPECT_MASK 0xF
#define AVI_FRAME_EXT_COLORIMETRY_MASK 0x7
#define AVI_FRAME_QUANT_RANGE_MASK 0x3
#define AVI_FRAME_YCC_QUANT_RANGE_MASK 0x3
#define AVI_FRAME_PIXEL_REPEAT_MASK 0xF

/* VENDOR InfoFrame Packet byte offset define */
#define VENDOR_OFFSET_TYPE       0
#define VENDOR_OFFSET_VERSION    1
#define VENDOR_OFFSET_LENGTH     2
#define VENDOR_OFFSET_CHECSUM    3
#define VENDOR_OFFSET_IEEE_LOWER 4
#define VENDOR_OFFSET_IEEE_UPPER 5
#define VENDOR_OFFSET_IEEE       6
#define VENDOR_OFFSET_FMT        7
#define VENDOR_OFFSET_VIC        8
#define VENDOR_OFFSET_3D_STRUCT  9
#define VENDOR_OFFSET_EXT_DATA   10
#define VENDOR_OFFSET_PB7        11
#define VENDOR_OFFSET_PB8 12
#define VENDOR_OFFSET_PB9 13
#define VENDOR_OFFSET_PB10 14
#define VENDOR_OFFSET_PB11 15
#define VENDOR_OFFSET_PB12 16
#define VENDOR_OFFSET_PB13 17
#define VENDOR_OFFSET_PB14 18
#define VENDOR_OFFSET_PB15 19
#define VENDOR_OFFSET_PB16 20
#define VENDOR_OFFSET_PB17 21
#define VENDOR_OFFSET_PB18 22
#define VENDOR_OFFSET_PB19 23
#define VENDOR_OFFSET_PB20 24
#define VENDOR_OFFSET_PB21 25
#define VENDOR_OFFSET_PB22 26
#define VENDOR_OFFSET_PB23 27
#define VENDOR_OFFSET_PB24 28
#define VENDOR_OFFSET_PB25 29
#define VENDOR_OFFSET_PB26 30

#define HDMI_1_4_RATIO_1_10 10
#define HDMI_2_0_RATIO_1_40 40
#define CEA861_F_VIDEO_CODES_MAX_4K 4
#define HDMI_HW_PARAM_SIZE 4

#ifdef HDMI_SUPPORT_DUAL_CHANNEL
#define HDMI_ID_MAX HDMI_DEVICE_ID_BUTT
#else
#define HDMI_ID_MAX HDMI_DEVICE_ID1
#endif

#define hdmi_set_bit(var, bit) ((var) |= 1 << (bit))
#define hdmi_clr_bit(var, bit) ((var) &= ~(1 << (bit)))

#ifdef HDMI_SUPPORT_DUAL_CHANNEL
#define OT_HDMI_ID_MAX OT_HDMI_ID_BUTT
#else
#define OT_HDMI_ID_MAX OT_HDMI_ID_1
#endif

#define hdmi_unequal_eok(ret)                              \
    do {                                                   \
        if ((ret) != EOK) {                                \
            hdmi_err("secure function error:%d\n", (ret)); \
        }                                                  \
    } while (0)

#define hdmi_id_check_return(hdmi_id, err_no) \
    do {                                      \
        if ((hdmi_id) >= OT_HDMI_ID_MAX) {    \
            hdmi_err("param err!\n");         \
            return (err_no);                  \
        }                                     \
    } while (0)

#define hdmi_if_null_return(p, ret)                 \
    do {                                            \
        if ((p) == TD_NULL) {                       \
            hdmi_warn("%s is null pointer!\n", #p); \
            return (ret);                           \
        }                                           \
    } while (0)

#define hdmi_if_null_return_void(p)                 \
    do {                                            \
        if ((p) == TD_NULL) {                       \
            hdmi_warn("%s is null pointer!\n", #p); \
            return;                                 \
        }                                           \
    } while (0)

#define hdmi_if_false_return(tmp, ret)         \
    do {                                       \
        if ((tmp) != TD_TRUE) {                \
            hdmi_warn("%s is FALSE!\n", #tmp); \
            return (ret);                      \
        }                                      \
    } while (0)

#define hdmi_if_false_return_void(tmp)         \
    do {                                       \
        if ((tmp) != TD_TRUE) {                \
            hdmi_warn("%s is false!\n", #tmp); \
            return;                            \
        }                                      \
    } while (0)

#define hdmi_if_fpga_return_void()    \
    do {                              \
        if (FPGA_SUPPORT) {           \
            hdmi_warn("FPGA CFG!\n"); \
            return;                   \
        }                             \
    } while (0)

#define hdmi_if_fpga_return(ret)      \
    do {                              \
        if (FPGA_SUPPORT) {           \
            hdmi_warn("FPGA CFG!\n"); \
            return (ret);             \
        }                             \
    } while (0)

#define hdmi_if_failure_return(tmp, ret)         \
    do {                                         \
        if ((tmp) != TD_SUCCESS) {               \
            hdmi_warn("%s is failure!\n", #tmp); \
            return (ret);                        \
        }                                        \
    } while (0)

#define hal_call_void(func, param...)                          \
    do {                                                       \
        if (hdmi_dev != TD_NULL && hdmi_dev->hal != TD_NULL && \
            hdmi_dev->hal->func != TD_NULL) {                  \
            hdmi_dev->hal->func(param);                        \
        } else {                                               \
            hdmi_warn("null pointer! \n");                     \
        }                                                      \
    } while (0)

#define hal_call_ret(ret, func, param...)                      \
    do {                                                       \
        if (hdmi_dev != TD_NULL && hdmi_dev->hal != TD_NULL && \
            hdmi_dev->hal->func != TD_NULL) {                  \
            ret = hdmi_dev->hal->func(param);                  \
        } else {                                               \
            ret = TD_FAILURE;                                  \
        }                                                      \
    } while (0)

typedef enum {
    HDMI_DEVICE_ID0,
    HDMI_DEVICE_ID1,
    HDMI_DEVICE_ID_BUTT
} hdmi_device_id;

typedef enum {
    HDMI_EVENT_HOTPLUG = 0x10,
    HDMI_EVENT_HOTUNPLUG,
    HDMI_EVENT_EDID_FAIL,
    HDMI_EVENT_HDCP_FAIL,
    HDMI_EVENT_HDCP_SUCCESS,
    HDMI_EVENT_RSEN_CONNECT,
    HDMI_EVENT_RSEN_DISCONNECT,
    HDMI_EVENT_HDCP_USERSETTING,
    HDMI_EVENT_HDCP_OFF,
    HDMI_EVENT_SCRAMBLE_FAIL,
    HDMI_EVENT_SCRAMBLE_SUCCESS,
    HDMI_EVENT_ZERO_DRMIF_TIMEOUT,
    HDMI_EVENT_SWITCH_TO_HDRMODE_TIMEOUT,
    HDMI_EVENT_BUTT
} hdmi_event;

typedef td_s32 (*hdmi_callback)(td_void *, hdmi_event);

typedef struct {
    td_void       *event_data;
    hdmi_callback event_callback;
    td_u32        hdmi_dev_id;
    td_char       *base_addr;
    td_char       *phy_addr;
} hdmi_hal_init;

#define hdmi_array_size(a)        ((sizeof(a)) / (sizeof(a[0])))
#define CEAVIDEO_CODES_MAX         44
#define CEA861_FVIDEO_CODES_MAX_4K 4

typedef enum {
    HDMI_DEEP_COLOR_24BIT,
    HDMI_DEEP_COLOR_30BIT,
    HDMI_DEEP_COLOR_36BIT,
    HDMI_DEEP_COLOR_48BIT,
    HDMI_DEEP_COLOR_OFF = 0xff,
    HDMI_DEEP_COLOR_BUTT
} hdmi_deep_color;

typedef enum {
    HDMI_VIDEO_BITDEPTH_8,
    HDMI_VIDEO_BITDEPTH_10,
    HDMI_VIDEO_BITDEPTH_12,
    HDMI_VIDEO_BITDEPTH_16,
    HDMI_VIDEO_BITDEPTH_OFF,
    HDMI_VIDEO_BITDEPTH_BUTT
} hdmi_video_bit_depth;

typedef enum {
    HDMI_HV_SYNC_POL_HPVP,
    HDMI_HV_SYNC_POL_HPVN,
    HDMI_HV_SYNC_POL_HNVP,
    HDMI_HV_SYNC_POL_HNVN,
    HDMI_HV_SYNC_POL_BUTT
} hdmi_hvsync_polarity;

typedef enum {
    HDMI_PICTURE_NON_UNIFORM__SCALING,
    HDMI_PICTURE_SCALING_H,
    HDMI_PICTURE_SCALING_V,
    HDMI_PICTURE_SCALING_HV
} hdmi_picture_scaling;

typedef struct {
    td_u32                      clk_fs;             /* VDP setting(in) */
    td_u32                      hdmi_adapt_pix_clk; /* HDMI adapt setting(out) */
    td_u32                      pixel_repeat;
    td_bool                     v_sync_pol;
    td_bool                     h_sync_pol;
    td_bool                     de_pol;
    hdmi_video_timing           video_timing;
    hdmi_3d_mode                stereo_mode;
    hdmi_colorspace             in_colorspace;
    hdmi_colorimetry            colorimetry;
    hdmi_extended_colorimetry   extended_colorimetry;
    hdmi_quantization           rgb_quantization;
    hdmi_ycc_quantization_range ycc_quantization;
    hdmi_picture_aspect         picture_aspect;
    hdmi_active_aspect          active_aspect;
    hdmi_picture_scaling        picture_scaling;
    hdmi_video_bit_depth        in_bit_depth;
    td_u32                      disp_fmt;
} hdmi_vo_attr;

typedef enum {
    HDMI_TMDS_MODE_NONE,
    HDMI_TMDS_MODE_DVI,
    HDMI_TMDS_MODE_HDMI_1_4,
    HDMI_TMDS_MODE_HDMI_2_0,
    HDMI_TMDS_MODE_AUTO,
    HDMI_TMDS_MODE_HDMI_2_1,
    HDMI_TMDS_MODE_BUTT
} hdmi_tmds_mode;

typedef enum {
    HDMI_DEFAULT_ACTION_NULL,
    HDMI_DEFAULT_ACTION_HDMI,
    HDMI_DEFAULT_ACTION_DVI,
    HDMI_DEFAULT_ACTION_BUTT
} hdmi_default_action;

typedef enum {
    HDMI_VIDEO_DITHER_12_10,
    HDMI_VIDEO_DITHER_12_8,
    HDMI_VIDEO_DITHER_10_8,
    HDMI_VIDEO_DITHER_DISABLE
} hdmi_video_dither;

typedef struct {
    td_bool             enable_hdmi;
    td_bool             enable_video;
    td_bool             enable_audio;
    hdmi_colorspace     out_colorspace;
    hdmi_quantization   out_csc_quantization;
    hdmi_deep_color     deep_color_mode;
    td_bool             bxv_ycc_mode;
    td_bool             enable_avi_infoframe;
    td_bool             enable_spd_infoframe;
    td_bool             enable_mpeg_infoframe;
    td_bool             enable_aud_infoframe;
    td_u32              debug_flag;
    td_bool             hdcp_enable;
    hdmi_default_action hdmi_action;
    td_bool             enable_clr_space_adapt;
    td_bool             enable_deep_clr_adapt;
    td_bool             auth_mode;
    td_bool             enable_drm_infoframe;
} hdmi_app_attr;

typedef struct {
    hdmi_vo_attr  vo_attr;
    hdmi_app_attr app_attr;
} hdmi_attr;

typedef struct {
    td_bool tx_hdmi_14;
    td_bool tx_hdmi_20;
    td_bool tx_hdmi_21;
    td_bool tx_hdcp_14;
    td_bool tx_hdcp_22;
    td_bool tx_rgb444;
    td_bool tx_ycbcr444;
    td_bool tx_ycbcr422;
    td_bool tx_ycbcr420;
    td_bool tx_deep_clr10_bit;
    td_bool tx_deep_clr12_bit;
    td_bool tx_deep_clr16_bit;
    td_bool tx_rgb_ycbcr444;
    td_bool tx_ycbcr444_422;
    td_bool tx_ycbcr422_420;
    td_bool tx_ycbcr420_422;
    td_bool tx_ycbcr422_444;
    td_bool tx_ycbcr444_rgb;
    td_bool tx_scdc;
    td_u32  tx_max_tmds_clk; /* in MHz */
} hdmi_tx_capability;

typedef enum {
    HDMI_CONV_STD_BT_709,
    HDMI_CONV_STD_BT_601,
    HDMI_CONV_STD_BT_2020_NON_CONST_LUMINOUS,
    HDMI_CONV_STD_BT_2020_CONST_LUMINOUS,
    HDMI_CONV_STD_BUTT
} hdmi_conversion_std;

typedef enum {
    HDMI_TRACE_LEN_0, /* 1.0 inch */
    HDMI_TRACE_LEN_1, /* 1.5 inch */
    HDMI_TRACE_LEN_2, /* 2.0 inch */
    HDMI_TRACE_LEN_3, /* 2.5 inch */
    HDMI_TRACE_LEN_4, /* 3.0 inch */
    HDMI_TRACE_LEN_5, /* 3.5 inch */
    HDMI_TRACE_LEN_6, /* 4.0 inch */
    HDMI_TRACE_LEN_7, /* 4.5 inch */
    HDMI_TRACE_LEN_8, /* 5.0 inch */
    HDMI_TRACE_DEFAULT, /* default config */
    HDMI_TRACE_BUTT
} hdmi_trace_len;

typedef struct {
    hdmi_video_timing    timing;
    td_u32               pixel_clk;
    td_u32               tmds_clk;
    td_bool              v_sync_pol;
    td_bool              h_sync_pol;
    td_bool              de_pol;
    hdmi_conversion_std  conv_std;
    hdmi_quantization    quantization;
    hdmi_colorspace      in_colorspace;
    hdmi_colorspace      out_colorspace;
    hdmi_deep_color      deep_color;
    hdmi_video_bit_depth in_bit_depth;
    hdmi_quantization    out_csc_quantization;
    td_bool              emi_enable;
    hdmi_trace_len       trace_len;
} hdmi_video_config;

typedef struct {
    td_bool source_scramble_on;
    td_bool sink_scramble_on;
    td_u8   tmds_bit_clk_ratio;
#if defined (HDMI_SUPPORT_LOGIC_VENDORV100)
    td_bool sink_read_quest;
    td_u32  scramble_timeout;  /* in unit of ms.for [0,200],force to default 200;or,set the value cfg(>200). */
    /* in unit of ms,range[20,200).for [0,20] or >=200,force to default 20;or,set the value cfg[20,200). */
    td_u32  scramble_interval;
#endif
} hdmi_scdc_status;

typedef enum {
    HDMI_VIDEO_UNKNOWN,
    HDMI_VIDEO_PROGRESSIVE,
    HDMI_VIDEO_INTERLACE,
    HDMI_VIDEO_BUTT
} hdmi_video_p_i_type;

typedef struct {
    hdmi_video_code     video_code;
    td_u32              pixclk;
    td_u32              rate;
    td_u32              hactive;
    td_u32              vactive;
    td_u32              hblank;
    td_u32              vblank;
    td_u32              hfront;
    td_u32              hsync;
    td_u32              hback;
    td_u32              vfront;
    td_u32              vsync;
    td_u32              vback;
    hdmi_picture_aspect aspect_ratio;
    hdmi_video_timing   timing;
    hdmi_video_p_i_type pi_type;
    const td_char       *fmt_str;
} hdmi_video_def;

typedef struct {
    hdmi_vsif_vic       hdmi_vic;
    hdmi_video_code     equivalent_video_code;  /* real vic */
    td_u32              pixclk;
    td_u32              rate;
    td_u32              hactive;
    td_u32              vactive;
    hdmi_picture_aspect aspect_ratio;
    hdmi_video_timing   timing;
    hdmi_video_p_i_type pi_type;
    const td_char       *fmt_str;
} hdmi_video_4k_def;

typedef struct {
    td_u32 i_de_main_clk;
    td_u32 i_de_main_data;
    td_u32 i_main_clk;
    td_u32 i_main_data;
    td_u32 ft_cap_clk;
    td_u32 ft_cap_data;
} hdmi_hw_param;

typedef struct {
    hdmi_hw_param hw_param[HDMI_HW_PARAM_SIZE];
} hdmi_hw_spec;

typedef struct {
    td_bool mute_pkg_en;
    td_bool mute_set;
    td_bool mute_clr;
    td_bool mute_rpt_en;
    td_u32  rpt_cnt;
} hdmi_avmute_cfg;

typedef enum {
    HDMI_PHY_MODE_CFG_TMDS,
    HDMI_PHY_MODE_CFG_FRL,
    HDMI_PHY_MODE_CFG_TXFFE
} hdmi_phy_mode_cfg;

typedef struct {
    td_bool emi_en;
    hdmi_trace_len trace_len;
} hdmi_mode_param;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_hw_spec   hw_spec;
} drv_hdmi_hw_spec;

typedef struct {
    td_u32            pixel_clk;
    td_u32            tmds_clk;   /* TMDS colck,in kHz */
    td_bool           emi_enable;
    hdmi_deep_color   deep_color; /* Deep color(color depth)  */
    hdmi_phy_mode_cfg mode_cfg;   /* TMDS/FRL/TxFFE */
    hdmi_trace_len    trace_len;
} hdmi_phy_cfg;

hdmi_video_code drv_hdmi_vic_search(hdmi_video_timing timing, hdmi_picture_aspect aspect, td_bool _3d_enable);

td_void hdmi_reg_write(volatile td_void *reg_addr, td_u32 value);

td_u32 hdmi_reg_read(volatile td_void *reg_addr);

td_void drv_hdmi_milli_delay(td_u32 m_sec);

td_u32 drv_hdmi_vic_to_index(td_u32 vic);

hdmi_video_timing drv_hdmi_video_timing_get(hdmi_video_code vic, hdmi_picture_aspect aspect);

hdmi_video_timing drv_hdmi_vsif_video_timing_get(hdmi_vsif_vic vic, hdmi_picture_aspect aspect);

td_void drv_hdmi_video_codes_4k_get(hdmi_14_vsif_content *h14_vsif_content, hdmi_video_timing timing);

#endif /* __DRV_HDMI_COMMON_H__ */

