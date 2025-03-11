// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_OTP_H
#define DRV_OTP_H

#include "ot_common_otp.h"

td_s32 drv_otp_init(void);

td_void drv_otp_deinit(void);

td_s32 drv_otp_set_user_data(const td_char *field_name,
    td_u32 offset, const td_u8 *value, td_u32 value_len);

td_s32 drv_otp_get_user_data(const td_char *field_name,
    td_u32 offset, td_u8 *value, td_u32 value_len);

td_s32 drv_otp_burn_product_pv(const ot_otp_burn_pv_item *pv, td_u32 num);

td_s32 drv_otp_read_product_pv(ot_otp_burn_pv_item *pv, td_u32 num);

td_s32 drv_otp_get_key_verify_status(const td_char *key_name, td_bool *status);

td_s32 drv_otp_load_key_to_klad(const td_char *key_name);

td_s32 drv_otp_set_user_data_lock(const td_char *field_name,
    td_u32 offset, td_u32 value_len);

td_s32 drv_otp_get_user_data_lock(const td_char *field_name,
    td_u32 offset, td_u32 value_len, ot_otp_lock_status *lock);

#endif /* DRV_OTP_H */