/**
	@brief Header file of video decoder module.\n
	This file contains the functions which is related to video decoder in the chip.

	@file hd_videodec.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef	_HD_VIDEODEC_H_
#define	_HD_VIDEODEC_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define HD_VIDEODEC_MAX_IN                 64      ///< max count of HD_IN() of this device (interface)
#define HD_VIDEODEC_MAX_OUT                64      ///< max count of HD_OUT() of this device (interface)
#define HD_VIDEODEC_MAX_DATA_TYPE          4       ///< max count of output pool of this device (interface)

/**
	@name HD_VIDEODEC flag for HD_VIDEODEC_USER_BS.user_flag
*/
#define HD_VIDEODEC_FLAG_NOT_DISPLAY       0x00000001   ///< decode it but not to display

/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/
#define HD_VIDEODEC_SET_COUNT(a, b)		((a)*10)+(b)	///< ex: use HD_VIDEODEC_SET_COUNT(1, 5) for setting 1.5
/********************************************************************
	TYPE DEFINITION
********************************************************************/
/**
     @name HD_VIDEODEC capability
*/
typedef enum _HD_VIDEODEC_CAPS {
	HD_VIDEODEC_CAPS_JPEG                   = 0x00000001, ///< support decode from jpeg
	HD_VIDEODEC_CAPS_H264                   = 0x00000002, ///< support decode from h264
	HD_VIDEODEC_CAPS_H265                   = 0x00000004, ///< support decode from h265
	ENUM_DUMMY4WORD(HD_VIDEODEC_CAPS)
} HD_VIDEODEC_CAPS;

typedef struct _HD_VIDEODEC_SYSCAPS {
	HD_DAL            dev_id;                        ///< device id
	UINT32            chip;                          ///< chip id of hardware
	UINT32            max_in_count;                  ///< supported max count of HD_IN()
	UINT32            max_out_count;                 ///< supported max count of HD_OUT()
	HD_DEVICE_CAPS    dev_caps;                      ///< capability of device, using HD_DEVICE_CAPS
	HD_VIDEODEC_CAPS  in_caps[HD_VIDEODEC_MAX_IN];   ///< capability of input, using HD_VIDEODEC_CAPS
	HD_VIDEO_CAPS     out_caps[HD_VIDEODEC_MAX_OUT]; ///< capability of output, using HD_VIDEO_CAPS
	HD_DIM            max_dim;                       ///< max dimension of videodecoder
	UINT32            max_bitrate;                   ///< max bitrate of videodecoder
} HD_VIDEODEC_SYSCAPS;

typedef enum _HD_VIDEODEC_ALIGN {
	HD_VIDEODEC_TIME_ALIGN_ENABLE  = 0xFEFE01FE,   ///< (default) playback time align by LCD period (ex. 60HZ is 33333us)
	HD_VIDEODEC_TIME_ALIGN_DISABLE = 0xFEFE07FE,   ///< play timestamp by hd_videodec_send_list called
	HD_VIDEODEC_TIME_ALIGN_USER    = 0xFEFE09FE,   ///< start to play at previous play point + time_diff(us)
	ENUM_DUMMY4WORD(HD_VIDEODEC_ALIGN)
} HD_VIDEODEC_ALIGN;

typedef struct _HD_VIDEODEC_USER_BS {
	UINT32            sign;                        ///< signature = MAKEFOURCC('V','S','T','M')
	HD_METADATA       *p_next;                     ///< pointer to next meta
	CHAR              *p_bs_buf;                   ///< bitstream buffer address pointer
	UINT32            bs_buf_size;                 ///< bitstream buffer size
	INT32             retval;                      ///< less than 0: send bistream fail.
	HD_VIDEODEC_ALIGN time_align;                  ///< timestamp alignment
	UINT32            time_diff;                   ///< time_diff(us): playback interval time by micro-second
	UINT64            timestamp;                   ///< Decode bs timestamp (unit: microsecond) to encode for transcode
	UINT32            user_flag;                   ///< Special flag to control, using HD_VIDEODEC_FLAG_XXX definitions
	UINT32            reserved[2];                 ///< reserved for future using
} HD_VIDEODEC_USER_BS;

typedef struct _HD_VIDEODEC_SEND_LIST {
	HD_PATH_ID          path_id;                   ///< path id
	HD_VIDEODEC_USER_BS user_bs;                   ///< video decode user bitstream
	INT32               retval;                    ///< less than 0: send bistream fail.
} HD_VIDEODEC_SEND_LIST;

typedef struct _HD_VIDEODEC_BUFINFO {
	HD_BUFINFO buf_info;                           ///< physical addr/size of bitstream buffer, for user space to mmap
} HD_VIDEODEC_BUFINFO;

typedef struct _HD_VIDEODEC_MAXMEM {
	HD_VIDEO_CODEC codec_type;                     ///< video codec type
	HD_DIM         dim;                            ///< video image dimension
	UINT32         frame_rate;                     ///< frame per second
	UINT32         max_bitrate;                    ///< max bitrate support
	UINT32         bs_counts;                      ///< bitstream buffer count
	UINT32         max_ref_num;                    ///< max reference number
	INT            ddr_id;                         ///< DDR ID
	UINT32         max_bs_size;                    ///< Max bitstream size (window size), 0 means AUTO
	UINT32         max_multi_slice_num;            ///< Max multi slice number
	UINT32         reserved[5];                    //< reserved for future using
} HD_VIDEODEC_MAXMEM;

typedef enum _HD_VIDEODEC_POOL_MODE {
	HD_VIDEODEC_POOL_AUTO = 0,
	HD_VIDEODEC_POOL_ENABLE = 1,
	HD_VIDEODEC_POOL_DISABLE = 2,
	ENUM_DUMMY4WORD(HD_VIDEODEC_POOL_MODE),
} HD_VIDEODEC_POOL_MODE;

typedef struct _HD_VIDEODEC_POOL {
	INT    ddr_id;                                 ///< DDR ID
	UINT32 counts;                                 ///< count of buffer, use HD_VIDEODEC_SET_COUNT to set
	UINT32 max_counts;                             ///< max counts of buffer, use HD_VIDEODEC_SET_COUNT to set
	UINT32 min_counts;                             ///< min counts of buffer, use HD_VIDEODEC_SET_COUNT to set
	INT    mode;                                   ///< pool mode, 0: auto, 1:enable, 2:disable
} HD_VIDEODEC_POOL;

typedef struct _HD_VIDEODEC_PATH_CONFIG {
	HD_VIDEODEC_MAXMEM max_mem;                               ///< maximum memory information
	HD_VIDEODEC_POOL   data_pool[HD_VIDEODEC_MAX_DATA_TYPE];  ///< pool memory information
} HD_VIDEODEC_PATH_CONFIG;

typedef struct _HD_VIDEODEC_IN {
	HD_VIDEO_CODEC codec_type;                     ///< video codec type
} HD_VIDEODEC_IN;

typedef struct _HD_H26XDEC_DESC {
	UINT32 addr;                                   ///< addr of h26x description header (h.264: sps, pps, h.265: vps, sps, pps)
	UINT32 len;                                    ///< size of h26x description header
} HD_H26XDEC_DESC;

typedef struct _HD_VIDEODEC_STATUS {
	UINT32 left_frames;                            ///< number of frames to be decoded
	UINT32 reserved_ref_frame;                     ///< reserved reference frame when unbind unit
	UINT32 done_frames;                            ///< number of decoded frames
	UINT32 reserved[7];                            ///< reserved for future using
} HD_VIDEODEC_STATUS;

typedef enum _HD_VIDEODEC_PARAM_ID{
	HD_VIDEODEC_PARAM_DEVCOUNT,                    ///< support get with ctrl path, using HD_DEVCOUNT struct (device id max count)
	HD_VIDEODEC_PARAM_SYSCAPS,                     ///< support get with ctrl path, using HD_VIDEODEC_SYSCAPS
	HD_VIDEODEC_PARAM_PATH_CONFIG,                 ///< support get/set with i/o path, using HD_VIDEODEC_PATH_CONFIG
	HD_VIDEODEC_PARAM_BUFINFO,                     ///< support get with i/o path, using HD_VIDEODEC_BUFINFO struct
	HD_VIDEODEC_PARAM_IN,                          ///< support get/set with i/o path, using HD_VIDEODEC_IN struct
	HD_VIDEODEC_PARAM_IN_DESC,                     ///< support get/set with i/o path, using _HD_H26XDEC_DESC struct
	HD_VIDEODEC_PARAM_STATUS,                      ///< support get/set with i/o path, using HD_VIDEODEC_STATUS struct
	HD_VIDEODEC_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEODEC_PARAM_ID)
} HD_VIDEODEC_PARAM_ID;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT hd_videodec_init(VOID);

HD_RESULT hd_videodec_bind(HD_OUT_ID out_id, HD_IN_ID dest_in_id);
HD_RESULT hd_videodec_unbind(HD_OUT_ID out_id);
HD_RESULT hd_videodec_open(HD_IN_ID in_id, HD_OUT_ID out_id, HD_PATH_ID *p_path_id);
HD_RESULT hd_videodec_start(HD_PATH_ID path_id);
HD_RESULT hd_videodec_stop(HD_PATH_ID path_id);
HD_RESULT hd_videodec_start_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_videodec_stop_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_videodec_close(HD_PATH_ID path_id);

HD_RESULT hd_videodec_get(HD_PATH_ID path_id, HD_VIDEODEC_PARAM_ID id, VOID *p_param);
HD_RESULT hd_videodec_set(HD_PATH_ID path_id, HD_VIDEODEC_PARAM_ID id, VOID *p_param);
HD_RESULT hd_videodec_push_in_buf(HD_PATH_ID path_id, HD_VIDEODEC_BS *p_in_videodec_bs, HD_VIDEO_FRAME *p_user_out_video_frame, INT32 wait_ms);
HD_RESULT hd_videodec_pull_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, INT32 wait_ms);
HD_RESULT hd_videodec_release_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame);
HD_RESULT hd_videodec_send_list(HD_VIDEODEC_SEND_LIST *p_videodec_list, UINT32 num, INT32 wait_ms);

HD_RESULT hd_videodec_uninit(VOID);

#endif

