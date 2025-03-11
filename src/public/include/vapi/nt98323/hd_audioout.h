/**
    @brief Header file of audio output module.\n
    This file contains the functions which is related to audio output in the chip.

    @file hd_audioout.h

    @ingroup mhdal

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_AUDIOOUT_H_
#define _HD_AUDIOOUT_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
    MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
    DRIVER RELATED TYPE DEFINITION
********************************************************************/

typedef enum _HD_AUDIOOUT_OUTPUT {
	HD_AUDIOOUT_OUTPUT_SPK     = 0,  ///< speaker
	HD_AUDIOOUT_OUTPUT_LINE    = 1,  ///< lineout
	HD_AUDIOOUT_OUTPUT_ALL     = 2,  ///< speaker and lineout
	HD_AUDIOOUT_OUTPUT_I2S     = 3,  ///< I2S
	HD_AUDIOOUT_OUTPUT_MAX_CNT = 4,  ///< maximum count
	ENUM_DUMMY4WORD(HD_AUDIOOUT_OUTPUT)
} HD_AUDIOOUT_OUTPUT;

#define AUDIOOUT_MAX_DEVICE_NUM 4
typedef struct _HD_AUDIOOUT_SSP_CONFIG {
	INT enable[AUDIOOUT_MAX_DEVICE_NUM];           ///< playout enable for each ssp interface
	INT resample_ratio[AUDIOOUT_MAX_DEVICE_NUM];   ///< resample ratio of for each ssp interface
	INT playback_chmap[AUDIOOUT_MAX_DEVICE_NUM];   ///< channel map of playbach ch
} HD_AUDIOOUT_SSP_CONFIG;

typedef struct _HD_AUDIOOUT_DRV_CONFIG {
	HD_AUDIO_MONO          mono;                   ///< audio mono channel
	HD_AUDIOOUT_OUTPUT     output;                 ///< audio output path
	HD_AUDIOOUT_SSP_CONFIG ssp_config;             ///< audio ssp config
} HD_AUDIOOUT_DRV_CONFIG;

/********************************************************************
    TYPE DEFINITION
********************************************************************/

#define HD_AUDIOOUT_MAX_IN   2 ///< max count of input of this device (interface)
#define HD_AUDIOOUT_MAX_OUT  1 ///< max count of output of this device (interface)

/**
     @name HD_AUDIOOUT capability
*/
typedef enum _HD_AUDIOOUT_DEVCAPS {
	HD_AUDIOOUT_DEVCAPS_SPK         = 0x00000100, ///< support speaker out
	HD_AUDIOOUT_DEVCAPS_LINEOUT     = 0x00000200, ///< support line out
	HD_AUDIOOUT_DEVCAPS_DIFF_SR     = 0x00000400, ///< support different in/out sampling rate
	HD_AUDIOOUT_DEVCAPS_VOLUME      = 0x00000800, ///< support volume setting
	HD_AUDIOOUT_DEVCAPS_SSP_CONFIG  = 0x00001000, ///< support ssp config
	HD_AUDIOOUT_DEVCAPS_CLEAR_BUF   = 0x00002000, ///< support clear buffer
	ENUM_DUMMY4WORD(HD_AUDIOOUT_DEVCAPS)
} HD_AUDIOOUT_DEVCAPS;

typedef enum _HD_AUDIOOUT_SRCAPS {
	HD_AUDIOOUT_SRCAPS_8000         = 0x00000001, ///< support HD_AUDIO_SR_8000
	HD_AUDIOOUT_SRCAPS_11025        = 0x00000002, ///< support HD_AUDIO_SR_11025
	HD_AUDIOOUT_SRCAPS_12000        = 0x00000004, ///< support HD_AUDIO_SR_12000
	HD_AUDIOOUT_SRCAPS_16000        = 0x00000008, ///< support HD_AUDIO_SR_16000
	HD_AUDIOOUT_SRCAPS_22050        = 0x00000010, ///< support HD_AUDIO_SR_22050
	HD_AUDIOOUT_SRCAPS_24000        = 0x00000020, ///< support HD_AUDIO_SR_24000
	HD_AUDIOOUT_SRCAPS_32000        = 0x00000040, ///< support HD_AUDIO_SR_32000
	HD_AUDIOOUT_SRCAPS_44100        = 0x00000080, ///< support HD_AUDIO_SR_44100
	HD_AUDIOOUT_SRCAPS_48000        = 0x00000100, ///< support HD_AUDIO_SR_48000
	ENUM_DUMMY4WORD(HD_AUDIOOUT_SRCAPS)
} HD_AUDIOOUT_SRCAPS;

typedef struct _HD_AUDIOOUT_SYSINFO {
	HD_DAL              dev_id;                                  ///< device id
	HD_AUDIO_SR         cur_out_sample_rate;                     ///< output sample rate
	HD_AUDIO_BIT_WIDTH  cur_sample_bit;                          ///< sample bit width
	HD_AUDIO_SOUND_MODE cur_mode;                                ///< sound mode
	HD_AUDIO_SR         cur_in_sample_rate[HD_AUDIOOUT_MAX_IN];  ///< input sample rate
} HD_AUDIOOUT_SYSINFO;

typedef struct _HD_AUDIOOUT_OUT {
	HD_AUDIO_SR         sample_rate;      ///< sample rate
	HD_AUDIO_BIT_WIDTH  sample_bit;       ///< sample bit width
	HD_AUDIO_SOUND_MODE mode;             ///< sound mode
} HD_AUDIOOUT_OUT;

typedef struct _HD_AUDIOOUT_IN {
	HD_AUDIO_SR         sample_rate;      ///< input sample rate. For resampling.
} HD_AUDIOOUT_IN;

typedef struct _HD_AUDIOOUT_DEV_CONFIG {
	HD_AUDIOOUT_OUT     out_max;          ///< maximum output setting.
	UINT32              frame_sample_max; ///< maximum sample count of each frame.
	UINT32              frame_num_max;    ///< maximum frame number in buffer.
	HD_AUDIOOUT_IN      in_max;           ///< maximum input setting. For resampling.
} HD_AUDIOOUT_DEV_CONFIG;

typedef struct _HD_AUDIOOUT_SYSCAPS {
	HD_DAL              dev_id;                                    ///< device id
	UINT32              chip_id;                                   ///< chip id of this device
	UINT32              max_in_count;                              ///< max count of input of this device
	UINT32              max_out_count;                             ///< max count of output of this device
	HD_DEVICE_CAPS      dev_caps;                                  ///< capability of device
	HD_AUDIO_CAPS       in_caps[HD_AUDIOOUT_MAX_IN];               ///< capability of input
	HD_AUDIO_CAPS       out_caps[HD_AUDIOOUT_MAX_OUT];             ///< capability of output
	HD_AUDIOOUT_SRCAPS  support_in_sr[HD_AUDIOOUT_MAX_IN];         ///< supported input resampling sample rate
	HD_AUDIOOUT_SRCAPS  support_out_sr[HD_AUDIOOUT_MAX_OUT];       ///< supported output sample rate
} HD_AUDIOOUT_SYSCAPS;

typedef struct _HD_AUDIOOUT_VOLUME {
	UINT32                 volume;   ///< audio output volume
} HD_AUDIOOUT_VOLUME;

typedef enum _HD_AUDIOOUT_PARAM_ID {
	HD_AUDIOOUT_PARAM_DEVCOUNT,      ///< support get with ctrl path, using HD_DEVCOUNT struct (device id max count)
	HD_AUDIOOUT_PARAM_SYSCAPS,       ///< support get with ctrl path, using HD_AUDIOOUT_SYSCAPS struct (system capabilitiy)
	HD_AUDIOOUT_PARAM_SYSINFO,       ///< support get with ctrl path, using HD_AUDIOOUT_SYSINFO struct
	HD_AUDIOOUT_PARAM_DEV_CONFIG,    ///< support get/set with ctrl path, using HD_AUDIOOUT_DEV_CONFIG struct
	HD_AUDIOOUT_PARAM_DRV_CONFIG,    ///< support get/set with ctrl path, using HD_AUDIOOUT_DRV_CONFIG struct
	HD_AUDIOOUT_PARAM_OUT,           ///< support get/set with i/o path, using HD_AUDIOOUT_OUT struct
	HD_AUDIOOUT_PARAM_IN,            ///< support get/set with i/o path, using HD_AUDIOOUT_IN struct
	HD_AUDIOOUT_PARAM_VOLUME,        ///< support get/set with ctrl path, using HD_AUDIOOUT_VOLUME struct
	HD_AUDIOOUT_PARAM_CLEAR_BUF,     ///< support set with i/o path, no parameter
	HD_AUDIOOUT_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_AUDIOOUT_PARAM_ID)
} HD_AUDIOOUT_PARAM_ID;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT hd_audioout_init(VOID);
HD_RESULT hd_audioout_open(HD_IN_ID in_id, HD_OUT_ID out_id, HD_PATH_ID *p_path_id);

HD_RESULT hd_audioout_start(HD_PATH_ID path_id);
HD_RESULT hd_audioout_stop(HD_PATH_ID path_id);
HD_RESULT hd_audioout_start_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_audioout_stop_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_audioout_set(HD_PATH_ID path_id, HD_AUDIOOUT_PARAM_ID id, VOID *p_param);
HD_RESULT hd_audioout_get(HD_PATH_ID path_id, HD_AUDIOOUT_PARAM_ID id, VOID *p_param);
HD_RESULT hd_audioout_push_in_buf(HD_PATH_ID path_id, HD_AUDIO_FRAME *p_in_audio_frame, INT32 wait_ms);
HD_RESULT hd_audioout_close(HD_PATH_ID path_id);
HD_RESULT hd_audioout_uninit(VOID);

#endif

