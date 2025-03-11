
#ifndef	_VENDOR_AUDIOIO_H_
#define	_VENDOR_AUDIOIO_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
#include "hd_common.h"

#ifdef  __cplusplus
extern "C" {
#endif

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDER_AUDIOIO_VERSION		0x010008

/********************************************************************
TYPE DEFINITION
********************************************************************/
typedef struct _VENDOR_AUDIOIO_UNDERRUN_CONFIG {
	UINT32 underrun_cnt;                                ///< underrun count value
	UINT32 ongoing_cnt;                                 ///< ongoing count value
	UINT32 done_cnt;                                    ///< done count value
} VENDOR_AUDIOIO_UNDERRUN_CONFIG, VENDOR_AUDIOIO_STATUS_INFO;

typedef struct _VENDOR_AUDIOIO_SYSCAPS {
	UINT32 in_buf_align;                   ///< input align
	UINT32 out_buf_align;                  ///< output align
} VENDOR_AUDIOIO_SYSCAPS;

typedef struct _VENDOR_AUDIOIO_INIT_I2S_CFG {
	UINT32 bit_width;
	UINT32 bit_clk_ratio;
	UINT32 op_mode;
	UINT32 tdm_ch;
} VENDOR_AUDIOIO_INIT_I2S_CFG;

typedef struct _VENDOR_AUDIOIO_AOUT_LB_DEV_INFO {
	UINT32 dev_id;
} VENDOR_AUDIOIO_AOUT_LB_DEV_INFO;

typedef struct _VENDOR_AUDIOIO_SYNC_TIME_INFO {
	HD_PATH_ID sync_target;
} VENDOR_AUDIOIO_SYNC_TIME_INFO;

typedef enum _VENDOR_AUDIOIO_ID {
	VENDOR_UNDERRUN_CNT,               ///< support get,use VENDOR_AUDIOIO_UNDERRUN_CONFIG struct to get audio playback underrun count
	VENDOR_AUDIOIO_PARAM_SYSCAPS,      ///< support get,use VENDOR_AUDIOIO_SYSCAPS struct
	VENDOR_AUDIOIO_STATUS,             ///< support get,use VENDOR_AUDIOIO_STATUS struct to get audio playback information
	VENDOR_AUDIOIO_CLEAR_BUF,          ///< support set,no parameter
	VENDOR_AUDIOIO_INIT_CFG,           ///< support set/get,VENDOR_AUDIOIO_INIT_I2S_CFG struct to setup audio device
	VENDOR_AUDIOIO_AOUT_LB_DEV_ID,     ///< support get,VENDOR_AUDIOIO_AOUT_LB_DEV_INFO struct to setup audio device
	VENDOR_AUDIOIO_SYNC_TIME,          ///< support set,use VENDOR_AUDIOIO_SYNC_TIME_INFO struct to get audio playback information
	VENDOR_AUDIOIO_MAX,
	ENUM_DUMMY4WORD(VENDOR_AUDIOIO_PARAM_ID)
} VENDOR_AUDIOIO_PARAM_ID;

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_audioio_get(HD_PATH_ID path_id, VENDOR_AUDIOIO_PARAM_ID id, void *p_param);
HD_RESULT vendor_audioio_set(HD_PATH_ID path_id, VENDOR_AUDIOIO_PARAM_ID id, void *p_param);

HD_RESULT vendor_audioio_set_livesound(HD_PATH_ID acap_path_id, HD_PATH_ID aout_path_id, UINT on_off);

#ifdef  __cplusplus
}
#endif

#endif // _VENDOR_AUDIOIO_H_
