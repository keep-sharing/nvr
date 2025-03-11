/**
    @brief Header file of audio decoder module.\n
    This file contains the functions which is related to audio decoder in the chip.

    @file hd_audiodec.h

    @ingroup mhdal

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_AUDIODEC_H_
#define _HD_AUDIODEC_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define HD_AUDIODEC_MAX_IN         32 ///< max count of input of this device (interface)
#define HD_AUDIODEC_MAX_OUT        32 ///< max count of output of this device (interface)
#define HD_AUDIODEC_MAX_DATA_TYPE   4 ///< max count of output pool of this device (interface)

/********************************************************************
    MACRO FUNCTION DEFINITIONS
********************************************************************/
#define HD_AUDIODEC_SET_COUNT(a, b)     ((a)*10)+(b)    ///< ex: use HD_AUDIODEC_SET_COUNT(1, 5) for setting 1.5

/********************************************************************
    TYPE DEFINITION
********************************************************************/

/**
     @name HD_AUDIODEC capability
*/
typedef enum _HD_AUDIODEC_CAPS {
	HD_AUDIODEC_CAPS_PCM        = 0x00010000, ///< support PCM
	HD_AUDIODEC_CAPS_AAC        = 0x00020000, ///< support decode from AAC
	HD_AUDIODEC_CAPS_ADPCM      = 0x00040000, ///< support decode from ADPCM
	HD_AUDIODEC_CAPS_ULAW       = 0x00080000, ///< support decode from G.711 u-law
	HD_AUDIODEC_CAPS_ALAW       = 0x00100000, ///< support decode from G.711 a-law
	ENUM_DUMMY4WORD(HD_AUDIODEC_CAPS)
} HD_AUDIODEC_CAPS;

typedef struct _HD_AUDIODEC_SYSCAPS {
	HD_DAL                 dev_id;                              ///< device id
	UINT32                 chip_id;                             ///< chip id of this device
	UINT32                 max_in_count;                        ///< max count of input of this device
	UINT32                 max_out_count;                       ///< max count of output of this device
	HD_DEVICE_CAPS         dev_caps;                            ///< capability of device, using HD_DEVICE_CAPS
	HD_AUDIODEC_CAPS       in_caps[HD_AUDIODEC_MAX_IN];         ///< capability of input, using HD_AUDIODEC_CAPS
} HD_AUDIODEC_SYSCAPS;

typedef struct _HD_AUDIODEC_BUFINFO {
	HD_BUFINFO buf_info;                        ///< physical addr/size of decode output buffer, for user space to mmap
} HD_AUDIODEC_BUFINFO;

typedef struct _HD_AUDIODEC_IN {
	HD_AUDIO_CODEC         codec_type;          ///< audio codec type
	HD_AUDIO_SR            sample_rate;         ///< audio sample rate
	HD_AUDIO_BIT_WIDTH     sample_bit;          ///< bits size per sample
	HD_AUDIO_SOUND_MODE    mode;                ///< sound mode (mono, stereo, etc.)
} HD_AUDIODEC_IN;

typedef HD_AUDIODEC_IN HD_AUDIODEC_MAXMEM;

typedef enum _HD_AUDIODEC_POOL_MODE {
	HD_AUDIODEC_POOL_AUTO = 0,
	HD_AUDIODEC_POOL_ENABLE = 1,
	HD_AUDIODEC_POOL_DISABLE = 2,
	ENUM_DUMMY4WORD(HD_AUDIODEC_POOL_MODE),
} HD_AUDIODEC_POOL_MODE;

typedef struct _HD_AUDIODEC_POOL {
	INT     ddr_id;             ///< DDR ID
	UINT32  frame_sample_size;  ///< the buffer size of audio frame
	UINT32  counts;             ///< count of buffer, use HD_VIDEOCAP_SET_COUNT to set
	UINT32  max_counts;         ///< max counts of buffer, use HD_VIDEOCAP_SET_COUNT to set
	UINT32  min_counts;         ///< min counts of buffer, use HD_VIDEOCAP_SET_COUNT to set
	HD_AUDIODEC_POOL_MODE mode; ///< pool mode
} HD_AUDIODEC_POOL;

typedef struct _HD_AUDIODEC_PATH_CONFIG {
	HD_AUDIODEC_MAXMEM     max_mem;                               ///< maximum memory information
	HD_AUDIODEC_POOL       data_pool[HD_AUDIODEC_MAX_DATA_TYPE];  ///< pool memory information
} HD_AUDIODEC_PATH_CONFIG;

typedef struct _HD_AUDIODEC_USER_BS {
	UINT32         sign;                ///< signature = MAKEFOURCC('A','S','T','M')
	HD_METADATA   *p_next;              ///< pointer to next meta
	HD_AUDIO_CODEC acodec_format;       ///< encoded format of audio frame
	UINT64         timestamp;           ///< encoded timestamp (unit: microsecond)

	/* provide by application */
	CHAR           *p_user_buf;         ///< Bitstream buffer pointer
	UINT32         user_buf_size;       ///< size of encoded data
} HD_AUDIODEC_USER_BS;


typedef struct _HD_AUDIODEC_SEND_LIST {
	HD_PATH_ID          path_id;        ///< path id
	HD_AUDIODEC_USER_BS user_bs;        ///< audio decode user bitstream
	INT32               retval;         ///< less than 0: send bistream fail.
} HD_AUDIODEC_SEND_LIST;

typedef enum _HD_AUDIODEC_PARAM_ID {
	HD_AUDIODEC_PARAM_DEVCOUNT,                 ///< support get with ctrl path, using HD_DEVCOUNT struct (device id max count)
	HD_AUDIODEC_PARAM_SYSCAPS,                  ///< support get with ctrl path, using HD_AUDIODEC_SYSCAPS struct (system capabilitiy)
	HD_AUDIODEC_PARAM_BUFINFO,                  ///< support get with i/o path, using HD_AUDIODEC_BUFINFO struct
	HD_AUDIODEC_PARAM_PATH_CONFIG,              ///< support get/set with i/o path, using HD_AUDIODEC_PATH_CONFIG
	HD_AUDIODEC_PARAM_IN,                       ///< support get/set with i/o path, using HD_AUDIODEC_IN
	HD_AUDIODEC_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_AUDIODEC_PARAM_ID)
} HD_AUDIODEC_PARAM_ID;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT hd_audiodec_init(VOID);

HD_RESULT hd_audiodec_bind(HD_OUT_ID out_id, HD_IN_ID dest_in_id);
HD_RESULT hd_audiodec_unbind(HD_OUT_ID out_id);
HD_RESULT hd_audiodec_open(HD_IN_ID in_id, HD_OUT_ID out_id, HD_PATH_ID *p_path_id);
HD_RESULT hd_audiodec_start(HD_PATH_ID path_id);
HD_RESULT hd_audiodec_stop(HD_PATH_ID path_id);
HD_RESULT hd_audiodec_start_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_audiodec_stop_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_audiodec_close(HD_PATH_ID path_id);

HD_RESULT hd_audiodec_get(HD_PATH_ID path_id, HD_AUDIODEC_PARAM_ID id, VOID *p_param);
HD_RESULT hd_audiodec_set(HD_PATH_ID path_id, HD_AUDIODEC_PARAM_ID id, VOID *p_param);

HD_RESULT hd_audiodec_push_in_buf(HD_PATH_ID path_id, HD_AUDIO_BS *p_in_audio_bs, HD_AUDIO_FRAME *p_user_out_audio_frame, INT32 wait_ms);
HD_RESULT hd_audiodec_pull_out_buf(HD_PATH_ID path_id, HD_AUDIO_FRAME *p_audio_frame, INT32 wait_ms);
HD_RESULT hd_audiodec_release_out_buf(HD_PATH_ID path_id, HD_AUDIO_FRAME *p_audio_frame);

HD_RESULT hd_audiodec_send_list(HD_AUDIODEC_SEND_LIST *p_audiodec_bs, UINT32 num, INT32 wait_ms);

HD_RESULT hd_audiodec_uninit(VOID);

#endif

