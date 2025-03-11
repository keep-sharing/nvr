// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "hdmi_hal_ddc.h"
#include "hdmi_reg_aon.h"
#include "hdmi_reg_ctrl.h"
#include "hdmi_hal_intf.h"
#include "hdmi_hal_ctrl.h"
#include "boot_hdmi_intf.h"

#define DDC_EDID_SALVE_ADDR 0xa0
#define DDC_HDCP_SALVE_ADDR 0x74
#define DDC_SCDC_SALVE_ADDR 0xa8
#define DDC_MAX_FIFO_SIZE   16
#define DDC_GPIO_DELAY      8
#define DDC_DATA_FIFO_DELAY 1

typedef enum {
    DDC_CMD_READ_SINGLE_NO_ACK  = 0x00,
    DDC_CMD_READ_SINGLE_ACK     = 0x01,
    DDC_CMD_READ_MUTI_NO_ACK    = 0x02,
    DDC_CMD_READ_MUTI_ACK       = 0x03,
    DDC_CMD_READ_SEGMENT_NO_ACK = 0x04,
    DDC_CMD_READ_SEGMENT_ACK    = 0x05,
    DDC_CMD_WRITE_MUTI_NO_ACK   = 0x06,
    DDC_CMD_WRITE_MUTI_ACK      = 0x07,
    DDC_CMD_FIFO_CLR            = 0x09,
    DDC_CMD_SCL_DRV             = 0x0a,
    DDC_CMD_MASTER_ABORT        = 0x0f
} ddc_issue_cmd;

static ddc_info g_ddc_info[HDMI_DEVICE_ID_BUTT];

static ddc_info *ddc_info_get(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        return &g_ddc_info[hdmi_id];
    }
    return TD_NULL;
}

static td_s32 ddc_access_enable_wait(td_u32 id, td_u32 timeout)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 tmp_time = 0;

    hdmi_ddc_cpu_ddc_req_set(id, TD_TRUE);
    while (!hdmi_ddc_cpu_ddc_req_ack_get(id)) {
        mdelay(1);
        tmp_time++;
        if (tmp_time > timeout) {
            ret = TD_FAILURE;
            break;
        }
    }

    return ret;
}

static td_s32 ddc_access_disable_wait(td_u32 id, td_u32 timeout)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 tmp_time = 0;

    hdmi_ddc_cpu_ddc_req_set(id, TD_FALSE);
    while (hdmi_ddc_cpu_ddc_req_ack_get(id)) {
        mdelay(1);
        tmp_time += 1;
        if (tmp_time > timeout) {
            ret = TD_FAILURE;
            break;
        }
    }

    return ret;
}

static td_s32 ddc_scl_wait(td_u32 id, td_u32 timeout)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 tmp_time = 0;

    while (!hdmi_ddc_scl_st_get(id)) {
        mdelay(1);
        tmp_time += 1;
        if (tmp_time > timeout) {
            ret = TD_FAILURE;
            break;
        }
    }

    return ret;
}

static td_s32 ddc_sda_wait(td_u32 id, td_u32 timeout)
{
    td_u32 tmp_timeout = 0;
    td_s32 ret = TD_SUCCESS;

    if (!hdmi_ddc_sda_st_get(id)) {
        hdmi_dcc_man_en_set(id, TD_TRUE);
        while ((!hdmi_ddc_sda_st_get(id)) && tmp_timeout++ < timeout) {
            /* pull scl high */
            hdmi_ddc_scl_oen_set(id, TD_TRUE);
            udelay(DDC_GPIO_DELAY);
            /* pull scl low */
            hdmi_ddc_scl_oen_set(id, TD_FALSE);
            udelay(DDC_GPIO_DELAY);
        }
        if (tmp_timeout < timeout && (hdmi_ddc_sda_st_get(id))) {
            /* pull sda low */
            hdmi_ddc_sda_oen_set(id, TD_FALSE);
            udelay(DDC_GPIO_DELAY);
            /* pull scl high */
            hdmi_ddc_scl_oen_set(id, TD_TRUE);
            udelay(DDC_GPIO_DELAY);
            /* pull sda high */
            hdmi_ddc_sda_oen_set(id, TD_TRUE);
            udelay(DDC_GPIO_DELAY);
            hdmi_info("deadlock clear success\n");
            ret = TD_SUCCESS;
        } else {
            hdmi_warn("deadlock clear fail\n");
            ret = TD_FAILURE;
        }
        hdmi_dcc_man_en_set(id, TD_FALSE);
    }

    return ret;
}

static td_s32 ddc_in_prog_wait(td_u32 id, td_u32 timeout)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 tmp_time = 0;

    while (hdmi_pwd_i2c_in_prog_get(id)) {
        mdelay(1);
        tmp_time += 1;
        if (tmp_time > timeout) {
            ret = TD_FAILURE;
            break;
        }
    }

    return ret;
}

static td_s32 ddc_cmd_issue(td_u32 id, const ddc_cfg *cfg, td_bool read_issue)
{
    td_u32 slave_addr = 0;
    td_u32 segment, offset, data_size;

    hdmi_if_null_return(cfg, TD_FAILURE);
    segment = cfg->segment;
    offset = cfg->offset;
    data_size = cfg->data_size;
    switch (cfg->func_type) {
        case DDC_FUNC_TYPE_EDID:
            slave_addr = DDC_EDID_SALVE_ADDR;
            break;
        case DDC_FUNC_TYPE_HDCP:
            slave_addr = DDC_HDCP_SALVE_ADDR;
            break;
        case DDC_FUNC_TYPE_SCDC:
            slave_addr = DDC_SCDC_SALVE_ADDR;
            break;
        default:
            hdmi_warn("invalid DDC function type, wrong slaveaddr!\n");
            break;
    }
    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        hdmi_pwd_mst_cmd_set(id, DDC_CMD_FIFO_CLR);
        hdmi_pwd_slave_addr_set(id, slave_addr);
        hdmi_pwd_slave_seg_set(id, segment);
        hdmi_pwd_slave_offset_set(id, offset);
        hdmi_pwd_data_out_cnt_set(id, data_size);
        udelay(DDC_GPIO_DELAY);
        hdmi_pwd_mst_cmd_set(id, cfg->issue_mode);
    } else {
        hdmi_err("invalid master_mode=%u\n", cfg->master_mode);
    }

    return TD_SUCCESS;
}

static td_s32 ddc_read(td_u32 id, td_u8 *data, td_u32 len, td_u32 timeout)
{
    td_u32 i, data_size;
    td_u8 *p = TD_NULL;

    p = data;
    for (data_size = 0; data_size < len; data_size++, p++) {
        /* when read-fifo empty, every byte wait a max timeout */
        for (i = 0;
            (i < timeout) && (hdmi_pwd_fifo_empty_get(id) || (hdmi_pwd_fifo_data_cnt_get(id) == 0));
             i++) {
            /* wait ddc status update after DDC cmd set. */
            mdelay(DDC_DATA_FIFO_DELAY);
            if (hdmi_ddc_i2c_no_ack_get(id) || hdmi_ddc_i2c_bus_low_get(id)) {
                hdmi_pwd_mst_cmd_set(id, DDC_CMD_MASTER_ABORT);
                hdmi_warn("DDC status error!\n");
                return TD_FAILURE;
            }
        }
        if (i >= timeout) {
            hdmi_warn("read fifo timeout=%u ms, size=%u!\n", timeout, len);
            return TD_FAILURE;
        }
        if (p != TD_NULL) {
            *p = hdmi_pwd_fifo_data_out_get(id);
            /*
             * the fifo status is not refresh promptly,
             * so re-read the fifo status and delay 1us if the fifo is empty,
             * wait the data ready. it must delay 1us(from wzg @2015.12.28) after read fifo data.
             */
            udelay(DDC_DATA_FIFO_DELAY);
        } else {
            hdmi_err("edid &data[%u]=null\n", data_size);
            return TD_FAILURE;
        }
    }

    return data_size;
}

static td_s32 ddc_write(td_u32 id, const td_u8 *data, td_u32 len, td_u32 timeout)
{
    td_u32 i, data_size;
    const td_u8 *p = TD_NULL;

    p = data;
    for (data_size = 0; data_size < len; data_size++, p++) {
        /* when write-fifo full, every byte wait a max timeout and retry times */
        for (i = 0; (((i < timeout) && (hdmi_pwd_fifo_data_cnt_get(id) >= DDC_MAX_FIFO_SIZE))); i++) {
            /* wait ddc status update after DDC cmd set. */
            mdelay(DDC_DATA_FIFO_DELAY);
            if (hdmi_ddc_i2c_no_ack_get(id) || hdmi_ddc_i2c_bus_low_get(id)) {
                hdmi_pwd_mst_cmd_set(id, DDC_CMD_MASTER_ABORT);
                hdmi_warn("DDC status error!\n");
                return TD_FAILURE;
            }
        }
        if (i >= timeout) {
            hdmi_err("write fifo timeout=%u ms, size=%u\n", timeout, len);
            return TD_FAILURE;
        }
        if (p != TD_NULL) {
            hdmi_pwd_fifo_data_in_set(id, *data);
            udelay(DDC_DATA_FIFO_DELAY);
        } else {
            hdmi_err("edid &data[%u]=null\n", data_size);
            return TD_FAILURE;
        }
    }

    return data_size;
}

static td_s32 ddc_data_issue(td_u32 id, const ddc_cfg *cfg, td_bool read_issue)
{
    td_u32 timeout;
    td_s32 data_size;

    timeout = (cfg->issue_timeout < DDC_DEFAULT_TIMEOUT_ISSUE) ? DDC_DEFAULT_TIMEOUT_ISSUE : cfg->sda_timeout;
    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        if (read_issue) {
            data_size = ddc_read(id, cfg->data, cfg->data_size, timeout);
        } else {
            data_size = ddc_write(id, cfg->data, cfg->data_size, timeout);
        }
        return data_size;
    } else {
        hdmi_err("invalid master_mode=%u, fail!\n", cfg->master_mode);
        return TD_FAILURE;
    }
}

td_s32 hal_hdmi_ddc_init(hdmi_device_id hdmi_id)
{
    ddc_info *info = ddc_info_get(hdmi_id);

    hdmi_if_null_return(info, TD_FAILURE);
    if (!info->run.init) {
        info->run.init = TD_TRUE;
    }

    return TD_SUCCESS;
}

td_s32 hal_hdmi_ddc_deinit(hdmi_device_id hdmi_id)
{
    ddc_info *info = ddc_info_get(hdmi_id);

    hdmi_if_null_return(info, TD_FAILURE);
    hdmi_if_false_return(info->run.init, TD_FAILURE);
    info->run.init = TD_FALSE;

    return TD_SUCCESS;
}

td_s32 hal_hdmi_ddc_err_clear(hdmi_device_id hdmi_id)
{
    td_s32 ret = TD_SUCCESS;
    ddc_info *info = ddc_info_get(hdmi_id);

    hdmi_if_null_return(info, TD_FAILURE);
    hdmi_if_false_return(info->run.init, TD_FAILURE);

    /* In prog check */
    if (ddc_in_prog_wait(hdmi_id, DDC_DEFAULT_TIMEOUT_IN_PROG) != TD_SUCCESS) {
        hdmi_warn("error clr, wait in prog timeout!\n");
        ret = TD_FAILURE;
    }
    /* scl check */
    if (ddc_scl_wait(hdmi_id, DDC_DEFAULT_TIMEOUT_SCL) != TD_SUCCESS) {
        hdmi_warn("error clr, wait scl timeout!\n");
        ret = TD_FAILURE;
    }
    /* sda check */
    if (ddc_sda_wait(hdmi_id, DDC_DEFAULT_TIMEOUT_SDA) != TD_SUCCESS) {
        hdmi_warn("error clr, wait sda timeout!\n");
        ret = TD_FAILURE;
    }
    if (ret == TD_SUCCESS) {
        hdmi_info("error clr success!\n");
    }

    return ret;
}

td_s32 hal_hdmi_ddc_issue(hdmi_device_id hdmi_id, const ddc_cfg *cfg)
{
    td_s32 ret = TD_SUCCESS;
    td_bool read_issue = TD_FALSE;
    ddc_info *info = ddc_info_get(hdmi_id);

    hdmi_if_null_return(info, TD_FAILURE);
    hdmi_if_null_return(cfg, TD_FAILURE);
    hdmi_if_false_return(info->run.init, TD_FAILURE);

    /* Access check */
    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        if (ddc_access_enable_wait(hdmi_id, cfg->access_timeout) != TD_SUCCESS) {
            hdmi_err("wait access bus timeout!\n");
            goto exit;
        }
    }
    /* scl check */
    if (ddc_scl_wait(hdmi_id, cfg->scl_timeout) != TD_SUCCESS) {
        hdmi_err("wait scl timeout!\n");
        goto exit;
    }
    /* sda check */
    if (ddc_sda_wait(hdmi_id, cfg->sda_timeout) != TD_SUCCESS) {
        hdmi_err("wait sda timeout!\n");
        goto exit;
    }
    /* jude read/write issue */
    if (cfg->issue_mode <= DDC_MODE_READ_SEGMENT_ACK) {
        read_issue = TD_TRUE;
    } else if (cfg->issue_mode < DDC_MODE_BUTT) {
        read_issue = TD_FALSE;
    } else {
        hdmi_err("invalid ddc issue mode!\n");
        goto exit;
    }
    /* Issue command */
    if (ddc_cmd_issue(hdmi_id, cfg, read_issue) != TD_SUCCESS) {
        hdmi_err("command issue fail!\n");
        goto exit;
    }
    /* issue data */
    ret = ddc_data_issue(hdmi_id, cfg, read_issue);
    if (ret <= 0) {
        hdmi_info("data issue fail!\n");
        goto exit;
    }

exit:
    if (ddc_in_prog_wait(hdmi_id, cfg->in_prog_timeout) != TD_SUCCESS) {
        hdmi_warn("wait in prog timeout!\n");
    }

    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        if (ddc_access_disable_wait(hdmi_id, cfg->access_timeout) != TD_SUCCESS) {
            hdmi_warn("wait access disable timeout!\n");
        }
    }

    return ret;
}

td_void hal_hdmi_ddc_default_cfg_get(hdmi_device_id hdmi, ddc_cfg *cfg)
{
    ddc_info *info = ddc_info_get(hdmi);

    hdmi_if_null_return_void(info);
    hdmi_if_null_return_void(cfg);
    hdmi_if_false_return_void(info->run.init);

    cfg->segment     = 0;
    cfg->offset      = 0;
    cfg->func_type   = DDC_FUNC_TYPE_EDID;
    cfg->issue_mode  = DDC_MODE_READ_MUTIL_NO_ACK;
    cfg->speed       = 0;
    cfg->master_mode = DDC_MASTER_MODE_PWD;
    cfg->access_timeout  = DDC_DEFAULT_TIMEOUT_ACCESS;
    cfg->hpd_timeout     = DDC_DEFAULT_TIMEOUT_HPD;
    cfg->in_prog_timeout = DDC_DEFAULT_TIMEOUT_IN_PROG;
    cfg->scl_timeout     = DDC_DEFAULT_TIMEOUT_SCL;
    cfg->sda_timeout     = DDC_DEFAULT_TIMEOUT_SDA;
    cfg->issue_timeout   = DDC_DEFAULT_TIMEOUT_ISSUE;
    cfg->data_size       = 0;
    cfg->data            = TD_NULL;

    return;
}

