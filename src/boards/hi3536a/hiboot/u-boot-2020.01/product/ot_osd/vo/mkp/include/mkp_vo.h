// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef MKP_VO_H
#define MKP_VO_H

#include "ot_common.h"
#include "ot_common_video.h"
#include "ot_common_vo.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define VO_LAYER_BUTT OT_VO_MAX_PHYS_LAYER_NUM

#define vo_err_trace(fmt, ...)  \
    OT_ERR_TRACE(OT_ID_VO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define vo_warn_trace(fmt, ...) \
    OT_WARN_TRACE(OT_ID_VO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef MKP_VO_H */
