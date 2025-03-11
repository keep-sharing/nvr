// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "boot_hdmi_intf.h"
#include "drv_hdmi_common.h"
#include "ot_hdmi.h"
#include "hdmi_product_define.h"
#include "ot_common_vo.h"
#include "hdmi_hal_intf.h"
#include "securec.h"

#define hdmi_multiple_2p0(x)      ((x) = ((x) * 2))
#define hdmi_multiple_1p5(x)      ((x) = ((x) * 3) >> 1)
#define hdmi_multiple_1p25(x)     ((x) = ((x) * 5) >> 2)
#define hdmi_multiple_0p5(x)      ((x) >>= 1)
#define HDMI_MAX_HDMI14_TMDS_RATE 340000
#define HDMI_VIC_VIRTUAL_BASE     255U
#define HDMI_VIC_PC_BASE          ((HDMI_VIC_VIRTUAL_BASE) + 0x100)
#define FMT_VIC_800X600           ((HDMI_VIC_PC_BASE) + 9)
#define FMT_VIC_1024X768          ((HDMI_VIC_PC_BASE) + 13)
#define FMT_VIC_1280X1024         ((HDMI_VIC_PC_BASE) + 25)
#define FMT_VIC_1366X768          ((HDMI_VIC_PC_BASE) + 28)
#define FMT_VIC_1920X1200         ((HDMI_VIC_PC_BASE) + 41)
#define HDMI_INVALID_PIXFREQ      0xffffffff
#define MAX_HDMI_HW_PARAM         63
#define MAX_HDMI_FT_PARAM         4
#define SCRAMBLE_INTERVAL         20
#define SCRAMBLE_TIMEOUT          200
#ifndef HDMI_PLATFORM_FMT
#define FMT_4K_50 VO_OUTPUT_3840X2160_50
#define FMT_4K_60 VO_OUTPUT_3840X2160_60
#else
#define FMT_4K_50 OT_VO_OUT_3840x2160_50
#define FMT_4K_60 OT_VO_OUT_3840x2160_60
#endif

typedef struct {
    td_u32 vic;
    td_u32 pixl_freq; /* KHz */
    td_u32 hor_active;
    td_u32 ver_active;
    td_u32 field_rate; /* 0.01Hz */
    td_u32 format;
} hdmi_fmt_param;

typedef struct {
    td_u32 disp_fmt;
    hdmi_video_timing timing;
} hdmi_fmt_transform;

#define video_2_colorspace(vid_mode, color_space)               \
    do {                                                        \
        if ((vid_mode) == OT_HDMI_VIDEO_MODE_RGB444) {          \
            color_space = HDMI_COLORSPACE_RGB;                  \
        } else if ((vid_mode) == OT_HDMI_VIDEO_MODE_YCBCR422) { \
            color_space = HDMI_COLORSPACE_YCBCR422;             \
        } else {                                                \
            color_space = HDMI_COLORSPACE_YCBCR444;             \
        }                                                       \
    } while (0)

static hdmi_device g_hdmi_dev[HDMI_DEVICE_ID_BUTT];

#ifndef HDMI_PLATFORM_FMT
static const hdmi_fmt_param g_fmt_param_table[] = {
    /*         VIC      PixFreq  HACT  VACT FldRate     EncFmt           */
    {                 0, 106500, 1440,  900, 6000, VO_OUTPUT_1440X900_60  },
    {                 0,  71000, 1280,  800, 6000, VO_OUTPUT_1280X800_60  },
    {                 0, 119000, 1680, 1560, 6000, VO_OUTPUT_1680X1050_60 },
    {                 0, 162000, 1600, 1200, 6000, VO_OUTPUT_1600X1200_60 },
    {                 0, 268500, 2560, 1600, 6000, VO_OUTPUT_2560X1600_60 },
    {                 0, 130000, 2560, 1440, 3000, VO_OUTPUT_2560X1440_30 },
    {                 0, 238750, 2560, 1440, 6000, VO_OUTPUT_2560X1440_60 },
    {                 0, 170000, 1920, 2160, 3000, VO_OUTPUT_1920X2160_30 },
    {                 1,  25175,  640,  480, 6000, VO_OUTPUT_640X480_60   },
    {                 2,  27000,  720,  480, 6000, VO_OUTPUT_480P60       },
    {                 3,  27000,  720,  480, 6000, VO_OUTPUT_480P60       },
    {                 4,  74170, 1280,  720, 6000, VO_OUTPUT_720P60       },
    {                 5,  74170, 1920, 1080, 6000, VO_OUTPUT_1080I60      },
    {                 6,  27000,  720,  480, 6000, VO_OUTPUT_NTSC         },
    {                 7,  27000,  720,  480, 6000, VO_OUTPUT_NTSC         },
    {                16, 148500, 1920, 1080, 6000, VO_OUTPUT_1080P60      },
    {                17,  27000,  720,  576, 5000, VO_OUTPUT_576P50       },
    {                18,  27000,  720,  576, 5000, VO_OUTPUT_576P50       },
    {                19,  74250, 1280,  720, 5000, VO_OUTPUT_720P50       },
    {                20,  74250, 1920, 1080, 5000, VO_OUTPUT_1080I50      },
    {                21,  27000, 1440,  576, 5000, VO_OUTPUT_PAL          },
    {                22,  27000, 1440,  576, 5000, VO_OUTPUT_PAL          },
    {                31, 148500, 1920, 1080, 5000, VO_OUTPUT_1080P50      },
    {                32,  74170, 1920, 1080, 2400, VO_OUTPUT_1080P24      },
    {                33,  74250, 1920, 1080, 2500, VO_OUTPUT_1080P25      },
    {                34,  74170, 1920, 1080, 3000, VO_OUTPUT_1080P30      },
    {                93, 297000, 3840, 2160, 2400, VO_OUTPUT_3840X2160_24 },
    {                94, 297000, 3840, 2160, 2500, VO_OUTPUT_3840X2160_25 },
    {                95, 297000, 3840, 2160, 3000, VO_OUTPUT_3840X2160_30 },
    {                96, 594000, 3840, 2160, 5000, VO_OUTPUT_3840X2160_50 },
    {                97, 594000, 3840, 2160, 6000, VO_OUTPUT_3840X2160_60 },
    {                98, 297000, 4096, 2160, 2400, VO_OUTPUT_4096X2160_24 },
    {                99, 297000, 4096, 2160, 2500, VO_OUTPUT_4096X2160_25 },
    {               100, 297000, 4096, 2160, 3000, VO_OUTPUT_4096X2160_30 },
    {               101, 594000, 4096, 2160, 5000, VO_OUTPUT_4096X2160_50 },
    {               102, 594000, 4096, 2160, 6000, VO_OUTPUT_4096X2160_60 },
    {   FMT_VIC_800X600,  37879,  800,  600, 6000, VO_OUTPUT_800X600_60   },
    {  FMT_VIC_1024X768,  65000, 1024,  768, 6000, VO_OUTPUT_1024X768_60  },
    { FMT_VIC_1280X1024, 108000, 1280, 1024, 6000, VO_OUTPUT_1280X1024_60 },
    {  FMT_VIC_1366X768,  85500, 1360,  768, 6000, VO_OUTPUT_1366X768_60  },
    { FMT_VIC_1920X1200, 154000, 1920, 1200, 6000, VO_OUTPUT_1920X1200_60 }
};

static const hdmi_fmt_transform g_hdmi_timing_transform_table[] = {
    {          VO_OUTPUT_PAL, HDMI_VIDEO_TIMING_1440X576I_50000  },
    {         VO_OUTPUT_NTSC, HDMI_VIDEO_TIMING_1440X480I_60000  },
    {      VO_OUTPUT_1080P24, HDMI_VIDEO_TIMING_1920X1080P_24000 },
    {      VO_OUTPUT_1080P25, HDMI_VIDEO_TIMING_1920X1080P_25000 },
    {      VO_OUTPUT_1080P30, HDMI_VIDEO_TIMING_1920X1080P_30000 },
    {       VO_OUTPUT_720P50, HDMI_VIDEO_TIMING_1280X720P_50000  },
    {       VO_OUTPUT_720P60, HDMI_VIDEO_TIMING_1280X720P_60000  },
    {      VO_OUTPUT_1080I50, HDMI_VIDEO_TIMING_1920X1080I_50000 },
    {      VO_OUTPUT_1080I60, HDMI_VIDEO_TIMING_1920X1080I_60000 },
    {      VO_OUTPUT_1080P50, HDMI_VIDEO_TIMING_1920X1080P_50000 },
    {      VO_OUTPUT_1080P60, HDMI_VIDEO_TIMING_1920X1080P_60000 },
    {       VO_OUTPUT_576P50, HDMI_VIDEO_TIMING_720X576P_50000   },
    {       VO_OUTPUT_480P60, HDMI_VIDEO_TIMING_720X480P_60000   },
    {   VO_OUTPUT_640X480_60, HDMI_VIDEO_TIMING_640X480P_60000   },
    { VO_OUTPUT_3840X2160_24, HDMI_VIDEO_TIMING_3840X2160P_24000 },
    { VO_OUTPUT_3840X2160_25, HDMI_VIDEO_TIMING_3840X2160P_25000 },
    { VO_OUTPUT_3840X2160_30, HDMI_VIDEO_TIMING_3840X2160P_30000 },
    { VO_OUTPUT_3840X2160_50, HDMI_VIDEO_TIMING_3840X2160P_50000 },
    { VO_OUTPUT_3840X2160_60, HDMI_VIDEO_TIMING_3840X2160P_60000 },
    { VO_OUTPUT_4096X2160_24, HDMI_VIDEO_TIMING_4096X2160P_24000 },
    { VO_OUTPUT_4096X2160_25, HDMI_VIDEO_TIMING_4096X2160P_25000 },
    { VO_OUTPUT_4096X2160_30, HDMI_VIDEO_TIMING_4096X2160P_30000 },
    { VO_OUTPUT_4096X2160_50, HDMI_VIDEO_TIMING_4096X2160P_50000 },
    { VO_OUTPUT_4096X2160_60, HDMI_VIDEO_TIMING_4096X2160P_60000 }
};
#else
static const hdmi_fmt_param g_fmt_param_table[] = {
    /*         VIC      PixFreq  HACT  VACT FldRate     EncFmt           */
    {                 0, 106500, 1440,  900, 6000, OT_VO_OUT_1440x900_60  },
    {                 0,  83500, 1280,  800, 6000, OT_VO_OUT_1280x800_60  },
    {                 0, 146250, 1680, 1560, 6000, OT_VO_OUT_1680x1050_60 },
    {                 0, 162000, 1600, 1200, 6000, OT_VO_OUT_1600x1200_60 },
    {                 0, 268500, 2560, 1600, 6000, OT_VO_OUT_2560x1600_60 },
    {                 0, 119375, 2560, 1440, 3000, OT_VO_OUT_2560x1440_30 },
    {                 0, 238750, 2560, 1440, 6000, OT_VO_OUT_2560x1440_60 },
    {                 0, 148500, 1920, 2160, 3000, OT_VO_OUT_1920x2160_30 },
    {                 0, 121750, 1400, 1050, 6000, OT_VO_OUT_1400x1050_60 },
    {                 1,  25175,  640,  480, 6000, OT_VO_OUT_640x480_60   },
    {                 2,  27000,  720,  480, 6000, OT_VO_OUT_480P60       },
    {                 3,  27000,  720,  480, 6000, OT_VO_OUT_480P60       },
    {                 4,  74250, 1280,  720, 6000, OT_VO_OUT_720P60       },
    {                 5,  74250, 1920, 1080, 6000, OT_VO_OUT_1080I60      },
    {                 6,  27000,  720,  480, 6000, OT_VO_OUT_NTSC         },
    {                 7,  27000,  720,  480, 6000, OT_VO_OUT_NTSC         },
    {                16, 148500, 1920, 1080, 6000, OT_VO_OUT_1080P60      },
    {                17,  27000,  720,  576, 5000, OT_VO_OUT_576P50       },
    {                18,  27000,  720,  576, 5000, OT_VO_OUT_576P50       },
    {                19,  74250, 1280,  720, 5000, OT_VO_OUT_720P50       },
    {                20,  74250, 1920, 1080, 5000, OT_VO_OUT_1080I50      },
    {                21,  27000, 1440,  576, 5000, OT_VO_OUT_PAL          },
    {                22,  27000, 1440,  576, 5000, OT_VO_OUT_PAL          },
    {                31, 148500, 1920, 1080, 5000, OT_VO_OUT_1080P50      },
    {                32,  74250, 1920, 1080, 2400, OT_VO_OUT_1080P24      },
    {                33,  74250, 1920, 1080, 2500, OT_VO_OUT_1080P25      },
    {                34,  74250, 1920, 1080, 3000, OT_VO_OUT_1080P30      },
    {                93, 297000, 3840, 2160, 2400, OT_VO_OUT_3840x2160_24 },
    {                94, 297000, 3840, 2160, 2500, OT_VO_OUT_3840x2160_25 },
    {                95, 297000, 3840, 2160, 3000, OT_VO_OUT_3840x2160_30 },
    {                96, 594000, 3840, 2160, 5000, OT_VO_OUT_3840x2160_50 },
    {                97, 594000, 3840, 2160, 6000, OT_VO_OUT_3840x2160_60 },
    {                98, 297000, 4096, 2160, 2400, OT_VO_OUT_4096x2160_24 },
    {                99, 297000, 4096, 2160, 2500, OT_VO_OUT_4096x2160_25 },
    {               100, 297000, 4096, 2160, 3000, OT_VO_OUT_4096x2160_30 },
    {               101, 594000, 4096, 2160, 5000, OT_VO_OUT_4096x2160_50 },
    {               102, 594000, 4096, 2160, 6000, OT_VO_OUT_4096x2160_60 },
    {   FMT_VIC_800X600,  37879,  800,  600, 6000, OT_VO_OUT_800x600_60   },
    {  FMT_VIC_1024X768,  65000, 1024,  768, 6000, OT_VO_OUT_1024x768_60  },
    { FMT_VIC_1280X1024, 108000, 1280, 1024, 6000, OT_VO_OUT_1280x1024_60 },
    {  FMT_VIC_1366X768,  85500, 1360,  768, 6000, OT_VO_OUT_1366x768_60  },
    { FMT_VIC_1920X1200, 154000, 1920, 1200, 6000, OT_VO_OUT_1920x1200_60 }
};

static const hdmi_fmt_transform g_hdmi_timing_transform_table[] = {
    {          OT_VO_OUT_PAL, HDMI_VIDEO_TIMING_1440X576I_50000  },
    {         OT_VO_OUT_NTSC, HDMI_VIDEO_TIMING_1440X480I_60000  },
    {      OT_VO_OUT_1080P24, HDMI_VIDEO_TIMING_1920X1080P_24000 },
    {      OT_VO_OUT_1080P25, HDMI_VIDEO_TIMING_1920X1080P_25000 },
    {      OT_VO_OUT_1080P30, HDMI_VIDEO_TIMING_1920X1080P_30000 },
    {       OT_VO_OUT_720P50, HDMI_VIDEO_TIMING_1280X720P_50000  },
    {       OT_VO_OUT_720P60, HDMI_VIDEO_TIMING_1280X720P_60000  },
    {      OT_VO_OUT_1080I50, HDMI_VIDEO_TIMING_1920X1080I_50000 },
    {      OT_VO_OUT_1080I60, HDMI_VIDEO_TIMING_1920X1080I_60000 },
    {      OT_VO_OUT_1080P50, HDMI_VIDEO_TIMING_1920X1080P_50000 },
    {      OT_VO_OUT_1080P60, HDMI_VIDEO_TIMING_1920X1080P_60000 },
    {       OT_VO_OUT_576P50, HDMI_VIDEO_TIMING_720X576P_50000   },
    {       OT_VO_OUT_480P60, HDMI_VIDEO_TIMING_720X480P_60000   },
    {   OT_VO_OUT_640x480_60, HDMI_VIDEO_TIMING_640X480P_60000   },
    { OT_VO_OUT_3840x2160_24, HDMI_VIDEO_TIMING_3840X2160P_24000 },
    { OT_VO_OUT_3840x2160_25, HDMI_VIDEO_TIMING_3840X2160P_25000 },
    { OT_VO_OUT_3840x2160_30, HDMI_VIDEO_TIMING_3840X2160P_30000 },
    { OT_VO_OUT_3840x2160_50, HDMI_VIDEO_TIMING_3840X2160P_50000 },
    { OT_VO_OUT_3840x2160_60, HDMI_VIDEO_TIMING_3840X2160P_60000 },
    { OT_VO_OUT_4096x2160_24, HDMI_VIDEO_TIMING_4096X2160P_24000 },
    { OT_VO_OUT_4096x2160_25, HDMI_VIDEO_TIMING_4096X2160P_25000 },
    { OT_VO_OUT_4096x2160_30, HDMI_VIDEO_TIMING_4096X2160P_30000 },
    { OT_VO_OUT_4096x2160_50, HDMI_VIDEO_TIMING_4096X2160P_50000 },
    { OT_VO_OUT_4096x2160_60, HDMI_VIDEO_TIMING_4096X2160P_60000 }
};
#endif

static td_u32 hdmi_pixel_freq_search(td_u32 fmt)
{
    td_u32 i;
    hdmi_fmt_param *fmt_param = TD_NULL;

    hdmi_info("fmt:%u.\n", fmt);
    if (fmt >= VO_OUTPUT_BUTT) {
        return HDMI_INVALID_PIXFREQ;
    }

    for (i = 0; i < hdmi_array_size(g_fmt_param_table); i++) {
        fmt_param = (hdmi_fmt_param *)&g_fmt_param_table[i];
        if ((fmt_param != TD_NULL) && (fmt_param->format == fmt)) {
            return fmt_param->pixl_freq;
        }
    }

    hdmi_info("pix_freq search fail, invalid format = %u\n", fmt);

    return HDMI_INVALID_PIXFREQ;
}

static hdmi_video_timing disp_fmt_to_hdmi_timing(td_u32 fmt)
{
    td_u32 i;

    for (i = 0; i < hdmi_array_size(g_hdmi_timing_transform_table); i++) {
        if (fmt == g_hdmi_timing_transform_table[i].disp_fmt) {
            return g_hdmi_timing_transform_table[i].timing;
        }
    }
    hdmi_info("Non CEA video timing:%u\n", fmt);
    /* 4k2k && vesa */
    return HDMI_VIDEO_TIMING_UNKNOWN;
}

static td_void hdmi_vo_attr_init(hdmi_vo_attr *video_attr)
{
    if (video_attr->video_timing == HDMI_VIDEO_TIMING_640X480P_60000 ||
        video_attr->video_timing == HDMI_VIDEO_TIMING_720X480P_60000 ||
        video_attr->video_timing == HDMI_VIDEO_TIMING_720X576P_50000) {
        video_attr->colorimetry = HDMI_COLORIMETRY_ITU_709;
        video_attr->picture_aspect = HDMI_PICTURE_ASPECT_4_3;
    } else if (video_attr->video_timing <= HDMI_VIDEO_TIMING_4096X2160P_60000 &&
        video_attr->video_timing >= HDMI_VIDEO_TIMING_4096X2160P_24000) {
        video_attr->colorimetry = HDMI_COLORIMETRY_ITU_709;
        video_attr->picture_aspect = HDMI_PICTURE_ASPECT_256_135;
    } else {
        video_attr->colorimetry = HDMI_COLORIMETRY_ITU_709;
        video_attr->picture_aspect = HDMI_PICTURE_ASPECT_16_9;
    }

    video_attr->de_pol = TD_FALSE;
    video_attr->h_sync_pol = TD_FALSE;
    video_attr->v_sync_pol = TD_FALSE;
    video_attr->pixel_repeat = 1;
    if (video_attr->video_timing == HDMI_VIDEO_TIMING_1440X480I_60000 ||
        video_attr->video_timing == HDMI_VIDEO_TIMING_1440X576I_50000) {
        video_attr->pixel_repeat = 2; /* 2: fmt is 1440X576I50 or 1440X480I60, pixel_repeat times 2, for protocol */
    }

    if (video_attr->video_timing <= HDMI_VIDEO_TIMING_640X480P_60000) {
        video_attr->rgb_quantization = HDMI_QUANTIZATION_RANGE_FULL;
    } else {
        video_attr->ycc_quantization = HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
    }

    if (video_attr->video_timing != HDMI_VIDEO_TIMING_UNKNOWN) {
        video_attr->in_colorspace = HDMI_COLORSPACE_YCBCR444;
    } else {
        video_attr->in_colorspace = HDMI_COLORSPACE_RGB;
    }

    video_attr->stereo_mode = HDMI_3D_BUTT;
    video_attr->in_bit_depth = HDMI_VIDEO_BITDEPTH_10;
    video_attr->active_aspect = HDMI_ACTIVE_ASPECT_PICTURE;

    return;
}

static td_s32 disp_fmt_to_video_attr(td_u32 disp_fmt, hdmi_vo_attr *video_attr)
{
    video_attr->video_timing = disp_fmt_to_hdmi_timing(disp_fmt);
    video_attr->clk_fs = hdmi_pixel_freq_search(disp_fmt);
    if (video_attr->clk_fs == HDMI_INVALID_PIXFREQ) {
        return TD_FAILURE;
    }

    hdmi_vo_attr_init(video_attr);

    return TD_SUCCESS;
}

hdmi_device *get_hdmi_device(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        g_hdmi_dev[hdmi_id].hdmi_dev_id = hdmi_id;
        return &g_hdmi_dev[hdmi_id];
    }
    return TD_NULL;
}

static td_void hdmi_device_init(hdmi_device *hdmi_dev)
{
    hdmi_vo_attr *video_attr = TD_NULL;
    hdmi_app_attr *app_attr = TD_NULL;

    video_attr = &hdmi_dev->attr.vo_attr;
    app_attr = &hdmi_dev->attr.app_attr;

    hdmi_dev->tmds_mode = HDMI_TMDS_MODE_HDMI_1_4;
    /* application attribute init */
    app_attr->enable_hdmi = TD_TRUE;
    app_attr->enable_clr_space_adapt = TD_TRUE;
    app_attr->enable_deep_clr_adapt = TD_TRUE;
    app_attr->deep_color_mode = HDMI_DEEP_COLOR_24BIT;

    /* video attribute init */
    video_attr->video_timing = HDMI_VIDEO_TIMING_1280X720P_50000;
    hdmi_vo_attr_init(video_attr);

    hdmi_dev->mode_param.trace_len = HDMI_TRACE_DEFAULT;

    return;
}

#ifdef HDMI_SCDC_SUPPORT
static td_void hdmi_scramble_enable_set(const hdmi_device *hdmi_dev, td_bool enable)
{
    hdmi_scdc_status scdc_status = {0};
    hdmi_scdc_status current_status = {0};

    hdmi_info("scdc enable = %u\n", enable);

    scdc_status.scramble_interval  = SCRAMBLE_INTERVAL;
    scdc_status.scramble_timeout   = SCRAMBLE_TIMEOUT;
    scdc_status.sink_read_quest    = TD_FALSE;
    scdc_status.sink_scramble_on   = enable;
    scdc_status.source_scramble_on = enable;
    scdc_status.tmds_bit_clk_ratio = enable ? HDMI_2_0_RATIO_1_40 : HDMI_1_4_RATIO_1_10;
    hal_call_void(hal_hdmi_scdc_status_get, hdmi_dev->hal, &current_status);

    if (current_status.tmds_bit_clk_ratio != scdc_status.tmds_bit_clk_ratio ||
        current_status.sink_scramble_on != scdc_status.sink_scramble_on ||
        current_status.source_scramble_on != scdc_status.source_scramble_on) {
        hdmi_info("cur clk_ratio=%u, sink_scramble_on=%u, source_scramble=%u, set scdc!\n",
            current_status.tmds_bit_clk_ratio, current_status.sink_scramble_on, current_status.source_scramble_on);
        hal_call_void(hal_hdmi_scdc_status_set, hdmi_dev->hal, &scdc_status);
    }

    return;
}
#endif

static td_void hdmi_mode_schedule(hdmi_device *hdmi_dev)
{
#ifdef HDMI_SCDC_SUPPORT
    td_bool scramble_enable;
#endif

    if (hdmi_dev->attr.vo_attr.hdmi_adapt_pix_clk >= HDMI_MAX_HDMI14_TMDS_RATE) {
        /* enOutColorSpace should change to YCbCr422 for 10bit out. */
        hdmi_dev->tmds_mode = HDMI_TMDS_MODE_HDMI_2_0;
    } else {
        if (hdmi_dev->attr.app_attr.enable_hdmi == TD_TRUE) {
            hdmi_dev->tmds_mode = HDMI_TMDS_MODE_HDMI_1_4;
        } else {
            hdmi_dev->tmds_mode = HDMI_TMDS_MODE_DVI;
            hdmi_dev->attr.app_attr.out_colorspace = HDMI_COLORSPACE_RGB;
        }
    }
    hdmi_info("Tmds mode switch to %d, outSpace=%d\n", hdmi_dev->tmds_mode, hdmi_dev->attr.app_attr.out_colorspace);
    hal_call_void(hal_hdmi_tmds_mode_set, hdmi_dev->hal, hdmi_dev->tmds_mode);
#ifdef HDMI_SCDC_SUPPORT
    scramble_enable = (hdmi_dev->tmds_mode == HDMI_TMDS_MODE_HDMI_2_0) ? TD_TRUE : TD_FALSE;
    hdmi_scramble_enable_set(hdmi_dev, scramble_enable);
#endif

    return;
}

td_s32 drv_hdmi_open(hdmi_device *hdmi_dev)
{
    td_s32 ret;
    hdmi_hal_init hal_init = {0};

    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    /* open hdmi hal module */
    hal_init.hdmi_dev_id = hdmi_dev->hdmi_dev_id;
    ret = hal_hdmi_open(&hal_init, &hdmi_dev->hal);
    hdmi_if_failure_return(ret, TD_FAILURE);

    hdmi_device_init(hdmi_dev);
    hdmi_if_null_return(hdmi_dev->hal, TD_FAILURE);
    hdmi_dev->hal->hal_ctx.hdmi_dev = hdmi_dev;
    hdmi_dev->hal->hal_ctx.hdmi_id = hdmi_dev->hdmi_dev_id;
    hal_call_void(hal_hdmi_hardware_init, hdmi_dev->hal);

    return TD_SUCCESS;
}

td_void drv_hdmi_avmute_set(const hdmi_device *hdmi_dev, td_bool avmute)
{
    hal_call_void(hal_hdmi_avmute_set, hdmi_dev->hal, avmute);
    return;
}

td_s32 drv_hdmi_start(hdmi_device *hdmi_dev)
{
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    hdmi_mode_schedule(hdmi_dev);
    hal_call_void(hal_hdmi_phy_power_enable_set, hdmi_dev->hal, TD_TRUE);
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, TD_TRUE);
    drv_hdmi_avmute_set(hdmi_dev, TD_FALSE);

    return TD_SUCCESS;
}

td_void drv_hdmi_stop(const hdmi_device *hdmi_dev)
{
    hdmi_if_null_return_void(hdmi_dev);

    drv_hdmi_avmute_set(hdmi_dev, TD_TRUE);
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, TD_FALSE);
    hal_call_void(hal_hdmi_phy_power_enable_set, hdmi_dev->hal, TD_FALSE);

    return;
}

td_s32 drv_hdmi_close(hdmi_device *hdmi_dev)
{
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    drv_hdmi_stop(hdmi_dev);
    hal_hdmi_close(&hdmi_dev->hal);
    hdmi_dev->hal = TD_NULL;

    return TD_SUCCESS;
}

td_s32 drv_hdmi_get_hw_spec(ot_hdmi_id hdmi, ot_hdmi_hw_spec *hw_spec)
{
    td_s32 ret;
    drv_hdmi_hw_spec param = {0};
    hdmi_device *hdmi_dev = TD_NULL;

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hw_spec, TD_FAILURE);
    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    (td_void)memset_s(hw_spec, sizeof(ot_hdmi_hw_spec), 0, sizeof(ot_hdmi_hw_spec));
    hal_call_ret(ret, hal_hdmi_phy_hw_spec_get, hdmi_dev->hal, &param.hw_spec);
    if (ret == TD_SUCCESS) {
        ret = memcpy_s(hw_spec, sizeof(ot_hdmi_hw_spec), &param.hw_spec, sizeof(hdmi_hw_spec));
        hdmi_unequal_eok(ret);
    }
    hdmi_info("<<< out...\n");

    return ret;
}

static td_s32 hdmi_hw_spec_check_valid(const ot_hdmi_hw_spec *hw_spec)
{
    td_u8 i;

    for (i = 0; i < OT_HDMI_HW_PARAM_NUM; i++) {
        if ((hw_spec->hw_param[i].i_de_main_clk > MAX_HDMI_HW_PARAM) ||
            (hw_spec->hw_param[i].i_de_main_data > MAX_HDMI_HW_PARAM) ||
            (hw_spec->hw_param[i].i_main_clk > MAX_HDMI_HW_PARAM) ||
            (hw_spec->hw_param[i].i_main_data > MAX_HDMI_HW_PARAM) ||
            (hw_spec->hw_param[i].i_main_clk == 0) ||
            (hw_spec->hw_param[i].i_main_data == 0)) {
            hdmi_err("stage(%u), i_de_main_clk:%u, i_de_main_data:%u, i_main_clk:%u, i_main_data:%u valid Range:0-%u\n",
                i, hw_spec->hw_param[i].i_de_main_clk, hw_spec->hw_param[i].i_de_main_data,
                hw_spec->hw_param[i].i_main_clk, hw_spec->hw_param[i].i_main_data, MAX_HDMI_HW_PARAM);
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

td_s32 drv_hdmi_set_hw_spec(ot_hdmi_id hdmi, const ot_hdmi_hw_spec *hw_spec)
{
    td_s32 ret;
    td_u32 tmds_clk;
    hdmi_hw_spec hw_param = {0};
    hdmi_hal_hw_param param = {0};
    hdmi_device *hdmi_dev = TD_NULL;

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hw_spec, TD_FAILURE);
    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    ret = hdmi_hw_spec_check_valid(hw_spec);
    hdmi_if_failure_return(ret, ret);

    ret = memcpy_s(&hw_param, sizeof(hdmi_hw_spec), hw_spec, sizeof(ot_hdmi_hw_spec));
    hdmi_unequal_eok(ret);

    tmds_clk = hdmi_dev->attr.vo_attr.hdmi_adapt_pix_clk;
    param.hw_spec = &hw_param;
    param.trace_len = hdmi_dev->mode_param.trace_len;
    param.tmds_clk = hdmi_dev->attr.vo_attr.hdmi_adapt_pix_clk;
    hal_call_ret(ret, hal_hdmi_phy_hw_spec_set, hdmi_dev->hal, tmds_clk, param);
    hdmi_info("<<< out...\n");

    return ret;
}

td_s32 drv_hdmi_get_mod_param(ot_hdmi_id hdmi, ot_hdmi_mod_param *mode_param)
{
    hdmi_device *hdmi_dev = TD_NULL;
    hdmi_mode_param *param = TD_NULL;

    hdmi_if_null_return(mode_param, TD_FAILURE);
    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    param = &hdmi_dev->mode_param;
    mode_param->trace_len = param->trace_len;
    mode_param->emi_en = param->emi_en;

    return TD_SUCCESS;
}

td_s32 drv_hdmi_set_mod_param(ot_hdmi_id hdmi, const ot_hdmi_mod_param *mode_param)
{
    hdmi_device *hdmi_dev = TD_NULL;
    hdmi_mode_param *param = TD_NULL;

    hdmi_if_null_return(mode_param, TD_FAILURE);
    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    if (mode_param->trace_len > OT_HDMI_TRACE_BUTT) {
        hdmi_err("param invalid!\n");
        return TD_FAILURE;
    }
    param = &hdmi_dev->mode_param;
    param->trace_len = mode_param->trace_len;
    param->emi_en = mode_param->emi_en;
    hdmi_info("set trace length: %u, set emi enable: %u\n", param->trace_len, param->emi_en);

    return TD_SUCCESS;
}

static td_s32 check_video_attr(const hdmi_vo_attr *vo_attr)
{
    if (vo_attr->video_timing >= HDMI_VIDEO_TIMING_BUTT) {
        hdmi_info("Hdmi video timing(%d) is wrong\n", vo_attr->video_timing);
        return TD_FAILURE;
    }

    if (vo_attr->in_colorspace >= HDMI_COLORSPACE_BUTT) {
        hdmi_info("Hdmi video InColorSpace(%d) is wrong\n", vo_attr->in_colorspace);
        return TD_FAILURE;
    }

    if (vo_attr->colorimetry <= HDMI_COLORIMETRY_NONE ||
        vo_attr->colorimetry > HDMI_COLORIMETRY_EXTENDED) {
        hdmi_info("Hdmi video Colorimetry(%d) is wrong\n", vo_attr->colorimetry);
        return TD_FAILURE;
    }

    if (vo_attr->rgb_quantization > HDMI_QUANTIZATION_RANGE_FULL) {
        hdmi_info("Hdmi video RGBQuantization(%d) is wrong\n", vo_attr->rgb_quantization);
        return TD_FAILURE;
    }

    if (vo_attr->picture_aspect < HDMI_PICTURE_ASPECT_4_3 ||
        vo_attr->picture_aspect > HDMI_PICTURE_ASPECT_256_135) {
        hdmi_info("Hdmi video PictureAspect(%d) is wrong\n", vo_attr->picture_aspect);
        return TD_FAILURE;
    }

    if (vo_attr->in_bit_depth > HDMI_VIDEO_BITDEPTH_12) {
        hdmi_info("Hdmi video InBitDepth(%d) is wrong\n", vo_attr->in_bit_depth);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void hdmi_video_path_set(hdmi_device *hdmi_dev, const hdmi_vo_attr *vo_attr)
{
    hdmi_video_config vid_cfg = {0};
    hdmi_app_attr *app_attr = TD_NULL;

    app_attr = &hdmi_dev->attr.app_attr;

    vid_cfg.in_bit_depth  = vo_attr->in_bit_depth;
    vid_cfg.quantization  = vo_attr->rgb_quantization;
    vid_cfg.in_colorspace = vo_attr->in_colorspace;
    vid_cfg.pixel_clk     = (app_attr->out_colorspace == HDMI_COLORSPACE_YCBCR420) ?
        (vo_attr->clk_fs >> 1) : vo_attr->clk_fs;
    vid_cfg.tmds_clk       = vo_attr->hdmi_adapt_pix_clk;
    vid_cfg.deep_color     = app_attr->deep_color_mode;
    vid_cfg.out_colorspace = app_attr->out_colorspace;
    vid_cfg.de_pol         = vo_attr->de_pol;
    vid_cfg.h_sync_pol     = vo_attr->h_sync_pol;
    vid_cfg.v_sync_pol     = vo_attr->v_sync_pol;
    vid_cfg.trace_len      = hdmi_dev->mode_param.trace_len;
    vid_cfg.emi_enable     = hdmi_dev->mode_param.emi_en;

    switch (vo_attr->colorimetry) {
        case HDMI_COLORIMETRY_ITU_601:
            vid_cfg.conv_std = HDMI_CONV_STD_BT_601;
            break;
        case HDMI_COLORIMETRY_ITU_709:
            vid_cfg.conv_std = HDMI_CONV_STD_BT_709;
            break;
        case HDMI_COLORIMETRY_EXTENDED:
            if (vo_attr->extended_colorimetry == HDMI_EXTENDED_COLORIMETRY_2020_NON_CONST_LUMINOUS) {
                vid_cfg.conv_std = HDMI_CONV_STD_BT_2020_NON_CONST_LUMINOUS;
            } else if (vo_attr->extended_colorimetry == HDMI_EXTENDED_COLORIMETRY_2020_CONST_LUMINOUS) {
                vid_cfg.conv_std = HDMI_CONV_STD_BT_2020_CONST_LUMINOUS;
            }
            break;
        default:
            break;
    }

    hdmi_info("pixel_clk(%u), tmds_clk(%u)\n", vid_cfg.pixel_clk, vid_cfg.tmds_clk);
    /* Video will mute and unmute in this function call */
    hal_call_void(hal_hdmi_video_path_set, hdmi_dev->hal, &vid_cfg);

    return;
}

td_void drv_hdmi_attr_get(const hdmi_device *hdmi_dev, hdmi_attr *attr)
{
    errno_t ret;

    hdmi_if_null_return_void(hdmi_dev);
    hdmi_if_null_return_void(attr);
    ret = memcpy_s(attr, sizeof(*attr), &hdmi_dev->attr, sizeof(hdmi_attr));
    hdmi_unequal_eok(ret);

    return;
}

static td_void hdmi_tmdsclk_get(hdmi_device *hdmi_dev)
{
    hdmi_app_attr *app_attr = TD_NULL;
    hdmi_vo_attr *video_attr = TD_NULL;

    app_attr = &hdmi_dev->attr.app_attr;
    video_attr = &hdmi_dev->attr.vo_attr;

    video_attr->hdmi_adapt_pix_clk = video_attr->clk_fs;
    if (app_attr->out_colorspace == HDMI_COLORSPACE_YCBCR420) {
        hdmi_multiple_0p5(video_attr->hdmi_adapt_pix_clk);
        hdmi_multiple_0p5(video_attr->clk_fs);
    }
    if (app_attr->out_colorspace != HDMI_COLORSPACE_YCBCR422) {
        switch (app_attr->deep_color_mode) {
            case HDMI_DEEP_COLOR_30BIT:
                hdmi_multiple_1p25(video_attr->hdmi_adapt_pix_clk);
                break;
            case HDMI_DEEP_COLOR_36BIT:
                hdmi_multiple_1p5(video_attr->hdmi_adapt_pix_clk);
                break;
            case HDMI_DEEP_COLOR_48BIT:
                hdmi_multiple_2p0(video_attr->hdmi_adapt_pix_clk);
                break;
            default:
                app_attr->deep_color_mode = HDMI_DEEP_COLOR_24BIT;
                break;
        }
    } else {
        app_attr->deep_color_mode = HDMI_DEEP_COLOR_OFF;
    }

    return;
}

static td_s32 hdmi_caps_check(td_u32 vosync)
{
#ifdef OT_HDMI_SUPPORT_1_4
    td_bool support;
    support = (vosync == FMT_4K_50 || vosync == FMT_4K_60);
    if (support == TD_TRUE) {
        hdmi_err("param is invalid, This chip is not support HDMI2.0\n");
        return TD_FAILURE;
    }
#endif

    return TD_SUCCESS;
}

td_s32 drv_hdmi_attr_set(hdmi_device *hdmi_dev, const hdmi_attr *attr)
{
    errno_t ret;
    const hdmi_vo_attr *video_attr = TD_NULL;

    hdmi_if_null_return(hdmi_dev, TD_FAILURE);
    hdmi_if_null_return(attr, TD_FAILURE);

    video_attr = &attr->vo_attr;
    if (check_video_attr(video_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }
    ret = memcpy_s(&hdmi_dev->attr, sizeof(hdmi_dev->attr), attr, sizeof(hdmi_attr));
    hdmi_unequal_eok(ret);

    /* YUV422 is not support when the fmt is PAL or NTSC. */
    if (hdmi_dev->attr.vo_attr.video_timing == HDMI_VIDEO_TIMING_1440X480I_60000 ||
        hdmi_dev->attr.vo_attr.video_timing == HDMI_VIDEO_TIMING_1440X576I_50000) {
        if (hdmi_dev->attr.app_attr.out_colorspace == HDMI_COLORSPACE_YCBCR422) {
            hdmi_info("Y422 is not support at pal and ntsc, force adapt to rgb.\n");
            hdmi_dev->attr.app_attr.out_colorspace = HDMI_COLORSPACE_RGB;
        }
    }

    hdmi_tmdsclk_get(hdmi_dev);
    hdmi_video_path_set(hdmi_dev, &hdmi_dev->attr.vo_attr);
    drv_hdmi_avi_infoframe_send(&hdmi_dev->infoframe, TD_TRUE);
    drv_hdmi_vendor_infoframe_send(&hdmi_dev->infoframe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 ot_drv_hdmi_init(td_void)
{
    return TD_SUCCESS;
}

td_s32 ot_drv_hdmi_open(ot_hdmi_id hdmi_id)
{
    if (hdmi_id >= OT_HDMI_ID_BUTT) {
        hdmi_err("param err!\n");
        return TD_FAILURE;
    }
    (td_void)memset_s(&g_hdmi_dev[hdmi_id], sizeof(hdmi_device), 0, sizeof(hdmi_device));
    g_hdmi_dev[hdmi_id].hdmi_dev_id = hdmi_id;

    return drv_hdmi_open(&g_hdmi_dev[hdmi_id]);
}

td_s32 ot_drv_hdmi_set_attr(ot_hdmi_id hdmi_id, const ot_drv_hdmi_attr *attr)
{
    td_s32 ret;
    hdmi_attr attr_tmp = {0};

    hdmi_if_null_return(attr, TD_FAILURE);
    hdmi_id_check_return(hdmi_id, TD_FAILURE);

    drv_hdmi_attr_get(&g_hdmi_dev[hdmi_id], &attr_tmp);
    ret = disp_fmt_to_video_attr(attr->disp_fmt, &attr_tmp.vo_attr);
    if (ret != TD_SUCCESS) {
        hdmi_info("Please input a correct Fmt(%u)!\n", attr->disp_fmt);
        return ret;
    }
    attr_tmp.app_attr.enable_hdmi = attr->enable_hdmi;
    video_2_colorspace(attr->vid_out_mode, attr_tmp.app_attr.out_colorspace);
    video_2_colorspace(attr->vid_in_mode, attr_tmp.vo_attr.in_colorspace);

    return drv_hdmi_attr_set(&g_hdmi_dev[hdmi_id], &attr_tmp);
}

td_s32 ot_drv_hdmi_get_hw_spec(ot_hdmi_id hdmi, ot_hdmi_hw_spec *hw_spec)
{
    hdmi_id_check_return(hdmi, TD_FAILURE);
    hdmi_if_null_return(hw_spec, TD_FAILURE);

    return drv_hdmi_get_hw_spec(hdmi, hw_spec);
}

td_s32 ot_drv_hdmi_set_hw_spec(ot_hdmi_id hdmi, ot_hdmi_hw_spec *hw_spec)
{
    hdmi_id_check_return(hdmi, TD_FAILURE);
    hdmi_if_null_return(hw_spec, TD_FAILURE);

    return drv_hdmi_set_hw_spec(hdmi, hw_spec);
}

td_s32 ot_drv_hdmi_get_mod_param(ot_hdmi_id hdmi, ot_hdmi_mod_param *mode_param)
{
    hdmi_id_check_return(hdmi, TD_FAILURE);
    hdmi_if_null_return(mode_param, TD_FAILURE);

    return drv_hdmi_get_mod_param(hdmi, mode_param);
}

td_s32 ot_drv_hdmi_set_mod_param(ot_hdmi_id hdmi, ot_hdmi_mod_param *mode_param)
{
    hdmi_id_check_return(hdmi, TD_FAILURE);
    hdmi_if_null_return(mode_param, TD_FAILURE);

    return drv_hdmi_set_mod_param(hdmi, mode_param);
}

#ifndef HDMI_SUPPORT_DUAL_CHANNEL
td_void hdmi_stop(td_void)
#else
td_void hdmi_stop(ot_hdmi_id hdmi_id)
#endif
{
    td_s32 ret;

#ifndef HDMI_SUPPORT_DUAL_CHANNEL
    ret = ot_drv_hdmi_close(OT_HDMI_ID_0);
#else
    ret = ot_drv_hdmi_close(hdmi_id);
#endif
    ret += ot_drv_hdmi_de_init();
    if (ret != TD_SUCCESS) {
        hdmi_err("hdmi stop err!\n");
    }

    return;
}

#ifndef HDMI_SUPPORT_DUAL_CHANNEL
td_s32 hdmi_display(td_u32 sync, td_u32 input_colorspace, td_u32 output_colorspace)
#else
td_s32 hdmi_display(ot_hdmi_id id, td_u32 sync, td_u32 input_colorspace, td_u32 output_colorspace)
#endif
{
    td_s32 ret;
    ot_hdmi_id hdmi_id = OT_HDMI_ID_0;
    ot_drv_hdmi_attr attr = {0};

#ifdef HDMI_SUPPORT_DUAL_CHANNEL
    hdmi_id = id;
    if (hdmi_id >= OT_HDMI_ID_MAX) {
        hdmi_err("param err!\n");
        return TD_FAILURE;
    }
#endif
    ret = hdmi_caps_check(sync);
    hdmi_if_failure_return(ret, TD_FAILURE);

    ret = ot_drv_hdmi_init();
    hdmi_if_failure_return(ret, TD_FAILURE);

    ret = ot_drv_hdmi_open(hdmi_id);
    hdmi_if_failure_return(ret, TD_FAILURE);

    attr.enable_hdmi  = TD_TRUE;
    attr.disp_fmt     = sync;
    attr.vid_out_mode = output_colorspace;
    attr.vid_in_mode  = input_colorspace;
    ret = ot_drv_hdmi_set_attr(hdmi_id, &attr);
    hdmi_if_failure_return(ret, TD_FAILURE);

    ret = ot_drv_hdmi_start(hdmi_id);
    hdmi_if_failure_return(ret, TD_FAILURE);

    return ret;
}

td_s32 ot_drv_hdmi_start(ot_hdmi_id hdmi_id)
{
    hdmi_id_check_return(hdmi_id, TD_FAILURE);
    return drv_hdmi_start(&g_hdmi_dev[hdmi_id]);
}

td_s32 ot_drv_hdmi_close(ot_hdmi_id hdmi_id)
{
    hdmi_id_check_return(hdmi_id, TD_FAILURE);
    return drv_hdmi_close(&g_hdmi_dev[hdmi_id]);
}

td_s32 ot_drv_hdmi_de_init(td_void)
{
    return TD_SUCCESS;
}

