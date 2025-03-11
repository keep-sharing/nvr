// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_LIB_H
#define DRV_LIB_H

#include "ot_type.h"

td_s32 klad_get_cpu_secure_sta(td_void);

td_bool klad_is_secure_cpu(td_void);

td_u32 ot_klad_get_klad_irq(td_void);

td_u32 ot_klad_get_rkp_irq(td_void);

extern int check_otp_cmd_mode(void);

/* must match kernel dts */
#define RKP_INT_NAME        (klad_is_secure_cpu() ? "sec_rkp" : "nsec_rkp")

#define KLAD_INT_NAME       (klad_is_secure_cpu() ? "sec_klad" : "nsec_klad")

#endif /* DRV_LIB_H */
