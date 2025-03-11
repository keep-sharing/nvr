// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_lib.h"
#include "drv_ioctl_otp.h"
#include "linux/compat.h"
#include "ot_debug_otp.h"
#if defined(OTP_SWITCH_CPU) || defined(OTP_SECURE_CPU)
#include "otp_reg_base.h"
#endif

#ifdef OTP_SWITCH_CPU
static td_bool s_otp_secure_cpu = TD_FALSE;
#endif

#ifdef OTP_TEE_REG_OFFSET
td_u32 g_tee_reg_offset = 0;
#endif

td_void ot_otp_print_arr_u8(const td_char *name, const td_u8 *msg, td_u32 msg_len)
{
#if OT_OTP_DEBUG
    td_u32 i;

    if (name != TD_NULL) {
        OT_PRINT("%s: \n", name);
    }

    for (i = 0; i < msg_len; i++) {
        if (i != 0 && align_16_bytes(i)) {
            OT_PRINT("\n");
        }
        OT_PRINT("%02X ", msg[i]);
    }
    OT_PRINT("\n");
#else
    ot_unused(name);
    ot_unused(msg);
    ot_unused(msg_len);
#endif
}

td_void ot_otp_print_arr_u32(const td_char *name, const td_u32 *msg, td_u32 msg_len)
{
#if OT_OTP_DEBUG
    td_u32 i;

    if (name != TD_NULL) {
        OT_PRINT("%s: \n", name);
    }

    for (i = 0; i < msg_len; i++) {
        if (i != 0 && align_16_bytes(i)) {
            OT_PRINT("\n");
        }
        OT_PRINT("%08X ", msg[i]);
    }
    OT_PRINT("\n");
#else
    ot_unused(name);
    ot_unused(msg);
    ot_unused(msg_len);
#endif
}

td_s32 ot_otp_word_big_endian(const td_u32 word_val, td_u8 *byte_buf, td_u32 len)
{
    ot_otp_formula_fail_return(len != WORD_BYTE_WIDTH, OT_ERR_OTP_INVALID_PARAM);

    byte_buf[WORD_IDX_0] = (word_val >> OFFSET_3_BYTE) & 0xff;
    byte_buf[WORD_IDX_1] = (word_val >> OFFSET_2_BYTE) & 0xff;
    byte_buf[WORD_IDX_2] = (word_val >> OFFSET_1_BYTE) & 0xff;
    byte_buf[WORD_IDX_3] = (word_val >> OFFSET_0_BYTE) & 0xff;

    return TD_SUCCESS;
}

td_u16 ot_otp_crc16_modbus(const td_u8 *msg, td_u32 msg_len, td_u32 klen)
{
    td_u32 i;
    td_u16 crc_in = 0xFFFF;
    const td_u16 crc_poly = 0x8005;
    td_u8 crc_char;

    while (klen-- != 0) {
        crc_char = *(msg++);
        crc_in ^= (crc_char << OFFSET_1_BYTE);
        for (i = 0; i < BYTE_BIT_WIDTH; i++) {
            if (crc_in & 0x8000) {
                crc_in = (crc_in << 1) ^ crc_poly;
            } else {
                crc_in = crc_in << 1;
            }
        }
    }
    ot_unused(msg_len);
    return crc_in;
}

td_s32 otp_copy_from_user(td_void *to, unsigned long to_len,
    const td_void *from, unsigned long from_len)
{
    td_s32 ret;
    if (from_len == 0) {
        return TD_SUCCESS;
    }

    ot_otp_formula_fail_return(to == TD_NULL, OT_ERR_OTP_NULL_PTR);
    ot_otp_formula_fail_return(from == TD_NULL, OT_ERR_OTP_NULL_PTR);
    ot_otp_formula_fail_return(from_len > to_len, OT_ERR_OTP_INVALID_PARAM);

    ret = copy_from_user(to, from, from_len);
    ot_otp_func_fail_return(copy_from_user, ret != TD_SUCCESS, OT_ERR_OTP_INVALID_PARAM);

    return TD_SUCCESS;
}

td_s32 otp_copy_to_user(td_void *to, unsigned long to_len,
    const td_void *from, unsigned long from_len)
{
    td_s32 ret;
    if (from_len == 0) {
        return TD_SUCCESS;
    }

    ot_otp_formula_fail_return(to == TD_NULL, OT_ERR_OTP_NULL_PTR);
    ot_otp_formula_fail_return(from == TD_NULL, OT_ERR_OTP_NULL_PTR);
    ot_otp_formula_fail_return(from_len > to_len, OT_ERR_OTP_INVALID_PARAM);

    ret = copy_from_user(to, from, from_len);
    ot_otp_func_fail_return(copy_from_user, ret != TD_SUCCESS, OT_ERR_OTP_INVALID_PARAM);

    return TD_SUCCESS;
}

td_s32 otp_get_cpu_secure_sta(td_void)
{
#if defined(OTP_SECURE_CPU)
#ifdef OTP_TEE_REG_OFFSET
    g_tee_reg_offset = OTP_TEE_REG_OFFSET;
#endif
#elif defined(OTP_SWITCH_CPU)
    if (check_otp_cmd_mode()) {
        s_otp_secure_cpu = TD_TRUE;
#ifdef OTP_TEE_REG_OFFSET
        g_tee_reg_offset = OTP_TEE_REG_OFFSET;
#endif
    } else {
        s_otp_secure_cpu = TD_FALSE;
    }
#endif
    return TD_SUCCESS;
}

/* OTP_SECURE_CPU force to tee cpu
 * OTP_SWITCH_CPU can switch to ree or tee cpu
 * else default ree cpu
 */
td_bool otp_is_secure_cpu(td_void)
{
#if defined(OTP_SECURE_CPU)
    return TD_TRUE;
#elif defined(OTP_SWITCH_CPU)
    return s_otp_secure_cpu;
#else
    return TD_FALSE;
#endif
}

