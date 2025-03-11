/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __OT_COMMON_VI_H__
#define __OT_COMMON_VI_H__

#include "ot_common.h"
#include "ot_errno.h"
#include "ot_common_video.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OT_VI_INVALID_FRAME_RATE (-1U)
#define OT_VI_MAX_USER_FRAME_DEPTH 8
#define OT_VI_COMPONENT_MASK_NUM 2
#define OT_VI_VBI_NUM 2

#define OT_ERR_VI_INVALID_DEV_ID OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_INVALID_DEV_ID)
#define OT_ERR_VI_INVALID_CHN_ID OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_INVALID_CHN_ID)
#define OT_ERR_VI_INVALID_PARAM OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_ILLEGAL_PARAM)
#define OT_ERR_VI_INVALID_NULL_PTR OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NULL_PTR)
#define OT_ERR_VI_FAILED_NOT_CFG OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_CFG)
#define OT_ERR_VI_NOT_SUPPORT OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_SUPPORT)
#define OT_ERR_VI_NOT_PERM OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_PERM)
#define OT_ERR_VI_NOT_ENABLE OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_ENABLE)
#define OT_ERR_VI_NOT_DISABLE OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_DISABLE)
#define OT_ERR_VI_NO_MEM OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NO_MEM)
#define OT_ERR_VI_BUF_EMPTY OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_BUF_EMPTY)
#define OT_ERR_VI_BUF_FULL OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_BUF_FULL)
#define OT_ERR_VI_NOT_READY OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_READY)
#define OT_ERR_VI_BUSY OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_BUSY)
#define OT_ERR_VI_NOT_BINDED OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_BINDED)
#define OT_ERR_VI_BINDED OT_DEFINE_ERR(OT_ID_VI, OT_ERR_LEVEL_ERROR, OT_ERR_BINDED)

/* interface mode of video input */
typedef enum {
    OT_VI_MODE_BT656 = 0,          /* ITU-R BT.656 YUV4:2:2 */
    OT_VI_MODE_BT1120_STANDARD,    /* BT.1120 progressive mode */
    OT_VI_MODE_BT1120_INTERLEAVED, /* BT.1120 interleave mode */
    OT_VI_MODE_MIPI_YUV422,        /* MIPI YUV 422 mode */

    OT_VI_MODE_BUTT
} ot_vi_intf_mode;

/* input mode */
typedef enum {
    OT_VI_IN_MODE_BT656 = 0,   /* ITU-R BT.656 YUV4:2:2 */
    OT_VI_IN_MODE_INTERLEAVED, /* interleave mode */
    OT_VI_IN_MODE_MIPI,        /* MIPI mode */

    OT_VI_IN_MODE_BUTT
} ot_vi_in_mode;

typedef enum {
    OT_VI_WORK_MODE_MULTIPLEX_1 = 0, /* 1 Multiplex mode */
    OT_VI_WORK_MODE_MULTIPLEX_2,     /* 2 Multiplex mode */
    OT_VI_WORK_MODE_MULTIPLEX_4,     /* 4 Multiplex mode */

    OT_VI_WORK_MODE_BUTT
} ot_vi_work_mode;

/* whether an input picture is interlaced or progressive */
typedef enum {
    OT_VI_SCAN_INTERLACED = 0,
    OT_VI_SCAN_PROGRESSIVE,

    OT_VI_SCAN_BUTT
} ot_vi_scan_mode;

typedef enum {
    /* the input sequence of the second component(only contains u and v) in BT.1120 mode */
    OT_VI_IN_DATA_VUVU = 0,
    OT_VI_IN_DATA_UVUV,

    /* the input sequence for yuv */
    OT_VI_IN_DATA_UYVY,
    OT_VI_IN_DATA_VYUY,
    OT_VI_IN_DATA_YUYV,
    OT_VI_IN_DATA_YVYU,

    OT_VI_IN_DATA_YUV_BUTT
} ot_vi_data_yuv_seq;

typedef enum {
    OT_VI_CLK_EDGE_SINGLE_UP = 0, /* single-edge mode and in rising edge */
    OT_VI_CLK_EDGE_SINGLE_DOWN,   /* single-edge mode and in falling edge */
    OT_VI_CLK_EDGE_DOUBLE,        /* double-edge mode */

    OT_VI_CLK_EDGE_BUTT
} ot_vi_clk_edge;

typedef enum {
    OT_VI_COMPONENT_MODE_SINGLE = 0, /* in single component mode */
    OT_VI_COMPONENT_MODE_DOUBLE = 1, /* in double component mode */
    OT_VI_COMPONENT_MODE_BUTT
} ot_vi_component_mode;

/* Y/C composite or separation mode */
typedef enum {
    OT_VI_COMBINE_COMPOSITE = 0, /* composite mode */
    OT_VI_COMBINE_SEPARATE,      /* separate mode */
    OT_VI_COMBINE_BUTT
} ot_vi_combine_mode;

/* attribute of the vertical synchronization signal */
typedef enum {
    OT_VI_VSYNC_FIELD = 0, /* field/toggle mode:a signal reversal means a new frame or a field */
    OT_VI_VSYNC_PULSE,     /* pulse/effective mode:a pulse or an effective signal means a new frame or a field */
    OT_VI_VSYNC_BUTT
} ot_vi_vsync;

/* polarity of the vertical synchronization signal */
typedef enum {
    OT_VI_VSYNC_NEG_HIGH = 0, /*
                               * if ot_vi_vsync = VIU_VSYNC_FIELD, then the vertical synchronization signal of
                               * even field is high-level, if ot_vi_vsync = VIU_VSYNC_PULSE, then the vertical
                               * synchronization pulse is positive pulse
                               */
    OT_VI_VSYNC_NEG_LOW, /*
                          * if ot_vi_vsync = VIU_VSYNC_FIELD, then the vertical synchronization signal of
                          * even field is low-level, if ot_vi_vsync = VIU_VSYNC_PULSE, then the vertical
                          * synchronization pulse is negative pulse
                          */
    OT_VI_VSYNC_NEG_BUTT
} ot_vi_vsync_neg;

/* attribute of the horizontal synchronization signal */
typedef enum {
    OT_VI_HSYNC_VALID_SIG = 0, /* the horizontal synchronization is valid signal mode */
    OT_VI_HSYNC_PULSE,         /*
                                * the horizontal synchronization is pulse mode,
                                * a new pulse means the beginning of a new line
                                */
    OT_VI_HSYNC_BUTT
} ot_vi_hsync;

/* polarity of the horizontal synchronization signal */
typedef enum {
    OT_VI_HSYNC_NEG_HIGH = 0, /*
                               * if ot_vi_hsync = VI_HSYNC_VALID_SINGNAL, then the valid horizontal
                               * synchronization signal is high-level;
                               * if ot_vi_hsync = OT_VI_HSYNC_PULSE, then the horizontal synchronization
                               * pulse is positive pulse
                               */
    OT_VI_HSYNC_NEG_LOW, /*
                          * if ot_vi_hsync = VI_HSYNC_VALID_SINGNAL, then the valid horizontal
                          * synchronization signal is low-level;
                          * if ot_vi_hsync = OT_VI_HSYNC_PULSE,then the horizontal synchronization
                          * pulse is negative pulse
                          */
    OT_VI_HSYNC_NEG_BUTT
} ot_vi_hsync_neg;

/* attribute of the valid vertical synchronization signal */
typedef enum {
    OT_VI_VSYNC_NORM_PULSE = 0, /* the vertical synchronization is pulse mode, a pulse means a new frame or field */
    OT_VI_VSYNC_VALID_SIG,      /*
                                 * the vertical synchronization is effective mode, a effective signal means a new
                                 * frame or field
                                 */
    OT_VI_VSYNC_VALID_BUTT
} ot_vi_vsync_valid;

/* polarity of the valid vertical synchronization signal */
typedef enum {
    OT_VI_VSYNC_VALID_NEG_HIGH = 0, /*
                                     * if ot_vi_vsync_valid = OT_VI_VSYNC_NORM_PULSE, a positive pulse means vertical
                                     * synchronization pulse;
                                     * if ot_vi_vsync_valid = OT_VI_VSYNC_VALID_SIG, the valid vertical synchronization
                                     * signal is high-level
                                     */
    OT_VI_VSYNC_VALID_NEG_LOW,      /*
                                     * if ot_vi_vsync_valid = OT_VI_VSYNC_NORM_PULSE, a negative pulse means
                                     * vertical synchronization pulse;
                                     * if ot_vi_vsync_valid = OT_VI_VSYNC_VALID_SIG, the valid vertical synchronization
                                     * signal is low-level
                                     */
    OT_VI_VSYNC_AVLID_NEG_BUTT
} ot_vi_vsync_valid_neg;

/* blank information of the input timing */
typedef struct {
    td_u32 hsync_hfb;   /* horizontal front blanking width */
    td_u32 hsync_act;   /* horizontal effective width */
    td_u32 hsync_hbb;   /* horizontal back blanking width */
    td_u32 vsync_vfb;   /* vertical front blanking height of one frame or odd-field frame picture */
    td_u32 vsync_vact;  /* vertical effective width of one frame or odd-field frame picture */
    td_u32 vsync_vbb;   /* vertical back blanking height of one frame or odd-field frame picture */
    td_u32 vsync_vbfb;  /*
                         * even-field vertical front blanking height when input mode is interlace
                         * (invalid when progressive input mode)
                         */
    td_u32 vsync_vbact; /*
                         * even-field vertical effective width when input mode is interlace
                         * (invalid when progressive input mode)
                         */
    td_u32 vsync_vbbb;  /*
                         * even-field vertical back blanking height when input mode is interlace
                         * (invalid when progressive input mode)
                         */
} ot_vi_timing_blank;

/* synchronization information about the BT.601 or DC timing */
typedef struct {
    ot_vi_vsync vsync;
    ot_vi_vsync_neg vsync_neg;
    ot_vi_hsync hsync;
    ot_vi_hsync_neg hsync_neg;
    ot_vi_vsync_valid vsync_valid;
    ot_vi_vsync_valid_neg vsync_valid_neg;
    ot_vi_timing_blank timing_blank;
} ot_vi_sync_cfg;

/* the highest bit of the BT.656 timing reference code */
typedef enum {
    OT_BT656_FIXCODE_1 = 0, /* the highest bit of the EAV/SAV data over the BT.656 protocol is always 1. */
    OT_BT656_FIXCODE_0,     /* the highest bit of the EAV/SAV data over the BT.656 protocol is always 0. */
    OT_BT656_FIXCODE_BUTT
} ot_bt656_fixcode;

/* polarity of the field indicator bit (F) of the BT.656 timing reference code */
typedef enum {
    OT_BT656_FIELD_POLAR_STD = 0, /* the standard BT.656 mode,the first filed F=0,the second filed F=1 */
    OT_BT656_FIELD_POLAR_NON_STD, /* the non-standard BT.656 mode,the first filed F=1,the second filed F=0 */
    OT_BT656_FIELD_POLAR_BUTT
} ot_bt656_field_polar;

typedef struct {
    ot_bt656_fixcode fix_code;
    ot_bt656_field_polar field_polar;
} ot_vi_bt656_sync_cfg;

typedef enum {
    OT_VI_VBI_LOCAL_ODD_FRONT = 0,
    OT_VI_VBI_LOCAL_ODD_END,
    OT_VI_VBI_LOCAL_EVEN_FRONT,
    OT_VI_VBI_LOCAL_EVEN_END,
    OT_VI_VBI_LOCAL_BUTT
} ot_vi_vbi_location;

typedef struct {
    ot_vi_vbi_location location; /* location of VBI */
    td_s32 x;                    /* horizontal original position of the VBI data */
    td_s32 y;                    /* vertical original position of the VBI data */
    td_u32 len;                  /* length of VBI data, by word(4 bytes) */
} ot_vi_vbi_attr;

typedef struct {
    ot_vi_vbi_attr vbi_attr[OT_VI_MAX_VBI_NUM];
} ot_vi_vbi_arg;

typedef enum {
    OT_VI_VBI_MODE_8BIT = 0,
    OT_VI_VBI_MODE_6BIT,

    VI_VBI_MODE_BUTT
} ot_vi_vbi_mode;

typedef struct {
    td_u32 data[OT_VI_MAX_VBI_LEN];
    td_u32 len;
} ot_video_vbi_info;

/* the extended attributes of VI device */
typedef struct {
    ot_vi_in_mode in_mode;     /* input mode */
    ot_vi_work_mode work_mode; /* 1-, 2-, or 4-channel multiplexed work mode */

    ot_vi_combine_mode combine_mode;     /* Y/C composite or separation mode */
    ot_vi_component_mode component_mode; /* component mode (single-component or dual-component) */
    ot_vi_clk_edge clk_edge;             /* clock edge mode (sampling on the rising or falling edge) */

    td_u32 component_mask[OT_VI_COMPONENT_MASK_NUM]; /* component mask */

    ot_vi_data_yuv_seq data_seq; /* input data sequence (only the YUV format is supported) */
    ot_vi_sync_cfg sync_cfg;     /* sync timing. this member must be configured in BT.601 mode or DC mode */

    ot_vi_bt656_sync_cfg bt656_sync_cfg; /* sync timing. this member must be configured in BT.656 mode */

    td_bool data_reverse_en; /* data reverse */

    td_bool mixed_capture_en; /* mixed capatrue switch */
} ot_vi_dev_attr_ex;

/* the attributes of a VI device */
typedef struct {
    ot_vi_intf_mode intf_mode; /* interface mode */
    ot_vi_work_mode work_mode; /* 1-, 2-, or 4-channel multiplexed work mode */

    td_u32 component_mask[OT_VI_COMPONENT_MASK_NUM]; /* component mask */

    ot_vi_clk_edge clk_edge; /* clock edge mode (sampling on the rising or falling, double edge) */

    ot_vi_data_yuv_seq data_seq; /* input data sequence (only the YUV format is supported) */
    ot_vi_sync_cfg sync_cfg;     /* sync timing. this member must be configured in BT.601 mode or DC mode */

    td_bool data_reverse_en; /* data reverse */

    td_bool mixed_capture_en; /* mixed capatrue switch */
} ot_vi_dev_attr;


typedef struct {
    ot_vi_dev vi_dev;
    ot_vi_way vi_way;
} ot_vi_chn_bind_attr;

/* capture selection of video input */
typedef enum {
    OT_VI_CAPTURE_SELECT_TOP = 0, /* top field */
    OT_VI_CAPTURE_SELECT_BOTTOM,  /* bottom field */
    OT_VI_CAPTURE_SELECT_BOTH,    /* top and bottom field */
    OT_VI_CAPTURE_SELECT_BUTT
} ot_vi_capture_select;

/* the attributes of a VI channel */
typedef struct {
    ot_rect capture_rect; /*
                           * the capture rect (corresponding to the size of the picture captured
                           * by a VI device).
                           * for primary channels, the capture_rect's width and height are static attributes.
                           * that is, the value of them can be changed only after primary and secondary
                           * channels are disabled.
                           * for secondary channels, capture_rect is an invalid attribute
                           */
    ot_size dst_size;     /* target picture size. */

    ot_vi_capture_select capture_select; /*
                                          * frame/field select. it is used only in interlaced mode.
                                          * for primary channels, capture_select is a static attribute
                                          */
    ot_vi_scan_mode scan_mode;           /* input scanning mode (progressive or interlaced) */
    ot_pixel_format pixel_format;        /*
                                          * pixel storage format.
                                          * only the formats semi-planar420, semi-planar422 or yuv400 are supported
                                          */
    td_bool mirror_en;                   /* whether to mirror */
    td_bool flip_en;                     /* whether to flip */
    ot_compress_mode compress_mode;      /* segment compress or no compress */
    td_s32 src_frame_rate; /* source frame rate. the value -1 indicates that the frame rate is not controlled */
    td_s32 dst_frame_rate; /* target frame rate. the value -1 indicates that the frame rate is not controlled */
} ot_vi_chn_attr;

typedef struct {
    td_bool enable;                    /* whether this channel is enabled */
    td_u32 interrupt_cnt;              /* the video frame interrupt count */
    td_u32 frame_rate;                 /* current frame rate */
    td_u32 lost_interrupt_cnt;         /* the interrupt is received but nobody care */
    td_u32 vb_fail_cnt;                /* video buffer malloc failure */
    td_u32 pic_width;                  /* current pic width */
    td_u32 pic_height;                 /* current pic height */
    td_u32 auto_disable_interrupt_cnt; /* auto disable interrupt count, when VIU detected too many interrupts */
    td_u32 cc_err_cnt;                 /* capture complete err count */
} ot_vi_chn_status;

typedef enum {
    OT_VI_USER_PIC_MODE_PIC = 0, /* YUV picture */
    OT_VI_USER_PIC_MODE_BG,      /* background picture only with a color */
    OT_VI_USER_PIC_MODE_BUTT,
} ot_vi_user_pic_mode;

typedef struct {
    td_u32 bg_color;
} ot_vi_user_pic_bg_info;

typedef struct {
    td_bool pub_en; /* whether the user picture information is shared by all VI devices and channels */
    ot_vi_user_pic_mode user_pic_mode; /* user picture mode */
    union {
        /*
         * information about a YUV picture
         * AUTO:ot_vi_user_pic_mode:OT_VI_USER_PIC_MODE_PIC;
         */
        ot_video_frame_info user_pic_frame_info;
        /*
         * information about a background picture only with a color
         * AUTO:ot_vi_user_pic_mode:OT_VI_USER_PIC_MODE_BG;
         */
        ot_vi_user_pic_bg_info user_pic_bg_info;
    } user_pic_info;
} ot_vi_user_pic_attr;


typedef struct {
    td_u32 luma; /* luma sum of current frame */
    td_u64 pts;  /* PTS of current frame */
} ot_vi_chn_luma;

typedef enum {
    OT_VI_SKIP_NONE = 0, /* no skip */
    OT_VI_SKIP_YXYX,     /* 1/2 skip,1010 */
    OT_VI_SKIP_YXXX,     /* 1/4 skip,1000 */
    OT_VI_SKIP_YXXY,     /* 1/2 skip,1001 */
    OT_VI_SKIP_BUTT,
} ot_vi_skip_mode;

typedef struct {
    td_bool enable;
    td_u32 y_mask; /* ymask,0000-1111 in binary */
    td_u32 c_mask; /* cmask,0000-1111 in binary */
} ot_vi_skip_mode_ex;

typedef struct {
    td_s32 detect_err_frame;
    td_s32 drop_err_frame;
    td_s32 int_gap_time;
    td_s32 min_cas_gap;
    td_s32 max_cas_gap;
    td_s32 stop_int_level;
    td_s32 int_time;
    td_s32 discard_int;
    td_s32 yc_reverse;
    td_s32 compat_mode;
    td_s32 time_ref_gap;
} ot_vi_mod_param;

#ifdef __cplusplus
}
#endif

#endif