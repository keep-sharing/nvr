/******************************************************************************
 Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : hi_defines.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2005/4/23
Last Modified :
Description   : The common data type defination
Function List :
History       :
******************************************************************************/
#ifndef __HI_DEFINES_H__
#define __HI_DEFINES_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HI3536_V100 0x35360100
#define HI3521A_V100 0x3521a100
#define HI3520D_V300 0x3520d300
#define HI3531A_V100 0x3531a100
#define HI35xx_Vxxx 0x35000000

#ifndef HICHIP
    #define HICHIP HI3536_V100
#endif

#if HICHIP==HI3536_V100
    #define CHIP_NAME    "Hi3536"
    #define MPP_VER_PRIX "_MPP_V"
#elif HICHIP==HI3521A_V100
    #define CHIP_NAME    "Hi3521A"
    #define MPP_VER_PRIX "_MPP_V"
#elif HICHIP==HI35xx_Vxxx
    #error HuHu, I am an dummy chip
#else
    #error HICHIP define may be error
#endif

/* For SYS*/
#define DEFAULT_ALIGN                   16
#define MAX_MMZ_NAME_LEN                16
#define VIDEO_COMPRESS_WIDTH_THRESHOLD  352    /* the startup condition of video compress */

/* For VDA */
#define VDA_MAX_NODE_NUM                32
#define VDA_MAX_INTERNAL                256
#define VDA_CHN_NUM_MAX                 128
#define VDA_MAX_WIDTH                   960
#define VDA_MAX_HEIGHT                  960
#define VDA_MIN_WIDTH                   32
#define VDA_MIN_HEIGHT                  32

/* For VENC */
#define VENC_MAX_NAME_LEN               16
#define VENC_MAX_CHN_NUM                128
#define VENC_MAX_GRP_NUM                128
#define H264E_MAX_WIDTH                 4480
#define H264E_MAX_HEIGHT                3840
#define H264E_MIN_WIDTH                 160
#define H264E_MIN_HEIGHT                64
#define H265E_MAX_WIDTH                 2592
#define H265E_MAX_HEIGHT                2592
#define H265E_MIN_WIDTH                 128
#define H265E_MIN_HEIGHT                128
#define JPEGE_MAX_WIDTH                 8192
#define JPEGE_MAX_HEIGHT                8192
#define JPEGE_MIN_WIDTH                 32
#define JPEGE_MIN_HEIGHT                32
#define VENC_MAX_ROI_NUM                8
#define H264E_MIN_HW_INDEX              0
#define H264E_MAX_HW_INDEX              11
#define H264E_MIN_VW_INDEX              0
#define H264E_MAX_VW_INDEX              3
#define MPEG4E_MAX_HW_INDEX             1
#define MPEG4E_MAX_VW_INDEX             0

/*For RC*/
#define RC_TEXTURE_THR_SIZE             12
#define RC_RQRATIO_SIZE                 8

/* For VDEC */
#define VDEC_MAX_COMPRESS_WIDTH         1920
#define VDEC_MAX_CHN_NUM                128
#define VDH_VEDU_CHN                    128
#define VEDU_MAX_CHN_NUM                128
#define MAX_VDEC_CHN                    VDEC_MAX_CHN_NUM
#define BUF_RESERVE_LENTH               64          /*reserve 64byte for hardware*/
#define H264D_MAX_SLICENUM              137
#define VEDU_H264D_ERRRATE              10
#define VEDU_H264D_FULLERR              100
#define H265_WIDTH_ALIGN                64
#define H265_HEIGHT_ALIGN               64
#define JPEG_WIDTH_ALIGN                64
#define H264_WIDTH_ALIGN                16
#define H264_HEIGHT_ALIGN               32

/* For VDH-H264  */
#define VDH_H264D_MAX_WIDTH             8192
#define VDH_H264D_MAX_HEIGHT            8192
#define VDH_H264D_MIN_WIDTH             64
#define VDH_H264D_MIN_HEIGHT            64
#define VDH_H264D_MAX_SPS               32
#define VDH_H264D_MIN_SPS               1
#define VDH_H264D_MAX_PPS               256
#define VDH_H264D_MIN_PPS               1
#define VDH_H264D_MAX_SLICE             136
#define VDH_H264D_MIN_SLICE             1

/* For VDH-HEVC  */
#define VDH_H265D_MAX_WIDTH             4096
#define VDH_H265D_MAX_HEIGHT            4096
#define VDH_H265D_MIN_WIDTH             128
#define VDH_H265D_MIN_HEIGHT            128
#define VDH_H265D_MAX_VPS               16
#define VDH_H265D_MIN_VPS               1
#define VDH_H265D_MAX_SPS               16
#define VDH_H265D_MIN_SPS               1
#define VDH_H265D_MAX_PPS               64
#define VDH_H265D_MIN_PPS               1
#define VDH_H265D_MAX_SLICE             200
#define VDH_H265D_MIN_SLICE             1

/* For VDH-MPEG4 */
#define VDH_MPEG4D_MAX_WIDTH            4096 
#define VDH_MPEG4D_MAX_HEIGHT           4096 
#define VDH_MPEG4D_MIN_WIDTH            64
#define VDH_MPEG4D_MIN_HEIGHT           64

/* For VDH-MPEG2 */
#define VDH_MPEG2D_MAX_WIDTH            1920 
#define VDH_MPEG2D_MAX_HEIGHT           1088 
#define VDH_MPEG2D_MIN_WIDTH            32
#define VDH_MPEG2D_MIN_HEIGHT           32


/* For JPEG  */
#define JPEGD_MAX_WIDTH                 8192
#define JPEGD_MAX_HEIGHT                8192
#define JPEGD_MIN_WIDTH                 8
#define JPEGD_MIN_HEIGHT                8

/* For REGION */
#define RGN_HANDLE_MAX                  1024
#define RGN_MIN_WIDTH                   2
#define RGN_MIN_HEIGHT                  2
#define RGN_MAX_WIDTH                   8190
#define RGN_MAX_HEIGHT                  8190
#define RGN_ALIGN                       2
/* region num of module */
#define OVERLAY_MAX_NUM_VENC            8
#define OVERLAYEX_MAX_NUM_PCIV          8
#define OVERLAY_MAX_NUM_VPSS            8
#define COVER_MAX_NUM_VPSS              4
#define OVERLAYEX_MAX_NUM_VPSS          8
#define COVEREX_MAX_NUM_VPSS            4
#define LINE_MAX_NUM_VPSS               4
#define COVEREX_MAX_NUM_VO              4
#define OVERLAYEX_MAX_NUM_VO            8
#define LINE_MAX_NUM_VO                 4
#define COVER_MAX_NUM_VI                0
#define COVEREX_MAX_NUM_VI              16
#define OVERLAY_MAX_NUM_VI              0
#define OVERLAYEX_MAX_NUM_VI            16

/* For VIU*/
#define VIU_MAX_DEV_NUM                 1   /* vi max dev number */
#define VIU_MAX_WAY_NUM_PER_DEV         1   /* way number in each device */
#define VIU_MAX_CHN_NUM_PER_DEV         1   /* channel number in each device */
#define VIU_MAX_PHYCHN_NUM              1   /* vi max channel number */
#define VIU_EXT_CHN_START               (VIU_MAX_PHYCHN_NUM + VIU_MAX_CAS_CHN_NUM)
#define VIU_MAX_EXT_CHN_NUM             0   /* vi ext channel number */
#define VIU_MAX_EXTCHN_BIND_PER_CHN     0   /* bind ext channel in each physical channel */
#define VIU_MAX_CAS_CHN_NUM             2   /* vi max cascade channel number */
#define VIU_CAS_CHN_START               1   	/* the first cascade channel number */ 
#define VIU_VBI_POSITION_X_1080P        0x114	/* horizontal original position of the VBI data  */
#define VIU_VBI_POSITION_Y_1080P        0x28	/*  vertical original position of the VBI data */
#define VIU_VBI_POSITION_X_4Kx2K        0x22C	/* horizontal original position of the VBI data  */
#define VIU_VBI_POSITION_Y_4Kx2K        0x55	/*  vertical original position of the VBI data */
#define VIU_MAX_VBI_NUM                 2		/* max number of VBI region*/
#define VIU_MAX_VBI_LEN                 0x20	/* max length of one VBI region (by word)*/
#define VIU_MAX_CHN_NUM                 (VIU_MAX_PHYCHN_NUM + VIU_MAX_CAS_CHN_NUM + VIU_MAX_EXT_CHN_NUM)
/*get the subchn index by main chn */
#define  SUBCHN(ViChn)   (ViChn + 16)

/* define cascade chn */
#define VI_CAS_CHN_1   1
#define VI_CAS_CHN_2   2

/* For VOU */
#define VO_MAX_DEV_NUM                  9       /* we have three VO physical device(HD0,HD1,SD), four virtual device(VIRT0,VIRT1,VIRT2,VIRT3) and  two cascade device(cas1 cas2) */
#define VO_MAX_LAYER_NUM                10      /* max layer num */
#define VHD_MAX_CHN_NUM                 128     /* max VHD chn num */
#define VO_MAX_CHN_NUM                  VHD_MAX_CHN_NUM      /* max chn num */
#define VO_MAX_LAYER_IN_DEV             2       /* max layer num of each dev */
#define VO_MAX_CAS_DEV_NUM              2       /* max cascade dev num*/
#define VO_CAS_DEV_1                    7       /* cascade display device 1 */
#define VO_CAS_DEV_2                    8       /* cascade display device 2 */
#define VO_CAS_MAX_PAT                  128     /* cascade pattern max number */
#define VO_CAS_MAX_POS_32RGN            32      /* cascade position max number */
#define VO_CAS_MAX_POS_64RGN            64      /* cascade position max number */
#define VO_MAX_VIRT_DEV_NUM             4       /* max virtual dev num*/
#define VO_VIRT_DEV_0                   3       /* virtual display device 1 */
#define VO_VIRT_DEV_1                   4       /* virtual display device 2 */
#define VO_VIRT_DEV_2                   5       /* virtual display device 3 */
#define VO_VIRT_DEV_3                   6       /* virtual display device 4 */
#define VO_MAX_GRAPHICS_LAYER_NUM       5       /* we have fb0,fb1,fb2,fb3 and fb4 */ 
#define MDDRC_ZONE_MAX_NUM              32
#define VO_MAX_WBC_NUM                  1
#define VO_MAX_PRIORITY                 2
#define VO_MIN_TOLERATE                 1       /* min play toleration 1ms */
#define VO_MAX_TOLERATE                 100000  /* max play toleration 100s */
#define VO_MIN_CHN_WIDTH                64      /* channel minimal width */
#define VO_MIN_CHN_HEIGHT               64      /* channel minimal height */
#define VO_MAX_ZOOM_RATIO               1000    /* max zoom ratio, 1000 means 100% scale */

/* For AUDIO */
#define AI_DEV_MAX_NUM                  1
#define AO_DEV_MIN_NUM                  0
#define AO_DEV_MAX_NUM                  3
#define AIO_MAX_NUM                     3
#define AIO_MAX_CHN_NUM                 16
#define AENC_MAX_CHN_NUM                32
#define ADEC_MAX_CHN_NUM                32

/* For VPSS */
#define VPSS_MIN_WIDTH  64
#define VPSS_MIN_HEIGHT 64
#define VPSS_MAX_WIDTH  8192
#define VPSS_MAX_HEIGHT 8192
#define VPSS_MAX_DEI_WIDTH      960
#define VPSS_MAX_PRESCALE_WIDTH     2560
#define VPSS_MAX_GRP_NUM                256
#define VPSS_MAX_PHY_CHN_NUM            4
#define VPSS_MAX_EXT_CHN_NUM            0
#define VPSS_MAX_CHN_NUM                (VPSS_MAX_PHY_CHN_NUM + VPSS_MAX_EXT_CHN_NUM)
#define VPSS_CHN0                       0
#define VPSS_CHN1                       1
#define VPSS_CHN2                       2
#define VPSS_CHN3                       3
#define VPSS_INVALID_CHN                -1
#define VPSS_LUMA_STAT_NUM              64
#define VPSS_MAX_SPLIT_NODE_NUM         3

/* For PCIV */
#define PCIV_MAX_CHN_NUM                128       /* max pciv channel number in each pciv device */

/*For FD*/
#define FD_MAX_CHN_NUM                 32
#define FD_MAX_WIDTH                   720
#define FD_MAX_HEIGHT                  576
#define FD_MIN_WIDTH                   64
#define FD_MIN_HEIGHT                  64
#define FD_MAX_STRIDE				   720

/* For SYS */

/* VB size calculate for compressed frame.
    [param input]
        w:  width
        h:  height
        fmt:    pixel format, 0: SP420, 1: SP422
        z:  compress mode, 0: no compress, 1: default compress
    [param output]
        size: vb blk size
        
 */
#define VB_W_ALIGN      16
#define VB_H_ALIGN      32
#define VB_ALIGN(x, a)  ((a) * (((x) + (a) - 1) / (a)))


/* VB size calculate for compressed frame.
    [param input]
        w:  width
        h:  height
        fmt:    pixel format, 0: SP420, 1: SP422
        z:  compress mode, 0: no compress, 1: default compress
    [param output]
        size: vb blk size       
 */
#define VB_HEADER_STRIDE    16

#define VB_PIC_Y_HEADER_SIZE(Width, Height, size)\
    do{\
            size = VB_HEADER_STRIDE * (Height);\
    }while(0)

#define VB_PIC_HEADER_SIZE(Width, Height, Type, size)\
    do{\
            if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == Type)\
            {\
                size = VB_HEADER_STRIDE * (Height) * 2;\
            }\
            else\
            {\
                size = (VB_HEADER_STRIDE * (Height) * 3) >> 1;\
            }\
    }while(0)


#define VB_PIC_BLK_SIZE(Width, Height, Type, size)\
    do{\
            unsigned int u32AlignWidth;\
            unsigned int u32AlignHeight;\
            if (Type==PT_H264 || Type==PT_MP4VIDEO || Type==PT_MPEG2VIDEO)\
            {\
                u32AlignWidth = VB_ALIGN(Width,H264_WIDTH_ALIGN);\
                u32AlignHeight= VB_ALIGN(Height,H264_HEIGHT_ALIGN);\
                size = ( (u32AlignWidth * u32AlignHeight) * 3) >> 1;\
            }\
            else if (Type==PT_H265)\
            {\
                u32AlignWidth = VB_ALIGN(Width,H265_WIDTH_ALIGN);\
                u32AlignHeight= VB_ALIGN(Height,H265_HEIGHT_ALIGN);\
                size = ( (u32AlignWidth * u32AlignHeight) * 3) >> 1;\
            }\
            else\
            {\
                u32AlignWidth = VB_ALIGN(Width,JPEG_WIDTH_ALIGN);\
                u32AlignHeight= VB_ALIGN(Height,16);\
                size = (u32AlignWidth * u32AlignHeight * 3) >> 1;\
            }\
    }while(0)

#define VB_PMV_BLK_SIZE(Width, Height, enType, size)\
    do{\
            unsigned int WidthInMb, HeightInMb;\
            unsigned int ColMbSize;\
            WidthInMb  = (enType == PT_H265) ? ((Width   + 63) >> 4) : ((Width   + 15) >> 4);\
            HeightInMb = (enType == PT_H265) ? ((Height  + 63) >> 4) : ((Height  + 15) >> 4);\
            ColMbSize  = (enType == PT_H265) ? (4*4) : (16*4);\
            size       = VB_ALIGN((ColMbSize * WidthInMb * HeightInMb), 128);\
    }while(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEFINES_H__ */

