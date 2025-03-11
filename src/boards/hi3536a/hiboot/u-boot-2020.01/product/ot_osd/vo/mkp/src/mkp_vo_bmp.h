// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef MKP_VO_BMP_H
#define MKP_VO_BMP_H

#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef struct {
    td_u16 bi_size;
    td_u32 bi_width;
    td_s32 bi_height;
    td_u16 bi_planes;
    td_u16 bi_bit_count;
    td_u32 bi_compression;
    td_u32 bi_size_image;
    td_u32 bi_x_pels_per_meter;
    td_u32 bi_y_pels_per_meter;
    td_u32 bi_clr_used;
    td_u32 bi_clr_important;
} osd_bitmap_info_header;

typedef struct {
    td_u8 rgb_blue;
    td_u8 rgb_green;
    td_u8 rgb_red;
    td_u8 rgb_reserved;
} osd_rgb_quad;

typedef struct {
    td_u32 bf_size;
    td_u16 bf_reserved1;
    td_u16 bf_reserved2;
    td_u32 bf_off_bits;
} osd_bitmap_file_header;

typedef struct {
    osd_bitmap_info_header bmi_header;
    osd_rgb_quad bmi_colors[1];
} osd_bitmap_info;

typedef struct {
    td_u32 width;       /* out */
    td_u32 height;      /* out */
    td_u32 stride;      /* in */
    td_phys_addr_t rgb_buffer; /* in */
} osd_logo_t;

typedef enum {
    OSD_BI_BIT_COUNT_1BPP = 1,
    OSD_BI_BIT_COUNT_4BPP = 4,
    OSD_BI_BIT_COUNT_8BPP = 8,
    OSD_BI_BIT_COUNT_16BPP = 16,
    OSD_BI_BIT_COUNT_24BPP = 24,
    OSD_BI_BIT_COUNT_32BPP = 32,
} osd_bi_bit_count;

td_s32 load_bmp(td_phys_addr_t bmp_addr, osd_logo_t *video_logo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef MKP_VO_BMP */
