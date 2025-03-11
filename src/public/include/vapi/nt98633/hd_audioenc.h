/**
    @brief Header file of audio encoder module.\n
    This file contains the functions which is related to audio encoder in the chip.

    @file hd_audioenc.h

    @ingroup mhdal

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_AUDIOENC_H_
#define _HD_AUDIOENC_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define HD_AUDIOENC_MAX_IN   64 ///< max count of input of this device (interface)
#define HD_AUDIOENC_MAX_OUT  64 ///< max count of output of this device (interface)

/********************************************************************
    MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
    TYPE DEFINITION
********************************************************************/

/**
     @name HD_AUDIOENC capability
*/

typedef enum _HD_AUDIOENC_CAPS {
	HD_AUDIOENC_CAPS_PCM            = 0x00010000, ///< support PCM
	HD_AUDIOENC_CAPS_AAC            = 0x00020000, ///< support encode to AAC
	HD_AUDIOENC_CAPS_ADPCM          = 0x00040000, ///< support encode to ADPCM
	HD_AUDIOENC_CAPS_ULAW           = 0x00080000, ///< support encode to G.711 u-law
	HD_AUDIOENC_CAPS_ALAW           = 0x00100000, ///< support encode to G.711 A-law
	ENUM_DUMMY4WORD(HD_AUDIOENC_CAPS)
} HD_AUDIOENC_CAPS;

typedef struct _HD_AUDIOENC_SYSCAPS {
	HD_DAL                 dev_id;                              ///< device id
	UINT32                 chip_id;                             ///< chip id of this device
	UINT32                 max_in_count;                        ///< max count of input of this device
	UINT32                 max_out_count;                       ///< max count of output of this device
	HD_DEVICE_CAPS         dev_caps;                            ///< capability of device, using HD_DEVICE_CAPS
	HD_AUDIO_CAPS          in_caps[HD_AUDIOENC_MAX_IN];         ///< capability of input, using HD_AUDIO_CAPS
	HD_AUDIOENC_CAPS       out_caps[HD_AUDIOENC_MAX_OUT];       ///< capability of output, using HD_AUDIOENC_CAPS
} HD_AUDIOENC_SYSCAPS;

typedef struct _HD_AUDIOENC_BUFINFO {
	HD_BUFINFO buf_info;                       ///< physical addr/size of bitstream buffer, for user space to mmap
} HD_AUDIOENC_BUFINFO;

typedef struct _HD_AUDIOENC_MAXMEM {
	HD_AUDIO_CODEC         codec_type;         ///< codec type, (need memory: AAC < G.711 < PCM)
	HD_AUDIO_SR            sample_rate;        ///< sample rate
	HD_AUDIO_BIT_WIDTH     sample_bit;         ///< sample bit width
	HD_AUDIO_SOUND_MODE    mode;               ///< sound mode
} HD_AUDIOENC_MAXMEM;

typedef struct _HD_AUDIOENC_PATH_CONFIG {
	HD_AUDIOENC_MAXMEM     max_mem;            ///< maximum memory information
} HD_AUDIOENC_PATH_CONFIG;

typedef struct _HD_AUDIOENC_MAXMEM2 {
	HD_AUDIO_CODEC         codec_type;         ///< codec type, (need memory: AAC < G.711 < PCM)
	HD_AUDIO_SR            sample_rate;        ///< sample rate
	HD_AUDIO_BIT_WIDTH     sample_bit;         ///< sample bit width
	HD_AUDIO_SOUND_MODE    mode;               ///< sound mode
	UINT32                 buf_ms;             ///< encode buffer length in millisecond (default: 2000 ms)
} HD_AUDIOENC_MAXMEM2;

typedef struct _HD_AUDIOENC_PATH_CONFIG2 {
	HD_AUDIOENC_MAXMEM2    max_mem;            ///< maximum memory information
} HD_AUDIOENC_PATH_CONFIG2;

typedef struct _HD_AUDIOENC_IN {
	HD_AUDIO_SR            sample_rate;        ///< sample rate
	HD_AUDIO_BIT_WIDTH     sample_bit;         ///< sample bit width
	HD_AUDIO_SOUND_MODE    mode;               ///< sound mode
} HD_AUDIOENC_IN;

typedef struct _HD_AUDIOENC_OUT {
	HD_AUDIO_CODEC         codec_type;         ///< codec type
	UINT32                 aac_adts;           ///< enable/disable AAC ADTS header (1: enable, 0: disable)
} HD_AUDIOENC_OUT;

typedef struct _HD_AUDIOENC_RET_EVENT {
	BOOL event;                      ///< poll status
	UINT32 bs_size;                  ///< bitstream size
} HD_AUDIOENC_RET_EVENT;

typedef struct _HD_AUDIOENC_POLL_LIST {
	HD_PATH_ID path_id;              ///< path ID
	HD_AUDIOENC_RET_EVENT revent;    ///< The returned event value
} HD_AUDIOENC_POLL_LIST;

typedef struct _HD_AUDIOENC_USER_BS {
	UINT32         sign;             ///< signature = MAKEFOURCC('A','S','T','M')
	HD_METADATA   *p_next;           ///< pointer to next meta
	HD_AUDIO_CODEC acodec_format;    ///< encoded format of audio frame
	UINT64         timestamp;        ///< encoded timestamp (unit: microsecond)
	UINT32         size;             ///< size of encoded data
	UINT32         newbs_flag;       ///< Flag notification of new seting, such as GM_FLAG_NEW_BITRATE

	/* provide by application */
	CHAR *p_user_buf;                ///< Bitstream buffer pointer
	UINT32 user_buf_size;            ///< AP provide bs_buf max size
} HD_AUDIOENC_USER_BS;

typedef struct _HD_AUDIOENC_RECV_LIST {
	HD_PATH_ID          path_id;     ///< path ID
	HD_AUDIOENC_USER_BS user_bs;     ///< audio encode user bitstream
	INT32               retval;      ///< less than 0: recv bistream fail.
} HD_AUDIOENC_RECV_LIST;

typedef enum _HD_AUDIOENC_PARAM_ID {
	HD_AUDIOENC_PARAM_DEVCOUNT,      ///< support get with ctrl path, using HD_DEVCOUNT struct (device id max count)
	HD_AUDIOENC_PARAM_SYSCAPS,       ///< support get with ctrl path, using HD_AUDIOENC_SYSCAPS struct (system capabilitiy)
	HD_AUDIOENC_PARAM_BUFINFO,       ///< support get with i/o path, using HD_AUDIOENC_BUFINFO struct
	HD_AUDIOENC_PARAM_PATH_CONFIG,   ///< support get/set with i/o path, using HD_AUDIOENC_PATH_CONFIG struct (path config)
	HD_AUDIOENC_PARAM_IN,            ///< support get/set with i/o path, using HD_AUDIOENC_IN struct (input frame paramter)
	HD_AUDIOENC_PARAM_OUT,           ///< support get/set with i/o path, using HD_AUDIOENC_OUT struct (output frame paramter)
	HD_AUDIOENC_PARAM_PATH_CONFIG2,  ///< support get/set with i/o path, using HD_AUDIOENC_PATH_CONFIG2 struct (path config)
	HD_AUDIOENC_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_AUDIOENC_PARAM_ID)
} HD_AUDIOENC_PARAM_ID;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT hd_audioenc_init(VOID);
HD_RESULT hd_audioenc_bind(HD_OUT_ID out_id, HD_IN_ID dest_in_id);
HD_RESULT hd_audioenc_unbind(HD_OUT_ID out_id);
HD_RESULT hd_audioenc_open(HD_IN_ID in_id, HD_OUT_ID out_id, HD_PATH_ID *p_path_id);

HD_RESULT hd_audioenc_start(HD_PATH_ID path_id);
HD_RESULT hd_audioenc_stop(HD_PATH_ID path_id);
HD_RESULT hd_audioenc_start_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_audioenc_stop_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_audioenc_get(HD_PATH_ID path_id, HD_AUDIOENC_PARAM_ID id, VOID *p_param);
HD_RESULT hd_audioenc_set(HD_PATH_ID path_id, HD_AUDIOENC_PARAM_ID id, VOID *p_param);
HD_RESULT hd_audioenc_push_in_buf(HD_PATH_ID path_id, HD_AUDIO_FRAME *p_in_audio_frame, HD_AUDIO_BS *p_user_out_audio_bs, INT32 wait_ms);
HD_RESULT hd_audioenc_pull_out_buf(HD_PATH_ID path_id, HD_AUDIO_BS *p_audio_bs, INT32 wait_ms);
HD_RESULT hd_audioenc_release_out_buf(HD_PATH_ID path_id, HD_AUDIO_BS *p_audio_bs);
HD_RESULT hd_audioenc_poll_list(HD_AUDIOENC_POLL_LIST *p_poll, UINT32 num, INT32 wait_ms);
HD_RESULT hd_audioenc_recv_list(HD_AUDIOENC_RECV_LIST *p_audio_bs, UINT32 num);

HD_RESULT hd_audioenc_close(HD_PATH_ID path_id);
HD_RESULT hd_audioenc_uninit(VOID);

#endif

