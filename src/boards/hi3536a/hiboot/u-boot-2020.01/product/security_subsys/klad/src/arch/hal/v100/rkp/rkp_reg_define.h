// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef RKP_REG_DEFINE_H
#define RKP_REG_DEFINE_H

/* Define the union rkp_init_stat */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    oem_jtag_deob_done    : 1; /* [0]  */
        unsigned int    ot_rk_deob_done     : 1; /* [1]  */
        unsigned int    func_jtag_kdf_done    : 1; /* [2]  */
        unsigned int    soc_jtag_kdf_done     : 1; /* [3]  */
        unsigned int    npu_jtag_kdf_done     : 1; /* [4]  */
        unsigned int    tee_prv_kdf_done      : 1; /* [5]  */
        unsigned int    reserved_0            : 26; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_init_stat;

/* Define the union rkp_dbg_stat */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    oem_rk_vld            : 4; /* [3..0]  */
        unsigned int    reserved_0            : 4; /* [7..4]  */
        unsigned int    ot_rk_vld           : 1; /* [8]  */
        unsigned int    reserved_1            : 7; /* [15..9]  */
        unsigned int    vmask_busy            : 1; /* [16]  */
        unsigned int    deob_busy             : 1; /* [17]  */
        unsigned int    kdf_busy              : 1; /* [18]  */
        unsigned int    reserved_2            : 13; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_dbg_stat;

/* Define the union rkp_state */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_init_rdy          : 1; /* [0]  */
        unsigned int    reserved_0            : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_state;

/* Define the union rkp_power_en */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_power_en          : 1; /* [0]  */
        unsigned int    reserved_0            : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_power_en;

/* Define the union rkp_alarm_info */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_crc4_alarm        : 1; /* [0]  */
        unsigned int    rkp_fsm_alarm         : 1; /* [1]  */
        unsigned int    rkp_core_alarm        : 1; /* [2]  */
        unsigned int    reserved_0            : 29; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_alarm_info;

/* Define the union rkp_int_en */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_int_en            : 1; /* [0]  */
        unsigned int    reserved_0            : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_int_en;

/* Define the union rkp_int_raw */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_int_raw           : 1; /* [0]  */
        unsigned int    reserved_0            : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_int_raw;

/* Define the union rkp_int */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_int               : 1; /* [0]  */
        unsigned int    reserved_0            : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_int;

/* Define the union rkp_sec_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_calc_type         : 1; /* [0]  */
        unsigned int    reserved_0            : 3; /* [3..1]  */
        unsigned int    rkp_static_sel        : 1; /* [4]  */
        unsigned int    reserved_1            : 27; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_sec_cfg;

/* Define the union rkp_calc_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_calc_start        : 1; /* [0]  */
        unsigned int    reserved_0            : 3; /* [3..1]  */
        unsigned int    rkp_rk_sel            : 3; /* [6..4]  */
        unsigned int    reserved_1            : 1; /* [7]  */
        unsigned int    rkp_remap             : 1; /* [8]  */
        unsigned int    reserved_2            : 23; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_calc_ctrl;

/* Define the union rkp_error */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rk_invld_err          : 1; /* [0]  */
        unsigned int    kl_busy_err           : 1; /* [1]  */
        unsigned int    kl_access_err         : 1; /* [2]  */
        unsigned int    reserved_0            : 29; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_error;

/* Define the union rkp_crc */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rkp_vmask_crc         : 8; /* [7..0]  */
        unsigned int    rkp_deob_crc          : 8; /* [15..8]  */
        unsigned int    rkp_kdf_crc           : 8; /* [23..16]  */
        unsigned int    reserved_0            : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} rkp_crc;

#endif /* RKP_REG_DEFINE_H */

