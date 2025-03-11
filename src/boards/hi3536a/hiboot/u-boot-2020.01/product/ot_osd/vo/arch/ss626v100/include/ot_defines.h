// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef OT_DEFINES_H
#define OT_DEFINES_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* For VO */
#define OT_VO_MAX_PHYS_DEV_NUM             3  /* max physical dev num */
/* max dev num */
#define OT_VO_MAX_DEV_NUM                  OT_VO_MAX_PHYS_DEV_NUM

#define OT_VO_MAX_PHYS_VIDEO_LAYER_NUM     4  /* max physical video layer num */
#define OT_VO_MAX_GFX_LAYER_NUM            5  /* max graphic layer num */
/* max physical layer num */
#define OT_VO_MAX_PHYS_LAYER_NUM           (OT_VO_MAX_PHYS_VIDEO_LAYER_NUM + OT_VO_MAX_GFX_LAYER_NUM)
/* max layer num */
#define OT_VO_MAX_LAYER_NUM                OT_VO_MAX_PHYS_LAYER_NUM
#define OT_VO_MAX_LAYER_IN_DEV             2 /* max video layer num of each dev */

#define OT_VO_LAYER_V0                     0  /* video layer 0 */
#define OT_VO_LAYER_V1                     1  /* video layer 1 */
#define OT_VO_LAYER_V2                     2  /* video layer 2 */
#define OT_VO_LAYER_V3                     3  /* video layer 3 */
#define OT_VO_LAYER_G0                     4  /* graphics layer 0 */
#define OT_VO_LAYER_G1                     5  /* graphics layer 1 */
#define OT_VO_LAYER_G2                     6  /* graphics layer 2 */
#define OT_VO_LAYER_G3                     7  /* graphics layer 3 */
#define OT_VO_LAYER_G4                     8  /* graphics layer 4 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_DEFINES_H */
