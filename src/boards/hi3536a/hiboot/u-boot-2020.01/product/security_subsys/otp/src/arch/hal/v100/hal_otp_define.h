// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_OTP_DEFINE_H
#define HAL_OTP_DEFINE_H

/* Define the union otp_user_work_mode */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int otp_user_work_mode : 3; /* [2..0]  */
        unsigned int reserved_0 : 29;        /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_user_work_mode;

/* Define the union otp_user_key_index */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int otp_index : 3;   /* [2..0]  */
        unsigned int reserved_0 : 29; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_user_key_index;

/* Define the union otp_user_flag_value */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int secure_boot_en : 1;         /* [0]  */
        unsigned int ddr_ca_en : 1;              /* [1]  */
        unsigned int jtag_ca_en : 1;             /* [2]  */
        unsigned int reserved_0 : 1;             /* [3]  */
        unsigned int jtag_prt_mode : 2;          /* [5..4]  */
        unsigned int jtag_prt_mode_lock_en : 1;  /* [6]  */
        unsigned int secure_iso_en : 1;          /* [7]  */
        unsigned int uboot_redundance : 1;       /* [8]  */
        unsigned int reserved_flag1 : 1;         /* [9]  */
        unsigned int reserved_flag2 : 21;        /* [30..10]  */
        unsigned int reserved_flag2_lock_en : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_user_flag_value;

/* Define the union otp_user_flag_index */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int flag_index : 3;  /* [2..0]  */
        unsigned int reserved_0 : 29; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_user_flag_index;

/* Define the union otp_user_rev_addr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int otp_user_reserved_addr : 10; /* [9..0]  */
        unsigned int reserved_0 : 22;             /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_user_rev_addr;

/* Define the union otp_user_lock_sta0 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int secure_boot_en_lock : 1;   /* [0]  */
        unsigned int ddr_ca_en_lock : 1;        /* [1]  */
        unsigned int jtag_pw_id_lock : 1;       /* [2] */
        unsigned int jtag_pw_lock : 1;          /* [3] */
        unsigned int root_key_lock : 1;         /* [4]  */
        unsigned int key0_lock : 1;             /* [5]  */
        unsigned int key1_lock : 1;             /* [6]  */
        unsigned int key2_lock : 1;             /* [7]  */
        unsigned int key3_lock : 1;             /* [8]  */
        unsigned int jtag_ca_en_lock : 1;       /* [9]  */
        unsigned int jtag_prt_mode_lock : 1;    /* [10]  */
        unsigned int reserved_2 : 1;            /* [11]  */
        unsigned int uboot_redundance_lock : 1; /* [12]  */
        unsigned int reserved_flag1_lock : 1;   /* [13]  */
        unsigned int reserved_flag2_lock : 1;   /* [14]  */
        unsigned int hmac_key_lock : 1;         /* [15]  */
        unsigned int reserved_3 : 16;           /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_user_lock_sta0;

/* Define the union otp_user_ctrl_sta */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int otp_op_busy : 1;           /* [0]  */
        unsigned int otp_user_cmd_finish : 1;   /* [1]  */
        unsigned int otp_user_lock_err : 1;     /* [2]  */
        unsigned int key_rd_back_check_err : 1; /* [3]  */
        unsigned int key_crc_check_ok_flag : 1; /* [4]  */
        unsigned int user_rev_rd_finish : 1;    /* [5]  */
        unsigned int user_rev_pgm_finish : 1;   /* [6]  */
        unsigned int flag_pgm_finish : 1;       /* [7]  */
        unsigned int key_crc_check_finish : 1;  /* [8]  */
        unsigned int key_pgm_finish : 1;        /* [9]  */
        unsigned int key_load_finish : 1;       /* [10]  */
        unsigned int rd_lock_sta_finish : 1;    /* [11]  */
        unsigned int key_index_finish : 4;      /* [15..12]  */
        unsigned int user_rev_addr_finish : 8;  /* [23..16]  */
        unsigned int flag_index_finish : 2;     /* [25..24]  */
        unsigned int reserved_0 : 6;            /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_user_ctrl_sta;

/* Define the union otp_flag_sta */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int uboot_redundance : 1; /* [0]  */
        unsigned int reserved_flag1 : 1;   /* [1]  */
        unsigned int reserved_flag2 : 21;  /* [22..2]  */
        unsigned int reserved_0 : 1;       /* [23]  */
        unsigned int jtag_prt_mode : 2;    /* [25..24]  */
        unsigned int jtag_ca_en : 1;       /* [26]  */
        unsigned int reserved_1 : 5;       /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} otp_flag_sta;

#endif /* HAL_OTP_DEFINE_H */
