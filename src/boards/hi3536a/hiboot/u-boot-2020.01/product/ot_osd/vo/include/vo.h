// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef VO_H
#define VO_H

#include "ot_common_vo.h"

#define VO_RECT_ALIGN                       2

int set_vobg(unsigned int dev, unsigned int rgb);
int start_vo(unsigned int dev, unsigned int type, unsigned int sync);
int stop_vo(unsigned int dev);
int start_gx(unsigned int layer, unsigned long addr, unsigned int strd, ot_rect gx_rect, unsigned int type);
int stop_gx(unsigned int layer);
int start_videolayer(unsigned int layer, unsigned long addr, unsigned int strd, ot_rect layer_rect);
int stop_videolayer(unsigned int layer);
td_void vo_dcache_range(td_phys_addr_t start_addr, td_u64 size);
typedef struct {
    td_bool video_enable;
    td_bool video_config;
    ot_vo_csc layer_csc;
    ot_vo_video_layer_attr vou_video_attr;
} vo_layer_info;

#endif /* end of VO_H */
