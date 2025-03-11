// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef OT_DEBUG_OTP_H
#define OT_DEBUG_OTP_H

#include "ot_type.h"
#include "ot_debug.h"

#define OT_OTP_LVL_ERR     0
#define OT_OTP_LVL_WARN    1
#define OT_OTP_LVL_INFO    2
#define OT_OTP_LVL_LOG     3

#define otp_trace(level, msg, fmt, ...)                                                             \
    do {                                                                                            \
        if (OT_OTP_DEBUG >= level) {                                                                \
            OT_PRINT("[%4s Line:%04d Func:%s] "fmt, msg, __LINE__, __FUNCTION__, ##__VA_ARGS__);    \
        }                                                                                           \
    } while (0)

#define ot_otp_error(fmt, ...)      otp_trace(OT_OTP_LVL_ERR, "ERR", fmt, ##__VA_ARGS__)
#if OT_OTP_DEBUG
#define ot_otp_warn(fmt, ...)       otp_trace(OT_OTP_LVL_WARN, "WARN", fmt, ##__VA_ARGS__)
#define ot_otp_info(fmt, ...)       otp_trace(OT_OTP_LVL_INFO, "INFO", fmt, ##__VA_ARGS__)
#define ot_otp_enter()              otp_trace(OT_OTP_LVL_LOG, "LOG", "enter--->\n")
#define ot_otp_exit()               otp_trace(OT_OTP_LVL_LOG, "LOG", "exit <---\n")
#else
#define ot_otp_warn(fmt, ...)
#define ot_otp_info(fmt, ...)
#define ot_otp_enter()
#define ot_otp_exit()
#endif

#define ot_otp_func_fail_return(_func, _formula, _errno) \
    do { \
        if (_formula) { \
            ot_otp_error("call %s failed, ret = 0x%08X\n", # _func, _errno); \
            return _errno; \
        } \
    } while (0)

#define ot_otp_formula_fail_return(_formula, _errno) \
    do { \
        if (_formula) { \
            ot_otp_error("%s is invalid, ret = 0x%08X\n", # _formula, _errno); \
            return _errno; \
        } \
    } while (0)

#endif /* OT_DEBUG_OTP_H */
