/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_DEFINES_H
#define OT_DEFINES_H

#include "autoconf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef OT_CHIP
#define OT_CHIP                          CONFIG_OT_SUBCHIP_TYPE
#endif

#define OT_CHIP_SS528V100                    0x01050100
#define OT_CHIP_SS524V100                    0x01050200
#define OT_CHIP_SS625V100                    0x01090500
#define OT_CHIP_SS928V100                     0x01090100
#define OT_CHIP_SS000V100                     0x01090300
#define OT_CHIP_SS626V100                    0x01090200

#define OT_CHIP_NAME                       "SS626V100"

#define OT_MAX_BIND_DST_NUM         64
#define OT_MPP_VER_PRIX                    "V"

#define OT_MAX_COLOR_COMPONENT             2
#ifdef CONFIG_LITEOS_OS
#define OT_ALIGN_NUM                       4
#else
#define OT_ALIGN_NUM                       8
#endif
#define OT_MAX_PRIVATE_DATA_NUM            2
#define OT_MAX_USER_DATA_NUM               2
#define OT_LUMA_PHYS_ALIGN                  32

#define OT_DEFAULT_ALIGN                   16
#define OT_MAX_ALIGN                       1024
#define OT_SEG_COMPRESS_LEN                256

#define OT_MAX_MMZ_NAME_LEN                32

#define OT_QUAD_POINT_NUM                  4

/* For VB */
#define OT_VB_MAX_POOLS                  768

/* For VENC */
#define OT_VENC_VEDU_IP_NUM              1
#define OT_VENC_JPGE_IP_NUM              1
#define OT_VENC_MAX_TILE_NUM             1
#define OT_VENC_MAX_CHN_NUM              64
#define OT_VENC_H264_MAX_WIDTH           4096
#define OT_VENC_H264_MAX_HEIGHT          4096
#define OT_VENC_H264_MIN_WIDTH           114
#define OT_VENC_H264_MIN_HEIGHT          114
#define OT_VENC_H265_MAX_WIDTH           4096
#define OT_VENC_H265_MAX_HEIGHT          4096
#define OT_VENC_H265_MIN_WIDTH           114
#define OT_VENC_H265_MIN_HEIGHT          114
#define OT_VENC_JPEG_MAX_WIDTH           16384
#define OT_VENC_JPEG_MAX_HEIGHT          16384
#define OT_VENC_JPEG_MIN_WIDTH           32
#define OT_VENC_JPEG_MIN_HEIGHT          32
#define OT_VENC_MAX_INPUT_FRAME_RATE     240
#define OT_VENC_MIN_INPUT_FRAME_RATE     1
#define OT_VENC_MAX_RECV_SRC             4
#define OT_VENC_H265_ADAPTIVE_FRAME_TYPE 4
#define OT_VENC_H265_ADAPTIVE_QP_TYPE    5
#define OT_VENC_H265E_LCU_SIZE           32

/* For RC */
#define OT_VENC_MAX_BITRATE              (60 * 1024)
#define OT_VENC_MIN_BITRATE              2

/* For VDEC */
#define OT_VDEC_MAX_CHN_NUM                128
#define OT_VDH_MAX_CHN_NUM                 128
#define OT_VEDU_CHN_START                  OT_VDH_MAX_CHN_NUM
#define OT_VEDU_H264D_ERRRATE              10
#define OT_VEDU_H264D_FULLERR              100

#define OT_H264D_ALIGN_W                   64
#define OT_H264D_ALIGN_H                   16
#define OT_H265D_ALIGN_W                   64
#define OT_H265D_ALIGN_H                   64
#define OT_JPEGD_ALIGN_W                   64
#define OT_JPEGD_ALIGN_H                   16
#define OT_JPEGD_RGB_ALIGN                 16

#define OT_H264D_MAX_SPS                   32
#define OT_H264D_MIN_SPS                   1
#define OT_H264D_MAX_PPS                   256
#define OT_H264D_MIN_PPS                   1
#define OT_H264D_MAX_SLICE                 300
#define OT_H264D_MIN_SLICE                 1

#define OT_H265D_MAX_VPS                   16
#define OT_H265D_MIN_VPS                   1
#define OT_H265D_MAX_SPS                   16
#define OT_H265D_MIN_SPS                   1
#define OT_H265D_MAX_PPS                   64
#define OT_H265D_MIN_PPS                   1
#define OT_H265D_MAX_SLICE                 600
#define OT_H265D_MIN_SLICE                 1

#define OT_VDH_MPEG4_MAX_WIDTH             4096
#define OT_VDH_MPEG4_MAX_HEIGHT            4096
#define OT_VDH_MPEG4_MIN_WIDTH             64
#define OT_VDH_MPEG4_MIN_HEIGHT            64
#define OT_VDH_H264D_MAX_WIDTH             8192
#define OT_VDH_H264D_MAX_HEIGHT            8192
#define OT_VDH_H264D_MIN_WIDTH             96
#define OT_VDH_H264D_MIN_HEIGHT            96
#define OT_VDH_H265D_MAX_WIDTH             8192
#define OT_VDH_H265D_MAX_HEIGHT            8192
#define OT_VDH_H265D_MIN_WIDTH             96
#define OT_VDH_H265D_MIN_HEIGHT            96
#define OT_VDH_MIN_MSG_NUM                 1
#define OT_VDH_MAX_MSG_NUM                 8

#define OT_JPEGD_IP_NUM                    2
#define OT_JPEGD_MAX_WIDTH                 16384
#define OT_JPEGD_MAX_HEIGHT                16384
#define OT_JPEGD_MIN_WIDTH                 8
#define OT_JPEGD_MIN_HEIGHT                8
#define OT_JPEGD_RGB_MAX_HEIGHT            8640

/* For VDA */
#define OT_VDA_MAX_NODE_NUM        32
#define OT_VDA_MAX_INTERNAL        256
#define OT_VDA_CHN_NUM_MAX         128
#define OT_VDA_MAX_WIDTH           960
#define OT_VDA_MAX_HEIGHT          960
#define OT_VDA_MIN_WIDTH           32
#define OT_VDA_MIN_HEIGHT          32

/* For Region */
#define OT_RGN_MIN_WIDTH                   2
#define OT_RGN_MIN_HEIGHT                  2

#define OT_RGN_COVER_MIN_X                 (-16382)
#define OT_RGN_COVER_MIN_Y                 (-8190)
#define OT_RGN_COVER_MAX_X                 16382
#define OT_RGN_COVER_MAX_Y                 8190
#define OT_RGN_COVER_MAX_WIDTH             16384
#define OT_RGN_COVER_MAX_HEIGHT            8192
#define OT_RGN_COVER_MIN_THICK             2
#define OT_RGN_COVER_MAX_THICK             8

#define OT_RGN_COVEREX_MIN_X               (-16382)
#define OT_RGN_COVEREX_MIN_Y               (-8190)
#define OT_RGN_COVEREX_MAX_X               16382
#define OT_RGN_COVEREX_MAX_Y               8190
#define OT_RGN_COVEREX_MAX_WIDTH           16384
#define OT_RGN_COVEREX_MAX_HEIGHT          8192
#define OT_RGN_COVEREX_MIN_THICK           2
#define OT_RGN_COVEREX_MAX_THICK           14
#define OT_RGN_COVEREX_THICK_ALIGN         2

#define OT_RGN_OVERLAY_MIN_X               0
#define OT_RGN_OVERLAY_MIN_Y               0
#define OT_RGN_OVERLAY_MAX_X               16382
#define OT_RGN_OVERLAY_MAX_Y               8190
#define OT_RGN_OVERLAY_MAX_WIDTH           16384
#define OT_RGN_OVERLAY_MAX_HEIGHT          8192
#define OT_RGN_OVERLAY_MIN_ALPHA           0
#define OT_RGN_OVERLAY_MAX_ALPHA           255
#define OT_RGN_OVERLAY_VENC_MAX_ALPHA      128

#define OT_RGN_OVERLAYEX_MIN_X             0
#define OT_RGN_OVERLAYEX_MIN_Y             0
#define OT_RGN_OVERLAYEX_MAX_X             16382
#define OT_RGN_OVERLAYEX_MAX_Y             8190
#define OT_RGN_OVERLAYEX_MAX_WIDTH         16384
#define OT_RGN_OVERLAYEX_MAX_HEIGHT        8192
#define OT_RGN_OVERLAYEX_MIN_ALPHA         0
#define OT_RGN_OVERLAYEX_MAX_ALPHA         255


#define OT_RGN_LINE_MIN_X                   0
#define OT_RGN_LINE_MIN_Y                   0
#define OT_RGN_LINE_MAX_X                   16382
#define OT_RGN_LINE_MAX_Y                   8190
#define OT_RGN_LINE_MIN_THICK               1
#define OT_RGN_LINE_MAX_THICK               7

#define OT_RGN_MOSAIC_X_ALIGN              4
#define OT_RGN_MOSAIC_Y_ALIGN              2
#define OT_RGN_MOSAIC_WIDTH_ALIGN          4
#define OT_RGN_MOSAIC_HEIGHT_ALIGN         2

#define OT_RGN_MOSAIC_MIN_X                (-16384)
#define OT_RGN_MOSAIC_MIN_Y                (-8192)
#define OT_RGN_MOSAIC_MAX_X                16380
#define OT_RGN_MOSAIC_MAX_Y                8190
#define OT_RGN_MOSAIC_MIN_WIDTH            8
#define OT_RGN_MOSAIC_MIN_HEIGHT           8
#define OT_RGN_MOSAIC_MAX_WIDTH            16384
#define OT_RGN_MOSAIC_MAX_HEIGHT           8192

#define OT_RGN_MOSAICEX_MIN_X              (-16384)
#define OT_RGN_MOSAICEX_MIN_Y              (-8192)
#define OT_RGN_MOSAICEX_MAX_X              16380
#define OT_RGN_MOSAICEX_MAX_Y              8190
#define OT_RGN_MOSAICEX_MIN_WIDTH          8
#define OT_RGN_MOSAICEX_MIN_HEIGHT         8
#define OT_RGN_MOSAICEX_MAX_WIDTH          16384
#define OT_RGN_MOSAICEX_MAX_HEIGHT         8192

#define OT_RGN_ALIGN                       2

#define OT_RGN_HANDLE_MAX                  1024
#define OT_RGN_MAX_BUF_NUM                 2

#define OT_RGN_VI_MAX_COVER_NUM            4

#define OT_RGN_VENC_MAX_OVERLAY_NUM        8

#define OT_RGN_VPSS_MAX_COVER_NUM          4
#define OT_RGN_VPSS_MAX_COVEREX_NUM        8
#define OT_RGN_VPSS_MAX_OVERLAY_NUM        8
#define OT_RGN_VPSS_MAX_OVERLAYEX_NUM      8
#define OT_RGN_VPSS_MAX_LINE_NUM           40
#define OT_RGN_VPSS_MAX_MOSAIC_NUM         4
#define OT_RGN_VPSS_MAX_MOSAICEX_NUM       4

#define OT_RGN_VO_MAX_COVEREX_NUM          4
#define OT_RGN_VO_MAX_OVERLAYEX_NUM        1
#define OT_RGN_VO_MAX_LINE_NUM             4

#define OT_RGN_PCIV_MAX_OVERLAYEX_NUM      1

#define OT_RGN_VGS_MAX_TASK_WIDTH          16384

/* For VI */
/* number of channel and device on video input unit of chip
 * Note! OT_VI_MAX_CHN_NUM is NOT equal to OT_VI_MAX_DEV_NUM
 * multiplied by OT_VI_MAX_CHN_NUM, because all VI devices
 * can't work at mode of 4 channels at the same time.
 */
#define OT_VI_MAX_PIPE_NUM              12
#define OT_VI_MAX_NORMAL_DEV_NUM        1
#define OT_VI_MAX_CAS_DEV_NUM           1
#define OT_VI_MAX_DEV_NUM               1
#define OT_VI_MAX_WAY_NUM_PER_DEV       1
#define OT_VI_MAX_CHN_NUM_PER_DEV       1
#define OT_VI_MAX_NORMAL_WAY_NUM        1
#define OT_VI_MAX_CAS_WAY_NUM           1
#define OT_VI_MAX_WAY_NUM               1
#define OT_VI_MAX_NORMAL_CHN_NUM        1
#define OT_VI_MAX_CAS_PHYS_CHN_NUM      1
#define OT_VI_MAX_PHYS_CHN_NUM          1

#define OT_VI_MAX_CAS_VIRT_CHN_NUM      2
#define OT_VI_CAS_VIRT_CHN_START        OT_VI_MAX_PHYS_CHN_NUM

#define OT_VI_MAX_CHN_NUM               (OT_VI_MAX_PHYS_CHN_NUM + OT_VI_MAX_CAS_VIRT_CHN_NUM)

/* define cascade chn */
#define OT_VI_CAS_PHYS_CHN              0
#define OT_VI_CAS_VIRT_CHN_0            1
#define OT_VI_CAS_VIRT_CHN_1            2

/* max number of VBI region */
#define OT_VI_MAX_VBI_NUM               2
/* max length of one VBI region (by word) */
#define OT_VI_MAX_VBI_LEN               0x20

/* For VO */
#define OT_VO_MAX_PHYS_DEV_NUM            3  /* max physical dev num */
#define OT_VO_MAX_VIRT_DEV_NUM            32 /* max virtual dev num */
#define OT_VO_MAX_CAS_DEV_NUM             0  /* max cascade dev num */
/* max dev num */
#define OT_VO_MAX_DEV_NUM                 (OT_VO_MAX_PHYS_DEV_NUM + OT_VO_MAX_VIRT_DEV_NUM + OT_VO_MAX_CAS_DEV_NUM)

#define OT_VO_VIRT_DEV_0                  3  /* virtual device 0 */
#define OT_VO_VIRT_DEV_1                  4  /* virtual device 1 */
#define OT_VO_VIRT_DEV_2                  5  /* virtual device 2 */
#define OT_VO_VIRT_DEV_3                  6  /* virtual device 3 */
#define OT_VO_CAS_DEV_1                   35 /* cascade device 1 */
#define OT_VO_CAS_DEV_2                   36 /* cascade device 2 */

#define OT_VO_MAX_PHYS_VIDEO_LAYER_NUM    4 /* max physical video layer num */
#define OT_VO_MAX_GFX_LAYER_NUM           5 /* max graphic layer num */
/* max physical layer num */
#define OT_VO_MAX_PHYS_LAYER_NUM          (OT_VO_MAX_PHYS_VIDEO_LAYER_NUM + OT_VO_MAX_GFX_LAYER_NUM)
/* max layer num */
#define OT_VO_MAX_LAYER_NUM               (OT_VO_MAX_PHYS_LAYER_NUM + OT_VO_MAX_VIRT_DEV_NUM + OT_VO_MAX_CAS_DEV_NUM)
#define OT_VO_MAX_LAYER_IN_DEV            2 /* max video layer num of each dev */

#define OT_VO_LAYER_V0                    0 /* video layer 0 */
#define OT_VO_LAYER_V1                    1 /* video layer 1 */
#define OT_VO_LAYER_V2                    2 /* video layer 2 */
#define OT_VO_LAYER_V3                    3 /* video layer 3 */
#define OT_VO_LAYER_G0                    4 /* graphics layer 0 */
#define OT_VO_LAYER_G1                    5 /* graphics layer 1 */
#define OT_VO_LAYER_G2                    6 /* graphics layer 2 */
#define OT_VO_LAYER_G3                    7 /* graphics layer 3 */
#define OT_VO_LAYER_G4                    8 /* graphics layer 4 */
#define ot_vo_get_virt_layer(vo_virt_dev) ((vo_virt_dev) + 6) /* get virtual layer of virtual dev */
#define ot_vo_get_cas_layer(vo_cas_dev)   ((vo_cas_dev) + 6)  /* get cascade layer of cascade dev */

#define OT_VO_MAX_PRIORITY                4      /* max layer priority */
#define OT_VO_MIN_TOLERATE                1      /* min play toleration 1ms */
#define OT_VO_MAX_TOLERATE                100000 /* max play toleration 100s */

#define OT_VO_MAX_CHN_NUM                 128  /* max chn num */
#define OT_VO_MIN_CHN_WIDTH               32   /* channel minimal width */
#define OT_VO_MIN_CHN_HEIGHT              32   /* channel minimal height */
#define OT_VO_MAX_ZOOM_RATIO              1000 /* max zoom ratio, 1000 means 100% scale */
#define OT_VO_MAX_NODE_NUM                16   /* max node num */

#define OT_VO_MAX_WBC_NUM                 1 /* max wbc num */

#define OT_VO_MAX_CAS_PATTERN             128   /* cascade pattern max number */
#define OT_VO_MAX_CAS_POS_32RGN           32    /* cascade position max number */
#define OT_VO_MAX_CAS_POS_64RGN           64    /* cascade position max number */
#define OT_VO_MAX_IMG_WIDTH               16384 /* vo max img width */
#define OT_VO_MAX_IMG_HEIGHT              8192  /* vo max img height */

/* For AUDIO */
#define OT_AI_DEV_MAX_NUM                  1
#define OT_AO_DEV_MIN_NUM                  0
#define OT_AO_DEV_MAX_NUM                  3
#define OT_AIO_MAX_NUM            ((OT_AO_DEV_MAX_NUM > OT_AI_DEV_MAX_NUM) ? OT_AO_DEV_MAX_NUM : OT_AI_DEV_MAX_NUM)
#define OT_AENC_MAX_CHN_NUM                49
#define OT_ADEC_MAX_CHN_NUM                32

#define OT_AI_MAX_CHN_NUM                  20
#define OT_AO_MAX_CHN_NUM                  3
#define OT_AO_SYS_CHN_ID                  (OT_AO_MAX_CHN_NUM - 1)

#define OT_AIO_MAX_CHN_NUM        ((OT_AO_MAX_CHN_NUM > OT_AI_MAX_CHN_NUM) ? OT_AO_MAX_CHN_NUM : OT_AI_MAX_CHN_NUM)

/* For VPSS */
#define OT_VPSS_MAX_GRP_NUM                256
#define OT_VPSS_VGS_GRP_NO                 256
#define OT_VPSS_IP_NUM                     3
#define OT_VPSS0                           0
#define OT_VPSS1                           1
#define OT_VPSS2                           2
#define OT_VPSS_MAX_PHYS_CHN_NUM            4
#define OT_VPSS_MAX_CHN_NUM                OT_VPSS_MAX_PHYS_CHN_NUM
#define OT_VPSS_MIN_IMG_WIDTH            64
#define OT_VPSS_MIN_IMG_HEIGHT           64
#define OT_VPSS_MAX_IMG_WIDTH            16384
#define OT_VPSS_MAX_DEI_WIDTH            960
#define OT_VPSS_MAX_SEG_COMPACT_WIDTH    2688
#define OT_VPSS_MAX_IMG_HEIGHT           8192
#define OT_VPSS_MAX_ZOOMIN                 16
#define OT_VPSS_MAX_ZOOMOUT                15

/* For PCIV */
#define OT_PCIV_MAX_CHN_NUM                128 /* max pciv channel number in each pciv device */

/* For IVS_MD */
#define OT_MD_MAX_CHN                      64
#define OT_MD_MAX_WIDTH                    1920
#define OT_MD_MAX_HEIGHT                   1080
#define OT_MD_MIN_WIDTH                    64
#define OT_MD_MIN_HEIGHT                   64

/* For VGS */
#define OT_VGS_IP_NUM                      2
#define OT_VGS0                            0
#define OT_VGS1                            1
#define OT_VGS_MAX_COVER_NUM               4
#define OT_VGS_MAX_OSD_NUM                 1
#define OT_VGS_MAX_MOSAIC_NUM              1
#define OT_VGS_MAX_CORNER_RECT_NUM         1

/* For GDC */
#define OT_FISHEYE_MIN_IN_IMAGE_WIDTH     1280
#define OT_FISHEYE_MIN_IN_IMAGE_HEIGHT    720
#define OT_FISHEYE_MIN_OUT_IMAGE_WIDTH    640
#define OT_FISHEYE_MIN_OUT_IMAGE_HEIGHT   360

#define OT_ROTATION_EX_MIN_IMAGE_WIDTH    480
#define OT_ROTATION_EX_MIN_IMAGE_HEIGHT   360

#define OT_PMF_MIN_IMAGE_WIDTH            480
#define OT_PMF_MIN_IMAGE_HEIGHT           360

#define OT_GDC_MAX_IMAGE_WIDTH            8192
#define OT_GDC_MAX_IMAGE_HEIGHT           8192
#define OT_GDC_IP_NUM                     1

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_DEFINES_H */
