// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_lib.h"
#ifdef KLAD_SWITCH_CPU
#include "ot_debug_klad.h"
#include "ot_common_klad.h"

static td_bool s_klad_secure_cpu = TD_FALSE;
#endif

td_s32 klad_get_cpu_secure_sta(td_void)
{
#ifdef KLAD_SWITCH_CPU
    if (check_otp_cmd_mode()) {
        s_klad_secure_cpu = TD_TRUE;
    } else {
        s_klad_secure_cpu = TD_FALSE;
    }
#endif
    return TD_SUCCESS;
}

/* KLAD_SECURE_CPU force to tee cpu
 * KLAD_SWITCH_CPU can switch to ree or tee cpu
 * else default ree cpu
 */
td_bool klad_is_secure_cpu(td_void)
{
#if defined(KLAD_SECURE_CPU)
    return TD_TRUE;
#elif defined(KLAD_SWITCH_CPU)
    return s_klad_secure_cpu;
#else
    return TD_FALSE;
#endif
}

