// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __HDMI_HAL_INTF_H__
#define __HDMI_HAL_INTF_H__

#include "drv_hdmi_common.h"
#include "hdmi_hal_ctrl.h"
#include "hdmi_hal_ddc.h"
#include "hdmi_hal_scdc.h"
#include "ot_type.h"

typedef struct {
    td_bool            init;
    hdmi_tx_capability tx_capability;
    td_u32             tmds_clk;
} hdmi_hal_cfg;

typedef struct {
    td_u32 tmds_clk;
    hdmi_trace_len trace_len;
    hdmi_hw_spec *hw_spec;
} hdmi_hal_hw_param;

#endif /* __HDMI_HAL_INTF_H__ */

