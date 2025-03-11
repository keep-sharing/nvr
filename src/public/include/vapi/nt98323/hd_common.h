/**
	@brief Common header file.\n
	This file contains the functions which is related to common part in the chip.

	@file hd_common.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_COMMON_H_
#define _HD_COMMON_H_

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
    TYPE DEFINITION
 ********************************************************************/
/*
 * Chip counter
 */
#define HD_CHIP_COUNT_1 0
#define HD_CHIP_COUNT_2 1
#define HD_CHIP_COUNT_3 2
#define HD_CHIP_COUNT_4 3
#define HD_CHIP_COUNT_5 4
#define HD_CHIP_COUNT_6 5
#define HD_CHIP_COUNT_7 6
#define HD_CHIP_COUNT_8 7

/*
 * DRAM size
 */
typedef enum _HD_COMMON_DRAM_SIZE {
	HD_COMMON_DRAM_SIZE_16MB,
	HD_COMMON_DRAM_SIZE_32MB,
	HD_COMMON_DRAM_SIZE_64MB,
	HD_COMMON_DRAM_SIZE_128MB,
	HD_COMMON_DRAM_SIZE_256MB,
	HD_COMMON_DRAM_SIZE_384MB,
	HD_COMMON_DRAM_SIZE_512MB,
	HD_COMMON_DRAM_SIZE_768MB,
	HD_COMMON_DRAM_SIZE_1024MB,        ///< 1GB
	HD_COMMON_DRAM_SIZE_1280MB,        ///< 1.25GB
	HD_COMMON_DRAM_SIZE_1536MB,        ///< 1.5GB
	HD_COMMON_DRAM_SIZE_1792MB,        ///< 1.75GB
	HD_COMMON_DRAM_SIZE_2048MB,        ///< 2GB
	HD_COMMON_DRAM_SIZE_3072MB,        ///< 3GB
	HD_COMMON_DRAM_SIZE_4096MB,        ///< 4GB
	HD_COMMON_DRAM_SIZE_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_DRAM_SIZE)
} HD_COMMON_DRAM_SIZE;

/*
 * Video input type
 */
typedef enum _HD_COMMON_VIDEO_IN_TYPE {
	HD_COMMON_VIDEO_IN_NONE = 0x00,
	HD_COMMON_VIDEO_IN_BT656_1MUX,     ///< BT656 1ch mode
	HD_COMMON_VIDEO_IN_BT656_2MUX,     ///< BT656 2ch mode
	HD_COMMON_VIDEO_IN_BT656_4MUX,     ///< BT656 4ch mode
	HD_COMMON_VIDEO_IN_BT1120,         ///< BT1120
	HD_COMMON_VIDEO_IN_MIPI_CSI,       ///< MIPI/CSI
	HD_COMMON_VIDEO_IN_LVDS,           ///< LVDS
	HD_COMMON_VIDEO_IN_SLVS_EC,        ///< SLVS EC
	HD_COMMON_VIDEO_IN_P_RAW,          ///< Parallel raw
	HD_COMMON_VIDEO_IN_P_AHD,          ///< Parallel AHD
	HD_COMMON_VIDEO_IN_MIPI_VX1,       ///< MIPI Vx1
	HD_COMMON_VIDEO_IN_P_RAW_VX1,      ///< Parallel raw Vx1
	HD_COMMON_VIDEO_IN_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_VIDEO_IN_TYPE)
} HD_COMMON_VIDEO_IN_TYPE;

/*
 * Video output type
 */
typedef enum _HD_COMMON_VIDEO_OUT_TYPE {
	HD_COMMON_VIDEO_OUT_TYPE_NONE = 0x00,
	HD_COMMON_VIDEO_OUT_RGB888,        ///< RGB888
	HD_COMMON_VIDEO_OUT_MIPI_DSI,      ///< DSI
	HD_COMMON_VIDEO_OUT_HDMI,          ///< HDMI
	HD_COMMON_VIDEO_OUT_VGA,           ///< VGA
	HD_COMMON_VIDEO_OUT_CVBS,          ///< CVBS
	HD_COMMON_VIDEO_OUT_LVDS_,         ///< LVDS
	HD_COMMON_VIDEO_OUT_SLVS_EC,       ///< SLVS_EC
	HD_COMMON_VIDEO_OUT_LCD,           ///< LCD
	HD_COMMON_VIDEO_OUT_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_VIDEO_OUT_TYPE)
} HD_COMMON_VIDEO_OUT_TYPE;

/*
 * Audio input type
 */
typedef enum _HD_COMMON_AUDIO_IN_TYPE {
	HD_COMMON_AUDIO_IN_NULL = 0x00,    ///< default value ; use NULL out if there's no
	HD_COMMON_AUDIO_IN_AUIN,           ///< AUIN
	HD_COMMON_AUDIO_IN_I2S,            ///< I2S in
	HD_COMMON_AUDIO_IN_I2S_MASTER,     ///< I2S in - I2S Master Mode  In
	HD_COMMON_AUDIO_IN_I2S_SLAVE,      ///< I2S in - I2S Slave Mode In
	HD_COMMON_AUDIO_IN_SPDIF,          ///< S/PDIF
	HD_COMMON_AUDIO_IN_SIF,            ///< ATV use SIF input
	HD_COMMON_AUDIO_IN_VIF,            ///< ATV use VIF input
	HD_COMMON_AUDIO_IN_DECODER,        ///< audio decoder (used in DTV & MM)
	HD_COMMON_AUDIO_IN_HDMI,           ///< HDMI Port Input
	HD_COMMON_AUDIO_IN_AUX,            ///< AUX
	HD_COMMON_AUDIO_IN_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_AUDIO_IN_TYPE)
} HD_COMMON_AUDIO_IN_TYPE;

/*
 * Audio output type
 */
typedef enum _HD_COMMON_AUDIO_OUT_TYPE {
	HD_COMMON_AUDIO_OUT_NULL,               ///< default value ; use NULL out if there's no
	HD_COMMON_AUDIO_OUT_I2S_TX,             ///< I2S Output
	HD_COMMON_AUDIO_OUT_I2S_DAC,            ///< Internal DAC Output
	HD_COMMON_AUDIO_OUT_I2S_HP,             ///< Internal HP Output
	HD_COMMON_AUDIO_OUT_SPDIF,              ///< SPDIF
	HD_COMMON_AUDIO_OUT_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_AUDIO_OUT_TYPE)
} HD_COMMON_AUDIO_OUT_TYPE;

/*
 * PCIe COMMUNCIATION
 */
typedef enum _HD_COMMON_PCIE_CHIP {
	COMMON_PCIE_CHIP_RC,                    ///< target is RC
	COMMON_PCIE_CHIP_EP0,                   ///< target is EP0
	COMMON_PCIE_CHIP_EP1,                   ///< target is EP1
	COMMON_PCIE_CHIP_EP2,                   ///< target is EP2
	COMMON_PCIE_CHIP_EP3,                   ///< target is EP3
	COMMON_PCIE_CHIP_EP4,                   ///< target is EP4
	COMMON_PCIE_CHIP_EP5,                   ///< target is EP5
	COMMON_PCIE_CHIP_EP6,                   ///< target is EP6
	COMMON_PCIE_CHIP_EP7,                   ///< target is EP7
	COMMON_PCIE_CHIP_EP8,                   ///< target is EP8
	COMMON_PCIE_CHIP_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_PCIE_CHIP)
} HD_COMMON_PCIE_CHIP;

typedef enum _HD_COMMON_PCIE_PARAM_ID {
	COMMON_PCIE_PARAM_GET_READ_MAX,         ///< get max read size once while reading
	COMMON_PCIE_PARAM_GET_WRITE_MAX,        ///< get max write size once while writing
	COMMON_PCIE_PARAM_SET_BUF_CONTIGUOUS,   ///< the buffer is physical contigous. If it is cachable, please flush it first. 1 for dma.
	COMMON_PCIE_PARAM_SET_MEMSHARE,         ///< small data can use this kind of channel to send/receive data, 1 for memory share
	COMMON_PCIE_PARAM_SET_INTERRUPT_FIRE,   ///< for small data channel use, send the interrupt by upper application. 1 for fire.
	COMMON_PCIE_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_PCIE_PARAM_ID)
} HD_COMMON_PCIE_PARAM_ID;


#define HD_COMMON_MEM_MAX_POOL_NUM	            128
#define HD_COMMON_MEM_VB_INVALID_BLK           -1
#define HD_COMMON_MEM_SHARED_POOL_NUM           1
#define HD_COMMON_MEM_MAX_MMAP_NUM	            1024
typedef INT32         HD_COMMON_MEM_VB_BLK;       ///< Signed 32 bits data type

typedef enum _HD_COMMON_MEM_POOL_TYPE {
	HD_COMMON_MEM_COMMON_POOL = 1,
	HD_COMMON_MEM_DISP0_IN_POOL,
	HD_COMMON_MEM_DISP1_IN_POOL,
	HD_COMMON_MEM_DISP2_IN_POOL,
	HD_COMMON_MEM_DISP3_IN_POOL,
	HD_COMMON_MEM_DISP4_IN_POOL,
	HD_COMMON_MEM_DISP5_IN_POOL,
	HD_COMMON_MEM_DISP0_CAP_OUT_POOL,
	HD_COMMON_MEM_DISP1_CAP_OUT_POOL,
	HD_COMMON_MEM_DISP2_CAP_OUT_POOL,            ///< 10
	HD_COMMON_MEM_DISP3_CAP_OUT_POOL,
	HD_COMMON_MEM_DISP4_CAP_OUT_POOL,
	HD_COMMON_MEM_DISP5_CAP_OUT_POOL,
	HD_COMMON_MEM_DISP0_ENC_SCL_OUT_POOL,
	HD_COMMON_MEM_DISP1_ENC_SCL_OUT_POOL,
	HD_COMMON_MEM_DISP2_ENC_SCL_OUT_POOL,
	HD_COMMON_MEM_DISP3_ENC_SCL_OUT_POOL,
	HD_COMMON_MEM_DISP4_ENC_SCL_OUT_POOL,
	HD_COMMON_MEM_DISP5_ENC_SCL_OUT_POOL,
	HD_COMMON_MEM_DISP0_ENC_OUT_POOL,            ///< 20
	HD_COMMON_MEM_DISP1_ENC_OUT_POOL,
	HD_COMMON_MEM_DISP2_ENC_OUT_POOL,
	HD_COMMON_MEM_DISP3_ENC_OUT_POOL,
	HD_COMMON_MEM_DISP4_ENC_OUT_POOL,
	HD_COMMON_MEM_DISP5_ENC_OUT_POOL,
	HD_COMMON_MEM_DISP_DEC_IN_POOL,
	HD_COMMON_MEM_DISP_DEC_OUT_POOL,
	HD_COMMON_MEM_DISP_DEC_OUT_RATIO_POOL,
	HD_COMMON_MEM_ENC_CAP_OUT_POOL,
	HD_COMMON_MEM_ENC_SCL_OUT_POOL,              ///< 30
	HD_COMMON_MEM_ENC_OUT_POOL,
	HD_COMMON_MEM_AU_ENC_AU_GRAB_OUT_POOL,
	HD_COMMON_MEM_AU_DEC_AU_RENDER_IN_POOL,
	HD_COMMON_MEM_ENC_REF_POOL,
	HD_COMMON_MEM_DEC_TILE_POOL,                 ///< 35
	HD_COMMON_MEM_FLOW_MD_POOL,
	HD_COMMON_MEM_USER_BLK,

	///< user controlled/defined pool, will not be internal managed, can't hd_common_mem_get_block()
	HD_COMMON_MEM_USER_POOL_BEGIN = 100,
	HD_COMMON_MEM_GLOBAL_MD_POOL,
	HD_COMMON_MEM_TMNR_MOTION_POOL,
	HD_COMMON_MEM_OSG_POOL,
	HD_COMMON_MEM_GFX_POOL,
	HD_COMMON_MEM_DSP_POOL,
	HD_COMMON_MEM_CNN_POOL,
	HD_COMMON_MEM_DISP0_FB_POOL,
	HD_COMMON_MEM_DISP1_FB_POOL,
	HD_COMMON_MEM_DISP2_FB_POOL,
	HD_COMMON_MEM_DISP3_FB_POOL,
	HD_COMMON_MEM_DISP4_FB_POOL,
	HD_COMMON_MEM_DISP5_FB_POOL,
	HD_COMMON_MEM_USER_DEFINIED_POOL,            ///< user can assign self-definied pool index after this
	ENUM_DUMMY4WORD(HD_COMMON_MEM_POOL_TYPE)
} HD_COMMON_MEM_POOL_TYPE;


typedef enum {
	DDR_ID_SYSTEM   = 0,
	DDR_ID0         = DDR_ID_SYSTEM,
	DDR_ID1         = 1,
	DDR_ID2         = 2,
	DDR_ID3         = 3,
	DDR_ID4         = 4,
	DDR_ID5         = 5,
	DDR_ID6         = 6,
	DDR_ID7         = 7,
	DDR_ID_MAX,
} HD_COMMON_MEM_DDR_ID;

typedef struct _HD_COMMON_MEM_POOL_INFO {
	HD_COMMON_MEM_DDR_ID     ddr_id;                                     ///< ddr id
	HD_COMMON_MEM_POOL_TYPE  type;                                       ///< main pool type
	UINT32                   blk_size;                                   ///< the block size of this pool
	UINT32                   blk_cnt;                                    ///< the block count of this pool
	UINT32                   start_addr;                                 ///< the starting address of this pool
	HD_COMMON_MEM_POOL_TYPE  shared_pool[HD_COMMON_MEM_SHARED_POOL_NUM]; ///< indicate which pool type is shared with main pool
} HD_COMMON_MEM_POOL_INFO;

typedef struct _HD_COMMON_MEM_CONFIG {
	HD_COMMON_MEM_POOL_INFO         pool_info[HD_COMMON_MEM_MAX_POOL_NUM];  ///< memory pool info
} HD_COMMON_MEM_INIT_CONFIG;

typedef struct _HD_COMMON_MEM_VIRT_INFO {
	const void           *va;                      ///< [in]  the virtual memory
	UINT32                pa;                      ///< [out] the physical memory
	BOOL                  cached;                  ///< [out] if cacheable memory
} HD_COMMON_MEM_VIRT_INFO;

typedef enum {
	HD_COMMON_MEM_MEM_TYPE_CACHE            =   0, ///< cache memory
	HD_COMMON_MEM_MEM_TYPE_NONCACHE         =   1, ///< non-cache memory
	HD_COMMON_MEM_MEM_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_MEM_MEM_TYPE)
} HD_COMMON_MEM_MEM_TYPE;

typedef enum _HD_COMMON_MEM_PARAM_ID {
	HD_COMMON_MEM_PARAM_INIT_CONFIG,           ///< get memory init configuration(single process only), referring to HD_COMMON_MEM_INIT_CONFIG.
	HD_COMMON_MEM_PARAM_POOL_CONFIG,           ///< get pool configuration(single process only), referring to HD_COMMON_MEM_POOL_INFO.
	HD_COMMON_MEM_PARAM_VIRT_INFO,             ///< get virtual memory info .
	HD_COMMON_MEM_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_COMMON_MEM_PARAM_ID)
} HD_COMMON_MEM_PARAM_ID;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
 ********************************************************************/

/**
 * <1> App Call to hd_common_init()
 * hd_common_init(INIT_DRAM_SIZE | ((HD_CHIP_COUNT_1 & 0x0f) << 8) );
 * where bit 0 to 11 relative definition are described like below, bit 12 ~ 31 were reserved:
 * ex. hd_common_init(INIT_DRAM_SIZE | ((HD_CHIP_COUNT_1 & 0x0f) << 8) );
*/
HD_RESULT hd_common_init(UINT32 sys_config_type);

/**
 * @brief This function sets system config settings. (no effect on IPCam)
 * @remarks If system configuration is needed after hd_common_init is called, it is used like for front or back end chip independent configuration. For example, set the maximium input paths
 * @param sys_config_type1 project model or chip dependent config value
 * @param sys_config_type2 project model or chip dependent config value
 * @param Type1mask type1_mask masking value
 * @param Type2mask type2_mask masking value
 * @return HD_RESULT HD_OK or else
 */
HD_RESULT hd_common_sysconfig(UINT32 sys_config_type1, UINT32 sys_config_type2, UINT32 type1_mask, UINT32 type2_mask);

/**
 * @brief This function gets system config settings. (no effect on IPCam)
 * @remarks This function is used to get the chip ependent configuration that defined in hdal depent project, like front or back end chip.
 * @param p_sys_config_type1 project chip dependent config value(ex: chip specific setting, display output setting etc)
 * @param p_sys_config_type2 project chip dependent config value(ex: chip specific setting, display output setting etc)
 * @return HD_RESULT HD_OK or else
 * @see
 */
HD_RESULT hd_common_get_sysconfig(UINT32 *p_sys_config_type1, UINT32 *p_sys_config_type2); // Depend on chip level configuration

/**
 * @brief de-initializes chip driver
 */
HD_RESULT hd_common_uninit(void);


/**
    Init common memory module and set fixed memory pools layout.

    @param p_mem_config: The fixed memory pools layout.

    @return HD_OK for success, < 0 when some error happened.

    Example:
    @code
    {
		HD_RESULT                 ret;
		HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

		mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[0].blk_size = 0x200000;
		mem_cfg.pool_info[0].blk_cnt = 3;
		mem_cfg.pool_info[0].ddr_id = DDR_ID0;
		mem_cfg.pool_info[1].type = HD_COMMON_MEM_OSG_POOL;
		mem_cfg.pool_info[1].blk_size = 0x200000;
		mem_cfg.pool_info[1].blk_cnt = 3;
		mem_cfg.pool_info[1].ddr_id = DDR_ID0;

		ret = hd_common_mem_init(&mem_cfg);
		if (HD_OK != ret) {
			printf("hd_common_mem_init err: %d\r\n", ret);
		}
    }
    @endcode
*/
HD_RESULT hd_common_mem_init(HD_COMMON_MEM_INIT_CONFIG *p_mem_config);

/**
    Calculate required buffer size for specified data format.

    @note Currently support data formats are HD_VDO_FRAME/HD_VDO_BITSTREAM/HD_AUD_FRAME/HD_AUD_BITSTREAM.

    @param p_hd_data: the hd_data pointer.

    @return the required buffer size.

    Example:
    @code
    {
		HD_VIDEO_FRAME my_frame = {0};

		my_frame.sign = MAKEFOURCC('V','F','R','M');
		my_frame.dim.w = 1920;
		my_frame.dim.h = 1080;
		my_frame.loff[0]= 1920;
		my_frame.loff[1]= 1920;
		my_frame.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		buffer_size = hd_common_mem_calc_buf_size((void *)&my_frame);
    }
    @endcode
*/
UINT32 hd_common_mem_calc_buf_size(void *p_hd_data);

/**
    Map a physical address to user space.

    @param phy_addr: physical address that want to map, the input address should be page aligned.
    @param size: memory size that want to map.

    @return
		- @b NULL : map has some error.
		- @b otherwise: the mapped address, the output addr will be page aligned.

    Example:
    @code
    {
		UINT32 va,pa;

		pa = 0x100000;
		blk_size = 0x4000;

		va = (UINT32)nvtmpp_sys_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa,blk_size);
		if (va == 0) {
			goto map_err;
		}
		printf("va = 0x%x\r\n",va);
	}
    @endcode
*/
void *hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE mem_type, UINT32 phy_addr, UINT32 size);

/**
    Un-map a user space memory.

    @param virt_addr: user space virtual address.
    @param size: memory size that want to unmap.

    @return
		- @b -1: unmap has some error.
		- @b  0: Success.

    Example:
    @code
    {
		UINT32 va,pa;

		pa = 0x100000;
		blk_size = 0x4000;

		va = (UINT32)hd_common_mem_munmap(pa, blk_size);
		if (va == 0) {
			goto map_err;
		}
		printf("va = 0x%x\r\n",va);
		hd_common_mem_munmap((void*)va, blk_size);
	}
    @endcode
*/
HD_RESULT hd_common_mem_munmap(void *virt_addr, unsigned int size);

/**
    Flush the cache memory data.

    @param virt_addr: the cache memory address.
    @param size: the memory size want to flush.

    @return HD_OK for success, < 0 when some error happened.
*/
HD_RESULT hd_common_mem_flush_cache(void *virt_addr, unsigned int size);

/**
    Get one new block from video buffer pools.

    Get one new block from video buffer pools.

    @param pool_type: the pool type.
    @param blk_size: the block size.
    @param ddr: DDR1 or DDR2 ...

    @return HD_COMMON_MEM_VB_INVALID_BLK for some error happened, success will return the block handle.

    Example:
    @code
    {
		HD_COMMON_MEM_POOL_TYPE  pool = HD_COMMON_MEM_COMMON_POOL;
		UINT32                blk_size = 1920 * 1080 * 2;
		HD_COMMON_MEM_DDR_ID     ddr = DDR_ID0;
		HD_COMMON_MEM_VB_BLK     blk;

		blk = hd_common_mem_get_block(pool, blk_size, ddr);
		if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
			printf("ge blk fail\r\n");
			return FALSE;
		}
    }
    @endcode
*/
HD_COMMON_MEM_VB_BLK hd_common_mem_get_block(HD_COMMON_MEM_POOL_TYPE pool_type, UINT32 blk_size, HD_COMMON_MEM_DDR_ID ddr);

/**
    Release the video buffer block.

    This API will cause video buffer block reference count-1.
    If reference count reaches zero, then the block will be released and return back to pools.

    @param blk: the block handle.

    @return HD_OK for success, < 0 when some error happened.

    Example:
    @code
    {
		HD_RESULT          ret;

		ret = hd_common_mem_release_block(g_blk);
		if (HD_OK != ret) {
			printf("release blk fail %d\r\n", ret);
			return FALSE;
		}
    }
    @endcode
*/
HD_RESULT hd_common_mem_release_block(HD_COMMON_MEM_VB_BLK blk);

/**
	Translate video buffer block handle to buffer physical address.

	@param blk: the block handle.

	@return the block buffer address, return 0 when some error happened,

	Example:
	@code
	{
		UINT32                 addr;
		HD_COMMON_MEM_VB_BLK      blk = g_blk;

		addr = hd_common_mem_blk2pa(blk);
		if (addr == 0) {
			printf("block2paddr fail, blk = 0x%x\r\n", blk);
			return FALSE;
		}
	}
	@endcode
*/
UINT32 hd_common_mem_blk2pa(HD_COMMON_MEM_VB_BLK blk);

/**
	Allocates the continuous memory in the user state.

	@note the allocated memory is cacheable.

	@param name:      [in]  the name to describe this memory.
	@param phy_addr:  [out] the physical address returned.
	@param virt_addr: [out] the virtual address returned.
	@param size:      [in]  the continuous memory size want to alloc.
	@param ddr:       [in]  ddr id.

    @return
		- @b  HD_OK: Success.
		- @b  < 0:   Some error happened.

	Example:
	@code
	{
		UINT32               pa, va;
		UINT32               size = 0x200000;
		HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
		HD_RESULT            ret;

		ret = hd_common_mem_alloc("osg1", &pa, (void **)&va, size, ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", size, ddr_id);
		}
		printf("pa = 0x%x, va = 0x%x\r\n", pa, va);
	}
	@endcode
*/
HD_RESULT hd_common_mem_alloc(CHAR *name, UINT32 *phy_addr, void **virt_addr, UINT32 size, HD_COMMON_MEM_DDR_ID ddr);

/**
	Releases the continuous memory in the user state.

	@param phy_addr:  the physical address.
	@param virt_addr: the virtual address.

    @return
		- @b  HD_OK: Success.
		- @b  < 0:   Some error happened.

	Example:
	@code
	{
		UINT32               pa, va;
		UINT32               size = 0x200000;
		HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
		HD_RESULT            ret;

		ret = hd_common_mem_alloc("osg1", &pa, (void **)&va, size, ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", size, ddr_id);
		}
		printf("pa = 0x%x, va = 0x%x\r\n", pa, va);
		ret = hd_common_mem_free(pa, (void *)va);
		if (ret != HD_OK) {
			printf("err:free pa = 0x%x, va = 0x%x\r\n", pa, va);
		}
	}
	@endcode
*/
HD_RESULT hd_common_mem_free(UINT32 phy_addr, void *virt_addr);

/**
	Get pool information in the user state.

	@param HD_COMMON_MEM_PARAM_ID:  the parameter ID.
	@param p_param: the parameters.

    @return
		- @b  HD_OK: Success.
		- @b  < 0:   Some error happened.

	Example:
	@code
	{
		HD_COMMON_MEM_POOL_INFO mem_info;
		mem_info.type = HD_COMMON_MEM_TMNR_MOTION_POOL;
		mem_info.ddr_id = DDR_ID0;
		if (hd_common_mem_get(HD_COMMON_MEM_PARAM_POOL_CONFIG, (VOID *)&mem_info) == HD_OK) {
			printf("get pool_type %d ddr_id %d with size %dKB count %d start_addr 0x%x\n",
				mem_info.type, mem_info.ddr_id, mem_info.blk_size, mem_info.blk_cnt, mem_info.start_addr);
		} else {
			printf("err:pool_type %d ddr_id %d\r\n", mem_info.type, mem_info.ddr_id);
		}
	}
	@endcode
*/
HD_RESULT hd_common_mem_get(HD_COMMON_MEM_PARAM_ID id, VOID *p_param);

/**
	DMA function to copy data from source to destination

	@param src_ddr: source ddr id
	@param src_phy_addr: source physical address
	@param dst_ddr: destination ddr id
	@param dst_phy_addr: destination physical address
	@param len: dma length (should be 16 bytes alignment)

	@return
		- @b  HD_OK: Success.
		- @b  < 0:   Some error happened.
 */
HD_RESULT hd_common_dmacopy(HD_COMMON_MEM_DDR_ID src_ddr, UINT32 src_phy_addr,
							HD_COMMON_MEM_DDR_ID dst_ddr, UINT32 dst_phy_addr,
							UINT32 len);

/**
	Un-initializes common memory module.
*/
HD_RESULT hd_common_mem_uninit(void);


/* pci_comm
 */
/**
 * global init chip driver.
 */
HD_RESULT hd_common_pcie_init(void);

/**
 * open a common_pcie channel. This channel only can be opened once.
 * @name: must give the name
 * @chip_id: which chip we would like to talk with
 * @chan_id: which channel of chip_id we would like to talk with
 * @mode: O_RDONLY/O_WRONLY/O_RDWR, this is identical to linux standard usage.
 */
HD_RESULT hd_common_pcie_open(char *name, HD_COMMON_PCIE_CHIP chip_id, int chan_id, int mode);
/**
 * close an opened common_pcie channel. open/close works in pair.
 * @chip_id/chan_id: which chip/channel we would like to close
 */
HD_RESULT hd_common_pcie_close(HD_COMMON_PCIE_CHIP chip_id, int chan_id);

/**
 * used to get the number of chips.
 * return: the number of chips.
 */
INT hd_common_pcie_get_chip_num(VOID);

/**
 * used to get max read/write length once.
 * @chip_id/chan_id: specify in pair to indicate an unique channel.
 * @id: COMMON_PCIE_PARAM_GET_READ_MAX/COMMON_PCIE_PARAM_GET_WRITE_MAX
 * @parm: integer value for length
 */
HD_RESULT hd_common_pcie_get(HD_COMMON_PCIE_CHIP chip_id, int chan_id, HD_COMMON_PCIE_PARAM_ID id, void *parm);

/**
 * used to set read/write blocking mode.
 * @chip_id/chan_id: specify in pair to indicate an unique channel.
 * @id: COMMON_PCIE_PARAM_SET_READ_OP/COMMON_PCIE_PARAM_SET_WRITE_OP
 * @parm: integer value
 *		0: non-blocking mode
 *		1: blocking mode
 *		default is blocking mode.
 */
HD_RESULT hd_common_pcie_set(HD_COMMON_PCIE_CHIP chip_id, int chan_id, HD_COMMON_PCIE_PARAM_ID id, void *parm);

/**
 * used to read data from the specified channel
 * @chip_id/chan_id: specify in pair to indicate an unique channel.
 * @pbuf: buffer to receive
 * @len: expected length. If the len exceeds the real length in common_pcie queue, the real length will be returned.
 * return: real length
 */
int hd_common_pcie_recv(HD_COMMON_PCIE_CHIP chip_id, int chan_id, unsigned char *pbuf, unsigned int len);

/**
 * used to send data to the specified channel
 * @chip_id/chan_id: specify in pair to indicate an unique channel.
 * @pbuf: buffer to send
 * @len: expected length. If the len exceeds the max length of common_pcie queue size, the real length will be returned.
 * return: real length
 */
int hd_common_pcie_send(HD_COMMON_PCIE_CHIP chip_id, int chan_id, unsigned char *pbuf, unsigned int len);

/**
 * @brief de-initializes chip driver
 */
HD_RESULT hd_common_pcie_uninit(void);

#endif /* _HD_COMMON_H_ */

