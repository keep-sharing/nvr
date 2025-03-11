/**
@brief Header file of vendor common
This file contains the functions which is related to videoenc in the chip.

@file vendor_common.h

@ingroup mhdal

@note Nothing.

Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef __VENDOR_COMMON_H__
#define __VENDOR_COMMON_H__

#define VENDOR_COMMON_VERSION 0x010011 //vendor common version
/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hdal.h"

#ifdef  __cplusplus
extern "C" {
#endif

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDOR_YUV420SP_TYPE_NV12           0
#define VENDOR_YUV420SP_TYPE_NV21           1
#define VENDOR_COMMON_PXLFMT_YUV420_TYPE    VENDOR_YUV420SP_TYPE_NV21   // it means 'HD_VIDEO_PXLFMT_YUV420' type in this platform
#define VENDOR_VIDEO_PXLFMT_YUV420_MB3      CIF_VIDEO_PXLFMT_YUV420_MB3 //0x510cd420 //CIF_VIDEO_PXLFMT_YUV420_MB3//8x4
/********************************************************************
TYPE DEFINITION
********************************************************************/
typedef struct {
	unsigned int size;
	unsigned int nr;
	unsigned int range;
} VENDOR_COMMON_MEM_LAYOUT_ATTR;

typedef enum _VENDOR_COMMON_MEM_DMA_DIR {
	VENDOR_COMMON_MEM_DMA_BIDIRECTIONAL,             ///< it means flush operation.
	VENDOR_COMMON_MEM_DMA_TO_DEVICE,                 ///< it means clean operation.
	VENDOR_COMMON_MEM_DMA_FROM_DEVICE,               ///< it means invalidate operation.
	ENUM_DUMMY4WORD(VENDOR_COMMON_MEM_DMA_DIR)
} VENDOR_COMMON_MEM_DMA_DIR;

typedef struct _VENDOR_OSG_STAMP_IMG {
	HD_VIDEO_PXLFMT fmt;                ///< RGB565/ARGB1555/ARGB4444/ARGB8888/
	HD_DIM dim;                         ///< image's width and height
	UINTPTR p_addr;                     ///< image's bitmap content
	UINT32 ddr_id;                       ///< p_addr's ddr_id
	HD_OSG_COPY_HW copy;                ///< use cpu or dma to copy image. only for hd_xxx_get()
} VENDOR_OSG_STAMP_IMG;

typedef enum _VENDOR_COMMON_TRI_MODULE {
	VENDOR_COMMON_TRIGGER_TYPE_CAP,
	VENDOR_COMMON_TRIGGER_TYPE_VPE,
	VENDOR_COMMON_TRIGGER_TYPE_DEC,
	VENDOR_COMMON_TRIGGER_TYPE_ADV_ENC,
	VENDOR_COMMON_TRIGGER_TYPE_OSG,
	VENDOR_COMMON_TRIGGER_TYPE_VIDEOOUT,
	VENDOR_COMMON_TRIGGER_TYPE_AU_RENDER,
	ENUM_DUMMY4WORD(VENDOR_COMMON_TRI_MODULE)
} VENDOR_COMMON_TRI_MODULE;

typedef struct {
	VENDOR_COMMON_TRI_MODULE type;
	int chk_in_buf;// < valid value:0 /1
	int chk_out_buf;// < valid value:0 /1
} VENDOR_CHK_TRI_BUF;


/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
/**
    Do the cache memory data synchronization.

    @param virt_addr: the cache memory address.
    @param size: the memory size want to sync.
    @param dir: the dma direction.

    @return HD_OK for success, < 0 when some error happened.
*/
HD_RESULT vendor_common_mem_cache_sync(void *virt_addr, unsigned int size, VENDOR_COMMON_MEM_DMA_DIR dir);
HD_RESULT vendor_common_pool_layout(HD_COMMON_MEM_POOL_TYPE pool_type, int attr_nr, VENDOR_COMMON_MEM_LAYOUT_ATTR *layout_attr,
									int del_layout);
HD_RESULT vendor_common_get_pool_usedbuf_info(HD_COMMON_MEM_POOL_TYPE pool_type, int *p_used_buf_nr, HD_BUFINFO *buf_info);
HD_RESULT vendor_common_clear_all_blk(void);
HD_RESULT vendor_common_clear_pool_blk(HD_COMMON_MEM_POOL_TYPE pool_type, INT ddrid);
HD_RESULT vendor_common_get_pool_size(HD_COMMON_MEM_POOL_TYPE pool_type, INT ddrid, UINT32 *pool_sz);
HD_RESULT vendor_common_get_ddrid(HD_COMMON_MEM_POOL_TYPE pool_type, INT chip_id, HD_COMMON_MEM_DDR_ID *ddr_id);
HD_RESULT vendor_common_set_chk_trigger_buf(VENDOR_CHK_TRI_BUF param);
HD_RESULT vendor_common_get_chk_trigger_buf(VENDOR_CHK_TRI_BUF *p_param);
HD_RESULT vendor_common_set_max_queue_start_pathid(UINT nu);
HD_RESULT vendor_common_get_max_queue_start_pathid(UINT *p_nu);
HD_RESULT vendor_common_set_start_wait(UINT time_ms);
HD_RESULT vendor_common_get_start_wait(UINT *p_time_ms);
//nu indicate pathid_list size, and return get pathid number
HD_RESULT vendor_common_get_all_queue_pathid(UINT *nu, HD_PATH_ID *pathid_list);
char* vendor_common_HD_VIDEO_PXLFMT_str(HD_VIDEO_PXLFMT pxlfmt);

void printlog(const char *fmt, ...);
HD_COMMON_MEM_VB_BLK vendor_common_mem_get_block_with_name(HD_COMMON_MEM_POOL_TYPE pool_type, UINT32 blk_size, HD_COMMON_MEM_DDR_ID ddr, 
	CHAR *usr_name);
#ifdef  __cplusplus
}
#endif

#endif // __VENDOR_COMMON_H__
