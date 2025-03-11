// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __OT_COMMON_H__
#define __OT_COMMON_H__

#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define ot_unused(x)             ((td_void)(x))

#define OT_MPP_MOD_CIPHER         "cipher"
#define OT_MPP_MOD_KLAD           "klad"
#define OT_MPP_MOD_OTP            "otp"

#define OT_INVALID_HANDLE      (-1)

typedef enum {
    OT_ID_CIPHER       = 71,
    OT_ID_KLAD         = 72,
    OT_ID_KEYSLOT      = 73,
    OT_ID_OTP          = 74,
    OT_ID_BUTT,
} ot_mod_id;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* __OT_COMMON_H__ */
