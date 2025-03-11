// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __JPEGD_REG_H__
#define __JPEGD_REG_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/* define the union u_jpeg_dec_start */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jpeg_dec_start : 1; /* [0]  */
        unsigned int rst_busy : 1;       /* [1]  */
        unsigned int axi_sep_bypass : 1; /* [2]  */
        unsigned int reserved_0 : 1;     /* [3]  */
        unsigned int axi_sep_typ : 1;    /* [4]  */
        unsigned int reserved_1 : 27;    /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_jpeg_dec_start;

/* define the union u_jpeg_resume_start */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jpeg_resume_start : 1;  /* [0] */
        unsigned int last_resume_in_pic : 1; /* [1] */
        unsigned int reserved : 30;          /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_jpeg_resume_start;

/* define the union u_jpeg_stride */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int y_stride : 16;  /* [15..0] */
        unsigned int uv_stride : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_jpeg_stride;

/* define the union u_picture_size */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int pic_width_in_mcu : 12;  /* [11..0] */
        unsigned int reserved_0 : 4;         /* [15..12] */
        unsigned int pic_height_in_mcu : 12; /* [27..16] */
        unsigned int reserved_1 : 4;         /* [31..28] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_picture_size;

/* define the union picture_type */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int pic_type : 3;  /* [2..0] */
        unsigned int reserved : 29; /* [31..3] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_picture_type;

/* define the union u_rgb_out_stride */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jdo_out_stride : 16; /* [15..0] */
        unsigned int reserved : 16;            /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_rgb_out_stride;

/* define the union u_pd_sum_cfg_cbcr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jvld_pd_cb : 16; /* [15..0] */
        unsigned int jcfg2jvld_pd_cr : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_pd_sum_cfg_cbcr;

/* define the union u_pd_sum_cfg_y */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jvld_pd_y : 16;   /* [15..0] */
        unsigned int jcfg2jbs_flag_cur : 1; /* [16] */
        unsigned int reserved : 15;         /* [31..17] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_pd_sum_cfg_y;

/* define the union u_freq_scale */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int freq_scale : 2;    /* [1..0]  */
        unsigned int jidct_emar_en : 1; /* [2] */
        unsigned int ck_gt_en : 1;      /* [3]  */
        unsigned int outstanding : 4;   /* [7..4]  */
        unsigned int axi_id : 4;        /* [11..8] */
        unsigned int reserved : 20;     /* [31..12] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_freq_scale;

/* define the union u_out_type */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int out_rgb_type_old : 2; /* [1..0]  */
        unsigned int jidct_yuv420_en : 1;  /* [2]  */
        unsigned int hor_med_en : 1;       /* [3]  */
        unsigned int ver_med_en : 1;       /* [4]  */
        unsigned int dither_en : 1;        /* [5]  */
        unsigned int reserved_0 : 2;       /* [7..6]  */
        unsigned int out_rgb_type : 3;     /* [10..8]  */
        unsigned int reserved_1 : 21;      /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_out_type;

/* Define the union U_JPGE_ALPHA; */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jpgd_a : 8;    /* [7..0] */
        unsigned int reserved : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_jpgd_alpha;

/* define the union u_hor_phase0_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase0_coef0 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase0_coef1 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase0_coef01;

/* define the union u_hor_phase0_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase0_coef2 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase0_coef3 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase0_coef23;

/* define the union u_hor_phase0_coef45 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase0_coef4 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase0_coef5 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase0_coef45;

/* define the union u_hor_phase0_coef67 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase0_coef6 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase0_coef7 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase0_coef67;

/* define the union u_hor_phase2_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase2_coef0 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase2_coef1 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase2_coef01;

/* define the union u_hor_phase2_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase2_coef2 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase2_coef3 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase2_coef23;

/* define the union u_hor_phase2_coef45 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase2_coef4 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase2_coef5 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase2_coef45;

/* define the union u_hor_phase2_coef67 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hor_phase2_coef6 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int hor_phase2_coef7 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hor_phase2_coef67;

/* define the union u_ver_phase0_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ver_phase0_coef0 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int ver_phase0_coef1 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_ver_phase0_coef01;

/* define the union u_ver_phase0_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ver_phase0_coef2 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int ver_phase0_coef3 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_ver_phase0_coef23;

/* define the union u_ver_phase2_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ver_phase2_coef0 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int ver_phase2_coef1 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_ver_phase2_coef01;

/* define the union u_ver_phase2_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ver_phase2_coef2 : 10; /* [9..0] */
        unsigned int reserved_0 : 6;        /* [15..10] */
        unsigned int ver_phase2_coef3 : 10; /* [25..16] */
        unsigned int reserved_1 : 6;        /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_ver_phase2_coef23;

/* define the union u_csc_in_dc_coef */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int csc_in_dc_coef0 : 9; /* [8..0] */
        unsigned int reserved_0 : 1;      /* [9] */
        unsigned int csc_in_dc_coef1 : 9; /* [18..10] */
        unsigned int reserved_1 : 1;      /* [19] */
        unsigned int csc_in_dc_coef2 : 9; /* [28..20] */
        unsigned int reserved_2 : 3;      /* [31..29] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_csc_in_dc_coef;

/* define the union u_csc_out_dc_coef */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int csc_out_dc_coef0 : 9; /* [8..0] */
        unsigned int reserved_0 : 1;       /* [9] */
        unsigned int csc_out_dc_coef1 : 9; /* [18..10] */
        unsigned int reserved_1 : 1;       /* [19] */
        unsigned int csc_out_dc_coef2 : 9; /* [28..20] */
        unsigned int reserved_2 : 3;       /* [31..29] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_csc_out_dc_coef;

/* define the union u_csc_trans_coef0 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int csc_p_00 : 15;  /* [14..0] */
        unsigned int reserved_0 : 1; /* [15] */
        unsigned int csc_p_01 : 15;  /* [30..16] */
        unsigned int reserved_1 : 1; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_csc_trans_coef0;

/* define the union u_csc_trans_coef1 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int csc_p_02 : 15;  /* [14..0] */
        unsigned int reserved_0 : 1; /* [15] */
        unsigned int csc_p_10 : 15;  /* [30..16] */
        unsigned int reserved_1 : 1; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_csc_trans_coef1;

/* define the union u_csc_trans_coef2 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int csc_p_11 : 15;  /* [14..0] */
        unsigned int reserved_0 : 1; /* [15] */
        unsigned int csc_p_12 : 15;  /* [30..16] */
        unsigned int reserved_1 : 1; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_csc_trans_coef2;

/* define the union u_csc_trans_coef3 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int csc_p_20 : 15;  /* [14..0] */
        unsigned int reserved_0 : 1; /* [15] */
        unsigned int csc_p_21 : 15;  /* [30..16] */
        unsigned int reserved_1 : 1; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_csc_trans_coef3;

/* define the union u_csc_trans_coef4 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int csc_p_22 : 15; /* [14..0] */
        unsigned int reserved : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_csc_trans_coef4;

/* define the union u_mcu_pro_startpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jvld_mcu_starty : 12; /* [11..0] */
        unsigned int reserved : 20;             /* [31..12] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_mcu_pro_startpos;

/* define the union u_piccut_startpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jytr_pic_startx : 15; /* [14..0]  */
        unsigned int reserved_0 : 1;            /* [15]  */
        unsigned int jcfg2jytr_pic_starty : 15; /* [30..16]  */
        unsigned int reserved_1 : 1;            /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_piccut_startpos;

/* define the union u_piccut_endpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jytr_pic_endx : 15; /* [14..0]  */
        unsigned int reserved_0 : 1;          /* [15]  */
        unsigned int jcfg2jytr_pic_endy : 15; /* [30..16]  */
        unsigned int reserved_1 : 1;          /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_piccut_endpos;

/* define the union u_mcu_pro_curpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jvld2jcfg_mcu_y : 12; /* [11..0] */
        unsigned int reserved : 20;        /* [31..12] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_mcu_pro_curpos;

/* define the union u_bs_res_bit_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jbs_bs_remain : 7; /* [6..0] */
        unsigned int reserved : 25;          /* [31..7] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_bs_res_bit_cfg;

/* define the union u_jpeg_int */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int int_dec_fnsh : 1;  /* [0] */
        unsigned int int_dec_err : 1;   /* [1] */
        unsigned int int_bs_res : 1;    /* [2] */
        unsigned int debug_state : 1;   /* [3] */
        unsigned int int_over_time : 1; /* [4] */
        unsigned int reserved : 27;     /* [31..5] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_jpeg_int;

/* define the union u_int_mask */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int intm_dec_fnsh : 1;  /* [0] */
        unsigned int intm_dec_err : 1;   /* [1] */
        unsigned int intm_bs_res : 1;    /* [2] */
        unsigned int reserved_0 : 1;     /* [3] */
        unsigned int intm_over_time : 1; /* [4] */
        unsigned int reserved_1 : 27;    /* [31..5] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_int_mask;

/* define the union u_pd_sum_y */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jvld2jcfg_pd_y : 16;   /* [15..0] */
        unsigned int jbs2jcfg_flag_cur : 1; /* [16] */
        unsigned int reserved : 15;         /* [31..17] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_pd_sum_y;

/* define the union u_pd_sum_cbcr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jvld2jcfg_pd_cb : 16; /* [15..0] */
        unsigned int jvld2jcfg_pd_cr : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_pd_sum_cbcr;

/* define the union u_luma_pix_sum1 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int luma_pix_sum1 : 4;     /* [3..0] */
        unsigned int reserved : 27;         /* [30..4] */
        unsigned int jidct_luma_sum_en : 1; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_luma_pix_sum1;

/* define the union u_axi_4k_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int wr_over_4k_cnt : 16; /* [15..0] */
        unsigned int rd_over_4k_cnt : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_axi_4k_cnt;

/* define the union u_axi_16m_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int wr_over_16m_cnt : 16; /* [15..0] */
        unsigned int rd_over_16m_cnt : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_axi_16m_cnt;

/* define the union u_sampling_factor */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int v_fac : 8;    /* [7..0] */
        unsigned int u_fac : 8;    /* [15..8] */
        unsigned int y_fac : 8;    /* [23..16] */
        unsigned int reserved : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_sampling_factor;

/* define the union u_dri */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jvld_ri : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;   /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_dri;

/* define the union u_dri_cnt_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jcfg2jvld_dri_cnt : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;        /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_dri_cnt_cfg;

/* define the union u_dri_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int jvld2jcfg_dri_cnt : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;        /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_dri_cnt;

/* define the union u_mem_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int mem_ctrl_rfs : 16; /* [15..0]  */
        unsigned int mem_ctrl_rft : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_mem_ctrl;

/* define the union u_bitbuffer_staddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int bb_staddr_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;   /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_bitbuffer_staddr_msb;

/* define the union u_bitbuffer_endaddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int bb_endaddr_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;    /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_bitbuffer_endaddr_msb;

/* Define the union U_BITSTREAM_STADDR_MSB */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int bs_staddr_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;   /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_bitstreams_staddr_msb;

/* Define the union U_BITSTREAM_ENDADDR_MSB */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int bs_endaddr_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;    /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_bitstreams_endaddr_msb;

/* define the union u_picture_ystaddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int pic_ystaddr_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;     /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_picture_ystaddr_msb;

/* define the union u_picture_uvstaddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int pic_uvstaddr_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;      /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_picture_uvstaddr_msb;

/* define the union u_mtn_address0_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int mtn_address0_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;      /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_mtn_address0_msb;

/* define the union u_mtn_address1_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int mtn_address1_msb : 2; /* [1..0]  */
        unsigned int reserved_0 : 30;      /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_mtn_address1_msb;

/* define the union u_quant_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int qtab_y : 8;   /* [7..0] */
        unsigned int qtab_cb : 8;  /* [15..8] */
        unsigned int qtab_cr : 8;  /* [23..16] */
        unsigned int reserved : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_quant_table;

/* define the union u_hdc_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hdc_lu : 12;  /* [11..0] */
        unsigned int hdc_ch : 12;  /* [23..12] */
        unsigned int reserved : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hdc_table;

/* define the union u_hac_min_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hac_lu_mincode_even : 8; /* [7..0] */
        unsigned int hac_lu_mincode_odd : 8;  /* [15..8] */
        unsigned int hac_ch_mincode_even : 8; /* [23..16] */
        unsigned int hac_ch_mincode_odd : 8;  /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hac_min_table;

/* define the union u_hac_base_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int hac_lu_base_even : 8; /* [7..0] */
        unsigned int hac_lu_base_odd : 8;  /* [15..8] */
        unsigned int hac_ch_base_even : 8; /* [23..16] */
        unsigned int hac_ch_base_odd : 8;  /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hac_base_table;

/* define the union u_hac_symbol_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int sel2_chn_wr : 8; /* [7..0]  */
        unsigned int sel1_chn_wr : 8; /* [15..8]  */
        unsigned int sel2_chn_rd : 8; /* [23..16]  */
        unsigned int reserved_0 : 8;  /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_hac_symbol_table;

/* Define the global struct */
typedef struct {
    volatile u_jpeg_dec_start jpeg_dec_start;       /* 0x00 */
    volatile u_jpeg_resume_start jpeg_resume_start; /* 0x04 */
    volatile unsigned int pic_vld_num;              /* 0x08 */
    volatile u_jpeg_stride jpeg_stride;             /* 0x0c */
    volatile u_picture_size picture_size;           /* 0x10 */
    volatile u_picture_type picture_type;           /* 0x14 */
    volatile unsigned int picdec_time;              /* 0x18 */
    volatile u_rgb_out_stride rgb_out_stride;       /* 0x1c */
    volatile unsigned int bitbuffer_staddr_lsb;     /* 0x20 */
    volatile unsigned int bitbuffer_endaddr_lsb;    /* 0x24 */
    volatile unsigned int bitstreams_staddr_lsb;    /* 0x28 */
    volatile unsigned int bitstreams_endaddr_lsb;   /* 0x2c */
    volatile unsigned int picture_ystaddr_lsb;      /* 0x30 */
    volatile unsigned int picture_uvstaddr_lsb;
    volatile u_pd_sum_cfg_cbcr pd_sum_cfg_cbcr;
    volatile u_pd_sum_cfg_y pd_sum_cfg_y;
    volatile u_freq_scale freq_scale; /* 0x40 */
    volatile u_out_type out_type;
    volatile u_jpgd_alpha jpgd_alpha;
    volatile u_hor_phase0_coef01 hor_phase0_coef01;           /* 0x4c */
    volatile u_hor_phase0_coef23 hor_phase0_coef23;           /* 0x50 */
    volatile u_hor_phase0_coef45 hor_phase0_coef45;           /* 0x54 */
    volatile u_hor_phase0_coef67 hor_phase0_coef67;           /* 0x58 */
    volatile unsigned int reserved0[4];                       /* 0x5c 0x60 0x64 0x68 */
    volatile u_hor_phase2_coef01 hor_phase2_coef01;           /* 0x6c */
    volatile u_hor_phase2_coef23 hor_phase2_coef23;           /* 0x70 */
    volatile u_hor_phase2_coef45 hor_phase2_coef45;           /* 0x74 */
    volatile u_hor_phase2_coef67 hor_phase2_coef67;           /* 0x78 */
    volatile unsigned int reserved1[4];                       /* 0x7c 0x80 0x84 0x88 */
    volatile u_ver_phase0_coef01 ver_phase0_coef01;           /* 0x8c */
    volatile u_ver_phase0_coef23 ver_phase0_coef23;           /* 0x90 */
    volatile unsigned int reserved2[2];                       /* 2:0x94 0x98 */
    volatile u_ver_phase2_coef01 ver_phase2_coef01;           /* 0x9c */
    volatile u_ver_phase2_coef23 ver_phase2_coef23;           /* 0xa0 */
    volatile unsigned int reserved3[2];                       /* 2:0xa4 0xa8 */
    volatile u_csc_in_dc_coef csc_in_dc_coef;                 /* 0xac */
    volatile u_csc_out_dc_coef csc_out_dc_coef;               /* 0xb0 */
    volatile u_csc_trans_coef0 csc_trans_coef0;               /* 0xb4 */
    volatile u_csc_trans_coef1 csc_trans_coef1;               /* 0xb8 */
    volatile u_csc_trans_coef2 csc_trans_coef2;               /* 0xbc */
    volatile u_csc_trans_coef3 csc_trans_coef3;               /* 0xc0 */
    volatile u_csc_trans_coef4 csc_trans_coef4;               /* 0xc4 */
    volatile unsigned int mtn_address0_lsb;                   /* 0xc8 */
    volatile unsigned int mtn_address1_lsb;                   /* 0xcc */
    volatile unsigned int reserved4;                          /* 0xd0 */
    volatile u_mcu_pro_startpos mcu_pro_startpos;             /* 0xd4 */
    volatile u_piccut_startpos piccut_startpos;               /* 0xd8 */
    volatile u_piccut_endpos piccut_endpos;                   /* 0xdc */
    volatile u_mcu_pro_curpos mcu_pro_curpos;                 /* 0xe0 */
    volatile unsigned int bs_consu;                           /* 0xe4 */
    volatile unsigned int bs_resume_data0;                    /* 0xe8 */
    volatile unsigned int bs_resume_data1;                    /* 0xec */
    volatile unsigned int bs_resume_bit;                      /* 0xf0 */
    volatile unsigned int bs_res_data_cfg0;                   /* 0xf4 */
    volatile unsigned int bs_res_data_cfg1;                   /* 0xf8 */
    volatile u_bs_res_bit_cfg bs_res_bit_cfg;                 /* 0xfc */
    volatile u_jpeg_int jpeg_int;                             /* 0x100 */
    volatile u_int_mask int_mask;                             /* 0x104 */
    volatile unsigned int over_time_thd;                      /* 0x108 */
    volatile u_pd_sum_y pd_sum_y;                             /* 0x10c */
    volatile u_pd_sum_cbcr pd_sum_cbcr;                       /* 0x110 */
    volatile unsigned int luma_pix_sum0;                      /* 0x114 */
    volatile u_luma_pix_sum1 luma_pix_sum1;                   /* 0x118 */
    volatile u_axi_4k_cnt axi_4k_cnt;                         /* 0x11c */
    volatile u_axi_16m_cnt axi_16m_cnt;                       /* 0x120 */
    volatile u_sampling_factor sampling_factor;               /* 0x124 */
    volatile u_dri dri;                                       /* 0x128 */
    volatile u_dri_cnt_cfg dri_cnt_cfg;                       /* 0x12c */
    volatile u_dri_cnt dri_cnt;                               /* 0x130 */
    volatile unsigned int reserved5[3];                       /* 0x134 0x138    0x13c */
    volatile u_mem_ctrl mem_ctrl;                             /* 0x140 */
    volatile unsigned int reserved6;                          /* 0x144 */
    volatile u_bitbuffer_staddr_msb bitbuffer_staddr_msb;     /* 0x148 */
    volatile u_bitbuffer_endaddr_msb bitbuffer_endaddr_msb;   /* 0x14c */
    volatile u_bitstreams_staddr_msb bitstreams_staddr_msb;   /* 0x150 */
    volatile u_bitstreams_endaddr_msb bitstreams_endaddr_msb; /* 0x154 */
    volatile u_picture_ystaddr_msb picture_ystaddr_msb;       /* 0x158 */
    volatile u_picture_uvstaddr_msb picture_uvstaddr_msb;     /* 0x15c */
    volatile u_mtn_address0_msb mtn_address0_msb;             /* 0x160 */
    volatile u_mtn_address1_msb mtn_address1_msb;             /* 0x164 */
    volatile unsigned int reserved7[38];                      /* 38:0x168~0x1fc */
    volatile u_quant_table quant_table[64];                   /* 64:0x200~0x2fc */
    volatile u_hdc_table hdc_table[12];                       /* 12:0x300~0x32c */
    volatile unsigned int reserved8[4];                       /* 4:0x330~0x33c */
    volatile u_hac_min_table hac_min_table[8];                /* 8:0x340~0x35c */
    volatile u_hac_base_table hac_base_table[8];              /* 8:0x360~0x37c */
    volatile unsigned int reserved9[32];                      /* 32:0x380~0x3fc */
    volatile unsigned int hac_symbol_table[176];              /* 176:0x400~0x6c0 */
} s_jpgd_regs_type;

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif
