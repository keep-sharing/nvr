// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_COEF_ORG_COMM_H
#define DRV_VO_COEF_ORG_COMM_H

#include "drv_vo_coef_comm.h"
#include "hal_vo_def.h"
#include "ot_common_vo.h"
#include "inner_vo.h"

#if vo_desc("UBOOT_VO")
const csc_coef *vo_get_csc_coef(ot_vo_csc_matrix csc_matrix);
const td_s32 *vo_get_sin_table(td_void);
const td_s32 *vo_get_cos_table(td_void);
#endif /* #if vo_desc("UBOOT_VO") */

#endif
