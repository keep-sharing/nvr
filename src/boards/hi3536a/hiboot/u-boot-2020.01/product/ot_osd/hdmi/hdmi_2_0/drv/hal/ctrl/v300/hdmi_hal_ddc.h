// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __HDMI_HAL_DDC_H__
#define __HDMI_HAL_DDC_H__

#include "drv_hdmi_common.h"
#include "ot_type.h"

#define DDC_DEFAULT_TIMEOUT_ACCESS  100
#define DDC_DEFAULT_TIMEOUT_HPD     100
#define DDC_DEFAULT_TIMEOUT_IN_PROG 20
#define DDC_DEFAULT_TIMEOUT_SCL     1
#define DDC_DEFAULT_TIMEOUT_SDA     30
#define DDC_DEFAULT_TIMEOUT_ISSUE   20

typedef enum {
    DDC_FUNC_TYPE_EDID,
    DDC_FUNC_TYPE_HDCP,
    DDC_FUNC_TYPE_SCDC,
    DDC_FUNC_TYPE_BUTT
} ddc_func_type;

typedef enum {
    DDC_MASTER_MODE_PWD,
    DDC_MASTER_MODE_AON
} ddc_master_mode;

typedef enum {
    DDC_MODE_READ_SINGLE_NO_ACK,
    DDC_MODE_READ_SINGLE_ACK,
    DDC_MODE_READ_MUTIL_NO_ACK,
    DDC_MODE_READ_MUTIL_ACK,
    DDC_MODE_READ_SEGMENT_NO_ACK,
    DDC_MODE_READ_SEGMENT_ACK,
    DDC_MODE_WRITE_MUTIL_NO_ACK,
    DDC_MODE_WRITE_MUTIL_ACK,
    DDC_MODE_BUTT
} ddc_issue_mode;

typedef struct {
    td_u8           segment;
    td_u8           offset;
    ddc_func_type   func_type;
    ddc_issue_mode  issue_mode;
    ddc_master_mode master_mode;
    td_u32          speed;
    td_u32          access_timeout;
    td_u32          hpd_timeout;
    td_u32          in_prog_timeout;
    td_u32          scl_timeout;
    td_u32          sda_timeout;
    td_u32          issue_timeout;
    td_s32          data_size;
    td_u8           *data;
} ddc_cfg;

typedef struct {
    td_bool init;
} ddc_run;

typedef struct {
    ddc_run run;
} ddc_info;

td_s32 hal_hdmi_ddc_init(hdmi_device_id hdmi_id);

td_s32 hal_hdmi_ddc_deinit(hdmi_device_id hdmi_id);

td_s32 hal_hdmi_ddc_err_clear(hdmi_device_id hdmi_id);

td_s32 hal_hdmi_ddc_issue(hdmi_device_id hdmi_id, const ddc_cfg *cfg);

td_void hal_hdmi_ddc_default_cfg_get(hdmi_device_id hdmi_id, ddc_cfg *cfg);

#endif /* __HDMI_HAL_DDC_H__ */

