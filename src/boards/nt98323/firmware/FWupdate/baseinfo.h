#ifndef __BASEINFO_H__
#define __BASEINFO_H__

#include "hiboot_version.h"

#ifndef str
#define str(s) #s
#endif

#ifndef xstr
#define xstr(s) str(s)
#endif

// define input image
#ifndef INPUT_BLD_IMG
#define INPUT_BLD_IMG	
#endif /* INPUT_BLD_IMG */
#ifndef INPUT_KER_IMG
#define INPUT_KER_IMG	
#endif /* INPUT_KER_IMG */
#ifndef INPUT_FS_IMG
#define INPUT_FS_IMG	
#endif /* INPUT_FS_IMG */
#ifndef INPUT_CMD_IMG
#define INPUT_CMD_IMG	
#endif /* INPUT_CMD_IMG */
#ifndef INPUT_JPG_IMG
#define INPUT_JPG_IMG	
#endif /* INPUT_JPG_IMG */
#ifndef INPUT_BLD2_IMG
#define INPUT_BLD2_IMG	
#endif /* INPUT_BLD2_IMG */
#ifndef INPUT_TOOL_IMG
#define INPUT_TOOL_IMG
#endif /* INPUT_TOOL_IMG */

// define image version
#define CMD_VER_MAIN	1
#define CMD_VER_SUB		0
#define CMD_VER_THIRD	1
#define UPDATE_CMD_VER	LINK_VER(0,CMD_VER_MAIN,CMD_VER_SUB,CMD_VER_THIRD)

#define INPUT_BLD_VER	VER_BLD_NUM
#define INPUT_KER_VER	VER_KER_NUM
#define INPUT_FS_VER	VER_FS_NUM
#define INPUT_CMD_VER	MS_VER_NUM(0,CMD_VER_MAIN,CMD_VER_SUB,CMD_VER_THIRD)
#define INPUT_JPG_VER   VER_JPG_NUM
#define INPUT_BLD2_VER	VER_BLD2_NUM
#define INPUT_TOOL_VER	MS_VER_NUM(0,0,1,0)


// define image load flag
#define PART_LOAD			0x0		/**< Load partition data */
#define PART_NO_LOAD		0x1		/**< Don't load part data */

#define BLD_LOAD_FLAG		PART_LOAD
#define KER_LOAD_FLAG		PART_LOAD
#define FS_LOAD_FLAG		PART_NO_LOAD

#define NO_LOAD_ADDR		0x0
#define BLD_LOAD_ADDR		NO_LOAD_ADDR//AMBOOT_BLD_RAM_START
#define KER_LOAD_ADDR		NO_LOAD_ADDR//KERNEL_RAM_START
#define FS_LOAD_ADDR		NO_LOAD_ADDR


// define makefile output image
#ifndef OUTPUT_UPFW_IMG
#error "Unknow output firmware name, please check macro OUTPUT_UPFW_IMG"
#endif /* OUTPUT_UPFW_IMG */
#define OUTPUT_UPFW_NAME	xstr(OUTPUT_UPFW_IMG)

// define IPC update firmware output name 
#define ANALYSIS_IMG_NAME	"/dev/shm/updateFM.bin"
#define ANALYSIS_BLD_NAME	"/dev/shm/bld.bin"
#define ANALYSIS_KER_NAME	"/dev/shm/ker.bin"
#define ANALYSIS_FS_NAME	"/dev/shm/ubifs.bin"
#define ANALYSIS_CMD_NAME	"/dev/shm/update"
#define ANALYSIS_JPG_NAME   	"/dev/shm/boot.jpg"
#define ANALYSIS_BLD2_NAME	"/dev/shm/bld2.bin"
#define ANALYSIS_TOOL_NAME	"/dev/shm/tool.bin"

#if defined(__YEAR__) && defined(__MONTH__) && defined(__DAY__)
#define UPFW_VER_DATE	MS_VER_DATE(__YEAR__,__MONTH__,__DAY__)
#else
#define UPFW_VER_DATE	MS_VER_DATE(2014,08,26)
#endif


#endif /* __BASEINFO_H__ */
