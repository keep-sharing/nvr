// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_DEF_H
#define HAL_VO_DEF_H

#include "ot_type.h"
#include "ot_defines.h"
#include "hal_vo_def_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#define VHD_REGS_LEN        0x1000 /* len of V0's regs */
#define GFX_REGS_LEN        0x800
#define DHD_REGS_LEN        0x1000
#define INTF_REGS_LEN       0x100
#define VID_REGS_LEN        0x200 /* len of VID regs */
#define GRF_REGS_LEN        0x200 /* len of GFX regs */

/* offset define */
/* 0x200 bytes, 0x200/4 regs */
#define FDR_VID_OFFSET      (0x200 / 4)
#define ZME_HPREC           (1 << 20)
#define ZME_VPREC           (1 << 12)

typedef enum {
    VO_DEV_DHD0 = 0, /* ultra high definition device */
    VO_DEV_DHD1 = 1, /* high definition device */
    VO_DEV_DSD0 = 2, /* standard definition device */
    VO_DEV_BUTT
} vo_hal_dev;

typedef enum {
    VO_HAL_LAYER_VHD0 = 0, /* V0 layer */
    VO_HAL_LAYER_VHD1 = 1, /* V1 layer */
    VO_HAL_LAYER_VHD2 = 2, /* V2 layer */
    VO_HAL_LAYER_VSD0 = 3, /* V3 layer */

    VO_HAL_LAYER_G0 = 4, /* G0 layer */
    VO_HAL_LAYER_G1 = 5, /* G1 layer */
    VO_HAL_LAYER_G2 = 6, /* G2 layer */
    VO_HAL_LAYER_G3 = 7, /* G3 layer */
    VO_HAL_LAYER_G4 = 8, /* G4 layer */

    VO_HAL_LAYER_WBC = 9, /* wbc layer */

    VO_HAL_LAYER_BUTT
} vo_hal_layer;

typedef enum {
    VO_SW_LAYER_VHD0 = 0,
    VO_SW_LAYER_VHD1 = 1,
    VO_SW_LAYER_VHD2 = 2,
    VO_SW_LAYER_VSD0 = 3,

    VOU_SW_LAYER_BUTT
} vo_sw_layer;

typedef enum {
    HAL_DISP_LAYER_VHD0 = 0,
    HAL_DISP_LAYER_VHD1 = 1,
    HAL_DISP_LAYER_VHD2 = 2,
    HAL_DISP_LAYER_VSD0 = 3,

    HAL_DISP_LAYER_GFX0 = 4,
    HAL_DISP_LAYER_GFX1 = 5,
    HAL_DISP_LAYER_GFX2 = 6,
    HAL_DISP_LAYER_GFX3 = 7,
    HAL_DISP_LAYER_GFX4 = 8,

    HAL_DISP_LAYER_WBC = 9,

    HAL_DISP_LAYER_BUTT,
} hal_disp_layer;

#define DEV_PHY_START       VO_DEV_DHD0
#define DEV_PHY_END         VO_DEV_DSD0

#define LAYER_VID_START     HAL_DISP_LAYER_VHD0  /* VHD0 */
#define LAYER_VID_END       HAL_DISP_LAYER_VSD0  /* VSD0 */

#define LAYER_GFX_START     HAL_DISP_LAYER_GFX0  /* GFX0 */
#define LAYER_GFX_END       HAL_DISP_LAYER_GFX4  /* GFX4 */

#define VO_DEAULT_VTTH_TIME 8
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
#endif /* end of HAL_VO_DEF_H */
