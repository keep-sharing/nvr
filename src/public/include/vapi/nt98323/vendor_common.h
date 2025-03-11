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

#define VENDOR_COMMON_VERSION 0x010004 //vendor common version
/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hdal.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDOR_YUV420SP_TYPE_NV12           0
#define VENDOR_YUV420SP_TYPE_NV21           1
#define VENDOR_COMMON_PXLFMT_YUV420_TYPE    VENDOR_YUV420SP_TYPE_NV21   // it means 'HD_VIDEO_PXLFMT_YUV420' type in this platform

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
HD_RESULT vendor_common_mem_cache_sync(void* virt_addr, unsigned int size, VENDOR_COMMON_MEM_DMA_DIR dir);

/**
    Do the cache memory data synchronization of the whole cache.

    @note User must make sure the cache data is only touched by one specified cpu_id.
          This API will get better performance on SMP when flush large size.
          If the task was bouned to one cpu, it will call cache_flush_all() API to fine tune performace.
          If the task was not bouned to one cpu, it will call cache_flush_range() API.

    @param virt_addr: the cache memory address.
    @param size: the memory size want to sync.
    @param dir: the dma direction.

    @return HD_OK for success, < 0 when some error happened.
*/
HD_RESULT vendor_common_mem_cache_sync_all(void* virt_addr, unsigned int size, VENDOR_COMMON_MEM_DMA_DIR dir);

HD_RESULT vendor_common_pool_layout(HD_COMMON_MEM_POOL_TYPE pool_type, int attr_nr, VENDOR_COMMON_MEM_LAYOUT_ATTR *layout_attr, 
		int del_layout);
HD_RESULT vendor_common_get_pool_usedbuf_info(HD_COMMON_MEM_POOL_TYPE pool_type, int *p_used_buf_nr, HD_BUFINFO *buf_info);
HD_RESULT vendor_common_clear_all_blk(void);
HD_RESULT vendor_common_clear_pool_blk(HD_COMMON_MEM_POOL_TYPE pool_type, int ddrid);
#endif // __VENDOR_COMMON_H__
