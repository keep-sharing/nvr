// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "mkp_vo_bmp.h"
#include "common.h"
#include "securec.h"

td_s32 load_bmp(td_phys_addr_t bmp_addr, osd_logo_t *video_logo)
{
    td_u16 bpp;
    osd_bitmap_file_header bmp_file_header;
    osd_bitmap_info bmp_info;
    td_u32 byte_count = OSD_BI_BIT_COUNT_16BPP / 8; /* 8 bits per bytes */

    (td_void)memcpy_s(&bmp_file_header, sizeof(osd_bitmap_file_header), (char *)(uintptr_t)(bmp_addr + sizeof(td_u16)),
        sizeof(osd_bitmap_file_header));
    (td_void)memcpy_s(&bmp_info, sizeof(osd_bitmap_info),
        (char *)(uintptr_t)(bmp_addr + sizeof(td_u16) + sizeof(osd_bitmap_file_header)), sizeof(osd_bitmap_info));
    video_logo->width = bmp_info.bmi_header.bi_width;
    video_logo->height = bmp_info.bmi_header.bi_height;
    if (bmp_info.bmi_header.bi_width > (UINT_MAX / byte_count)) {
        printf("bmp's width=%u is illegal.\n", bmp_info.bmi_header.bi_width);
        return TD_FAILURE;
    }
    video_logo->stride = bmp_info.bmi_header.bi_width * byte_count;
    video_logo->rgb_buffer = bmp_addr + bmp_file_header.bf_off_bits;

    bpp = bmp_info.bmi_header.bi_bit_count;
    if (bpp != OSD_BI_BIT_COUNT_16BPP) { /* 16 bits, 2bytes per pixel bitmap format */
        printf("bitmap format bpp=%d not supported! only supoort format bpp=%d\n", bpp, OSD_BI_BIT_COUNT_16BPP);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}
