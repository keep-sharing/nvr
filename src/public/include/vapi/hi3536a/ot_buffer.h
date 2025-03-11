/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_BUFFER_H
#define OT_BUFFER_H

#include "ot_math.h"
#include "ot_type.h"
#include "ot_common.h"
#include "ot_common_video.h"
#include "securec.h"

#ifdef __KERNEL__
#include "ot_osal.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define OT_MAXI_NUM_LIMIT 30000

#define OT_SEG_RATIO_8BIT_LUMA 1600
#define OT_SEG_RATIO_8BIT_CHROMA 2000
#define OT_SEG_WIDTH_BIT_ALIGN 128

typedef struct {
    td_u32            width;
    td_u32            height;
    td_u32            align;
    ot_data_bit_width bit_width;
    ot_pixel_format   pixel_format;
    ot_compress_mode  compress_mode;
} ot_pic_buf_attr;

typedef struct {
    td_bool           share_buf_en;
    td_u32            frame_buf_ratio;
    ot_pic_buf_attr   pic_buf_attr;
} ot_venc_buf_attr;

__inline static td_u32 ot_common_get_bit_width(ot_data_bit_width bit_width)
{
    td_u32 res_bit_width;

    switch (bit_width) {
        case OT_DATA_BIT_WIDTH_8: {
            res_bit_width = 8;
            break;
        }
        case OT_DATA_BIT_WIDTH_10: {
            res_bit_width = 10;
            break;
        }
        case OT_DATA_BIT_WIDTH_12: {
            res_bit_width = 12;
            break;
        }
        case OT_DATA_BIT_WIDTH_14: {
            res_bit_width = 14;
            break;
        }
        case OT_DATA_BIT_WIDTH_16: {
            res_bit_width = 16;
            break;
        }
        default: {
            res_bit_width = 0;
            break;
        }
    }
    return res_bit_width;
}

__inline static td_void ot_copy_vb_calc_cfg(ot_vb_calc_cfg *dst_cfg, const ot_vb_calc_cfg *src_cfg)
{
    if (dst_cfg == TD_NULL || src_cfg == TD_NULL) {
        return;
    }

    dst_cfg->vb_size     = src_cfg->vb_size;
    dst_cfg->head_stride = src_cfg->head_stride;
    dst_cfg->head_size   = src_cfg->head_size;
    dst_cfg->head_y_size = src_cfg->head_y_size;
    dst_cfg->main_stride = src_cfg->main_stride;
    dst_cfg->main_size   = src_cfg->main_size;
    dst_cfg->main_y_size = src_cfg->main_y_size;

    return;
}

__inline static td_void ot_common_get_compress_none_pic_buf_cfg(const ot_pic_buf_attr *buf_attr,
    td_s32 bit_width, ot_vb_calc_cfg *calc_cfg)
{
    td_u32 align_height;
    ot_vb_calc_cfg cfg          = {0};

    if (buf_attr == TD_NULL || calc_cfg == TD_NULL) {
        return;
    }

    if ((buf_attr->width > OT_MAXI_NUM_LIMIT) || (buf_attr->height > OT_MAXI_NUM_LIMIT)) {
        (td_void)memset_s(calc_cfg, sizeof(*calc_cfg), 0, sizeof(*calc_cfg));
        return;
    }

    align_height = OT_ALIGN_UP(buf_attr->height, 2);
    cfg.main_stride = OT_ALIGN_UP((buf_attr->width * (td_u32)bit_width + 7) >> 3, buf_attr->align);
    cfg.main_y_size = cfg.main_stride * align_height;

    if ((buf_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
        cfg.main_size = ((cfg.main_stride * align_height) * 3) >> 1;
    } else if ((buf_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422)) {
        cfg.main_size = cfg.main_stride * align_height * 2;
    } else if ((buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_400) ||
        (buf_attr->pixel_format == OT_PIXEL_FORMAT_S16C1)) {
        cfg.main_size = cfg.main_stride * align_height;
    } else {
        cfg.main_size = cfg.main_stride * align_height * 3;
    }

    cfg.vb_size = cfg.main_size;

    ot_copy_vb_calc_cfg(calc_cfg, &cfg);
    return;
}

__inline static td_void ot_common_get_compact_pic_buf_cfg(const ot_pic_buf_attr *buf_attr,
    td_u32 bit_width, ot_vb_calc_cfg *cfg)
{
    td_u32 c_size;
    td_u32 align_c_height;
    td_u32 align_height;
    const td_u32 cmp_ratio_luma   = OT_SEG_RATIO_8BIT_LUMA;
    const td_u32 cmp_ratio_chroma = OT_SEG_RATIO_8BIT_CHROMA;

    if (buf_attr == TD_NULL || cfg == TD_NULL) {
        return;
    }

    if ((buf_attr->width > OT_MAXI_NUM_LIMIT) || (buf_attr->height > OT_MAXI_NUM_LIMIT)) {
        (td_void)memset_s(cfg, sizeof(*cfg), 0, sizeof(*cfg));
        return;
    }

    align_height = OT_ALIGN_UP(buf_attr->height, 2);
    cfg->main_stride = OT_ALIGN_UP(buf_attr->width * bit_width, OT_SEG_WIDTH_BIT_ALIGN);

    if ((buf_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
        align_c_height = align_height / 2;
    } else if ((buf_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422)) {
        align_c_height = align_height;
    } else if (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_400) {
        align_c_height = 0;
    } else {
        align_c_height = align_height * 2;
    }

#ifdef __KERNEL__
    cfg->main_y_size = osal_div64_u64(cfg->main_stride * align_height * 1000ULL, cmp_ratio_luma * bit_width);
    c_size = osal_div64_u64(cfg->main_stride * align_c_height * 1000ULL, cmp_ratio_chroma * bit_width);
#else
    cfg->main_y_size = cfg->main_stride * align_height * 1000ULL / (cmp_ratio_luma * bit_width);
    c_size = cfg->main_stride * align_c_height * 1000ULL / (cmp_ratio_chroma * bit_width);
#endif

    c_size = OT_ALIGN_UP(c_size, OT_DEFAULT_ALIGN);
    cfg->main_y_size = OT_ALIGN_UP(cfg->main_y_size, OT_DEFAULT_ALIGN);

    cfg->main_stride = cfg->main_stride >> 3;
    cfg->head_y_size = cfg->head_stride * align_height;
    cfg->head_size = cfg->head_stride * (align_height + align_c_height);
    cfg->main_size = cfg->main_y_size + c_size;

    return;
}

__inline static td_void ot_common_get_compact_none_pic_buf_cfg(const ot_pic_buf_attr *buf_attr,
    ot_vb_calc_cfg *cfg)
{
    td_u32 align_height;

    if (buf_attr == TD_NULL || cfg == TD_NULL) {
        return;
    }

    if ((buf_attr->width > OT_MAXI_NUM_LIMIT) || (buf_attr->height > OT_MAXI_NUM_LIMIT)) {
        (td_void)memset_s(cfg, sizeof(*cfg), 0, sizeof(*cfg));
        return;
    }

    align_height = OT_ALIGN_UP(buf_attr->height, 2);
    cfg->main_stride = OT_ALIGN_UP(buf_attr->width, buf_attr->align);
    cfg->head_y_size = cfg->head_stride * align_height;
    cfg->main_y_size = cfg->main_stride * align_height;

    if ((buf_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420)) {
        cfg->head_size = (cfg->head_stride * align_height * 3) >> 1;
        cfg->main_size = (cfg->main_stride * align_height * 3) >> 1;
    } else if ((buf_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422)) {
        cfg->head_size = cfg->head_stride * align_height * 2;
        cfg->main_size = cfg->main_stride * align_height * 2;
    } else if (buf_attr->pixel_format == OT_PIXEL_FORMAT_YUV_400) {
        cfg->head_size = cfg->head_stride * align_height;
        cfg->main_size = cfg->main_stride * align_height;
    } else {
        cfg->head_size = cfg->head_stride * align_height * 3;
        cfg->main_size = cfg->main_stride * align_height * 3;
    }

    return;
}

__inline static td_void ot_common_get_compress_pic_buf_cfg(const ot_pic_buf_attr *buf_attr,
    td_u32 bit_width, ot_vb_calc_cfg *calc_cfg)
{
    ot_vb_calc_cfg cfg          = {0};

    if (buf_attr == TD_NULL || calc_cfg == TD_NULL) {
        return;
    }

    if ((buf_attr->width > OT_MAXI_NUM_LIMIT) || (buf_attr->height > OT_MAXI_NUM_LIMIT)) {
        (td_void)memset_s(calc_cfg, sizeof(*calc_cfg), 0, sizeof(*calc_cfg));
        return;
    }

    if (buf_attr->width <= 4096) {
        cfg.head_stride = 16;
    } else if (buf_attr->width <= 8192) {
        cfg.head_stride = 32;
    } else {
        cfg.head_stride = 64;
    }

    if (bit_width == 8) {
        if (buf_attr->compress_mode == OT_COMPRESS_MODE_SEG_COMPACT && buf_attr->width > 256) { /* 256 compact width */
            ot_common_get_compact_pic_buf_cfg(buf_attr, bit_width, &cfg);
        } else {
            ot_common_get_compact_none_pic_buf_cfg(buf_attr, &cfg);
        }
    }

    cfg.head_size = OT_ALIGN_UP(cfg.head_size, buf_attr->align);

    cfg.vb_size = cfg.head_size + cfg.main_size;

    ot_copy_vb_calc_cfg(calc_cfg, &cfg);
    return;
}

__inline static td_void ot_common_get_pic_buf_cfg(ot_pic_buf_attr *buf_attr, ot_vb_calc_cfg *calc_cfg)
{
    td_u32 bit_width;

    if (buf_attr == TD_NULL || calc_cfg == TD_NULL) {
        return;
    }

    if ((buf_attr->width > OT_MAXI_NUM_LIMIT) || (buf_attr->height > OT_MAXI_NUM_LIMIT)) {
        (td_void)memset_s(calc_cfg, sizeof(*calc_cfg), 0, sizeof(*calc_cfg));
        return;
    }

    /* align: 0 is automatic mode, alignment size following system. Non-0 for specified alignment size */
    if (buf_attr->align == 0) {
        buf_attr->align = OT_DEFAULT_ALIGN;
    } else if (buf_attr->align > OT_MAX_ALIGN) {
        buf_attr->align = OT_MAX_ALIGN;
    } else {
        buf_attr->align = (OT_ALIGN_UP(buf_attr->align, OT_DEFAULT_ALIGN));
    }

    bit_width = ot_common_get_bit_width(buf_attr->bit_width);

    if (buf_attr->compress_mode == OT_COMPRESS_MODE_NONE) {
        ot_common_get_compress_none_pic_buf_cfg(buf_attr, bit_width, calc_cfg);
    } else {
        ot_common_get_compress_pic_buf_cfg(buf_attr, bit_width, calc_cfg);
    }

    return;
}

__inline static td_u32 ot_common_get_pic_buf_size(ot_pic_buf_attr *buf_attr)
{
    ot_vb_calc_cfg calc_cfg;

    if (buf_attr == TD_NULL) {
        return 0;
    }
    ot_common_get_pic_buf_cfg(buf_attr, &calc_cfg);

    return calc_cfg.vb_size;
}

__inline static td_bool ot_vdec_check_pic_size(ot_payload_type type, td_u32 width, td_u32 height)
{
    if ((type == OT_PT_H265) && (width <= OT_VDH_H265D_MAX_WIDTH) && (height <= OT_VDH_H265D_MAX_HEIGHT)) {
        return TD_TRUE;
    }
    if ((type == OT_PT_H264) && (width <= OT_VDH_H264D_MAX_WIDTH) && (height <= OT_VDH_H264D_MAX_HEIGHT)) {
        return TD_TRUE;
    }
    if ((type == OT_PT_MP4VIDEO) && (width <= OT_VDH_MPEG4_MAX_WIDTH) && (height <= OT_VDH_MPEG4_MAX_HEIGHT)) {
        return TD_TRUE;
    }
    if ((type == OT_PT_JPEG || type == OT_PT_MJPEG) &&
        (width <= OT_JPEGD_MAX_WIDTH) && (height <= OT_JPEGD_MAX_HEIGHT)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

__inline static td_u32 ot_vdec_get_pic_buf_size(ot_payload_type type,
    const ot_pic_buf_attr *buf_attr)
{
    td_u32 align_width, align_height;
    td_u32 header_size;
    td_u32 header_stride;
    td_u32 extra_size = 0;
    td_u32 size;

    if (buf_attr == TD_NULL) {
        return 0;
    }

    if (ot_vdec_check_pic_size(type, buf_attr->width, buf_attr->height) == TD_FALSE) {
        return 0;
    }

    if ((type == OT_PT_H264) || (type == OT_PT_MP4VIDEO)) {
        align_width = OT_ALIGN_UP(buf_attr->width, OT_H264D_ALIGN_W);
        align_height = OT_ALIGN_UP(buf_attr->height, OT_H264D_ALIGN_H);
        if (buf_attr->width <= 8192) {
            header_stride = OT_ALIGN_UP(align_width, 2048) >> 6;
        } else {
            header_stride = OT_ALIGN_UP(align_width, 8192) >> 6;
        }
        header_size = header_stride * (OT_ALIGN_UP(align_height, OT_H264D_ALIGN_H) >> 2);
        size = ((header_size + align_width * align_height) * 3) >> 1;
    } else if (type == OT_PT_H265) {
        align_width = OT_ALIGN_UP(buf_attr->width, OT_H265D_ALIGN_W);
        align_height = OT_ALIGN_UP(buf_attr->height, OT_H265D_ALIGN_H);
        if (buf_attr->width <= 8192) {
            header_stride = OT_ALIGN_UP(align_width, 2048) >> 6;
        } else {
            header_stride = OT_ALIGN_UP(align_width, 8192) >> 6;
        }
        header_size = header_stride * (OT_ALIGN_UP(align_height, OT_H265D_ALIGN_H) >> 2);

        if (buf_attr->bit_width == OT_DATA_BIT_WIDTH_10) {
            extra_size = OT_ALIGN_UP((align_width * 2) >> 3, 16) * OT_ALIGN_UP(align_height, 32);
        }

        size = ((header_size + align_width * align_height + extra_size) * 3) >> 1;
    } else if ((type == OT_PT_JPEG) || (type == OT_PT_MJPEG)) {
        /* for OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 */
        align_width = OT_ALIGN_UP(buf_attr->width, OT_JPEGD_ALIGN_W);
        align_height = OT_ALIGN_UP(buf_attr->height, OT_JPEGD_ALIGN_H);
        if (buf_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) {
            size = align_width * align_height * 2; /* 2:Y+1/2U+1/2V */
        } else {
            size = (align_width * align_height * 3) >> 1; /* 3:Y+1/4U+1/4V */
        }
    } else {
        size = 0;
    }

    return size;
}

__inline static td_u32 ot_vdec_get_tmv_buf_size(ot_payload_type type, td_u32 width, td_u32 height)
{
    td_u32 width_in_mb, height_in_mb;
    td_u32 col_mb_size;
    td_u32 size;

    if (ot_vdec_check_pic_size(type, width, height) == TD_FALSE) {
        return 0;
    }

    if ((type == OT_PT_H264) || (type == OT_PT_MP4VIDEO)) {
        width_in_mb = OT_ALIGN_UP(width, 16) >> 4;
        height_in_mb = OT_ALIGN_UP(height, 16) >> 4;
        col_mb_size = 16 * 4;
        size = OT_ALIGN_UP((col_mb_size * width_in_mb * height_in_mb), 128);
    } else if (type == OT_PT_H265) {
        width_in_mb = OT_ALIGN_UP(width, 64) >> 4;
        height_in_mb = OT_ALIGN_UP(height, 64) >> 4;
        col_mb_size = 4 * 4;
        size = OT_ALIGN_UP((col_mb_size * width_in_mb * height_in_mb), 128);
    } else {
        size = 0;
    }

    return size;
}

__inline static td_u32 ot_venc_get_pic_info_buf_size(ot_payload_type type, const ot_venc_buf_attr *attr)
{
    td_u32 pic_info_size;
    td_u32 tmv_size, pme_stride, pme_size, pme_info_size;
    td_u32 width, height;
    td_bool share_buf;

    if (type != OT_PT_H264 && type != OT_PT_H265) {
        return 0;
    }

    if (attr == NULL) {
        return 0;
    }

    if (attr->share_buf_en != TD_FALSE && attr->share_buf_en != TD_TRUE) {
        return 0;
    }

    if (type == OT_PT_H264 &&
        (attr->pic_buf_attr.width > OT_VENC_H264_MAX_WIDTH || attr->pic_buf_attr.height > OT_VENC_H264_MAX_HEIGHT)) {
        return 0;
    }

    if (type == OT_PT_H265 &&
        (attr->pic_buf_attr.width > OT_VENC_H265_MAX_WIDTH || attr->pic_buf_attr.height > OT_VENC_H265_MAX_HEIGHT)) {
        return 0;
    }

    width = attr->pic_buf_attr.width;
    height = attr->pic_buf_attr.height;
    share_buf = attr->share_buf_en;

    if (type == OT_PT_H264) {
        tmv_size = 0;
        pme_stride = OT_ALIGN_UP(width, 64) / 64;
        pme_size = pme_stride * (OT_ALIGN_UP(height, 16) / 16) * 64;
        pme_info_size = (OT_ALIGN_UP(width, 1024) / 16) * (OT_ALIGN_UP(height, 16) / 16) * 2;
    } else {
        tmv_size = ((OT_ALIGN_UP(width, 32) / 32) * (OT_ALIGN_UP(height, 32) / 32) + 1) / 2 * 2 * 16;
        pme_stride = OT_ALIGN_UP(width, 32) / 32;
        pme_size = pme_stride * (OT_ALIGN_UP(height, 32) / 32) * 64;
        pme_info_size = (OT_ALIGN_UP(width, 512) / 32) * (OT_ALIGN_UP(height, 32) / 32) * 2;
    }

    if (share_buf == TD_FALSE) {
        pic_info_size = tmv_size + pme_size + pme_info_size;
    } else {
        pic_info_size = tmv_size + pme_info_size;
    }

    return pic_info_size;
}

__inline static td_bool ot_venc_check_pic_buf_size(ot_payload_type type, ot_venc_buf_attr *attr)
{
    if (type != OT_PT_H264 && type != OT_PT_H265) {
        return TD_FALSE;
    }

    if (attr == NULL) {
        return TD_FALSE;
    }

    if (attr->share_buf_en != TD_FALSE && attr->share_buf_en != TD_TRUE) {
        return TD_FALSE;
    }

    if (attr->frame_buf_ratio < 70 || attr->frame_buf_ratio > 100) {
        return TD_FALSE;
    }

    if (type == OT_PT_H264 &&
        (attr->pic_buf_attr.width > OT_VENC_H264_MAX_WIDTH || attr->pic_buf_attr.height > OT_VENC_H264_MAX_HEIGHT)) {
        return TD_FALSE;
    }

    if (type == OT_PT_H265 &&
        (attr->pic_buf_attr.width > OT_VENC_H265_MAX_WIDTH || attr->pic_buf_attr.height > OT_VENC_H265_MAX_HEIGHT)) {
        return TD_FALSE;
    }

    if (attr->pic_buf_attr.bit_width != OT_DATA_BIT_WIDTH_8) {
        return TD_FALSE;
    }

    return TD_TRUE;
}

__inline static td_u32 ot_venc_get_h264_pic_buf_size(ot_payload_type type, ot_venc_buf_attr *attr)
{
    td_u32 y_header_stride, c_header_stride;
    td_u32 y_header_size, c_header_size;
    td_u32 y_size, c_size;
    td_u32 blk_height_y, blk_height_c;
    td_u32 pme_stride, pme_size, pme_ext_size;
    td_u32 width, height;
    td_bool share_buf;
    td_u32 frame_buf_ratio;
    td_u32 pic_size;

    width = attr->pic_buf_attr.width;
    height = attr->pic_buf_attr.height;
    share_buf = attr->share_buf_en;
    frame_buf_ratio = attr->frame_buf_ratio;

    if (width <= 2048) {
        y_header_stride = 128;
    } else if (width <= 4096) {
        y_header_stride = 256;
    } else if (width <= 6144) {
        y_header_stride = 384;
    } else {
        y_header_stride = 512;
    }

    c_header_stride = y_header_stride / 2;

    y_header_size = y_header_stride * (OT_ALIGN_UP(height, 16) / 16);
    c_header_size = c_header_stride * (OT_ALIGN_UP(height, 16) / 16);

    if (share_buf == TD_FALSE) {
        blk_height_y = 0;
        blk_height_c = 0;
    } else {
        blk_height_y = ((16 * 4 + 16 + 16 + 15) / 16) * 16;
        blk_height_c = blk_height_y / 2;
    }

    y_size = OT_ALIGN_UP(width * (height + blk_height_y) * frame_buf_ratio / 100, 256);
    c_size = OT_ALIGN_UP(width * (height / 2 + blk_height_c) * frame_buf_ratio / 100, 256);

    pme_stride = OT_ALIGN_UP(width, 64) / 64;
    pme_size = pme_stride * (OT_ALIGN_UP(height, 16) / 16) * 64;
    pme_ext_size = OT_ALIGN_UP(width, 64);

    if (share_buf == TD_FALSE) {
        pic_size = y_header_size + c_header_size + y_size + c_size;
    } else {
        pme_size += pme_ext_size;
        pic_size = (y_header_size + c_header_size) * 2 + y_size + c_size + pme_size;
    }

    return pic_size;
}

__inline static td_u32 ot_venc_get_h265_pic_buf_size(ot_payload_type type, ot_venc_buf_attr *attr)
{
    td_u32 y_header_stride, c_header_stride;
    td_u32 y_header_size, c_header_size;
    td_u32 y_size, c_size;
    td_u32 blk_height_y, blk_height_c;
    td_u32 pme_stride, pme_size, pme_ext_size;
    td_u32 width, height;
    td_bool share_buf;
    td_u32 frame_buf_ratio;
    td_u32 pic_size;

    width = attr->pic_buf_attr.width;
    height = attr->pic_buf_attr.height;
    share_buf = attr->share_buf_en;
    frame_buf_ratio = attr->frame_buf_ratio;

    if (width <= 2048) { // 2048: width
        y_header_stride = 256;
    } else if (width <= 4096) { // 4096: width
        y_header_stride = 512;
    } else if (width <= 6144) { // 6144: width
        y_header_stride = 768;
    } else {
        y_header_stride = 1024;
    }

    c_header_stride = y_header_stride / 2;

    y_header_size = y_header_stride * (OT_ALIGN_UP(height, 32) / 32);
    c_header_size = c_header_stride * (OT_ALIGN_UP(height, 32) / 32);

    if (share_buf == TD_FALSE) {
        blk_height_y = 0;
        blk_height_c = 0;
    } else {
        blk_height_y = ((20 * 4 + 16 + 32 + 15) / 16) * 16;
        blk_height_c = blk_height_y / 2;
    }

    y_size = OT_ALIGN_UP(width * (height + blk_height_y) * frame_buf_ratio / 100, 256);
    c_size = OT_ALIGN_UP(width * (height / 2 + blk_height_c) * frame_buf_ratio / 100, 256);

    pme_stride = OT_ALIGN_UP(width, 32) / 32;
    pme_size = pme_stride * (OT_ALIGN_UP(height, 32) / 32) * 64;
    pme_ext_size = OT_ALIGN_UP(width, 32) * 2;

    if (share_buf == TD_FALSE) {
        pic_size = y_header_size + c_header_size + y_size + c_size;
    } else {
        pme_size += pme_ext_size;
        pic_size = (y_header_size + c_header_size) * 2 + y_size + c_size + pme_size;
    }

    return pic_size;
}

__inline static td_u32 ot_venc_get_pic_buf_size(ot_payload_type type, ot_venc_buf_attr *attr)
{
    if (ot_venc_check_pic_buf_size(type, attr) == TD_FALSE) {
        return 0;
    }

    if (type == OT_PT_H264) {
        return ot_venc_get_h264_pic_buf_size(type, attr);
    } else {
        return ot_venc_get_h265_pic_buf_size(type, attr);
    }
}

__inline static td_u32 ot_venc_get_qpmap_stride(td_u32 width)
{
    if (width > OT_MAXI_NUM_LIMIT) {
        return 0;
    }

    return OT_ALIGN_UP(width, 256) / 16;
}

__inline static td_u32 ot_venc_get_qpmap_size(ot_payload_type type, td_u32 width, td_u32 height)
{
    td_u32 stride, align_height;

    if ((width > OT_MAXI_NUM_LIMIT) || (height > OT_MAXI_NUM_LIMIT)) {
        return 0;
    }

    stride = ot_venc_get_qpmap_stride(width);
    if (type == OT_PT_H265) {
        align_height = OT_ALIGN_UP(height, 32) / 16;
    } else if (type == OT_PT_H264) {
        align_height = OT_ALIGN_UP(height, 16) / 16;
    } else {
        align_height = 0;
    }

    return stride * align_height;
}

__inline static td_u32 ot_venc_get_skip_weight_stride(ot_payload_type type, td_u32 width)
{
    td_u32 stride;

    if (width > OT_MAXI_NUM_LIMIT) {
        return 0;
    }

    if (type == OT_PT_H265) {
        stride = OT_ALIGN_UP(width, 1024) / 64;
    } else if (type == OT_PT_H264) {
        stride = OT_ALIGN_UP(width, 512) / 32;
    } else {
        stride = 0;
    }

    return stride;
}

__inline static td_u32 ot_venc_get_skip_weight_size(ot_payload_type type, td_u32 width, td_u32 height)
{
    td_u32 stride, align_height;

    if ((width > OT_MAXI_NUM_LIMIT) || (height > OT_MAXI_NUM_LIMIT)) {
        return 0;
    }

    stride = ot_venc_get_skip_weight_stride(type, width);

    if (type == OT_PT_H265) {
        align_height = OT_ALIGN_UP(height, 32) / 32;
    } else if (type == OT_PT_H264) {
        align_height = OT_ALIGN_UP(height, 16) / 16;
    } else {
        align_height = 0;
    }

    return stride * align_height;
}

__inline static td_u32 ot_venc_get_roimap_stride(ot_payload_type type, td_u32 width)
{
    td_u32 stride;

    if (width > OT_MAXI_NUM_LIMIT) {
        return 0;
    }

    if (type == OT_PT_JPEG || type == OT_PT_MJPEG) {
        stride = OT_ALIGN_UP(width, 1024) / 64; // 1024 64: hardware align requirement
    } else {
        stride = 0;
    }

    return stride;
}

__inline static td_u32 ot_venc_get_roimap_size(ot_payload_type type, td_u32 width, td_u32 height)
{
    td_u32 stride, align_height;

    if ((width > OT_MAXI_NUM_LIMIT) || (height > OT_MAXI_NUM_LIMIT)) {
        return 0;
    }

    stride = ot_venc_get_roimap_stride(type, width);

    if (type == OT_PT_JPEG || type == OT_PT_MJPEG) {
        align_height = OT_ALIGN_UP(height, 16) / 16; // 16: hardware align requirement
    } else {
        align_height = 0;
    }

    return stride * align_height;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OT_BUFFER_H */

