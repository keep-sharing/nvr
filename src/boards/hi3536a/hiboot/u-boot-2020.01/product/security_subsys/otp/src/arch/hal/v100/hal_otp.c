// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hal_otp.h"
#include "drv_lib.h"
#include "drv_osal_init.h"
#include "securec.h"
#include "ot_debug_otp.h"
#include "otp_reg_base.h"
#include "drv_ioctl_otp.h"
#include "hal_otp_define.h"

#ifdef OT_OTP_V100

typedef enum {
    OTP_WORK_MODE_READ_LOCK_STA = 0,
    OTP_WORK_MODE_LOAD_CIPHER_KEY,
    OTP_WORK_MODE_BURN_KEY,
    OTP_WORK_MODE_VERIFY_KEY_CRC,
    OTP_WORK_MODE_ENABLE_FLAG_CFG,
    OTP_WORK_MODE_WRITE_USER_ROOM,
    OTP_WORK_MODE_READ_USER_ROOM,
    OTP_WORK_MODE_BUTT,
} hal_otp_work_mode;

typedef enum {
    OTP_USER_KEY0 = 0,
    OTP_USER_KEY1,
    OTP_USER_KEY2,
    OTP_USER_KEY3,
    OTP_USER_KEY_JTAG_PW_ID,
    OTP_USER_KEY_JTAG_PW,
    OTP_USER_KEY_ROOTKEY,
    OTP_USER_KEY_HMACKEY,
    OTP_USER_KEY_BUTT,
} hal_otp_key_index;

typedef enum {
    OTP_USER_FLAG_SECURE_BOOT_EN = 0,
    OTP_USER_FLAG_DDR_CA_EN,
    OTP_USER_FLAG_JTAG_CA_EN,
    OTP_USER_FLAG_JTAG_PRT_MODE,
    OTP_USER_FLAG_SECURE_ISO_EN,
    OTP_USER_FLAG_UBOOT_REDUNDANCE,
    OTP_USER_FLAG_RESERVED,
    OTP_USER_FLAG_BUTT,
} hal_otp_flag_index;

#define _hal_otp_read_sta0(sta)              hal_otp_read_word(OTP_USER_LOCK_STA0, sta)
#define _hal_otp_read_rev_data(data)         hal_otp_read_word(OTP_USER_REV_RDATA, data)
#define _hal_otp_read_ctrl_sta(sta)          hal_otp_read_word(OTP_USER_CTRL_STA, sta)

#define _hal_otp_set_rev_addr(offset)        hal_otp_write_word(OTP_USER_REV_ADDR, offset)
#define _hal_otp_set_rev_data(data)          hal_otp_write_word(OTP_USER_REV_WDATA, data)
#define _hal_otp_set_work_mode(mode)         hal_otp_write_word(OTP_USER_WORK_MODE, mode)
#define _hal_otp_set_key_index(idx)          hal_otp_write_word(OTP_USER_KEY_INDEX, idx)
#define _hal_otp_set_flag_index(idx)         hal_otp_write_word(OTP_USER_FLAG_INDEX, idx)
#define _hal_otp_set_flag_value(value)       hal_otp_write_word(OTP_USER_FLAG_VALUE, value)
#define _hal_otp_op_start()                  hal_otp_write_word(OTP_USER_OP_START, OTP_OP_START_VAL)

static td_s32 _hal_otp_wait_op_free(td_void)
{
    otp_user_ctrl_sta ctrl_sta;
    td_u32 time_out_cnt = OTP_WAIT_TIME_OUT;

    while (time_out_cnt--) {
        _hal_otp_read_ctrl_sta(&ctrl_sta.u32);

        if (ctrl_sta.bits.otp_op_busy == 0) {
            return TD_SUCCESS;
        }
        otp_udelay(1);
    }

    ot_otp_error("otp wait op timeout!\n");
    return OT_ERR_OTP_WAIT_TIMEOUT;
}

static td_s32 _hal_otp_wait_cmd_finish(td_void)
{
    return _hal_otp_wait_op_free();
}

static td_s32 _hal_otp_get_user_lock_sta(td_u32 *sta0)
{
    td_s32 ret;

    ot_otp_enter();

    /* 1. wait otp ctrl free */
    ret = _hal_otp_wait_op_free();
    ot_otp_func_fail_return(_hal_otp_wait_op_free, ret != TD_SUCCESS, ret);

    /* 2. set work mode to read lock sta */
    _hal_otp_set_work_mode(OTP_WORK_MODE_READ_LOCK_STA);

    /* 3. start otp ctrl */
    _hal_otp_op_start();

    /* 4. wait user cmd finish */
    ret = _hal_otp_wait_cmd_finish();
    ot_otp_func_fail_return(_hal_otp_wait_cmd_finish, ret != TD_SUCCESS, ret);

    /* 5. read lock sta */
    _hal_otp_read_sta0(sta0);

    ot_otp_exit();

    return ret;
}

/*
 * write key: Writes key from a high address to a low address by word
 * calculate crc: calculate crc according to the key write sequence by byte
 */
static td_s32 _hal_otp_write_key_data(const td_u8 *u8_key, td_u32 u8_klen)
{
    td_s32 ret;
    td_u32 u32_key[OTP_USER_KEY_MAX_WORDS];
    td_u8 u8_tmp[OTP_USER_KEY_MAX_BYTES];
    td_u32 crc, i, u32_klen;

    ot_otp_formula_fail_return(!align_word(u8_klen), OT_ERR_OTP_INVALID_PARAM);

    (td_void)memset_s(u32_key, sizeof(u32_key), 0, sizeof(u32_key));
    (td_void)memset_s(u8_tmp, sizeof(u8_tmp), 0, sizeof(u8_tmp));

    u32_klen = u8_klen / WORD_BYTE_WIDTH;

    ret = memcpy_s((td_u8 *)u32_key, sizeof(u32_key), u8_key, u8_klen);
    ot_otp_func_fail_return(memcpy_s, ret != EOK, OT_ERR_OTP_FAILED_SEC_FUNC);

    for (i = 0; i < u32_klen; i++) {
        hal_otp_write_word(OTP_USER_KEY_DATA0 + i * WORD_BYTE_WIDTH, u32_key[i]);

        ret = ot_otp_word_big_endian(u32_key[i], &u8_tmp[i * WORD_BYTE_WIDTH], WORD_BYTE_WIDTH);
        ot_otp_func_fail_return(ot_otp_word_big_endian, ret != TD_SUCCESS, ret);
    }

    crc = (0x0000ffff & ot_otp_crc16_modbus(u8_tmp, OTP_USER_KEY_MAX_BYTES, u8_klen));
    hal_otp_write_word(OTP_USER_KEY_DATA0 + i * WORD_BYTE_WIDTH, crc);

    return TD_SUCCESS;
}

static td_s32 _hal_otp_write_flag_value(hal_otp_flag_index flag_index, td_u32 value, td_u32 lock)
{
    otp_user_flag_value flag_value;

    flag_value.u32 = 0;

    if (flag_index == OTP_USER_FLAG_JTAG_PRT_MODE) {
        ot_otp_formula_fail_return(value > 0x03, OT_ERR_OTP_INVALID_PARAM);

        flag_value.bits.jtag_prt_mode = value;
        if (lock == TD_TRUE) {
            flag_value.bits.jtag_prt_mode_lock_en = 1;
        }
    } else {
        ot_otp_formula_fail_return(value > 0x01, OT_ERR_OTP_INVALID_PARAM);
        switch (flag_index) {
            case OTP_USER_FLAG_SECURE_BOOT_EN:
                flag_value.bits.secure_boot_en = value;
                break;
            case OTP_USER_FLAG_DDR_CA_EN:
                flag_value.bits.ddr_ca_en = value;
                break;
            case OTP_USER_FLAG_JTAG_CA_EN:
                flag_value.bits.jtag_ca_en = value;
                break;
            case OTP_USER_FLAG_UBOOT_REDUNDANCE:
                flag_value.bits.uboot_redundance = value;
                break;
            default:
                ot_otp_error("flag index %d is invalid\n", flag_index);
                return OT_ERR_OTP_INVALID_PARAM;
        }
    }
    _hal_otp_set_flag_value(flag_value.u32);

    return TD_SUCCESS;
}

td_s32 hal_otp_get_key_lock_sta(td_u32 offset, td_bool *lock_sta)
{
    td_s32 ret;
    otp_user_lock_sta0 sta0;

    ret = _hal_otp_get_user_lock_sta(&sta0.u32);
    ot_otp_func_fail_return(_hal_otp_get_user_lock_sta, ret != TD_SUCCESS, ret);

    if ((offset == OTP_USER_KEY0 && sta0.bits.key0_lock == 1) ||
        (offset == OTP_USER_KEY1 && sta0.bits.key1_lock == 1) ||
        (offset == OTP_USER_KEY2 && sta0.bits.key2_lock == 1) ||
        (offset == OTP_USER_KEY3 && sta0.bits.key3_lock == 1) ||
        (offset == OTP_USER_KEY_JTAG_PW_ID && sta0.bits.jtag_pw_id_lock == 1) ||
        (offset == OTP_USER_KEY_JTAG_PW && sta0.bits.jtag_pw_lock == 1) ||
        (offset == OTP_USER_KEY_ROOTKEY && sta0.bits.root_key_lock == 1) ||
        (offset == OTP_USER_KEY_HMACKEY && sta0.bits.hmac_key_lock == 1)) {
        *lock_sta = TD_TRUE;
    } else {
        *lock_sta = TD_FALSE;
    }
    return TD_SUCCESS;
}

td_s32 hal_otp_get_flag_lock_sta(td_u32 offset, td_bool *lock_sta)
{
    td_s32 ret;
    otp_user_lock_sta0 sta0;

    ret = _hal_otp_get_user_lock_sta(&sta0.u32);
    ot_otp_func_fail_return(_hal_otp_get_user_lock_sta, ret != TD_SUCCESS, ret);

    if ((offset == OTP_USER_FLAG_SECURE_BOOT_EN && sta0.bits.secure_boot_en_lock == 1) ||
        (offset == OTP_USER_FLAG_DDR_CA_EN && sta0.bits.ddr_ca_en_lock == 1) ||
        (offset == OTP_USER_FLAG_JTAG_CA_EN && sta0.bits.jtag_ca_en_lock == 1) ||
        (offset == OTP_USER_FLAG_JTAG_PRT_MODE && sta0.bits.jtag_prt_mode_lock == 1) ||
        (offset == OTP_USER_FLAG_UBOOT_REDUNDANCE && sta0.bits.uboot_redundance_lock == 1)) {
        *lock_sta = TD_TRUE;
    } else {
        *lock_sta = TD_FALSE;
    }

    return TD_SUCCESS;
}

td_s32 hal_otp_set_user_data_word(td_u32 index, td_u32 value, td_bool lock)
{
    td_s32 ret;
    otp_user_ctrl_sta ctrl_sta;

    ot_unused(lock);

    /* 1. wait otp ctrl free */
    ret = _hal_otp_wait_op_free();
    ot_otp_func_fail_return(_hal_otp_wait_op_free, ret != TD_SUCCESS, ret);

    /* 2. set rev addr */
    _hal_otp_set_rev_addr(index);

    /* 3. write rev data */
    _hal_otp_set_rev_data(value);

    /* 4. set work mode to write user room */
    _hal_otp_set_work_mode(OTP_WORK_MODE_WRITE_USER_ROOM);

    /* 5. start otp ctrl */
    _hal_otp_op_start();

    /* 6. wait user cmd finish & check lock status */
    ret = _hal_otp_wait_cmd_finish();
    ot_otp_func_fail_return(_hal_otp_wait_cmd_finish, ret != TD_SUCCESS, ret);
    _hal_otp_read_ctrl_sta(&ctrl_sta.u32);
    if (ctrl_sta.bits.otp_user_lock_err == 1) {
        ot_otp_error("index %u has locked\n", index);
        return OT_ERR_OTP_ZONE_LOCKED;
    }
    return ret;
}

td_s32 hal_otp_get_user_data_word(td_u32 index, td_u32 *value)
{
    td_s32 ret;

    /* 1. wait otp ctrl free */
    ret = _hal_otp_wait_op_free();
    ot_otp_func_fail_return(_hal_otp_wait_op_free, ret != TD_SUCCESS, ret);

    /* 2. set rev addr */
    _hal_otp_set_rev_addr(index);

    /* 3. set work mode to read user room */
    _hal_otp_set_work_mode(OTP_WORK_MODE_READ_USER_ROOM);

    /* 4. start otp ctrl */
    _hal_otp_op_start();

    /* 5. wait user cmd finish */
    ret = _hal_otp_wait_cmd_finish();
    ot_otp_func_fail_return(_hal_otp_wait_cmd_finish, ret != TD_SUCCESS, ret);

    /* 6. read rev data */
    _hal_otp_read_rev_data(value);

    return ret;
}

td_s32 hal_otp_load_key_to_klad(td_u32 key_index)
{
    td_s32 ret;

    ot_otp_enter();

    /* 1. wait otp ctrl free */
    ret = _hal_otp_wait_op_free();
    ot_otp_func_fail_return(_hal_otp_wait_op_free, ret != TD_SUCCESS, ret);

    /* 2. set key index */
    _hal_otp_set_key_index(key_index);

    /* 3. set work mode to load key */
    _hal_otp_set_work_mode(OTP_WORK_MODE_LOAD_CIPHER_KEY);

    /* 4. start otp ctrl */
    _hal_otp_op_start();

    /* 5. wait user cmd finish */
    ret = _hal_otp_wait_cmd_finish();
    ot_otp_func_fail_return(_hal_otp_wait_cmd_finish, ret != TD_SUCCESS, ret);

    ot_otp_exit();

    return ret;
}

td_s32 hal_otp_verify_key(td_u32 key_index, td_bool *flag)
{
    td_s32 ret;
    otp_user_ctrl_sta ctrl_sta;

    ot_otp_enter();

    /* 1. wait otp ctrl free */
    ret = _hal_otp_wait_op_free();
    ot_otp_func_fail_return(_hal_otp_wait_op_free, ret != TD_SUCCESS, ret);

    /* 2. set key index */
    _hal_otp_set_key_index(key_index);

    /* 3. set work mode to verify key crc */
    _hal_otp_set_work_mode(OTP_WORK_MODE_VERIFY_KEY_CRC);

    /* 4. start otp ctrl */
    _hal_otp_op_start();

    /* 5. wait user cmd finish & check key crc flag */
    ret = _hal_otp_wait_cmd_finish();
    ot_otp_func_fail_return(_hal_otp_wait_cmd_finish, ret != TD_SUCCESS, ret);

    _hal_otp_read_ctrl_sta(&ctrl_sta.u32);
    if (ctrl_sta.bits.key_crc_check_ok_flag == 1) {
        *flag = TD_TRUE;
        ot_otp_info("verify key crc %d ok\n", key_index);
    } else {
        *flag = TD_FALSE;
        ot_otp_info("verify key crc %d failed\n", key_index);
    }

    ot_otp_exit();

    return ret;
}

td_s32 hal_otp_burn_key(const otp_data_item *data_item, const td_u8 *key, td_u32 klen)
{
    td_s32 ret;
    td_bool lock_sta;

    ot_otp_enter();

    /* 1. check user key status */
    ret = hal_otp_get_key_lock_sta(data_item->offset, &lock_sta);
    ot_otp_func_fail_return(hal_otp_get_key_lock_sta, ret != TD_SUCCESS, ret);
    if (lock_sta == TD_TRUE) {
        ot_otp_error("key slot %s had locked\n", data_item->field_name);
        return OT_ERR_OTP_ZONE_ALREADY_SET;
    }

    /* 2. wait otp ctrl free */
    ret = _hal_otp_wait_op_free();
    ot_otp_func_fail_return(_hal_otp_wait_op_free, ret != TD_SUCCESS, ret);

    /* 3. set user key index */
    _hal_otp_set_key_index(data_item->offset);

    /* 4. write key data */
    ret = _hal_otp_write_key_data(key, klen);
    ot_otp_func_fail_return(_hal_otp_write_key_data, ret != TD_SUCCESS, ret);

    /* 5. set work mode */
    _hal_otp_set_work_mode(OTP_WORK_MODE_BURN_KEY);

    /* 6. start otp ctrl */
    _hal_otp_op_start();

    /* 7. wait user cmd finish */
    ret = _hal_otp_wait_cmd_finish();
    ot_otp_func_fail_return(_hal_otp_wait_cmd_finish, ret != TD_SUCCESS, ret);

    ot_otp_exit();

    return ret;
}

td_s32 hal_otp_enable_flag(const otp_data_item *data_item, td_u32 flag, td_bool lock)
{
    td_s32 ret;
    td_bool lock_sta;

    ot_otp_enter();

    /* 1. check flag status */
    ret = hal_otp_get_flag_lock_sta(data_item->offset, &lock_sta);
    ot_otp_func_fail_return(hal_otp_get_flag_lock_sta, ret != TD_SUCCESS, ret);
    if (lock_sta == TD_TRUE) {
        ot_otp_error("flag %s had locked!!!\n", data_item->field_name);
        return OT_ERR_OTP_ZONE_ALREADY_SET;
    }

    /* 2. wait otp ctrl free */
    ret = _hal_otp_wait_op_free();
    ot_otp_func_fail_return(_hal_otp_wait_op_free, ret != TD_SUCCESS, ret);

    /* 3. set flag index */
    _hal_otp_set_flag_index(data_item->offset);

    /* 4. set flag value */
    ret = _hal_otp_write_flag_value(data_item->offset, flag, lock);
    ot_otp_func_fail_return(_hal_otp_write_flag_value, ret != TD_SUCCESS, ret);

    /* 5. set work mode to enable flag config */
    _hal_otp_set_work_mode(OTP_WORK_MODE_ENABLE_FLAG_CFG);

    /* 6. start otp ctrl */
    _hal_otp_op_start();

    /* 7. wait user cmd finish */
    ret = _hal_otp_wait_cmd_finish();
    ot_otp_func_fail_return(_hal_otp_wait_cmd_finish, ret != TD_SUCCESS, ret);

    ot_otp_exit();

    return ret;
}

#endif
