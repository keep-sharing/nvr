// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_OTP_H
#define HAL_OTP_H

#include "ot_common_otp.h"
#include "otp_data.h"
#include "hal_otp_comm.h"

#define OTP_WAIT_TIME_OUT           10000

#define OTP_USER_KEY_MAX_BYTES      32
#define OTP_USER_KEY_MAX_WORDS      (OTP_USER_KEY_MAX_BYTES / 4)

/* get lock status */
td_s32 hal_otp_get_key_lock_sta(td_u32 offset, td_bool *lock_sta);
td_s32 hal_otp_get_flag_lock_sta(td_u32 offset, td_bool *lock_sta);
td_s32 hal_otp_get_user_data_lock_sta(td_u32 index, td_bool *lock_sta);

/* operate user data */
td_s32 hal_otp_set_user_data_word(td_u32 index, td_u32 value, td_bool lock);
td_s32 hal_otp_get_user_data_word(td_u32 index, td_u32 *value);

/* operate key */
td_s32 hal_otp_load_key_to_klad(td_u32 key_index);
td_s32 hal_otp_verify_key(td_u32 key_index, td_bool *flag);
td_s32 hal_otp_burn_key(const otp_data_item *data_item, const td_u8 *key, td_u32 klen);

/* enable flag */
td_s32 hal_otp_enable_flag(const otp_data_item *data_item, td_u32 flag, td_bool lock);

#endif /* HAL_OTP_H */
