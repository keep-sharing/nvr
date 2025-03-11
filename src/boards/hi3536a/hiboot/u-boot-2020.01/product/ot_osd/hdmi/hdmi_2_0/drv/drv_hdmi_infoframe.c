// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "drv_hdmi_infoframe.h"
#include "boot_hdmi_intf.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_common.h"
#include "hdmi_hal.h"
#include "securec.h"

#define INFOFRAME_CHECKSUM 256
#define INFOFRAME_BUFFER_LEN 32
#define INFOFRAME_CHECKSUM_OFFSET 3
#define HDMI_AVI_VERSION3 3
#define HDMI_AVI_VERSION4 4
#define HDMI_AVI_Y2_MASK (0x1 << 2)
#define AVI_FRAME_COLOSR_SPACE_MASK 0x3
#define AVI_FRAME_SCAN_INFOMATION_MASK 0x3
#define AVI_FRAME_NUP_SCAN_MASK 0x3
#define AVI_FRAME_CONTENT_TYPE_MASK 0x3
#define AVI_FRAME_BAR_MODE_MASK 0xFF
#define AVI_INFORFRAME_BAR_MODE_OFFSET 8
#define AVI_INFORFRAME_LENGTH 13

hdmi_video_timing drv_hdmi_video_timing_lookup(hdmi_video_code vic, hdmi_picture_aspect aspect)
{
    return drv_hdmi_video_timing_get(vic, aspect);
}

hdmi_video_timing drv_hdmi_vsif_video_timing_lookup(hdmi_vsif_vic vic, hdmi_picture_aspect aspect)
{
    return drv_hdmi_vsif_video_timing_get(vic, aspect);
}

static td_void hdmi_infoframe_checksum(td_u8 *buffer, td_u32 size)
{
    td_u8 *ptr = TD_NULL;
    td_u8 csum = 0;
    td_u32 i;

    if (buffer == TD_NULL) {
        return;
    }
    if (size < INFOFRAME_CHECKSUM_OFFSET) {
        return;
    }
    ptr = buffer;

    /* compute checksum */
    for (i = 0; i < size; i++) {
        csum += ptr[i];
    }
    if (csum > 0) {
        ptr[INFOFRAME_CHECKSUM_OFFSET] = INFOFRAME_CHECKSUM - csum;
    } else {
        ptr[INFOFRAME_CHECKSUM_OFFSET] = 0;
    }
}

static hdmi_video_code avi_header_pack(hdmi_avi_infoframe *frame, td_u8 *buffer, hdmi_3d_mode _3d_mode)
{
    td_u8 *ptr = TD_NULL;
    td_bool _3d_enable;
    hdmi_video_code video_code;

    _3d_enable = (_3d_mode != HDMI_3D_BUTT) ? TD_FALSE : TD_TRUE;
    video_code = drv_hdmi_vic_search(frame->video_timing, frame->picture_aspect, _3d_enable);

    ptr = buffer;
    ptr[AVI_OFFSET_TYPE] = frame->type;
    if (((td_u32)frame->colorspace & HDMI_AVI_Y2_MASK) || (video_code > HDMI_5120X2160P100_64_27)) {
        /* Y2 = 1 or vic >= 128, version shall use 3 */
        frame->version = HDMI_AVI_VERSION3;
    } else if (frame->colorimetry == HDMI_COLORIMETRY_EXTENDED &&
               frame->extended_colorimetry == HDMI_EXTENDED_COLORIMETRY_ADDITIONAL) {
        /* (C1,C0)=(1,1) and (EC2,EC1,EC0)=(1,1,1), version shall use 4 */
        frame->version = HDMI_AVI_VERSION4;
    }
    ptr[AVI_OFFSET_VERSION]  = frame->version;
    ptr[AVI_OFFSET_LENGTH]   = frame->length;
    ptr[AVI_OFFSET_CHECKSUM] = 0;

    return video_code;
}

static td_s32 hdmi_avi_infoframe_pack(hdmi_avi_infoframe *frame, td_u8 *buffer, td_u32 size, hdmi_3d_mode mode)
{
    td_u32 length;
    td_u8 *ptr = TD_NULL;
    hdmi_video_code video_code;

    length = frame->length + HDMI_INFOFRAME_HEADER_SIZE;
    if (size < length) {
        return -1;
    }

    (td_void)memset_s(buffer, length, 0, length);
    video_code = avi_header_pack(frame, buffer, mode);

    ptr = buffer;
    /* 5, offset for byte, bit[6:5]: color space */
    ptr[AVI_OFFSET_PB1] = (((td_u8)frame->colorspace & AVI_FRAME_COLOSR_SPACE_MASK) << 5) |
                          ((td_u8)frame->scan_mode & AVI_FRAME_SCAN_INFOMATION_MASK);
    if (frame->active_info_valid) {
        /* 4, offset for byte, [4]: active information present */
        hdmi_set_bit(ptr[AVI_OFFSET_PB1], 4);
    }
    if (frame->horizontal_bar_valid) {
        /* 3, offset for byte, [3]: horizontal bar valid */
        hdmi_set_bit(ptr[AVI_OFFSET_PB1], 3);
    }
    if (frame->vertical_bar_valid) {
        /* 2, offset for byte, [2]: vertical bar valid */
        hdmi_set_bit(ptr[AVI_OFFSET_PB1], 2);
    }
    /* 6, offset for byte, [7,6]: colorimetry */
    ptr[AVI_OFFSET_PB2] = (((td_u32)frame->colorimetry & AVI_FRAME_COLORIMETRY_MASK) << 6) |
        /* 4, offset for byte, [5,4]: frame aspect ratio */
        (((td_u32)frame->picture_aspect & AVI_FRAME_PIC_ASPECT_MASK) << 4) |
        ((td_u32)frame->active_aspect & AVI_FRAME_ACTIVE_ASPECT_MASK);
    /* 4, offset for byte, [6,4]: Extended Colorimetry */
    ptr[AVI_OFFSET_PB3] = (((td_u32)frame->extended_colorimetry & AVI_FRAME_EXT_COLORIMETRY_MASK) << 4) |
        /* 2, offset for byte, [3,2]: RGB quantization range */
        (((td_u32)frame->quantization_range & AVI_FRAME_QUANT_RANGE_MASK) << 2) |
        ((td_u32)frame->nups & AVI_FRAME_NUP_SCAN_MASK);
    if (frame->itc) {
        /* 7, offset for byte, [7]: IT content */
        hdmi_set_bit(ptr[AVI_OFFSET_PB3], 7);
    }
    ptr[AVI_OFFSET_VIC] = (td_u8)video_code;
    /* 6, offset for byte, [7,6]: quantization range */
    ptr[AVI_OFFSET_PB5] = (((td_u32)frame->ycc_quantization_range & AVI_FRAME_YCC_QUANT_RANGE_MASK) << 6) |
        /* 4, offset for byte, [5,4]: quantization range */
        (((td_u32)frame->content_type & AVI_FRAME_CONTENT_TYPE_MASK) << 4) |
        ((frame->pixel_repeat - 1) & AVI_FRAME_PIXEL_REPEAT_MASK);
    ptr[AVI_OFFSET_TOP_BAR_LOWER] = frame->top_bar & AVI_FRAME_BAR_MODE_MASK;
    ptr[AVI_OFFSET_TOP_BAR_UPPER] = (frame->top_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_FRAME_BAR_MODE_MASK;
    ptr[AVI_OFFSET_BOTTOM_BAR_LOWER] = frame->bottom_bar & AVI_FRAME_BAR_MODE_MASK;
    ptr[AVI_OFFSET_BOTTOM_BAR_UPPER] =
        (frame->bottom_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_FRAME_BAR_MODE_MASK;
    ptr[AVI_OFFSET_LEFT_BAR_LOWER] = frame->left_bar & AVI_FRAME_BAR_MODE_MASK;
    ptr[AVI_OFFSET_LEFT_BAR_UPPER] = (frame->left_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_FRAME_BAR_MODE_MASK;
    ptr[AVI_OFFSET_RIGHT_BAR_LOWER] = frame->right_bar & AVI_FRAME_BAR_MODE_MASK;
    ptr[AVI_OFFSET_RIGHT_BAR_UPPER] =
        (frame->right_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_FRAME_BAR_MODE_MASK;
    hdmi_infoframe_checksum(buffer, length);

    return length;
}

static td_s32 hdmi_vendor_specific_infoframe_pack(const hdmi_vendor_infoframe *frame, td_u8 *buffer, td_u32 size)
{
    td_u8  *ptr = TD_NULL;
    td_u32 length;

    hdmi_if_null_return(frame, TD_FAILURE);
    hdmi_if_null_return(buffer, TD_FAILURE);

    ptr = buffer;
    length = HDMI_INFOFRAME_HEADER_SIZE + frame->length;
    if (size < length) {
        return -1;
    }
    (td_void)memset_s(buffer, length, 0, length);

    ptr[VENDOR_OFFSET_TYPE] = frame->type;
    ptr[VENDOR_OFFSET_VERSION] = frame->version;
    ptr[VENDOR_OFFSET_LENGTH] = frame->length;
    ptr[VENDOR_OFFSET_CHECSUM] = 0; /* checksum */

    if (frame->vsif_content.h14_vsif_content.ieee == HDMI14_IEEE_OUI) { /* for HDMI 1.4 */
        const hdmi_14_vsif_content *vsif_content = &frame->vsif_content.h14_vsif_content;
        /* IEEE OUI */
        ptr[VENDOR_OFFSET_IEEE_LOWER] = HDMI14_IEEE_OUI & 0xff;
        ptr[VENDOR_OFFSET_IEEE_UPPER] = (HDMI14_IEEE_OUI >> 8) & 0xff; /* get [15,8]bit, shift right 8 */
        ptr[VENDOR_OFFSET_IEEE] = (HDMI14_IEEE_OUI >> 16) & 0xff; /* get [23,16]bit, shift right 16 */
        /* format in [7,5], shift left 5 bit */
        ptr[VENDOR_OFFSET_FMT] = ((td_u8)vsif_content->format & 0x7) << 5;
        if (vsif_content->format == HDMI_VIDEO_FORMAT_4K) {
            ptr[VENDOR_OFFSET_VIC] = vsif_content->vic;
        } else if (vsif_content->format == HDMI_VIDEO_FORMAT_3D) {
            /* _3d_structure in [7,4], shift left 4 bit */
            ptr[VENDOR_OFFSET_VIC] = ((td_u8)vsif_content->_3d_structure & 0xf) << 4;
        }
    }
    hdmi_infoframe_checksum(buffer, length);

    return length;
}

td_void drv_hdmi_infoframe_send(const hdmi_infoframe_cfg *infoframe, hdmi_infoframe_id id,
                                hdmi_infoframe *curr_infoframe)
{
    td_u8 buffer[INFOFRAME_BUFFER_LEN] = {0};
    hdmi_device *hdmi_dev = TD_NULL;

    hdmi_if_null_return_void(infoframe);
    hdmi_if_null_return_void(curr_infoframe);

    hdmi_dev = container_of(infoframe, __typeof__(*hdmi_dev), infoframe);
    hdmi_if_null_return_void(hdmi_dev);

    /* pack infoframe */
    switch (id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            curr_infoframe->vendor_infoframe.type    = HDMI_INFOFRAME_TYPE_VENDOR;
            curr_infoframe->vendor_infoframe.version = 1;
            curr_infoframe->vendor_infoframe.length  = HDMI_VENDOR_INFOFRAME_SIZE;
            hdmi_vendor_specific_infoframe_pack(&curr_infoframe->vendor_infoframe, buffer, INFOFRAME_BUFFER_LEN);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            curr_infoframe->avi_infoframe.type = HDMI_INFOFRAME_TYPE_AVI;
            curr_infoframe->avi_infoframe.version = 2; /* avi infoframe version 2 */
            curr_infoframe->avi_infoframe.length = HDMI_AVI_INFOFRAME_SIZE;
            hdmi_avi_infoframe_pack(&curr_infoframe->avi_infoframe, buffer,
                INFOFRAME_BUFFER_LEN, hdmi_dev->attr.vo_attr.stereo_mode);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO:
            break;
        default:
            return;
    }

    drv_hdmi_infoframe_enable_set(infoframe, id, TD_FALSE);
    hal_call_void(hal_hdmi_infoframe_set, hdmi_dev->hal, id, buffer);
    drv_hdmi_infoframe_enable_set(infoframe, id, TD_TRUE);

    return;
}

td_void drv_hdmi_infoframe_enable_set(const hdmi_infoframe_cfg *infoframe, hdmi_infoframe_id id, td_bool enable)
{
    hdmi_device *hdmi_dev = TD_NULL;

    hdmi_if_null_return_void(infoframe);
    hdmi_dev = container_of(infoframe, __typeof__(*hdmi_dev), infoframe);
    hal_call_void(hal_hdmi_infoframe_enable_set, hdmi_dev->hal, id, enable);

    return;
}

td_s32 drv_hdmi_avi_infoframe_send(hdmi_infoframe_cfg *infoframe, td_bool enable)
{
    errno_t ret;
    hdmi_infoframe curr_infoframe = {0};
    hdmi_device *hdmi_dev = TD_NULL;
    hdmi_vo_attr *vo_attr = TD_NULL;
    hdmi_app_attr *app_attr = TD_NULL;
    hdmi_avi_infoframe *avi_infoframe = TD_NULL;

    hdmi_if_null_return(infoframe, TD_FAILURE);

    if (enable == TD_FALSE) {
        drv_hdmi_infoframe_enable_set(infoframe, HDMI_INFOFRAME_TYPE_AVI, TD_FALSE);
        return TD_SUCCESS;
    }

    hdmi_dev = container_of(infoframe, __typeof__(*hdmi_dev), infoframe);
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    vo_attr  = &hdmi_dev->attr.vo_attr;
    app_attr = &hdmi_dev->attr.app_attr;
    avi_infoframe = &infoframe->avi_infoframe;
    (td_void)memset_s(avi_infoframe, sizeof(hdmi_avi_infoframe), 0, sizeof(hdmi_avi_infoframe));
    avi_infoframe->disp_fmt               = vo_attr->disp_fmt;
    avi_infoframe->video_timing           = vo_attr->video_timing;
    avi_infoframe->picture_aspect         = vo_attr->picture_aspect;
    avi_infoframe->active_aspect          = vo_attr->active_aspect;
    avi_infoframe->nups                   = vo_attr->picture_scaling;
    avi_infoframe->pixel_repeat           = vo_attr->pixel_repeat;
    avi_infoframe->active_info_valid      = TD_TRUE;
    avi_infoframe->horizontal_bar_valid   = TD_FALSE;
    avi_infoframe->vertical_bar_valid     = TD_FALSE;
    avi_infoframe->scan_mode              = HDMI_SCAN_MODE_NONE;
    avi_infoframe->itc                    = TD_FALSE;
    avi_infoframe->content_type           = HDMI_CONTENT_TYPE_NONE;
    avi_infoframe->top_bar                = 0;
    avi_infoframe->bottom_bar             = 0;
    avi_infoframe->left_bar               = 0;
    avi_infoframe->right_bar              = 0;
    avi_infoframe->extended_colorimetry   = vo_attr->extended_colorimetry;
    avi_infoframe->quantization_range     = vo_attr->rgb_quantization;
    avi_infoframe->ycc_quantization_range = vo_attr->ycc_quantization;
    avi_infoframe->colorspace             = app_attr->out_colorspace;
    avi_infoframe->colorimetry            = app_attr->bxv_ycc_mode ? HDMI_COLORIMETRY_EXTENDED : vo_attr->colorimetry;
    ret = memcpy_s(&curr_infoframe.avi_infoframe, sizeof(curr_infoframe.avi_infoframe),
                   avi_infoframe, sizeof(hdmi_avi_infoframe));
    hdmi_unequal_eok(ret);
    drv_hdmi_infoframe_send(infoframe, HDMI_INFOFRAME_TYPE_AVI, &curr_infoframe);

    return TD_SUCCESS;
}

td_s32 drv_hdmi_vendor_infoframe_send(hdmi_infoframe_cfg *infoframe, td_bool enable)
{
    errno_t ret;
    td_u32 vic;
    hdmi_infoframe curr_infoframe = {0};
    hdmi_vo_attr *vo_attr = TD_NULL;
    hdmi_device *hdmi_dev = TD_NULL;
    hdmi_14_vsif_content *h14_vsif_content = TD_NULL;
    hdmi_vendor_infoframe *vendor_infoframe = TD_NULL;
    td_bool vendor_infoframe_enable;

    hdmi_if_null_return(infoframe, TD_FAILURE);
    if (enable == TD_FALSE) {
        drv_hdmi_infoframe_enable_set(infoframe, HDMI_INFOFRAME_TYPE_VENDOR, TD_FALSE);
        return TD_SUCCESS;
    }
    hdmi_dev = container_of(infoframe, __typeof__(*hdmi_dev), infoframe);
    hdmi_if_null_return(hdmi_dev, TD_FAILURE);

    vo_attr = &hdmi_dev->attr.vo_attr;
    vendor_infoframe = &infoframe->vendor_infoframe;
    (td_void)memset_s(vendor_infoframe, sizeof(hdmi_vendor_infoframe), 0, sizeof(hdmi_vendor_infoframe));

    h14_vsif_content = &vendor_infoframe->vsif_content.h14_vsif_content;
    vic = drv_hdmi_vic_search(vo_attr->video_timing, vo_attr->picture_aspect, TD_FALSE); /* find the real Vic */
    if (((vic >= HDMI_3840X2160P24_16_9 && vic <= HDMI_3840X2160P30_16_9) ||
        vic == HDMI_4096X2160P24_256_135) && vo_attr->stereo_mode == HDMI_3D_BUTT) {
        h14_vsif_content->ieee   = HDMI14_IEEE_OUI;
        h14_vsif_content->format = HDMI_VIDEO_FORMAT_4K;
        h14_vsif_content->vic    = 0;
        drv_hdmi_video_codes_4k_get(h14_vsif_content, vo_attr->video_timing);
        vendor_infoframe_enable = TD_TRUE;
    } else if (vo_attr->stereo_mode < HDMI_3D_BUTT) {
        h14_vsif_content->ieee = HDMI14_IEEE_OUI;
        h14_vsif_content->format = HDMI_VIDEO_FORMAT_3D;
        h14_vsif_content->_3d_structure = vo_attr->stereo_mode;
        vendor_infoframe_enable = TD_TRUE;
    } else {
        h14_vsif_content->ieee = HDMI14_IEEE_OUI;
        h14_vsif_content->format = HDMI_VIDEO_FORMAT_NONE;
        h14_vsif_content->_3d_structure = vo_attr->stereo_mode;
        vendor_infoframe_enable = TD_TRUE;
    }
    if (vendor_infoframe_enable == TD_TRUE) {
        ret = memcpy_s(&curr_infoframe.vendor_infoframe, sizeof(curr_infoframe.vendor_infoframe),
                       vendor_infoframe, sizeof(hdmi_vendor_infoframe));
        hdmi_unequal_eok(ret);
        drv_hdmi_infoframe_send(infoframe, HDMI_INFOFRAME_TYPE_VENDOR, &curr_infoframe);
    }

    return TD_SUCCESS;
}

