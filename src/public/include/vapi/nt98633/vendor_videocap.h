
#ifndef	_VENDOR_VIDEOCAP_H_
#define	_VENDOR_VIDEOCAP_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
#include "hd_common.h"
#include "hd_videocapture.h"

#ifdef  __cplusplus
extern "C" {
#endif

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDER_VIDEOCAP_VERSION		0x010002



typedef enum VENDOR_VIDEOCAP_PATGEN_CNG_PARAM_ID {
	VENDOR_VIDEOCAP_PATGEN_NORM = 0,	//norm[] , "640x480@25", "640x480@30" ...
	VENDOR_VIDEOCAP_PATGEN_ENABLE,	    //0:disable, 1:enable
	VENDOR_VIDEOCAP_PATGEN_MODE,	    //0:fix_pattern, 1:LFSR
	VENDOR_VIDEOCAP_PATGEN_FIX_PATTERN, //default: 0x10801080
	VENDOR_VIDEOCAP_PATGEN_LFSR_INIT_TAP, //default: 0x12345678
	VENDOR_VIDEOCAP_PATGEN_MAX, //max
	ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_PATGEN_CNG_PARAM_ID)
} VENDOR_VIDEOCAP_PATGEN_CNG_PARAM_ID;

typedef enum VENDOR_VIDEOCAP_PATGEN_SRCTYPE {
    VENDOR_VCAP_IOC_VI_CH_DATA_SRC_VI = 0,                     ///<  video interface
    VENDOR_VCAP_IOC_VI_CH_DATA_SRC_PATGEN,                     ///<  pattern generator
    ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_PATGEN_SRCTYPE)
} VENDOR_VIDEOCAP_PATGEN_SRCTYPE;

typedef struct _VENDOR_VIDEOCAP_CROPCTRL_INFO {
	HD_CROP_MODE mode;			//HD_CROP_ON/HD_CROP_OFF
	HD_IRECT     rect;
	HD_DIM		 coord;
} VENDOR_VIDEOCAP_CROPCTRL_INFO;

typedef struct _VENDOR_VIDEOCAP_PATGEN_CNG_PARAM {
	VENDOR_VIDEOCAP_PATGEN_CNG_PARAM_ID id;
	unsigned int value;
} VENDOR_VIDEOCAP_PATGEN_CNG_PARAM;

typedef struct _VENDOR_VIDEOCAP_PATGEN_PARAM {
	unsigned int norm;
	unsigned int enable;
	unsigned int mode;
	unsigned int pattern_value;
	unsigned int lfsr_init_tap;
} VENDOR_VIDEOCAP_PATGEN_PARAM;

typedef struct _VENDOR_VIDEOCAP_SYSCAPS {
	HD_VIDEO_PXLFMT wanted_pxlfmt;                  ///< [in] vcap output pxlfmt
	HD_DIM min_dim;                                 ///< [out] minimum dimension
	HD_DIM max_dim;                                 ///< [out] maximum dimension
	HD_DIM crop_min_dim;                            ///< [out] crop out minimum dimension
	HD_DIM output_align;                            ///< [out] alignment for w/h
} VENDOR_VIDEOCAP_SYSCAPS;

typedef struct _VENDOR_TWO_CHANNEL_PARAM {
	UINT8 enable;							///< two channel mode enable
	HD_PATH_ID sec_path_id;					///< right frame path id
} VENDOR_TWO_CHANNEL_PARAM;

typedef enum VENDOR_VIDEOCAP_PARAM_ID {
	VENDOR_VIDEOCAP_CROPCTRL_PARAM,			///<  support set, VENDOR_VIDEOCAP_CROPCTRL_INFO for crop setting
	VENDOR_VIDEOCAP_SRCTYPE_PARAM,			///<  support set, VENDOR_VIDEOCAP_SRCTYPE_PARAM for srctype setting
	VENDOR_VIDEOCAP_VICH_NORM_PARAM,		///<  support get, HD_VIDEOCAP_VI_CH_NORM for norm setting
	VENDOR_VIDEOCAP_PATGEN_INIT,		    ///<  support set, only pathid needed
	VENDOR_VIDEOCAP_PATGEN_MODIFY,		    ///<  support set, VENDOR_VIDEOCAP_PATGEN_CNG_PARAM for changing
	VENDOR_VIDEOCAP_PARAM_SYSCAPS,		    ///<  support get, VENDOR_VIDEOCAP_SYSCAPS for getting
	VENDOR_VIDEOCAP_TWO_CHANNEL,		    ///<  support get, VENDOR_TWO_CHANNEL_PARAM for two channel capture
	ENUM_DUMMY4WORD(VENDOR_VIDEOCAP_PARAM_ID)
} VENDOR_VIDEOCAP_PARAM_ID;

typedef struct _VENDOR_VIDEOCAP_VI_CH_V2_INFO {
    HD_PATH_ID pathid;                 ///< ch   index

    unsigned int data_src;
	unsigned int data_src_valid;
} VENDOR_VIDEOCAP_VI_CH_V2_INFO;


HD_RESULT vendor_videocap_set(HD_PATH_ID path_id, VENDOR_VIDEOCAP_PARAM_ID id, void *p_param);
HD_RESULT vendor_videocap_push_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME* p_video_frame, INT32 wait_ms);
HD_RESULT vendor_videocap_pull_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME* p_video_frame, INT32 wait_ms);
HD_RESULT vendor_videocap_init(void);
HD_RESULT vendor_videocap_uninit(void);
HD_RESULT vendor_videocap_patgen_init(HD_PATH_ID path_id);
HD_RESULT vendor_videocap_get(HD_PATH_ID path_id, VENDOR_VIDEOCAP_PARAM_ID id, void *p_param);
HD_RESULT vendor_videocap_set_srctype(VENDOR_VIDEOCAP_VI_CH_V2_INFO *p_ch_info);



#define FLOW_CROP_BIT          (22)
#define FLOW_CROP_FLAG         (0x1 << FLOW_CROP_BIT)
extern void hdal_flow_log_p(unsigned int flag, const char *msg_with_format, ...) __attribute__((format(printf, 2, 3)));

#define _HD_MODULE_PRINT_FLOW(flag, module, fmtstr, args...)  do { \
															hdal_flow_log_p(flag, fmtstr, ##args); \
													} while(0)
#define VENDOR_CROP_FLOW(fmtstr, args...) _HD_MODULE_PRINT_FLOW(FLOW_CROP_FLAG, VEN, fmtstr, ##args)

#ifdef  __cplusplus
}
#endif

#endif // _VENDOR_VIDEOCAP_H_
