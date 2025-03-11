/**
	@brief Header file of video encoder module.\n
	This file contains the functions which is related to video encoder in the chip.

	@file hd_videoenc.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef	_HD_VIDEOENC_H_
#define	_HD_VIDEOENC_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define HD_VIDEOENC_MAX_IN         64   ///< max count of input of this device (interface)
#define HD_VIDEOENC_MAX_OUT        64   ///< max count of output of this device (interface)
#define HD_VIDEOENC_MAX_DATA_TYPE   4   ///< max count of output pool of this device (interface)


/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/
#define HD_VIDEOENC_SET_COUNT(a, b)    ((a)*10)+(b)  ///< ex: use HD_VIDEOENC_SET_COUNT(1, 5) for setting 1.5

/********************************************************************
	TYPE DEFINITION
********************************************************************/

typedef enum HD_VIDEOENC_INCAPS {
	HD_VIDEOENC_INCAPS_ONEBUF     = 0x02000000, ///< caps of HD_VIDEOENC_INFUNC_ONEBUF
	HD_VIDEOENC_INCAPS_LOWLATENCY = 0x04000000, ///< caps of HD_VIDEOENC_INFUNC_LOWLATENCY
	ENUM_DUMMY4WORD(HD_VIDEOENC_INCAPS)
} HD_VIDEOENC_INCAPS;

typedef enum _HD_VIDEOENC_CAPS {
	HD_VIDEOENC_CAPS_JPEG = 0x00000001, ///< support encode to jpeg
	HD_VIDEOENC_CAPS_H264 = 0x00000002, ///< support encode to h264
	HD_VIDEOENC_CAPS_H265 = 0x00000004, ///< support encode to h265
	ENUM_DUMMY4WORD(HD_VIDEOENC_CAPS)
} HD_VIDEOENC_CAPS;


typedef enum _HD_VIDEOENC_SVC_LAYER {
	HD_SVC_DISABLE = 0,          ///< Disable Scalable Video Coding
	HD_SVC_2X = 1,               ///< 2X Scalable Video Coding
	HD_SVC_4X = 2,               ///< 4X Scalable Video Coding
	HD_SVC_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOENC_SVC_LAYER)
} HD_VIDEOENC_SVC_LAYER;

typedef enum _HD_VIDEOENC_PROFILE {
	//H264 profile
	HD_H264E_BASELINE_PROFILE = 66,
	HD_H264E_MAIN_PROFILE     = 77,
	HD_H264E_HIGH_PROFILE     = 100,

	//H265 profile
	HD_H265E_MAIN_PROFILE     = 1,
	ENUM_DUMMY4WORD(HD_VIDEOENC_PROFILE)
} HD_VIDEOENC_PROFILE;

typedef enum _HD_VIDEOENC_LEVEL {
	//H264 level
	HD_H264E_LEVEL_1   = 10,
	HD_H264E_LEVEL_1_1 = 11,
	HD_H264E_LEVEL_1_2 = 12,
	HD_H264E_LEVEL_1_3 = 13,
	HD_H264E_LEVEL_2   = 20,
	HD_H264E_LEVEL_2_1 = 21,
	HD_H264E_LEVEL_2_2 = 22,
	HD_H264E_LEVEL_3   = 30,
	HD_H264E_LEVEL_3_1 = 31,
	HD_H264E_LEVEL_3_2 = 32,
	HD_H264E_LEVEL_4   = 40,
	HD_H264E_LEVEL_4_1 = 41,
	HD_H264E_LEVEL_4_2 = 42,
	HD_H264E_LEVEL_5   = 50,
	HD_H264E_LEVEL_5_1 = 51,

	//H265 level
	HD_H265E_LEVEL_1   = 30,
	HD_H265E_LEVEL_2   = 60,
	HD_H265E_LEVEL_2_1 = 63,
	HD_H265E_LEVEL_3   = 90,
	HD_H265E_LEVEL_3_1 = 93,
	HD_H265E_LEVEL_4   = 120,
	HD_H265E_LEVEL_4_1 = 123,
	HD_H265E_LEVEL_5   = 150,
	HD_H265E_LEVEL_5_1 = 153,
	HD_H265E_LEVEL_5_2 = 156,
	HD_H265E_LEVEL_6   = 180,
	HD_H265E_LEVEL_6_1 = 183,
	HD_H265E_LEVEL_6_2 = 186,
	ENUM_DUMMY4WORD(HD_VIDEOENC_LEVEL)

} HD_VIDEOENC_LEVEL;


typedef enum _HD_VIDEOENC_CODING {
	//H264 coding
	HD_H264E_CAVLC_CODING = 0,       ///< CAVLC entropy cooding
	HD_H264E_CABAC_CODING = 1,       ///< CABAC entropy cooding

	//H265 coding
	HD_H265E_CABAC_CODING = 2,       ///< CABAC entropy cooding
	ENUM_DUMMY4WORD(HD_VIDEOENC_CODING)
} HD_VIDEOENC_CODING;


typedef enum _HD_VIDEOENC_RC_MODE {
	HD_RC_MODE_CBR = 1,     ///< Constant Bitrate
	HD_RC_MODE_VBR,         ///< Variable Bitrate
	HD_RC_MODE_FIX_QP,      ///< Fix QP
	HD_RC_MODE_EVBR,        ///< Enhanced Variable Bitrate
	HD_RC_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOENC_RC_MODE)
} HD_VIDEOENC_RC_MODE;

typedef enum _HD_VIDEOENC_QPMODE {
	HD_VIDEOENC_QPMODE_DELTA      = 0,  ///< delta
	HD_VIDEOENC_QPMODE_RESERVED   = 1,  ///< reserved
	HD_VIDEOENC_QPMODE_DISABLE_AQ = 2,  ///< disable aq
	HD_VIDEOENC_QPMODE_FIXED_QP   = 3,  ///< fixed qp
	ENUM_DUMMY4WORD(HD_VIDEOENC_QPMODE)
} HD_VIDEOENC_QPMODE;


/************ encode feature ************/
typedef struct _HD_VIDEOENC_SYSCAPS {
	HD_DAL             dev_id;                  ///< device id
	UINT32             chip_id;                 ///< chip id of this device
	UINT32             max_in_count;            ///< max count of input of this device
	UINT32             max_out_count;           ///< max count of output of this device
	HD_DEVICE_CAPS     dev_caps;                            ///< capability of device, using HD_DEVICE_CAPS
	HD_VIDEO_CAPS      in_caps[HD_VIDEOENC_MAX_IN];         ///< capability of input, using HD_VIDEO_CAPS and HD_VIDEOENC_INCAPS
	HD_VIDEOENC_CAPS   out_caps[HD_VIDEOENC_MAX_OUT];       ///< capability of output, using HD_VIDEOENC_CAPS
	UINT32             max_in_stamp;            ///< max input stamp
	UINT32             max_in_stamp_ex;         ///< max input stamp_ex
	UINT32             max_in_mask;             ///< max input mask
	UINT32             max_in_mask_ex;          ///< max input mask_ex
} HD_VIDEOENC_SYSCAPS;

typedef struct _HD_VIDEOENC_BUFINFO {
	HD_BUFINFO buf_info;    ///< physical addr/size of bitstream buffer, for user space to mmap
} HD_VIDEOENC_BUFINFO;

typedef struct _HD_VIDEOENC_MAXMEM {
	HD_VIDEO_CODEC          codec_type;     ///< codec type (need memory: JPEG < H265 < H264)
	HD_DIM                  max_dim;        ///< max support width/height
	UINT32                  bitrate;        ///< target bitrate(unit:bps)
	UINT32                  enc_buf_ms;     ///< bitstream buffer time (unit : ms)
	HD_VIDEOENC_SVC_LAYER   svc_layer;      ///< max svc layer supported
	BOOL                    ltr;            ///< support ltr function : yes/no
	BOOL                    rotate;         ///< support rotate function : yes/no
	BOOL                    source_output;  ///< support snapshot source out buffer : yes/no
} HD_VIDEOENC_MAXMEM;

typedef enum _HD_VIDEOENC_POOL_MODE {
	HD_VIDEOENC_POOL_AUTO    = 0,
	HD_VIDEOENC_POOL_ENABLE  = 1,
	HD_VIDEOENC_POOL_DISABLE = 2,
	ENUM_DUMMY4WORD(HD_VIDEOENC_POOL_MODE),
} HD_VIDEOENC_POOL_MODE;

typedef struct _HD_VIDEOENC_POOL {
	INT     ddr_id;                 ///< DDR ID
	UINT32  counts;                 ///< counts of buffer
	UINT32	max_counts;             ///< max counts of buffer, use HD_VIDEOCAP_SET_COUNT to set
	UINT32	min_counts;             ///< min counts of buffer, use HD_VIDEOCAP_SET_COUNT to set
	HD_VIDEOENC_POOL_MODE  mode;    ///< pool mode
} HD_VIDEOENC_POOL;

typedef struct _HD_VIDEOENC_PATH_CONFIG {
	HD_VIDEOENC_MAXMEM     max_mem;                               ///< maximum memory information
	UINT32                 isp_id;                                ///< ISP id. range: 0~7 or 0xffffffff = ignore
	HD_VIDEOENC_POOL       data_pool[HD_VIDEOENC_MAX_DATA_TYPE];  ///< pool memory information
} HD_VIDEOENC_PATH_CONFIG;

typedef enum _HD_VIDEOENC_INFUNC {
	HD_VIDEOENC_INFUNC_ONEBUF =     0x00000002, ///< enable one-buffer from vdoprc
	HD_VIDEOENC_INFUNC_LOWLATENCY = 0x00000004, ///< enable low-latency from vdoprc
	ENUM_DUMMY4WORD(HD_VIDEOENC_INFUNC)
} HD_VIDEOENC_INFUNC;

typedef struct _HD_VIDEOENC_FUNC_CONFIG {
	UINT32                 ddr_id;      ///< DDR ID
	HD_VIDEOENC_INFUNC     in_func;     ///< additional function of in (bit-wise mask)
} HD_VIDEOENC_FUNC_CONFIG;

typedef struct _HD_VIDEOENC_FRC {
	HD_VIDEO_FRC frc;                   ///< frame-control
} HD_VIDEOENC_FRC;

typedef struct _HD_VIDEOENC_IN {
	HD_DIM          dim;                ///< encode width/height
	HD_VIDEO_PXLFMT pxl_fmt;            ///< source format
	HD_VIDEO_DIR    dir;                ///< input direction
	HD_VIDEO_FRC    frc;                ///< input frame-control
} HD_VIDEOENC_IN;

typedef struct _HD_H26X_CONFIG {
	UINT32          gop_num;            ///< I frame period. range: 0~4096 (0: only one I frame)
	INT8            chrm_qp_idx;        ///< chroma qp offset.        default: 0, range: -12~12
	INT8            sec_chrm_qp_idx;    ///< second chroma QP offset. default: 0, range: -12~12
	UINT32          ltr_interval;       ///< long-term reference frame interval. default: 0, range: 0~4095 (0: disable)
	BOOL            ltr_pre_ref;        ///< long-term reference setting. default: 0, range: 0~1 (0: all long-term reference to IDR frame, 1: reference latest long-term reference frame)
	BOOL            gray_en;            ///< encode color to gray. default: 0, range: 0~1 (0: disable, 1: enable)
	BOOL            source_output;      ///< Source output. default: 0, range: 0~1 (0: disable, 1: enable)
	HD_VIDEOENC_PROFILE profile;        ///< Profile IDC. default(H.264): VDOENC_H264E_HIGH_PROFILE, default(H.265): VDOENC_H265E_MAIN_PROFILE
	HD_VIDEOENC_LEVEL level_idc;        ///< Level IDC. default(H.264): VDOENC_H264E_LEVEL_4_1, default(H.265): VDOENC_H265E_LEVEL_5_0
	HD_VIDEOENC_SVC_LAYER svc_layer;    ///< SVC Layer. default: 0, range: 0~2 (0: disable, 1: 2x, 2: 4x)
	HD_VIDEOENC_CODING entropy_mode;    ///< Entropy coding method. default: 1, range(H.264): 0~1, range(H.265): 1 (0: CAVLC, 1: CABAC)
} HD_H26X_CONFIG;

typedef struct _HD_JPEG_CONFIG {
	UINT32          retstart_interval;  ///< JPEG restart interval. default: 0, range: 0~image size/256
	UINT32          image_quality;      ///< JPEG image quality. default: 50, range 1~100
} HD_JPEG_CONFIG;

typedef struct _HD_JPEG_CONFIG2 {
	UINT32          retstart_interval;  ///< JPEG restart interval. default: 0, range: 0~image size/256
	UINT32          image_quality;      ///< JPEG image quality. default: 50, range 1~100, (1) if bitrate=0, this means fixed quality value (2) if bitrate>0, this means initial quality for CBR
	UINT32          bitrate;            ///< Bit rate (bits per second)  , 0: fix quality mode , >0: bitrate for CBR mode
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
} HD_JPEG_CONFIG2;


typedef struct _HD_VIDEOENC_OUT {
	HD_VIDEO_CODEC  codec_type;         ///< codec type
	union {
		HD_H26X_CONFIG  h26x;           ///< H26x config
		HD_JPEG_CONFIG  jpeg;           ///< Jpeg config
	};
} HD_VIDEOENC_OUT;

typedef struct _HD_VIDEOENC_OUT2 {
	HD_VIDEO_CODEC  codec_type;         ///< codec type
	union {
		HD_H26X_CONFIG  h26x;           ///< H26x config
		HD_JPEG_CONFIG2 jpeg;           ///< Jpeg config
	};
} HD_VIDEOENC_OUT2;

/************ vui ************/
typedef struct _HD_H26XENC_VUI {
	BOOL            vui_en;             ///< enable vui. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32          sar_width;          ///< Horizontal size of the sample aspect ratio. default: 0, range: 0~65535
	UINT32          sar_height;         ///< Vertical size of the sample aspect rat. default: 0, range: 0~65535
	UINT8           matrix_coef;        ///< Matrix coefficients are used to derive the luma and Chroma signals from green, blue, and red primaries. default: 2, range: 0~255
	UINT8           transfer_characteristics; ///< The opto-electronic transfers characteristic of the source pictures. default: 2, range: 0~255
	UINT8           colour_primaries;   ///< Chromaticity coordinates the source primaries. default: 2, range: 0~255
	UINT8           video_format;       ///< Indicate the representation of pictures. default: 5, range: 0~7
	UINT8           color_range;        ///< Indicate the black level and range of the luma and Chroma signals. default: 0, range: 0~1 (0: Not full range, 1: Full range)
	BOOL            timing_present_flag; ///< timing info present flag. default: 0, range: 0~1 (0: disable, 1: enable)
} HD_H26XENC_VUI;

/************ deblock ************/
#define MAKE_DIS_ILF_IDC(db_disable, across_slice_en, across_tile_en) (db_disable ? 1:((across_slice_en ? 2:0)|(across_tile_en ? 4:0))) ///< db_disable: 0~1 (1: No Filter, 0: Filter), H264/H265.
																	///< across_slice_en: enable deblocking across slice  (1: enable, 0: disable), only work if db_disable = 0(Filter), H264/H265.
																	///< across_tile_en:  enable deblocking across  tile  (1: enable, 0: disable), only work if db_disable = 0(Filter), H265 ONLY.

typedef struct _HD_H26XENC_DEBLOCK {
	UINT8           dis_ilf_idc;        ///< Disable loop filter in slice header. default: 0, range: 0~6 , please use MAKE_DIS_ILF_IDC() to set this value.
	INT8            db_alpha;           ///< Alpha & C0 offset. default: 0, range: -12~12
	INT8            db_beta;            ///< Beta offset.       default: 0, range: -12~12
} HD_H26XENC_DEBLOCK;

/************ rc ************/
typedef struct _HD_H26XENC_CBR {
	UINT32          bitrate;            ///< Bit rate (bits per second)
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
	UINT8           init_i_qp;          ///< Rate control's init I qp.        default: 26, range: 0~51
	UINT8           max_i_qp;           ///< Rate control's max I qp.         default: 51, range: 0~51
	UINT8           min_i_qp;           ///< Rate control's min I qp.         default:  1, range: 0~51
	UINT8           init_p_qp;          ///< Rate control's init P qp.        default: 26, range: 0~51
	UINT8           max_p_qp;           ///< Rate control's max P qp.         default: 51, range: 0~51
	UINT8           min_p_qp;           ///< Rate control's min P qp.         default:  1, range: 0~51
	UINT32          static_time;        ///< Rate control's static time.      default:  0, range: 0~20
	INT32           ip_weight;          ///< Rate control's I/P frame weight. default:  0, range: -100~100
} HD_H26XENC_CBR;

typedef struct _HD_H26XENC_CBR2 {
	UINT32          bitrate;            ///< Bit rate (bits per second)
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
	UINT8           init_i_qp;          ///< Rate control's init I qp.        default: 26, range: 0~51
	UINT8           max_i_qp;           ///< Rate control's max I qp.         default: 51, range: 0~51
	UINT8           min_i_qp;           ///< Rate control's min I qp.         default:  1, range: 0~51
	UINT8           init_p_qp;          ///< Rate control's init P qp.        default: 26, range: 0~51
	UINT8           max_p_qp;           ///< Rate control's max P qp.         default: 51, range: 0~51
	UINT8           min_p_qp;           ///< Rate control's min P qp.         default:  1, range: 0~51
	UINT32          static_time;        ///< Rate control's static time.      default:  0, range: 0~20
	INT32           ip_weight;          ///< Rate control's I/P frame weight. default:  0, range: -100~100
	UINT32          key_p_period;       ///< Key P frame interval.  default: 0, range: 0~4096
	INT32           kp_weight;          ///< Rate control's KP/P frame weight. default: 0, range: -100~100
	INT32           p2_weight;          ///< Rate control's SVC P2/P frame weight. default: 0, range: -100~100
	INT32           p3_weight;          ///< Rate control's SVC P3/P frame weight. default: 0, range: -100~100
	INT32           lt_weight;          ///< Rate control's long term P/P frame weight. default: 0, range: -100~100
	INT32           motion_aq_str;      ///< Motion aq strength for smart ROI. default: 0, range: -15~15
	UINT32          max_frame_size;     ///< max frame size. Default: 0, 0: limited by buffer size. Others: max size limited (byte)
} HD_H26XENC_CBR2;

typedef struct _HD_H26XENC_VBR {
	UINT32          bitrate;            ///< Bit rate (bits per second)
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
	UINT8           init_i_qp;          ///< Rate control's init I qp.        default: 26, range: 0~51
	UINT8           max_i_qp;           ///< Rate control's max I qp.         default: 51, range: 0~51
	UINT8           min_i_qp;           ///< Rate control's min I qp.         default:  1, range: 0~51
	UINT8           init_p_qp;          ///< Rate control's init P qp.        default: 26, range: 0~51
	UINT8           max_p_qp;           ///< Rate control's max P qp.         default: 51, range: 0~51
	UINT8           min_p_qp;           ///< Rate control's min P qp.         default:  1, range: 0~51
	UINT32          static_time;        ///< Rate control's static time.      default:  0, range: 0~20
	INT32           ip_weight;          ///< Rate control's I/P frame weight. default:  0, range: -100~100
	UINT32          change_pos;         ///< Early limit bitate.              default:  0, range: 0~100 (0: disable)
} HD_H26XENC_VBR;

typedef struct _HD_H26XENC_VBR2 {
	UINT32          bitrate;            ///< Bit rate (bits per second)
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
	UINT8           init_i_qp;          ///< Rate control's init I qp.        default: 26, range: 0~51
	UINT8           max_i_qp;           ///< Rate control's max I qp.         default: 51, range: 0~51
	UINT8           min_i_qp;           ///< Rate control's min I qp.         default:  1, range: 0~51
	UINT8           init_p_qp;          ///< Rate control's init P qp.        default: 26, range: 0~51
	UINT8           max_p_qp;           ///< Rate control's max P qp.         default: 51, range: 0~51
	UINT8           min_p_qp;           ///< Rate control's min P qp.         default:  1, range: 0~51
	UINT32          static_time;        ///< Rate control's static time.      default:  0, range: 0~20
	INT32           ip_weight;          ///< Rate control's I/P frame weight. default:  0, range: -100~100
	UINT32          change_pos;         ///< Early limit bitate.              default:  0, range: 0~100 (0: disable)
	UINT32          key_p_period;       ///< Key P frame interval.  default: 0, range: 0~4096
	INT32           kp_weight;          ///< Rate control's KP/P frame weight. default: 0, range: -100~100
	INT32           p2_weight;          ///< Rate control's SVC P2/P frame weight. default: 0, range: -100~100
	INT32           p3_weight;          ///< Rate control's SVC P3/P frame weight. default: 0, range: -100~100
	INT32           lt_weight;          ///< Rate control's long term P/P frame weight. default: 0, range: -100~100
	INT32           motion_aq_str;      ///< Motion aq strength for smart ROI. default: 0, range: -15~15
	UINT32          max_frame_size;     ///< max frame size. Default: 0, 0: limited by buffer size. Others: max size limited (byte)
} HD_H26XENC_VBR2;

typedef struct _HD_H26XENC_FIXQP {
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
	UINT8           fix_i_qp;           ///< Fix qp of I frame. default: 26, range: 0~51
	UINT8           fix_p_qp;           ///< Fix qp of P frame. default: 26, range: 0~51
} HD_H26XENC_FIXQP;

typedef struct _HD_H26XENC_EVBR {
	UINT32          bitrate;            ///< Bit rate (bits per second)
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
	UINT8           init_i_qp;          ///< Rate control's init I qp.        default: 26, range: 0~51
	UINT8           max_i_qp;           ///< Rate control's max I qp.         default: 51, range: 0~51
	UINT8           min_i_qp;           ///< Rate control's min I qp.         default:  1, range: 0~51
	UINT8           init_p_qp;          ///< Rate control's init P qp.        default: 26, range: 0~51
	UINT8           max_p_qp;           ///< Rate control's max P qp.         default: 51, range: 0~51
	UINT8           min_p_qp;           ///< Rate control's min P qp.         default:  1, range: 0~51
	UINT32          static_time;        ///< Rate control's static time.      default:  0, range: 0~20
	INT32           ip_weight;          ///< Rate control's I/P frame weight. default:  0, range: -100~100
	UINT32          key_p_period;       ///< Key P frame interval.  default: frame rate*2, range: 0~4096
	INT32           kp_weight;          ///< Rate control's KP/P frame weight. default: 0, range: -100~100
	UINT32          still_frame_cnd;    ///< Condition of still environment of EVBR. default: 100, range: 1~4096
	UINT32          motion_ratio_thd;   ///< Threshold of motion ratio to decide motion frame and still frame. default: 30, range: 1~100
	INT32           motion_aq_str;      ///< Motion aq strength for smart ROI. default: -6, range: -15~15
	UINT32          still_i_qp;         ///< Still mode qp of I frame.        default: 28, range: 0~51
	UINT32          still_p_qp;         ///< Still mode qp of P frame.        default: 30, range: 0~51
	UINT32          still_kp_qp;        ///< Still mode qp of key P frame.    default: 36, range: 0~51
} HD_H26XENC_EVBR;

typedef struct _HD_H26XENC_EVBR2 {
	UINT32          bitrate;            ///< Bit rate (bits per second)
	UINT32          frame_rate_base;    ///< Frame rate
	UINT32          frame_rate_incr;    ///< Frame rate = uiFrameRateBase / uiFrameRateIncr
	UINT8           init_i_qp;          ///< Rate control's init I qp.        default: 26, range: 0~51
	UINT8           max_i_qp;           ///< Rate control's max I qp.         default: 51, range: 0~51
	UINT8           min_i_qp;           ///< Rate control's min I qp.         default:  1, range: 0~51
	UINT8           init_p_qp;          ///< Rate control's init P qp.        default: 26, range: 0~51
	UINT8           max_p_qp;           ///< Rate control's max P qp.         default: 51, range: 0~51
	UINT8           min_p_qp;           ///< Rate control's min P qp.         default:  1, range: 0~51
	UINT32          static_time;        ///< Rate control's static time.      default:  0, range: 0~20
	INT32           ip_weight;          ///< Rate control's I/P frame weight. default:  0, range: -100~100
	UINT32          key_p_period;       ///< Key P frame interval.  default: frame rate*2, range: 0~4096
	INT32           kp_weight;          ///< Rate control's KP/P frame weight. default: 0, range: -100~100
	UINT32          still_frame_cnd;    ///< Condition of still environment of EVBR. default: 100, range: 1~4096
	UINT32          motion_ratio_thd;   ///< Threshold of motion ratio to decide motion frame and still frame. default: 30, range: 1~100
	INT32           motion_aq_str;      ///< Motion aq strength for smart ROI. default: -6, range: -15~15
	UINT32          still_i_qp;         ///< Still mode qp of I frame.        default: 28, range: 0~51
	UINT32          still_p_qp;         ///< Still mode qp of P frame.        default: 36, range: 0~51
	UINT32          still_kp_qp;        ///< Still mode qp of key P frame.    default: 30, range: 0~51
	INT32           p2_weight;          ///< Rate control's SVC P2/P frame weight. default: 0, range: -100~100
	INT32           p3_weight;          ///< Rate control's SVC P3/P frame weight. default: 0, range: -100~100
	INT32           lt_weight;          ///< Rate control's long term P/P frame weight. default: 0, range: -100~100
	UINT32          max_frame_size;     ///< max frame size. Default: 0, 0: limited by buffer size. Others: max size limited (byte)
} HD_H26XENC_EVBR2;

typedef struct _HD_H26XENC_RATE_CONTROL {
	HD_VIDEOENC_RC_MODE    rc_mode;      ///< rate control mode. default: 1, range: 1~4 (1: CBR, 2: VBR, 3: FixQP, 4: EVBR)
	union {
		HD_H26XENC_CBR     cbr;          ///< parameter of rate control mode CBR
		HD_H26XENC_VBR     vbr;          ///< parameter of rate control mode VBR
		HD_H26XENC_FIXQP   fixqp;        ///< parameter of rate control mode FixQP
		HD_H26XENC_EVBR    evbr;         ///< parameter of rate control mode EVBR
	};
} HD_H26XENC_RATE_CONTROL;

typedef struct _HD_H26XENC_RATE_CONTROL2 {
	HD_VIDEOENC_RC_MODE    rc_mode;      ///< rate control mode. default: 1, range: 1~4 (1: CBR, 2: VBR, 3: FixQP, 4: EVBR)
	union {
		HD_H26XENC_CBR2    cbr;          ///< parameter of rate control mode CBR
		HD_H26XENC_VBR2    vbr;          ///< parameter of rate control mode VBR
		HD_H26XENC_FIXQP   fixqp;        ///< parameter of rate control mode FixQP
		HD_H26XENC_EVBR2   evbr;         ///< parameter of rate control mode EVBR
	};
} HD_H26XENC_RATE_CONTROL2;

/************ user qp ************/
#define MAKE_QP_MAP_ADDR(mode, qp)     ((mode << 6) | (qp))    ///< macro to make qp_map_addr

typedef struct _HD_H26XENC_USR_QP {
	BOOL            enable;             ///< enable user qp. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32          qp_map_addr;        ///< buffer address of user qp map.
} HD_H26XENC_USR_QP;

/************ multi slice ************/
typedef struct _HD_H26XENC_SLICE_SPLIT {
	UINT32          enable;             ///< enable multiple slice. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32          slice_row_num;      ///< number of macroblock/ctu rows occupied by a slice, range: 1 ~ number of macroblock/ctu row
} HD_H26XENC_SLICE_SPLIT;

/************ gdr ************/
typedef struct _HD_H26XENC_GDR {
	BOOL            enable;             ///< enable gdr. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32          period;             ///< intra refresh period. default: 0, range: 0~0xFFFFFFFF (0: always refresh, others: intra refresh frame period)
	UINT32          number;             ///< intra refresh row number. default: 1, range: 1 ~ number of macroblock/ctu row
} HD_H26XENC_GDR;

/************ roi ************/
#define HD_H26XENC_ROI_WIN_COUNT  10    ///< ROI windows count
typedef struct _HD_H26XENC_ROI_WIN {
	BOOL                enable;         ///< enable roi qp. default: 0, range: 0~1 (0: disable, 1: enable)
	HD_URECT            rect;           ///< rectangle of roi
	HD_VIDEOENC_QPMODE  mode;           ///< could set for individual window. available value: HD_VIDEOENC_QPMODE_DELTA / HD_VIDEOENC_QPMODE_DISABLE_AQ / HD_VIDEOENC_QPMODE_FIXED_QP
	INT8                qp;             ///< qp value. default: 0.
										///<   if roi_qp_mode is HD_VIDEOENC_QPMODE_FIXED_QP, then qp value means fixed qp (range: 0~51); otherwise HD_VIDEOENC_QPMODE_DELTA then qp value means delta qp, (range: -26~25)
										///<   if qp mode is HD_VIDEOENC_QPMODE_FIXED_QP, then qp value means fixed qp (range: 0~51); otherwise HD_VIDEOENC_QPMODE_DELTA then qp value means delta qp, (range: -32~31)
} HD_H26XENC_ROI_WIN;

typedef struct _HD_H26XENC_ROI {
	HD_VIDEOENC_QPMODE    roi_qp_mode;  ///< roi qp mode for all windows.  available value: HD_VIDEOENC_QPMODE_FIXED_QP(default) / HD_VIDEOENC_QPMODE_DELTA
	HD_H26XENC_ROI_WIN    st_roi[HD_H26XENC_ROI_WIN_COUNT];   ///< roi window settings.
										///< ROIs can be overlaid, and the priority of the ROIs is based on index number, index 0 is highest priority and index 9 is lowest.
} HD_H26XENC_ROI;

/************ row rc ************/
typedef struct _HD_H26XENC_ROW_RC {
	BOOL            enable;             ///< enable row rc. default: 1, range: 0~1 (0: disable, 1: enable)
	UINT8           i_qp_range;         ///< qp range of I/P frame for row-level rata control.
	UINT8           i_qp_step;          ///< qp step of I/P frame for row-level rata control.
	UINT8           p_qp_range;         ///< qp range of P frame for row-level rata control. default: 4, range: 0~15
	UINT8           p_qp_step;          ///< qp step  of P frame for row-level rata control. default: 1, range: 0~15
	UINT8           min_i_qp;           ///< min qp of I frame for row-level rata control. default:  1, range: 0~51
	UINT8           max_i_qp;           ///< max qp of I frame for row-level rata control. default: 51, range: 0~51
	UINT8           min_p_qp;           ///< min qp of P frame for row-level rata control. default:  1, range: 0~51
	UINT8           max_p_qp;           ///< max qp of P frame for row-level rata control. default: 51, range: 0~51
} HD_H26XENC_ROW_RC;

/************ aq ************/
#define HD_H26XENC_AQ_MAP_TABLE_NUM  30 ///< AP table num
typedef struct _HD_H26XENC_AQ {
	BOOL            enable;             ///< AQ enable. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT8           i_str;              ///< aq strength of I frame. default: 3, range: 1~8
	UINT8           p_str;              ///< aq strength of P frame. default: 1, range: 1~8
	INT8            max_delta_qp;       ///< max delta qp of aq.
	INT8            min_delta_qp;       ///< min delta qp of aq.
	UINT8           depth;              ///< AQ depth. default: 2, range(H.264): 2, range(H.265): 0~2 (0: cu64, 1: cu32, 2: cu16)
	INT16           thd_table[HD_H26XENC_AQ_MAP_TABLE_NUM];
										///< non-linear AQ mapping table. range: -512~511, default: {-120,-112,-104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 7, 15, 23, 31, 39,47, 55, 63, 71, 79, 87, 95, 103, 111, 119}
										///<           for ( dqp = -15; dqp < 15; dqp++ ) if ( Cu.RelativeTextureComplexity(x_str) <= thd_table[dqp+15] ) break; Cu.DeltaQP_AQ = MIN ( MAX (min_delta_qp, dqp), max_delta_qp );
} HD_H26XENC_AQ;

typedef struct _HD_VIDEO_ENC_PSNR_INFO {
	UINT32 y_mse;               ///< mean square error of y. psnr = 10 x log10(255*255 / mse)
	UINT32 u_mse;               ///< mean square error of cb. psnr = 10 x log10(255*255 / mse)
	UINT32 v_mse;               ///< mean square error of cr. psnr = 10 x log10(255*255 / mse)
} HD_VIDEO_ENC_PSNR_INFO;

typedef struct _HD_VIDEO_ENC_QP_RATIO {
	UINT8 qp;                           ///< user specified qp value, range: 0~51
	UINT8 ratio;                        ///< the compress ratio of qp value
} HD_VIDEO_ENC_QP_RATIO;

typedef struct _HD_VIDEO_ENC_MIN_COMPRESS_RATIO {
	HD_VIDEO_CODEC  codec_type;         ///< codec type
	HD_VIDEO_ENC_QP_RATIO qp_ratio;     ///< the qp ratio structure
} HD_VIDEO_ENC_MIN_COMPRESS_RATIO;

typedef struct _HD_H26XENC_REQUEST_IFRAME {
	BOOL            enable;             ///< request i-frame enable. default: 0, range: 0~1 (0: disable, 1: enable)
} HD_H26XENC_REQUEST_IFRAME;

typedef struct _HD_H26XENC_TRIG_SNAPSHOT {
	UINT32 phy_addr;                    ///< [w]physical address of encoded data (user provide memory space to put JPEG result)
	UINT32 size;                        ///< [w]user buffer size provided
										///< [r]real size of encoded data
	UINT32 image_quality;               ///< JPEG image quality. range 1~100
} HD_H26XENC_TRIG_SNAPSHOT;

typedef struct {
	BOOL event;                         ///< poll status
	UINT32 bs_size;                     ///< bitstream size
	HD_FRAME_TYPE frame_type;           ///< The frame type
	HD_SVC_LAYER_TYPE svc_layer_type;   ///< svc layer type
} HD_ENC_RET_EVENT;

typedef struct _HD_VIDEOENC_POLL_LIST {
	HD_PATH_ID path_id;                 ///< path ID
	HD_ENC_RET_EVENT revent;            ///< The returned event value
} HD_VIDEOENC_POLL_LIST;

typedef struct _HD_VIDEOENC_USER_BS_DATA {
	HD_NALU_TYPE pack_type;             ///< Pack Type
	UINT32 user_buf_addr;               ///< user buffer address of encoded data
	UINT32 size;                        ///< size of bs data
} HD_VIDEOENC_USER_BS_DATA;

#define VENC_USER_BS_PACK_MAX        4       ///< bs pack max num
#define VENC_USER_SLICE_MAX          16      ///< max user slice count

typedef struct _HD_VIDEOENC_USER_BS {
	UINT32 sign;                             ///< signature = MAKEFOURCC('V','S','T','M')
	HD_METADATA* p_next;                     ///< pointer to next meta
	HD_VIDEO_CODEC vcodec_format;            ///< Encoded format of video frame
	UINT32 pack_num;                         ///< Pack number in video frame
	UINT64 timestamp;                        ///< Encode bs timestamp (unit: microsecond)
	HD_FRAME_TYPE frame_type;                ///< The frame type
	HD_SVC_LAYER_TYPE svc_layer_type;        ///< svc layer type
	HD_VIDEOENC_USER_BS_DATA video_pack[VENC_USER_BS_PACK_MAX];  ///< Pack array of encoded data
	HD_VIDEO_BS_PSNR_INFO psnr_info;         ///< The PSNR information
	HD_VIDEO_BS_BLOCK_PARTITION_INFO blk_info;  ///< The block partition information
	UINT32 newbs_flag;                       ///< Flag notification of new seting, such as GM_FLAG_NEW_BITRATE
	UINT32 qp;                               ///< The qp value
	HD_VIDEO_EVBR_STATE evbr_state;          ///< The evbr state value (significant when rate-control is EVBR)
	UINT16 slice_offset[VENC_USER_SLICE_MAX];///< multi-slice offset 0~VENC_USER_SLICE_MAX
	UINT32 motion_ratio;                     ///< motion ratio
	/* provide by application */
	CHAR *p_user_buf;                        ///< Bitstream buffer pointer
	UINT32 user_buf_size;                    ///< AP provide bs_buf max size
} HD_VIDEOENC_USER_BS;

typedef struct _HD_VIDEOENC_RECV_LIST {
	HD_PATH_ID path_id;                      ///< path ID
	HD_VIDEOENC_USER_BS user_bs;             ///< HD_VIDEOENC_USER_BS struct
	INT32 retval;                            ///< less than 0: recv bistream fail.
} HD_VIDEOENC_RECV_LIST;

typedef struct _HD_VIDEOENC_STATUS {
	UINT32      left_frames;                 ///< number of frames to be encoded
	UINT32      done_frames;                 ///< number of encoded frames
	UINT32      reserved[8];                 ///< reserved for future using
} HD_VIDEOENC_STATUS;

typedef enum _HD_VIDEOENC_PARAM_ID {
	HD_VIDEOENC_PARAM_DEVCOUNT,           ///< support get with ctrl path, using HD_DEVCOUNT struct (device id max count)
	HD_VIDEOENC_PARAM_SYSCAPS,            ///< support get with ctrl path, using HD_VIDEOENC_SYSCAPS struct (system capabilitiy)
	HD_VIDEOENC_PARAM_PATH_CONFIG,        ///< support get/set with i/o path, using HD_VIDEOENC_PATH_CONFIG struct
	HD_VIDEOENC_PARAM_BUFINFO,            ///< support get with i/o path, using HD_VIDEOENC_BUFINFO struct
	HD_VIDEOENC_PARAM_IN,                 ///< support get/set with i/o path,  using HD_VIDEOENC_IN struct
	HD_VIDEOENC_PARAM_OUT_ENC_PARAM,      ///< support get/set with i/o path, using HD_VIDEOENC_OUT struct
	HD_VIDEOENC_PARAM_OUT_VUI,            ///< support get/set with i/o path, using HD_H26XENC_VUI struct
	HD_VIDEOENC_PARAM_OUT_DEBLOCK,        ///< support get/set with i/o path, using HD_H26XENC_DEBLOCK struct
	HD_VIDEOENC_PARAM_OUT_RATE_CONTROL,   ///< support get/set with i/o path, using HD_H26XENC_RATE_CONTROL struct
	HD_VIDEOENC_PARAM_OUT_USR_QP,         ///< support get/set with i/o path, using HD_H26XENC_USR_QP struct
	HD_VIDEOENC_PARAM_OUT_SLICE_SPLIT,    ///< support get/set with i/o path, using HD_H26XENC_SLICE_SPLIT struct
	HD_VIDEOENC_PARAM_OUT_ENC_GDR,        ///< support get/set with i/o path, using HD_H26XENC_GDR struct
	HD_VIDEOENC_PARAM_OUT_ROI,            ///< support get/set with i/o path, using HD_H26XENC_ROI struct
	HD_VIDEOENC_PARAM_OUT_ROW_RC,         ///< support get/set with i/o path, using HD_H26XENC_ROW_RC struct
	HD_VIDEOENC_PARAM_OUT_AQ,             ///< support get/set with i/o path, using HD_H26XENC_AQ struct
	HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, ///< support set with i/o path, using HD_H26XENC_REQUEST_IFRAME struct
	HD_VIDEOENC_PARAM_OUT_TRIG_SNAPSHOT,  ///< support set with i/o path, using HD_H26XENC_TRIG_SNAPSHOT struct
	HD_VIDEOENC_PARAM_IN_STAMP_BUF,       ///< support get/set with i/stamp path, using HD_OSG_STAMP_BUF struct (stamp buffer parameter)
	HD_VIDEOENC_PARAM_IN_STAMP_IMG,       ///< support get/set with i/stamp path, using HD_OSG_STAMP_IMG struct (stamp image parameter)
	HD_VIDEOENC_PARAM_IN_STAMP_ATTR,      ///< support get/set with i/stamp path, using HD_OSG_STAMP_ATTR struct (stamp display attribute)
	HD_VIDEOENC_PARAM_IN_MASK_ATTR,       ///< support get/set with i/mask path, using HD_OSG_MASK_ATTR struct (mask display attribute)
	HD_VIDEOENC_PARAM_IN_MOSAIC_ATTR,     ///< support get/set with i/mask path, using HD_OSG_MOSAIC_ATTR struct (mosaic display attribute)
	HD_VIDEOENC_PARAM_IN_PALETTE_TABLE,   ///< support get/set with i path, using HD_OSG_PALETTE_TBL struct and HD_PALETTE_TBL struct
	HD_VIDEOENC_PARAM_STATUS,             ///< support get with i/o path, using HD_VIDEOENC_STATUS struct
	HD_VIDEOENC_PARAM_IN_FRC,             ///< support get/set with i/o path, using HD_VIDEOENC_FRC struct (input frc parameter)
	HD_VIDEOENC_PARAM_MIN_COMPRESS_RATIO, ///< support get/set with ctrl path, using HD_VIDEO_ENC_MIN_COMPRESS_RATIO struct
	HD_VIDEOENC_PARAM_FUNC_CONFIG,        ///< support get/set with i/o path, using HD_VIDEOENC_FUNC_CONFIG struct
	HD_VIDEOENC_PARAM_OUT_ENC_PARAM2,     ///< support get/set with i/o path, using HD_VIDEOENC_OUT2 struct
	HD_VIDEOENC_PARAM_OUT_RATE_CONTROL2,  ///< support get/set with i/o path, using HD_H26XENC_RATE_CONTROL2 struct
	HD_VIDEOENC_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOENC_PARAM_ID)
} HD_VIDEOENC_PARAM_ID;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT hd_videoenc_init(VOID);

HD_RESULT hd_videoenc_bind(HD_OUT_ID out_id, HD_IN_ID dest_in_id);
HD_RESULT hd_videoenc_unbind(HD_OUT_ID out_id);
HD_RESULT hd_videoenc_open(HD_IN_ID in_id, HD_OUT_ID out_id, HD_PATH_ID *p_path_id);
HD_RESULT hd_videoenc_start(HD_PATH_ID path_id);
HD_RESULT hd_videoenc_stop(HD_PATH_ID path_id);
HD_RESULT hd_videoenc_start_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_videoenc_stop_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_videoenc_close(HD_PATH_ID path_id);

HD_RESULT hd_videoenc_get(HD_PATH_ID path_id, HD_VIDEOENC_PARAM_ID id, VOID *p_param);
HD_RESULT hd_videoenc_set(HD_PATH_ID path_id, HD_VIDEOENC_PARAM_ID id, VOID *p_param);
HD_RESULT hd_videoenc_push_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_in_video_frame, HD_VIDEOENC_BS *p_user_out_videoenc_bs, INT32 wait_ms);
HD_RESULT hd_videoenc_pull_out_buf(HD_PATH_ID path_id, HD_VIDEOENC_BS *p_videoenc_bs, INT32 wait_ms);
HD_RESULT hd_videoenc_release_out_buf(HD_PATH_ID path_id, HD_VIDEOENC_BS *p_videoenc_bs);

HD_RESULT hd_videoenc_poll_list(HD_VIDEOENC_POLL_LIST *p_poll, UINT32 num, INT32 wait_ms);
HD_RESULT hd_videoenc_recv_list(HD_VIDEOENC_RECV_LIST *p_videoenc_list, UINT32 num);

HD_RESULT hd_videoenc_uninit(VOID);

#endif

