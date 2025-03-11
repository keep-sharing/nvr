/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __OT_COMMON_VDA_H__
#define __OT_COMMON_VDA_H__

#include "ot_type.h"
#include "ot_debug.h"
#include "ot_common_video.h"
#include "ot_common.h"
#include "ot_errno.h"
#include "ot_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OT_VDA_OD_RGN_NUM_MAX 4

typedef enum {
    OT_VDA_REF_MODE_DYNAMIC = 0, /* reference picture dynamic */
    OT_VDA_REF_MODE_STATIC, /* reference picture static */
    OT_VDA_REF_MODE_USER, /* reference picture user */
    OT_VDA_REF_MODE_BUTT
} ot_vda_ref_mode;

typedef enum {
    OT_VDA_ALG_BG = 0, /* base on background picture */
    OT_VDA_ALG_REF, /* base on reference picture */
    OT_VDA_ALG_BUTT
} ot_vda_alg;

typedef enum {
    OT_VDA_MB_8PIXEL, /* 8*8 */
    OT_VDA_MB_16PIXEL, /* 16*16 */
    OT_VDA_MB_BUTT
} ot_vda_mb_size;

typedef enum {
    OT_VDA_MB_SAD_8BIT = 0, /* SAD precision 8bits */
    OT_VDA_MB_SAD_16BIT, /* SAD precision 16bits */
    OT_VDA_MB_SAD_BUTT
} ot_vda_mb_sad_bits;

typedef enum {
    OT_VDA_WORK_MODE_MD = 0, /* motion detection */
    OT_VDA_WORK_MODE_OD, /* Occlusion detection */
    OT_VDA_WORK_MODE_BUTT
} ot_vda_work_mode;

typedef struct {
    td_u16 upper_left_x;
    td_u16 upper_left_y;
    td_u16 lower_right_x;
    td_u16 lower_right_y;
} ot_vda_rgn;

typedef struct {
    /* static attribute */
    ot_rect rect; /*
                   * region rect
                   * X:[0,OT_VDA_MAX_WIDTH),align:16
                   * Y:[0,OT_VDA_MAX_HEIGHT)
                   * W:[16,OT_VDA_MAX_WIDTH],align:16
                   * H:[16,OT_VDA_MAX_HEIGHT],align:16
                   * X+W <= channel wight
                   * Y+H <= channel height
                   */

    /* dynamic attribute */
    td_u32 sad_threshold; /* SAD threshold,range:[0,4080] */
    td_u32 area_threshold; /* alarm area threshold,range:[0,100] */
    td_u32 occlusion_cnt_threshold; /* alarm frame count threshold,range:[1,256] */
    td_u32 uncover_cnt_threshold; /* the max uncover count,range:[0,256] */
} ot_vda_od_rgn_attr;

typedef struct {
    /* static attribute */
    ot_vda_alg         vda_alg; /* arithmetic */
    ot_vda_mb_size     mb_size; /* MB size */
    ot_vda_mb_sad_bits mb_sad_bits; /* MB SAD size */
    ot_vda_ref_mode    ref_mode; /* reference picture mode */
    td_u32             md_buf_num; /* result buffer number,range:[1,16] */

    /* dynamic attribute */
    td_u32             vda_interval; /* VDA interval,range:[0,256] */

    td_u32             bg_up_src_wgt;  /*
                                        * the source picture weight,
                                        * back ground update total weight 256,
                                        * range:[1,255],recommendatory value:128
                                        */

    td_u32          sad_threshold; /* SAD threshold,range:[0,4080],recommendatory value:100 */
    td_u32          obj_num_max;   /* max OBJ number,range:[1,128] */
} ot_vda_md_attr;

typedef struct {
    td_u32             rgn_num; /* region number */
    ot_vda_od_rgn_attr od_rgn_attr[OT_VDA_OD_RGN_NUM_MAX]; /* region attribute */
    /* static attribute */
    ot_vda_alg         vda_alg;   /* arithmetic */
    ot_vda_mb_size     mb_size;   /* MB size */
    ot_vda_mb_sad_bits mb_sad_bits; /* MB SAD size */
    ot_vda_ref_mode    ref_mode; /* reference picture mode */
    /* dynamic attribute */
    td_u32             vda_interval;  /* VDA interval,[0,256] */
    td_u32             bg_up_src_wgt; /* back ground update total weight 256,the source picture weight,[1,255] */
} ot_vda_od_attr;

typedef union {
    ot_vda_md_attr md_attr;  /* MD attribute. AUTO:ot_vda_work_mode:OT_VDA_WORK_MODE_MD; */
    ot_vda_od_attr od_attr;  /* OD attribute. AUTO:ot_vda_work_mode:OT_VDA_WORK_MODE_OD; */
} ot_vda_work_mode_attr;

typedef struct {
    ot_vda_work_mode work_mode; /* work mode */
    ot_vda_work_mode_attr attr; /* work mode attribute */
    td_u32 width;               /* the width of CHNL,[OT_VDA_MIN_WIDTH,OT_VDA_MAX_WIDTH] */
    td_u32 height;              /* the height of CHNL,[OT_VDA_MIN_HEIGHT,OT_VDA_MAX_HEIGHT] */
} ot_vda_chn_attr;


typedef struct {
    td_u32             stride;  /* stride */
    ot_vda_mb_sad_bits mb_sad_bits; /* the MB SAD size */
    td_void ATTRIBUTE  *addr; /* address */
} ot_vda_mb_sad_data;

typedef struct {
    td_u32 obj_num;  /* OBJ number */
    ot_vda_rgn  *addr; /* OBJ data address */

    td_u32 idx_of_max_obj; /* index of max OBJ */
    td_u32 size_of_max_obj; /* size of max OBJ */
    td_u32 size_of_total_obj; /* total size of all OBJ */
} ot_vda_obj_data;


typedef struct {
    td_bool            mb_sad_valid; /* SAD data is valid */
    ot_vda_mb_sad_data mb_sad_data; /* MB SAD data */

    td_bool            obj_valid; /* OBJ data is valid */
    ot_vda_obj_data    obj_data;  /* OBJ data */

    td_bool            pixel_num_valid; /* alarm pixel number data is valid */
    td_u32             alarm_pixel_cnt; /* motion pix of picture */
} ot_vda_md_data;


typedef struct {
    td_u32  rgn_num; /* region number */
    td_bool rgn_alarm_en[OT_VDA_OD_RGN_NUM_MAX]; /* TD_TRUE:alarm */
} ot_vda_od_data;

typedef union {
    ot_vda_md_data md_data; /* MD data. AUTO:ot_vda_work_mode:OT_VDA_WORK_MODE_MD; */
    ot_vda_od_data od_data; /* OD data. AUTO:ot_vda_work_mode:OT_VDA_WORK_MODE_OD; */
} ot_vda_data_type;

typedef struct {
    ot_vda_work_mode work_mode; /* work mode */
    ot_vda_data_type data;  /* VDA data type */
    ot_vda_mb_size   mb_size;  /* MB size */
    td_u32           mb_width;  /* VDA channel width in MB */
    td_u32           mb_height; /* VDA channel height in MB */
    td_u64           pts;  /* time */
} ot_vda_data;

typedef struct {
    td_bool recv_pic_en; /* start receive picture */
    td_u32  left_pic; /* left picture */
    td_u32  left_data; /* left data */
} ot_vda_chn_status;

#define OT_ERR_VDA_INVALID_DEV_ID    OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_INVALID_DEV_ID)
#define OT_ERR_VDA_INVALID_CHN_ID    OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_INVALID_CHN_ID)
#define OT_ERR_VDA_ILLEGAL_PARAM     OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_ILLEGAL_PARAM)
#define OT_ERR_VDA_EXIST             OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_EXIST)
#define OT_ERR_VDA_UNEXIST           OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_UNEXIST)
#define OT_ERR_VDA_NULL_PTR          OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_NULL_PTR)
#define OT_ERR_VDA_NOT_CFG           OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_CFG)
#define OT_ERR_VDA_NOT_SUPPORT       OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_SUPPORT)
#define OT_ERR_VDA_NOT_PERM          OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_PERM)
#define OT_ERR_VDA_NO_MEM            OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_NO_MEM)
#define OT_ERR_VDA_NO_BUF            OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_NO_BUF)
#define OT_ERR_VDA_BUF_EMPTY         OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_BUF_EMPTY)
#define OT_ERR_VDA_BUF_FULL          OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_BUF_FULL)
#define OT_ERR_VDA_BAD_ADDR          OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_BAD_ADDR)
#define OT_ERR_VDA_BUSY              OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_BUSY)
#define OT_ERR_VDA_NOT_READY         OT_DEFINE_ERR(OT_ID_VDA, OT_ERR_LEVEL_ERROR, OT_ERR_NOT_READY)

#ifdef __cplusplus
}
#endif

#endif
