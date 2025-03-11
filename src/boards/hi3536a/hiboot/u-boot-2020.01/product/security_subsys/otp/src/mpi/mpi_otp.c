// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ot_mpi_otp.h"
#include "drv_ioctl_otp.h"
#include "securec.h"
#include "ot_debug_otp.h"
#include "drv_otp_intf.h"
#include "drv_lib.h"

#define OTP_MUTEX_T                     td_s32
#define otp_mutex_lock(x)
#define otp_mutex_unlock(x)
#define otp_open(name, flags, mode)     ((td_void)otp_get_cpu_secure_sta(), otp_entry())
#define otp_close(fd)                   (otp_exit(), 1)
#define otp_ioctl(fd, request, var)     intf_otp_ioctl(request, var)

typedef struct {
    OTP_MUTEX_T lock;
    td_s32 ref_count;
    td_s32 otp_fd;
} mpi_otp_mgmt;

static mpi_otp_mgmt g_mpi_otp_mgmt = {
    .lock = 1,
    .ref_count = -1,
    .otp_fd = -1,
};

#define OTP_INIT_MAX_NUM     0x7FFFFFFF

#define _mpi_otp_not_init_return(ref_count)                     \
    do {                                                        \
        if ((ref_count) < 0) {                                  \
            ot_otp_error("otp init counter %d!\n", ref_count);  \
            return OT_ERR_OTP_NOT_INIT;                         \
        }                                                       \
    } while (0)

static mpi_otp_mgmt *_mpi_otp_get_mgmt(td_void)
{
    return &g_mpi_otp_mgmt;
}

td_s32 ot_mpi_otp_init(td_void)
{
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    otp_mutex_lock(&mgmt->lock);

    if (mgmt->ref_count >= 0) {
        if (mgmt->ref_count < OTP_INIT_MAX_NUM) {
            mgmt->ref_count++;
        }
        otp_mutex_unlock(&mgmt->lock);
        return TD_SUCCESS;
    }

    mgmt->otp_fd = otp_open("/dev/"UMAP_DEVNAME_OTP_BASE, O_RDWR, 0);
    if (mgmt->otp_fd < 0) {
        otp_mutex_unlock(&mgmt->lock);
        ot_otp_error("open device /dev/%s failed\n", UMAP_DEVNAME_OTP_BASE);
        return OT_ERR_OTP_FAILED_INIT;
    }

    mgmt->ref_count = 0;

    otp_mutex_unlock(&mgmt->lock);

    return TD_SUCCESS;
}

td_s32 ot_mpi_otp_deinit(td_void)
{
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    otp_mutex_lock(&mgmt->lock);

    if (mgmt->ref_count != 0) {
        if (mgmt->ref_count < OTP_INIT_MAX_NUM) {
            mgmt->ref_count--;
        }
        otp_mutex_unlock(&mgmt->lock);
        return TD_SUCCESS;
    }

    if (otp_close(mgmt->otp_fd) < 0) {
        mgmt->otp_fd = -1;
        mgmt->ref_count = -1;
        otp_mutex_unlock(&mgmt->lock);
        ot_otp_error("close /dev/%s failed\n", UMAP_DEVNAME_OTP_BASE);
        return OT_ERR_OTP_FAILED_INIT;
    }

    mgmt->otp_fd = -1;
    mgmt->ref_count = -1;

    otp_mutex_unlock(&mgmt->lock);

    return TD_SUCCESS;
}

td_s32 ot_mpi_otp_set_user_data(const td_char *field_name,
    td_u32 offset, const td_u8 *value, td_u32 value_len)
{
    td_s32 ret;
    otp_user_data user_data;
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    _mpi_otp_not_init_return(mgmt->ref_count);

    (td_void)memset_s(&user_data, sizeof(otp_user_data), 0, sizeof(otp_user_data));

    ret = memcpy_s(user_data.field_name, sizeof(user_data.field_name), field_name, strlen(field_name));
    ot_otp_func_fail_return(memcpy_s, ret != EOK, OT_ERR_OTP_FAILED_SEC_FUNC);

    user_data.value = (td_u8 *)value;
    user_data.offset = offset;
    user_data.value_len = value_len;

    ret = otp_ioctl(mgmt->otp_fd, CMD_OTP_SET_USER_DATA, &user_data);
    ot_otp_func_fail_return(otp_ioctl, ret != TD_SUCCESS, ret);

    return TD_SUCCESS;
}

td_s32 ot_mpi_otp_get_user_data(const td_char *field_name,
    td_u32 offset, td_u8 *value, td_u32 value_len)
{
    td_s32 ret;
    otp_user_data user_data;
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    _mpi_otp_not_init_return(mgmt->ref_count);

    (td_void)memset_s(&user_data, sizeof(otp_user_data), 0, sizeof(otp_user_data));

    ret = memcpy_s(user_data.field_name, sizeof(user_data.field_name), field_name, strlen(field_name));
    ot_otp_func_fail_return(memcpy_s, ret != EOK, OT_ERR_OTP_FAILED_SEC_FUNC);

    user_data.value = value;
    user_data.offset = offset;
    user_data.value_len = value_len;

    ret = otp_ioctl(mgmt->otp_fd, CMD_OTP_GET_USER_DATA, &user_data);
    ot_otp_func_fail_return(otp_ioctl, ret != TD_SUCCESS, ret);

    return ret;
}

td_s32 ot_mpi_otp_set_user_data_lock(const td_char *field_name,
    td_u32 offset, td_u32 value_len)
{
    td_s32 ret;
    otp_user_data_lock data_lock;
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    _mpi_otp_not_init_return(mgmt->ref_count);

    (td_void)memset_s(&data_lock, sizeof(otp_user_data_lock), 0, sizeof(otp_user_data_lock));
    data_lock.offset = offset;
    data_lock.value_len = value_len;
    ret = memcpy_s(data_lock.field_name, OT_OTP_PV_NAME_MAX_LEN, field_name, strlen(field_name));
    ot_otp_func_fail_return(memcpy_s, ret != EOK, OT_ERR_OTP_FAILED_SEC_FUNC);

    ret = otp_ioctl(mgmt->otp_fd, CMD_OTP_SET_USER_DATA_LOCK, &data_lock);
    ot_otp_func_fail_return(otp_ioctl, ret != TD_SUCCESS, ret);

    return ret;
}

td_s32 ot_mpi_otp_get_user_data_lock(const td_char *field_name,
    td_u32 offset, td_u32 value_len, ot_otp_lock_status *lock)
{
    td_s32 ret;
    otp_user_data_lock data_lock;
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    _mpi_otp_not_init_return(mgmt->ref_count);

    ot_otp_formula_fail_return(lock == TD_NULL, OT_ERR_OTP_NULL_PTR);

    (td_void)memset_s(&data_lock, sizeof(otp_user_data_lock), 0, sizeof(otp_user_data_lock));
    data_lock.offset = offset;
    data_lock.value_len = value_len;
    ret = memcpy_s(data_lock.field_name, OT_OTP_PV_NAME_MAX_LEN, field_name, strlen(field_name));
    ot_otp_func_fail_return(memcpy_s, ret != EOK, OT_ERR_OTP_FAILED_SEC_FUNC);

    ret = otp_ioctl(mgmt->otp_fd, CMD_OTP_GET_USER_DATA_LOCK, &data_lock);
    ot_otp_func_fail_return(otp_ioctl, ret != TD_SUCCESS, ret);

    *lock = data_lock.lock;

    return ret;
}

td_s32 ot_mpi_otp_burn_product_pv(const ot_otp_burn_pv_item *pv, td_u32 num)
{
    td_s32 ret;
    otp_product_pv product_pv;
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    _mpi_otp_not_init_return(mgmt->ref_count);

    (td_void)memset_s(&product_pv, sizeof(otp_product_pv), 0, sizeof(otp_product_pv));

    product_pv.pv = (ot_otp_burn_pv_item *)pv;
    product_pv.num = num;

    ret = otp_ioctl(mgmt->otp_fd, CMD_OTP_BURN_PRODUCT_PV, &product_pv);
    ot_otp_func_fail_return(otp_ioctl, ret != TD_SUCCESS, ret);

    return ret;
}

td_s32 ot_mpi_otp_read_product_pv(ot_otp_burn_pv_item *pv, td_u32 num)
{
    td_s32 ret;
    otp_product_pv product_pv;
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    _mpi_otp_not_init_return(mgmt->ref_count);

    (td_void)memset_s(&product_pv, sizeof(otp_product_pv), 0, sizeof(otp_product_pv));

    product_pv.pv = pv;
    product_pv.num = num;

    ret = otp_ioctl(mgmt->otp_fd, CMD_OTP_READ_PRODUCT_PV, &product_pv);
    ot_otp_func_fail_return(otp_ioctl, ret != TD_SUCCESS, ret);

    return ret;
}

td_s32 ot_mpi_otp_get_key_verify_status(const td_char *key_name, td_bool *status)
{
    td_s32 ret;
    otp_key_verify_status key_sta;
    mpi_otp_mgmt *mgmt = _mpi_otp_get_mgmt();

    _mpi_otp_not_init_return(mgmt->ref_count);

    ot_otp_formula_fail_return(status == TD_NULL, OT_ERR_OTP_NULL_PTR);

    (td_void)memset_s(&key_sta, sizeof(otp_key_verify_status), 0, sizeof(otp_key_verify_status));

    ret = memcpy_s(key_sta.field_name, OT_OTP_PV_NAME_MAX_LEN, key_name, strlen(key_name));
    ot_otp_func_fail_return(memcpy_s, ret != EOK, OT_ERR_OTP_FAILED_SEC_FUNC);

    ret = otp_ioctl(mgmt->otp_fd, CMD_OTP_KEY_VERIFY, &key_sta);
    ot_otp_func_fail_return(otp_ioctl, ret != TD_SUCCESS, ret);

    *status = key_sta.status;

    return ret;
}

