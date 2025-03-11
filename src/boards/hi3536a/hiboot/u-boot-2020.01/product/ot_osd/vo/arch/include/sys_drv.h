// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef SYS_DRV_H
#define SYS_DRV_H

#include "ot_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

td_s32 sys_drv_drv_ioctrl(const ot_mpp_chn *mpp_chn, td_u32 func_id, const td_void *io_args);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef SYS_DRV_H */