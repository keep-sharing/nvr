// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hal_otp_comm.h"
#include "drv_osal_init.h"
#include "otp_reg_base.h"
#include "ot_debug_otp.h"
#include "ot_common_otp.h"

static td_u8 *g_otp_base_vir_addr = TD_NULL;

td_s32 hal_otp_init(td_void)
{
#ifdef OT_OTP_V100
    td_u32 crg_value;
    td_u32 *sys_addr = TD_NULL;

    /* otp clock configure */
    sys_addr = otp_ioremap_nocache(REG_SYS_OTP_CLK_ADDR_PHY, 0x100);
    ot_otp_func_fail_return(otp_ioremap_nocache, sys_addr == TD_NULL, OT_ERR_OTP_FAILED_MEM);

    crg_value = otp_read(sys_addr);
    crg_value |= OTP_CRG_CLOCK_BIT;   /* set the bit 0, clock opened */
    otp_write(sys_addr, crg_value);

    otp_iounmap(sys_addr, 0x100);
#endif

    /* otp reg addr map */
    g_otp_base_vir_addr = otp_ioremap_nocache(OTP_REG_BASE_ADDR_PHY, OTP_REG_BASE_ADDR_SIZE);
    ot_otp_func_fail_return(otp_ioremap_nocache, g_otp_base_vir_addr == TD_NULL, OT_ERR_OTP_FAILED_MEM);

    return TD_SUCCESS;
}

td_void hal_otp_deinit(td_void)
{
    if (g_otp_base_vir_addr != TD_NULL) {
        otp_iounmap(g_otp_base_vir_addr, OTP_REG_BASE_ADDR_SIZE);
        g_otp_base_vir_addr = TD_NULL;
    }
}

td_s32 hal_otp_read_word(td_u32 offset, td_u32 *value)
{
    ot_otp_formula_fail_return(value == TD_NULL, OT_ERR_OTP_NULL_PTR);
    ot_otp_formula_fail_return(g_otp_base_vir_addr == TD_NULL, OT_ERR_OTP_NULL_PTR);

    *value = otp_read((volatile td_u8 *)(g_otp_base_vir_addr + (offset)));

    ot_otp_info(" read  offset %04X value %x\n", offset, *value);
    return TD_SUCCESS;
}

td_s32 hal_otp_write_word(td_u32 offset, td_u32 value)
{
    ot_otp_formula_fail_return(g_otp_base_vir_addr == TD_NULL, OT_ERR_OTP_NULL_PTR);

    otp_write((volatile td_u8 *)(g_otp_base_vir_addr + (offset)), value);

    ot_otp_info("write offset %04X value %x\n", offset, value);
    return TD_SUCCESS;
}
