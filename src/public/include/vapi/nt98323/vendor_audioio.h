
#ifndef	_VENDOR_AUDIOIO_H_
#define	_VENDOR_AUDIOIO_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
#include "hd_common.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDER_AUDIOIO_VERSION		0x010001

/********************************************************************
TYPE DEFINITION
********************************************************************/
typedef struct _VENDOR_AUDIOIO_LIVESOUND_CONFIG {
	unsigned int src_vch;                                ///< livesound audio in channel
	unsigned int dst_vch;                                ///< livesound audio out channel
	unsigned int on_off;                                 ///< turn on or turn off livesound
} VENDOR_AUDIOIO_LIVESOUND_CONFIG;

typedef struct _VENDOR_AUDIOIO_UNDERRUN_CONFIG {
	UINT32 underrun_cnt;                                ///< underrun count value
} VENDOR_AUDIOIO_UNDERRUN_CONFIG;

typedef enum _VENDOR_AUDIOIO_ID {
	VENDOR_LIVESOUND,            ///< support set,VENDOR_LIVESOUND to set audio livesound in/out channel
	VENDOR_UNDERRUN_CNT,         ///< support get,VENDOR_UNDERRUN_CNT to get audio playback underrun count
	VENDOR_AUDIOIO_MAX,
	ENUM_DUMMY4WORD(VENDOR_AUDIOIO_PARAM_ID)
} VENDOR_AUDIOIO_PARAM_ID;

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_audioio_get(HD_PATH_ID path_id, VENDOR_AUDIOIO_PARAM_ID id, void *p_param);
HD_RESULT vendor_audioio_set(HD_PATH_ID path_id, VENDOR_AUDIOIO_PARAM_ID id, void *p_param);

HD_RESULT vendor_audioio_set_livesound(unsigned int src_vch, unsigned int dst_vch, int on_off);

#endif // _VENDOR_AUDIOIO_H_
