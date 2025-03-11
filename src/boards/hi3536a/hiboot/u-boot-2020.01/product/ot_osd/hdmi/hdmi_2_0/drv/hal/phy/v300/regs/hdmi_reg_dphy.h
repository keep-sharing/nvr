// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __HDMI_REG_DPHY_H__
#define __HDMI_REG_DPHY_H__

#include "ot_type.h"

typedef union {
    struct {
        td_u32 gpport0 : 16; /* [15:0] */
        td_u32 rsv_0   : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} t2gpport0;

typedef union {
    struct {
        td_u32 gpport1 : 16; /* [15:0] */
        td_u32 rsv_1   : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} t2gpport1;

typedef union {
    struct {
        td_u32 stb_cs_en : 16; /* [15:0] */
        td_u32 rsv_2     : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} phy_csen;

typedef union {
    struct {
        td_u32 stb_wen : 1; /* [0] */
        td_u32 rsv_3 : 31;  /* [31:1] */
    } bits;
    td_u32 u32;
} phy_wr;

typedef union {
    struct {
        td_u32 resetn : 1;  /* [0] */
        td_u32 rsv_4  : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} resetn;

typedef union {
    struct {
        td_u32 stb_addr : 4;  /* [3:0] */
        td_u32 rsv_5    : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} phy_addr;

typedef union {
    struct {
        td_u32 stb_wdata : 8;  /* [7:0] */
        td_u32 rsv_6     : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} phy_wdata;

typedef union {
    struct {
        td_u32 stb_rdata : 8;  /* [7:0] */
        td_u32 rsv_7     : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} phy_rdata;

typedef union {
    struct {
        td_u32 zcal     : 5;  /* [4:0] */
        td_u32 zcaldone : 1;  /* [5] */
        td_u32 zcalsub  : 2;  /* [7:6] */
        td_u32 rxsense  : 4;  /* [11:8] */
        td_u32 rsv_8    : 20; /* [31:12] */
    } bits;
    td_u32 u32;
} zcalreg;

typedef union {
    struct {
        td_u32 zcalclk : 1;  /* [0] */
        td_u32 rsv_9   : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} zcalclk;

typedef union {
    struct {
        td_u32 c0shortdet  : 1;  /* [0] */
        td_u32 c1shortdet  : 1;  /* [1] */
        td_u32 c2shortdet  : 1;  /* [2] */
        td_u32 clkshortdet : 1;  /* [3] */
        td_u32 rsv_10      : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} shortdet;

typedef union {
    struct {
        td_u32 rsv_11        : 12; /* [11:0] */
        td_u32 t2_plllkvdetl : 1;  /* [12] */
        td_u32 t2_plllkcdet  : 1;  /* [13] */
        td_u32 t2_plllkvdet2 : 1;  /* [14] */
        td_u32 t2_lkvdetlow  : 1;  /* [15] */
        td_u32 t2_lkvdethigh : 1;  /* [16] */
        td_u32 rsv_12        : 15; /* [31:17] */
    } bits;
    td_u32 u32;
} det;

typedef union {
    struct {
        td_u32 src_lock_val      : 8;  /* [7:0] */
        td_u32 src_lock_cnt      : 8;  /* [15:8] */
        td_u32 src_enable        : 1;  /* [16] */
        td_u32 fdsrcparam_unused : 3;  /* [19:17] */
        td_u32 rsv_13            : 12; /* [31:20] */
    } bits;
    td_u32 u32;
} fdsrcparam;

typedef union {
    struct {
        td_u32 src_cnt_opt        : 3;  /* [2:0] */
        td_u32 fdsrcfreq_unused_1 : 1;  /* [3] */
        td_u32 src_freq_opt       : 2;  /* [5:4] */
        td_u32 fdsrcfreq_unused_2 : 2;  /* [7:6] */
        td_u32 src_freq_ext       : 16; /* [23:8] */
        td_u32 rsv_14             : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} fdsrcfreq;

typedef union {
    struct {
        td_u32 src_det_stat : 4;  /* [3:0] */
        td_u32 src_cnt_out  : 20; /* [23:4] */
        td_u32 rsv_15       : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} fdsrcres;

typedef union {
    struct {
        td_u32 i_enable      : 1;  /* [0] */
        td_u32 i_run         : 1;  /* [1] */
        td_u32 ctset0_unused : 2;  /* [3:2] */
        td_u32 rsv_16        : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} ctset0;

typedef union {
    struct {
        td_u32 i_mpll_fcon   : 10; /* [9:0] */
        td_u32 i_mpll_divn   : 3;  /* [12:10] */
        td_u32 i_mpll_ctlck  : 1;  /* [13] */
        td_u32 ctset1_unused : 18; /* [31:14] */
    } bits;
    td_u32 u32;
} ctset1;

typedef union {
    struct {
        td_u32 i_deci_cnt_len     : 8; /* [7:0] */
        td_u32 i_vco_st_wait_len  : 8; /* [15:8] */
        td_u32 i_vco_end_wait_len : 8; /* [23:16] */
        td_u32 i_ref_cnt_len      : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} fccntr0;

typedef union {
    struct {
        td_u32 i_ct_sel       : 1;  /* [0] */
        td_u32 i_clkdet_sel   : 1;  /* [1] */
        td_u32 i_ct_mode      : 2;  /* [3:2] */
        td_u32 fcopt_unused_1 : 4;  /* [7:4] */
        td_u32 i_ct_en        : 1;  /* [8] */
        td_u32 fcopt_unused_2 : 3;  /* [11:9] */
        td_u32 i_ct_idx_sel   : 1;  /* [12] */
        td_u32 i_deci_try_sel : 1;  /* [13] */
        td_u32 fcopt_unused   : 2;  /* [15:14] */
        td_u32 rsv_17         : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fcopt;

typedef union {
    struct {
        td_u32 clk_ok          : 1;  /* [0] */
        td_u32 busy            : 1;  /* [1] */
        td_u32 done            : 1;  /* [2] */
        td_u32 error           : 1;  /* [3] */
        td_u32 divn            : 3;  /* [6:4] */
        td_u32 fcstat_unused_1 : 1;  /* [7] */
        td_u32 ref_clk_stat    : 1;  /* [8] */
        td_u32 pllvco_clk_stat : 1;  /* [9] */
        td_u32 fcstat_unused_2 : 2;  /* [11:10] */
        td_u32 confin_stat     : 6;  /* [17:12] */
        td_u32 fcstat_unused_3 : 2;  /* [19:18] */
        td_u32 fcon_init       : 10; /* [29:20] */
        td_u32 rsv_18          : 2;  /* [31:30] */
    } bits;
    td_u32 u32;
} fcstat;

typedef union {
    struct {
        td_u32 cnt_ref : 16; /* [15:0] */
        td_u32 rsv_19  : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fccntval0;

typedef union {
    struct {
        td_u32 cnt_mpll : 16; /* [15:0] */
        td_u32 rsv_20   : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fccntval1;

typedef union {
    struct {
        td_u32 divn_mpll       : 3;  /* [2:0] */
        td_u32 fcresval_unused : 1;  /* [3] */
        td_u32 fcon_mpll       : 10; /* [13:4] */
        td_u32 rsv_21          : 18; /* [31:14] */
    } bits;
    td_u32 u32;
} fcresval;

typedef union {
    struct {
        td_u32 divn_h20             : 3;  /* [2:0] */
        td_u32 fcdstepset_unused    : 1;  /* [3] */
        td_u32 up_sampler_ratio_sel : 1;  /* [4] */
        td_u32 rsv_22               : 27; /* [31:5] */
    } bits;
    td_u32 u32;
} fcdstepsetl;

typedef union {
    struct {
        td_u32 i_h2_sel   : 1;  /* [0] */
        td_u32 i_deci_sel : 1;  /* [1] */
        td_u32 rsv_23     : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} fcdstepth;

typedef union {
    struct {
        td_u32 i_deci2x_th : 16; /* [15:0] */
        td_u32 i_deci4x_th : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fcdstepth0;

typedef union {
    struct {
        td_u32 i_deci8x_th  : 16; /* [15:0] */
        td_u32 i_deci16x_th : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fcdstepth1;

typedef union {
    struct {
        td_u32 i_ref_cnt : 16; /* [15:0] */
        td_u32 rsv_24    : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fccntr1;

typedef union {
    struct {
        td_u32 contin_upd_en       : 1;  /* [0] */
        td_u32 contin_upd_opt      : 1;  /* [1] */
        td_u32 contin_upd_pol      : 1;  /* [2] */
        td_u32 fccontinset0_unused : 1;  /* [3] */
        td_u32 contin_upd_step     : 4;  /* [7:4] */
        td_u32 rsv_25              : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} fccontinset0;

typedef union {
    struct {
        td_u32 contin_upd_rate : 28; /* [27:0] */
        td_u32 contin_upd_time : 4;  /* [31:28] */
    } bits;
    td_u32 u32;
} fccontinset1;

typedef union {
    struct {
        td_u32 contin_upd_th_dn    : 10; /* [9:0] */
        td_u32 fccontinset2_unused : 2;  /* [11:10] */
        td_u32 contin_upd_th_up    : 10; /* [21:12] */
        td_u32 rsv_26              : 10; /* [31:22] */
    } bits;
    td_u32 u32;
} fccontinset2;

typedef union {
    struct {
        td_u32 init   : 1;  /* [0] */
        td_u32 ctrl   : 1;  /* [1] */
        td_u32 mod    : 1;  /* [2] */
        td_u32 sdm    : 1;  /* [3] */
        td_u32 rsv_27 : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} fdivset0;

typedef union {
    struct {
        td_u32 step_d : 8;  /* [7:0] */
        td_u32 step_t : 8;  /* [15:8] */
        td_u32 step_n : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fdivset1;

typedef union {
    struct {
        td_u32 up     : 1;  /* [0] */
        td_u32 dn     : 1;  /* [1] */
        td_u32 rsv_28 : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} fdivset2;

typedef union {
    struct {
        td_u32 mod_len : 8;  /* [7:0] */
        td_u32 mod_t   : 8;  /* [15:8] */
        td_u32 mod_n   : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fdivset3;

typedef union {
    struct {
        td_u32 mod_d  : 16;  /* [15:0] */
        td_u32 rsv_29 : 16;  /* [31:16] */
    } bits;
    td_u32 u32;
} fdivset4;

typedef union {
    struct {
        td_u32 mod_up          : 1;  /* [0] */
        td_u32 mod_dn          : 1;  /* [1] */
        td_u32 fdivset5_unused : 2;  /* [3:2] */
        td_u32 rsv_30          : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} fdivset5;

typedef union {
    struct {
        td_u32 stc_run : 1;  /* [0] */
        td_u32 stc_dir : 1;  /* [1] */
        td_u32 stc_ov  : 1;  /* [2] */
        td_u32 stc_un  : 1;  /* [3] */
        td_u32 stc_cnt : 16; /* [19:4] */
        td_u32 rsv_31  : 12; /* [31:20] */
    } bits;
    td_u32 u32;
} fdivs_tat0;

typedef union {
    struct {
        td_u32 i_fdiv_in : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} fdivs_tat1;

typedef union {
    struct {
        td_u32 div_out : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} fdivs_tat2;

typedef union {
    struct {
        td_u32 div_sdm : 16; /* [15:0] */
        td_u32 rsv_32  : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fdivs_tat3;

typedef union {
    struct {
        td_u32 stm_run          : 1;  /* [0] */
        td_u32 stm_ph           : 2;  /* [2:1] */
        td_u32 stm_ov           : 1;  /* [3] */
        td_u32 stm_un           : 1;  /* [4] */
        td_u32 fdivstat4_unused : 3;  /* [7:5] */
        td_u32 stm_cnt          : 16; /* [23:8] */
        td_u32 rsv_33           : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} fdivs_tat4;

typedef union {
    struct {
        td_u32 i_manual_en       : 4; /* [3:0] */
        td_u32 i_divn            : 3; /* [6:4] */
        td_u32 fdivmanual_unused : 1; /* [7] */
        td_u32 i_mdiv            : 4; /* [11:8] */
        td_u32 i_ref_cnt_div     : 2; /* [13:12] */
        td_u32 i_dc_sel          : 2; /* [15:14] */
        td_u32 i_vic             : 8; /* [23:16] */
        td_u32 rsv_34            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} fdivmanual;

typedef union {
    struct {
        td_u32 t2_refclksel       : 1;  /* [0] */
        td_u32 t2_refclksel2      : 1;  /* [1] */
        td_u32 i_ref_clk_sel      : 1;  /* [2] */
        td_u32 refclksel_unused_1 : 1;  /* [3] */
        td_u32 t2_pixelclksel     : 1;  /* [4] */
        td_u32 refclksel_unused_2 : 1;  /* [5] */
        td_u32 pr_enc_val         : 2;  /* [7:6] */
        td_u32 rsv_35             : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} refclksel;

typedef union {
    struct {
        td_u32 pll_lock_val      : 8;  /* [7:0] */
        td_u32 pll_lock_cnt      : 8;  /* [15:8] */
        td_u32 pll_enable        : 1;  /* [16] */
        td_u32 fdpllparam_unused : 3;  /* [19:17] */
        td_u32 rsv_36            : 12; /* [31:20] */
    } bits;
    td_u32 u32;
} fdpllparam;

typedef union {
    struct {
        td_u32 pll_cnt_opt        : 3;  /* [2:0] */
        td_u32 fdpllfreq_unused_1 : 1;  /* [3] */
        td_u32 pll_freq_opt       : 2;  /* [5:4] */
        td_u32 fdpllfreq_unused_2 : 2;  /* [7:6] */
        td_u32 pll_freq_ext       : 16; /* [23:8] */
        td_u32 rsv_37             : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} fdpllfreq;

typedef union {
    struct {
        td_u32 pll_det_stat : 4;  /* [3:0] */
        td_u32 pll_cnt_out  : 20; /* [23:4] */
        td_u32 rsv_38       : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} fdpllres;

typedef union {
    struct {
        td_u32 fcg_en        : 1;  /* [0] */
        td_u32 fcg_dlf_en    : 1;  /* [1] */
        td_u32 fcg_dither_en : 1;  /* [2] */
        td_u32 fcg_lock_en   : 1;  /* [3] */
        td_u32 rsv_39        : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} fcgset;

typedef union {
    struct {
        td_u32 tmds_cnt_val : 16; /* [15:0] */
        td_u32 cnt1_target  : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} fcgcnt;

typedef union {
    struct {
        td_u32 lock_cnt  : 8; /* [7:0] */
        td_u32 lock_th   : 8; /* [15:8] */
        td_u32 ki        : 6; /* [21:16] */
        td_u32 lock_mode : 1; /* [22] */
        td_u32 rsv_40    : 9; /* [31:23] */
    } bits;
    td_u32 u32;
} fcgparam;

typedef union {
    struct {
        td_u32 dlf_lock : 1;  /* [0] */
        td_u32 dlf_ov   : 1;  /* [1] */
        td_u32 dlf_un   : 1;  /* [2] */
        td_u32 rsv_41   : 29; /* [31:3] */
    } bits;
    td_u32 u32;
} fcgstate;

typedef union {
    struct {
        td_u32 ch_en_h20          : 4; /* [3:0] */
        td_u32 prbs_clr_h20       : 4; /* [7:4] */
        td_u32 ch_en_h21          : 4; /* [11:8] */
        td_u32 prbs_clr_h21       : 4; /* [15:12] */
        td_u32 test_pat_type      : 3; /* [18:16] */
        td_u32 ch_test_en         : 1; /* [19] */
        td_u32 test_4to1_mux_sel0 : 2; /* [21:20] */
        td_u32 test_4to1_mux_sel1 : 2; /* [23:22] */
        td_u32 test_4to1_mux_sel2 : 2; /* [25:24] */
        td_u32 test_4to1_mux_sel3 : 2; /* [27:26] */
        td_u32 rsv_42             : 4; /* [31:28] */
    } bits;
    td_u32 u32;
} txteloset;

typedef union {
    struct {
        td_u32 test_pat_ch0   : 20; /* [19:0] */
        td_u32 test_pat_ch1_l : 10; /* [29:20] */
        td_u32 rsv_43         : 2;  /* [31:30] */
    } bits;
    td_u32 u32;
} txtelocont0;

typedef union {
    struct {
        td_u32 test_pat_ch1_h : 10; /* [9:0] */
        td_u32 test_pat_ch2   : 20; /* [29:10] */
        td_u32 rsv_44         : 2;  /* [31:30] */
    } bits;
    td_u32 u32;
} txtelocont1;

typedef union {
    struct {
        td_u32 test_pat_ch3 : 20; /* [19:0] */
        td_u32 rsv_45       : 12; /* [31:20] */
    } bits;
    td_u32 u32;
} txtelocont2;

typedef union {
    struct {
        td_u32 pr_en_h20         : 1;  /* [0] */
        td_u32 enable_h20        : 1;  /* [1] */
        td_u32 txfifoset0_unused : 6;  /* [7:2] */
        td_u32 rsv_46            : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} txfifoset0;

typedef union {
    struct {
        td_u32 pol_inv0_h20   : 4; /* [3:0] */
        td_u32 data_swap0_h20 : 4; /* [7:4] */
        td_u32 ch_sel0_h20    : 8; /* [15:8] */
        td_u32 pol_inv1_h20   : 4; /* [19:16] */
        td_u32 data_swap1_h20 : 4; /* [23:20] */
        td_u32 ch_sel1_h20    : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} txfifoset1;

typedef union {
    struct {
        td_u32 pr_fifo_state_h20  : 12; /* [11:0] */
        td_u32 txfifostat0_unused : 12; /* [23:12] */
        td_u32 rsv_47             : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} tx_fifo_stat0;

typedef union {
    struct {
        td_u32 txfifostat1_unused_0 : 12; /* [11:0] */
        td_u32 txfifostat1_unused_1 : 12; /* [23:12] */
        td_u32 rsv_48               : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} tx_fifo_stat1;

typedef union {
    struct {
        td_u32 tx_fifo_state_h20  : 12; /* [11:0] */
        td_u32 txfifostat2_unused : 12; /* [23:12] */
        td_u32 rsv_49             : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} tx_fifo_stat2;

typedef union {
    struct {
        td_u32 txfifostat3_unused_0 : 12; /* [11:0] */
        td_u32 txfifostat3_unused_1 : 12; /* [23:12] */
        td_u32 rsv_50               : 8;  /* [31:24] */
    } bits;
    td_u32 u32;
} tx_fifo_stat3;

typedef union {
    struct {
        td_u32 dataclkinv : 1;  /* [0] */
        td_u32 rsv_51     : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} data_clk_inv;

typedef union {
    struct {
        td_u32 ch_out_sel : 2;  /* [1:0] */
        td_u32 rsv_52     : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} tx_data_out_sel;

typedef union {
    struct {
        td_u32 reg_hdmi_mode_en : 1;  /* [0] */
        td_u32 rsv_53           : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} hdmi_mode;

typedef union {
    struct {
        td_u32 reg_clk_data_phase0 : 10; /* [9:0] */
        td_u32 reg_clk_data_phase1 : 10; /* [19:10] */
        td_u32 reg_sw_clk_en       : 1;  /* [20] */
        td_u32 rsv_54              : 11; /* [31:21] */
    } bits;
    td_u32 u32;
} clk_data1;

typedef union {
    struct {
        td_u32 reg_clk_data_phase2 : 10; /* [9:0] */
        td_u32 reg_clk_data_phase3 : 10; /* [19:10] */
        td_u32 rsv_55              : 12; /* [31:20] */
    } bits;
    td_u32 u32;
} clk_data2;

typedef union {
    struct {
        td_u32 reg_18to20_fifo_rd_rst : 1;  /* [0] */
        td_u32 reg_18to20_fifo_wr_rst : 1;  /* [1] */
        td_u32 reg_rd_bypass          : 1;  /* [2] */
        td_u32 reg_status_rrst        : 1;  /* [3] */
        td_u32 reg_status_wrst        : 1;  /* [4] */
        td_u32 rsv_56                 : 27; /* [31:5] */
    } bits;
    td_u32 u32;
} cfg_18_to_20;

typedef union {
    struct {
        td_u32 empty_status  : 1;  /* [0] */
        td_u32 aempty_status : 1;  /* [1] */
        td_u32 full_status   : 1;  /* [2] */
        td_u32 afull_status  : 1;  /* [3] */
        td_u32 rsv_57        : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} fifo_stat_18_to_20;

typedef union {
    struct {
        td_u32 hsset  : 2;   /* [1:0] */
        td_u32 rsv_58 : 30;  /* [31:2] */
    } bits;
    td_u32 u32;
} hsset;

typedef union {
    struct {
        td_u32 w_hsrxsense : 2;  /* [1:0] */
        td_u32 rsv_59      : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} hsrxsense;

typedef union {
    struct {
        td_u32 fifo_state_hs : 12; /* [11:0] */
        td_u32 rsv_60        : 20; /* [31:12] */
    } bits;
    td_u32 u32;
} hs_fifo_stat;

typedef union {
    struct {
        td_u32 hs_fifo_empty_intr          : 1;  /* [0] */
        td_u32 hs_fifo_full_intr           : 1;  /* [1] */
        td_u32 up_sample_fifo_empty_intr   : 1;  /* [2] */
        td_u32 up_sample_fifo_full_intr    : 1;  /* [3] */
        td_u32 trinsmitter_fifo_empty_intr : 1;  /* [4] */
        td_u32 trinsmitter_fifo_full_intr  : 1;  /* [5] */
        td_u32 lkvdethigh_intr             : 1;  /* [6] */
        td_u32 lkvdetlow_intr              : 1;  /* [7] */
        td_u32 ct_fcon_intr                : 1;  /* [8] */
        td_u32 rsv_61                      : 23; /* [31:9] */
    } bits;
    td_u32 u32;
} intr_stat;

typedef union {
    struct {
        td_u32 hs_fifo_empty_intr_mask          : 1;  /* [0] */
        td_u32 hs_fifo_full_intr_mask           : 1;  /* [1] */
        td_u32 up_sample_fifo_empty_intr_mask   : 1;  /* [2] */
        td_u32 up_sample_fifo_full_intr_mask    : 1;  /* [3] */
        td_u32 trinsmitter_fifo_empty_intr_mask : 1;  /* [4] */
        td_u32 trinsmitter_fifo_full_intr_mask  : 1;  /* [5] */
        td_u32 lkvdethigh_intr_mask             : 1;  /* [6] */
        td_u32 lkvdetlow_intr_mask              : 1;  /* [7] */
        td_u32 ct_fcon_intr_mask                : 1;  /* [8] */
        td_u32 rsv_62                           : 23; /* [31:9] */
    } bits;
    td_u32 u32;
} intr_mask;

typedef union {
    struct {
        td_u32 hs_fifo_empty_triger_type          : 3; /* [2:0] */
        td_u32 hs_fifo_empty_intr_en              : 1; /* [3] */
        td_u32 hs_fifo_full_triger_type           : 3; /* [6:4] */
        td_u32 hs_fifo_full_intr_en               : 1; /* [7] */
        td_u32 up_sample_fifo_empty_triger_type   : 3; /* [10:8] */
        td_u32 up_sample_fifo_empty_intr_en       : 1; /* [11] */
        td_u32 up_sample_fifo_full_triger_type    : 3; /* [14:12] */
        td_u32 up_sample_fifo_full_intr_en        : 1; /* [15] */
        td_u32 transmitter_fifo_empty_triger_type : 3; /* [18:16] */
        td_u32 transmitter_fifo_empty_intr_en     : 1; /* [19] */
        td_u32 transmitter_fifo_full_triger_type  : 3; /* [22:20] */
        td_u32 transmitter_fifo_full_intr_en      : 1; /* [23] */
        td_u32 lkvdethigh_triger_type             : 3; /* [26:24] */
        td_u32 lkvdethigh_intr_en                 : 1; /* [27] */
        td_u32 lkvdetlow_triger_type              : 3; /* [30:28] */
        td_u32 lkvdetlow_intr_en                  : 1; /* [31] */
    } bits;
    td_u32 u32;
} intr_set;

typedef union {
    struct {
        td_u32 fd_clk_sel  : 2;  /* [1:0] */
        td_u32 refclk_sel  : 2;  /* [3:2] */
        td_u32 ctman       : 2;  /* [5:4] */
        td_u32 modclk_sel  : 1;  /* [6] */
        td_u32 fdivclk_sel : 1;  /* [7] */
        td_u32 mod_div_val : 4;  /* [11:8] */
        td_u32 rsv_63      : 20; /* [31:12] */
    } bits;
    td_u32 u32;
} clk_set;

typedef union {
    struct {
        td_u32 sw_reset_mod_clock          : 1;  /* [0] */
        td_u32 sw_reset_tmds_clock         : 1;  /* [1] */
        td_u32 sw_reset_mpll_clock         : 1;  /* [2] */
        td_u32 sw_reset_nco_clock          : 1;  /* [3] */
        td_u32 sw_reset_fd_clock           : 1;  /* [4] */
        td_u32 sw_reset_mod_and_mpll_clock : 1;  /* [5] */
        td_u32 sw_reset_mod_and_nco_clock  : 1;  /* [6] */
        td_u32 sw_reset_mod_and_fd_clock   : 1;  /* [7] */
        td_u32 sw_reset_hsfifo_clock       : 1;  /* [8] */
        td_u32 sw_reset_txfifo_clock       : 1;  /* [9] */
        td_u32 sw_reset_data_clock         : 1;  /* [10] */
        td_u32 sw_reset_hs_clock           : 1;  /* [11] */
        td_u32 sw_reset_pllref_clock       : 1;  /* [12] */
        td_u32 sw_reset_dac_clock          : 1;  /* [13] */
        td_u32 dac_clock_gat               : 1;  /* [14] */
        td_u32 up_sample_fifo_clock_swrst  : 1;  /* [15] */
        td_u32 sw_reset_frl_clock          : 1;  /* [16] */
        td_u32 swreset_unused              : 14; /* [30:17] */
        td_u32 global_reset                : 1;  /* [31] */
    } bits;
    td_u32 u32;
} sw_reset;

typedef union {
    struct {
        td_u32 clk0_div : 4; /* [3:0] */
        td_u32 clk1_div : 4; /* [7:4] */
        td_u32 clk2_div : 4; /* [11:8] */
        td_u32 clk3_div : 4; /* [15:12] */
        td_u32 clk4_div : 4; /* [19:16] */
        td_u32 clk5_div : 4; /* [23:20] */
        td_u32 clk6_div : 4; /* [27:24] */
        td_u32 clk7_div : 4; /* [31:28] */
    } bits;
    td_u32 u32;
} glueset0;

typedef union {
    struct {
        td_u32 clk8_div          : 4;  /* [3:0] */
        td_u32 glueset1_unused_1 : 4;  /* [7:4] */
        td_u32 clk10_div         : 4;  /* [11:8] */
        td_u32 clk11_div         : 4;  /* [15:12] */
        td_u32 clk_sel           : 4;  /* [19:16] */
        td_u32 glueset1_unused_2 : 12; /* [31:20] */
    } bits;
    td_u32 u32;
} glueset1;

typedef union {
    struct {
        td_u32 ct_fcon_triger_type : 3;  /* [2:0] */
        td_u32 ct_fcon_intr_en     : 1;  /* [3] */
        td_u32 rsv_64              : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} ct_intr_set;

typedef union {
    struct {
        td_u32 hw_info : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} hw_info;

typedef union {
    struct {
        td_u32 hw_vers_unused_1  : 4;  /* [3:0] */
        td_u32 hdmi20_compliance : 1;  /* [4] */
        td_u32 hdmi21_compliance : 1;  /* [5] */
        td_u32 hw_vers_unused_2  : 26; /* [31:6] */
    } bits;
    td_u32 u32;
} hw_vers;

typedef union {
    struct {
        td_u32 ras_mode : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} hw_ras_mode;

typedef union {
    struct {
        td_u32 rfs_mode : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} hw_rfs_mode;

typedef union {
    struct {
        td_u32 rft_mode : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} hw_rft_mode;

typedef union {
    struct {
        td_u32 req_length  : 2;  /* [1:0] */
        td_u32 stb_delay2  : 4;  /* [5:2] */
        td_u32 stb_delay1  : 4;  /* [9:6] */
        td_u32 stb_delay0  : 4;  /* [13:10] */
        td_u32 stb_acc_sel : 1;  /* [14] */
        td_u32 stb_cs_sel  : 1;  /* [15] */
        td_u32 rsv_65      : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} stb_opt;

typedef union {
    struct {
        td_u32 req_done : 1;  /* [0] */
        td_u32 rsv_66   : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} stb_req;

typedef union {
    struct {
        td_u32 stb_auto_rdata : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} stb_data;

typedef struct {
    volatile t2gpport0          t2gp_port0;        /* 0 */
    volatile t2gpport1          t2gp_port1;        /* 4 */
    volatile phy_csen           stb_cs_en;         /* 8 */
    volatile phy_wr             stb_write_en;      /* C */
    volatile resetn             stb_reset;         /* 10 */
    volatile phy_addr           stb_addr;          /* 14 */
    volatile phy_wdata          stb_wdata;         /* 18 */
    volatile phy_rdata          stb_rdata;         /* 1C */
    volatile zcalreg            zcal_reg;          /* 20 */
    volatile zcalclk            zcal_clk;          /* 24 */
    volatile shortdet           short_det;         /* 28 */
    volatile det                stb_det;           /* 2C */
    volatile fdsrcparam         fd_src_param;      /* 30 */
    volatile fdsrcfreq          fd_src_freq;       /* 34 */
    volatile fdsrcres           fd_src_res;        /* 38 */
    volatile ctset0             ct_set0;           /* 3C */
    volatile ctset1             ct_set1;           /* 40 */
    td_u32                      reserved_0;        /* 44 */
    volatile fccntr0            fc_cntr0;          /* 48 */
    volatile fcopt              fc_opt;            /* 4C */
    volatile fcstat             fc_stat;           /* 50 */
    volatile fccntval0          fc_cnt_val0;       /* 54 */
    volatile fccntval1          fc_cnt_val1;       /* 58 */
    volatile fcresval           fc_res_sval;       /* 5C */
    volatile fcdstepsetl        fc_dstep_set;      /* 60 */
    volatile fcdstepth          fc_dstep_th;       /* 64 */
    volatile fcdstepth0         fc_dstep_th0;      /* 68 */
    volatile fcdstepth1         fc_dstep_th1;      /* 6C */
    volatile fccntr1            fc_cntr1;          /* 70 */
    volatile fccontinset0       fc_contin_set0;    /* 74 */
    volatile fccontinset1       fc_contin_set1;    /* 78 */
    volatile fccontinset2       fc_contin_set2;    /* 7C */
    td_u32                      reserved_1[4];     /* 80-8C */
    volatile fdivset0           fdiv_set0;         /* 90 */
    volatile fdivset1           fdiv_set1;         /* 94 */
    volatile fdivset2           fdiv_set2;         /* 98 */
    volatile fdivset3           fdiv_set3;         /* 9C */
    volatile fdivset4           fdiv_set4;         /* A0 */
    volatile fdivset5           fdiv_set5;         /* A4 */
    volatile fdivs_tat0         fdiv_stat0;        /* A8 */
    volatile fdivs_tat1         fdiv_stat1;        /* AC */
    volatile fdivs_tat2         fdiv_stat2;        /* B0 */
    volatile fdivs_tat3         fdiv_stat3;        /* B4 */
    volatile fdivs_tat4         fdiv_stat4;        /* B8 */
    volatile fdivmanual         fdiv_manual;       /* BC */
    volatile refclksel          ref_clk_sel;       /* C0 */
    td_u32                      reserved_2[15];    /* C4-FC */
    volatile fdpllparam         fd_pll_param;      /* 100 */
    volatile fdpllfreq          fd_pll_freq;       /* 104 */
    volatile fdpllres           fd_pll_res;        /* 108 */
    td_u32                      reserved_3[5];     /* 10c-11c */
    volatile fcgset             fcg_set;           /* 120 */
    volatile fcgcnt             fcg_cnt;           /* 124 */
    volatile fcgparam           fcg_param;         /* 128 */
    volatile fcgstate           fcg_state;         /* 12C */
    td_u32                      reserved_4[52];    /* 130-1FC */
    volatile txteloset          telo_set;          /* 200 */
    volatile txtelocont0        telo_cnt0;         /* 204 */
    volatile txtelocont1        telo_cnt1;         /* 208 */
    volatile txtelocont2        telo_cnt2;         /* 20C */
    volatile txfifoset0         tx_fifo_set0;      /* 210 */
    volatile txfifoset1         tx_fifo_set1;      /* 214 */
    volatile tx_fifo_stat0      tx_fifo_stat0;     /* 218 */
    volatile tx_fifo_stat1      tx_fifo_stat1;     /* 21C */
    volatile tx_fifo_stat2      tx_fifo_stat2;     /* 220 */
    volatile tx_fifo_stat3      tx_fifo_stat3;     /* 224 */
    volatile data_clk_inv       data_clk_inv;      /* 228 */
    volatile tx_data_out_sel    data_out_sel;      /* 22C */
    volatile hdmi_mode          stb_hdmi_mode;     /* 230 */
    volatile clk_data1          clk_data1;         /* 234 */
    volatile clk_data2          clk_data2;         /* 238 */
    volatile cfg_18_to_20       cfg15_to_20;       /* 23C */
    volatile fifo_stat_18_to_20 fifo_stat18_to_20; /* 240 */
    td_u32                      reserved_5[7];     /* 244-25c */
    volatile hsset              hs_set;            /* 260 */
    volatile hsrxsense          hs_rxsense;        /* 264 */
    volatile hs_fifo_stat       hs_fifo_stat;      /* 268 */
    td_u32                      reserved_6[37];    /* 26C-2FC */
    volatile intr_stat          intr_stat;         /* 300 */
    volatile intr_mask          intr_mask;         /* 304 */
    volatile intr_set           intr_set;          /* 308 */
    volatile clk_set            clk_set;           /* 30C */
    volatile sw_reset           sw_rst;            /* 310 */
    volatile glueset0           glue_set0;         /* 314 */
    volatile glueset1           glue_set1;         /* 318 */
    volatile ct_intr_set        ct_intr_set;       /* 31C */
    td_u32                      reserved_7[56];    /* 320-3FC */
    volatile hw_info            info_hw;           /* 400 */
    volatile hw_vers            vers_hw;           /* 404 */
    td_u32                      reserved_8[6];     /* 408-41C */
    volatile hw_ras_mode        ras_mode;          /* 420 */
    volatile hw_rfs_mode        rfs_mode;          /* 424 */
    volatile hw_rft_mode        rft_mode;          /* 428 */
    td_u32                      reserved_9[181];   /* 42C-6FC */
    volatile stb_opt            opt;               /* 700 */
    volatile stb_req            req;               /* 704 */
    volatile stb_data           rdata;             /* 708 */
} hdmitx21_dphy_regs_type;

td_s32 hdmi_reg_tx_phy_init(td_u32 id);
td_u32 *hdmi_reg_tx_get_phy_addr(td_u32 id);
td_s32 hdmi_reg_tx_phy_deinit(td_u32 id);
td_void hdmi_reg_stb_cs_en_set(td_u32 id, td_u16 stb_cs_en);
td_void hdmi_reg_stb_wen_set(td_u32 id, td_u8 stb_wen);
td_void hdmi_reg_resetn_set(td_u32 id, td_u8 reg_resetn);
td_u8 hdmi_reg_resetn_get(td_u32 id);
td_void hdmi_reg_stb_addr_set(td_u32 id, td_u8 stb_addr);
td_void hdmi_reg_stb_wdata_set(td_u32 id, td_u8 stb_wdata);
td_u8 hdmi_reg_stb_rdata_get(td_u32 id);
td_void hdmi_reg_src_lock_val_set(td_u32 id, td_u8 src_lock_val);
td_void hdmi_reg_src_lock_cnt_set(td_u32 id, td_u8 src_lock_cnt);
td_void hdmi_reg_src_enable_set(td_u32 id, td_u8 src_enable);
td_void hdmi_reg_src_cnt_opt_set(td_u32 id, td_u8 src_cnt_opt);
td_void hdmi_reg_fdsrcfreq_unused1_set(td_u32 id, td_u8 fdsrcfreq_unused_1);
td_void hdmi_reg_src_freq_opt_set(td_u32 id, td_u8 src_freq_opt);
td_void hdmi_reg_fdsrcfreq_unused2_set(td_u32 id, td_u8 fdsrcfreq_unused_2);
td_void hdmi_reg_src_freq_ext_set(td_u32 id, td_u16 src_freq_ext);
td_u8 hdmi_reg_src_det_stat_get(td_u32 id);
td_u32 hdmi_reg_src_cnt_out_get(td_u32 id);
td_void hdmi_reg_enable_set(td_u32 id, td_u8 i_enable);
td_void hdmi_reg_run_set(td_u32 id, td_u8 i_run);
td_void hdmi_reg_ctset0_unused_set(td_u32 id, td_u8 ctset0_unused);
td_void hdmi_reg_vco_st_wait_len_set(td_u32 id, td_u8 i_vco_st_wait_len);
td_void hdmi_reg_vco_end_wait_len_set(td_u32 id, td_u8 i_vco_end_wait_len);
td_void hdmi_reg_ref_cnt_len_set(td_u32 id, td_u8 i_ref_cnt_len);
td_void hdmi_reg_ct_sel_set(td_u32 id, td_u8 i_ct_sel);
td_void hdmi_reg_clkdet_sel_set(td_u32 id, td_u8 i_clkdet_sel);
td_void hdmi_reg_ct_mode_set(td_u32 id, td_u8 i_ct_mode);
td_void hdmi_reg_fcopt_unused1_set(td_u32 id, td_u8 fcopt_unused_1);
td_void hdmi_reg_ct_en_set(td_u32 id, td_u8 i_ct_en);
td_void hdmi_reg_fcopt_unused2_set(td_u32 id, td_u8 fcopt_unused_2);
td_void hdmi_reg_ct_idx_sel_set(td_u32 id, td_u8 i_ct_idx_sel);
td_void hdmi_reg_fcopt_unused_set(td_u32 id, td_u8 fcopt_unused);
td_u8 hdmi_reg_busy_get(td_u32 id);
td_void hdmi_reg_divn_h20_set(td_u32 id, td_u8 up_divn_h20);
td_void hdmi_reg_up_sampler_ratio_sel_set(td_u32 id, td_u8 up_sampler_ratio_sel);
td_void hdmi_reg_ref_cnt_set(td_u32 id, td_u16 i_ref_cnt);
td_void hdmi_reg_contin_upd_en_set(td_u32 id, td_u8 up_contin_upd_en);
td_void hdmi_reg_contin_upd_opt_set(td_u32 id, td_u8 up_contin_upd_opt);
td_void hdmi_reg_contin_upd_pol_set(td_u32 id, td_u8 up_contin_upd_pol);
td_void hdmi_reg_contin_upd_step_set(td_u32 id, td_u8 up_contin_upd_step);
td_void hdmi_reg_contin_upd_rate_set(td_u32 id, td_u32 up_contin_upd_rate);
td_void hdmi_reg_contin_upd_time_set(td_u32 id, td_u8 up_contin_upd_time);
td_void hdmi_reg_contin_upd_th_dn_set(td_u32 id, td_u16 up_contin_upd_th_dn);
td_void hdmi_reg_fccontinset2_unused_set(td_u32 id, td_u8 fccontinset2_unused);
td_void hdmi_reg_contin_upd_th_up_set(td_u32 id, td_u16 up_contin_upd_th_up);
td_void hdmi_reg_init_set(td_u32 id, td_u8 init);
td_void hdmi_reg_en_ctrl_set(td_u32 id, td_u8 en_ctrl);
td_u8 hdmi_reg_en_ctrl_get(td_u32 id);
td_void hdmi_reg_en_mod_set(td_u32 id, td_u8 en_mod);
td_u8 hdmi_reg_en_mod_get(td_u32 id);
td_void hdmi_reg_en_sdm_set(td_u32 id, td_u8 en_sdm);
td_u8 hdmi_reg_en_sdm_get(td_u32 id);
td_void hdmi_reg_mod_len_set(td_u32 id, td_u8 mod_len);
td_void hdmi_reg_mod_t_set(td_u32 id, td_u8 mod_t);
td_void hdmi_reg_mod_n_set(td_u32 id, td_u16 mod_n);
td_void hdmi_reg_mod_d_set(td_u32 id, td_u16 mod_d);
td_void hdmi_reg_fdiv_in_set(td_u32 id, td_u32 i_fdiv_in);
td_void hdmi_reg_manual_en_set(td_u32 id, td_u8 i_manual_en);
td_void hdmi_reg_mdiv_set(td_u32 id, td_u8 i_mdiv);
td_void hdmi_reg_ref_clk_sel_set(td_u32 id, td_u8 i_ref_clk_sel);
td_void hdmi_reg_pr_en_h20_set(td_u32 id, td_u8 up_pr_en_h20);
td_void hdmi_reg_enable_h20_set(td_u32 id, td_u8 up_enable_h20);
td_void hdmi_reg_ch_out_sel_set(td_u32 id, td_u8 up_ch_out_sel);
td_u8 hdmi_reg_ch_out_sel_get(td_u32 id);
td_void hdmi_reg_hsset_set(td_u32 id, td_u8 up_hsset);
td_void hdmi_reg_fd_clk_sel_set(td_u32 id, td_u8 up_fd_clk_sel);
td_void hdmi_reg_refclk_sel_set(td_u32 id, td_u8 up_refclk_sel);
td_void hdmi_reg_ctman_set(td_u32 id, td_u8 up_ctman);
td_void hdmi_reg_modclk_sel_set(td_u32 id, td_u8 up_modclk_sel);
td_void hdmi_reg_fdivclk_sel_set(td_u32 id, td_u8 up_fdivclk_sel);
td_void hdmi_reg_mod_div_val_set(td_u32 id, td_u8 mod_div_val);
td_void hdmi_reg_dac_clock_gat_set(td_u32 id, td_u8 dac_clock_gat);
td_void hdmi_reg_swreset_unused_set(td_u32 id, td_u16 swreset_unused);
td_void hdmi_reg_global_reset_set(td_u32 id, td_u8 global_reset);
td_void hdmi_reg_sw_reset_mod_clock_set(td_u32 id, td_u8 sw_reset_mod_clock);
td_void hdmi_reg_sw_reset_tmds_clock_set(td_u32 id, td_u8 sw_reset_tmds_clock);
td_void hdmi_reg_sw_reset_mpll_clock_set(td_u32 id, td_u8 sw_reset_mpll_clock);
td_void hdmi_reg_sw_reset_nco_clock_set(td_u32 id, td_u8 sw_reset_nco_clock);
td_void hdmi_reg_sw_reset_fd_clock_set(td_u32 id, td_u8 sw_reset_fd_clock);
td_void hdmi_reg_sw_reset_mod_and_mpll_clock_set(td_u32 id, td_u8 sw_reset_mod_and_mpll_clock);
td_void hdmi_reg_sw_reset_mod_and_nco_clock_set(td_u32 id, td_u8 sw_reset_mod_and_nco_clock);
td_void hdmi_reg_sw_reset_mod_and_fd_clock_set(td_u32 id, td_u8 sw_reset_mod_and_fd_clock);
td_void hdmi_reg_sw_reset_hsfifo_clock_set(td_u32 id, td_u8 sw_reset_hsfifo_clock);
td_void hdmi_reg_sw_reset_txfifo_clock_set(td_u32 id, td_u8 sw_reset_txfifo_clock);
td_void hdmi_reg_sw_reset_data_clock_set(td_u32 id, td_u8 sw_reset_data_clock);
td_void hdmi_reg_sw_reset_hs_clock_set(td_u32 id, td_u8 sw_reset_hs_clock);
td_void hdmi_reg_sw_reset_pllref_clock_set(td_u32 id, td_u8 sw_reset_pllref_clock);
td_void hdmi_reg_sw_reset_dac_clock_set(td_u32 id, td_u8 sw_reset_dac_clock);
td_void hdmi_reg_up_sample_fifo_clock_swrst_set(td_u32 id, td_u8 up_sample_fifo_clock_swrst);
td_void hdmi_reg_fccontinset0_unused_set(td_u32 id, td_u8 fccontinset0_unused);
td_void hdmi_reg_mpll_fcon_set(td_u32 id, td_u16 i_mpll_fcon);
td_void hdmi_reg_mpll_ctlck_set(td_u32 id, td_u8 i_mpll_ctlck);
td_void hdmi_reg_ctset1_unused_set(td_u32 id, td_u32 ctset1_unused);
td_void hdmi_reg_fcdstepset_unused_set(td_u32 id, td_u8 fcdstepset_unused);
td_void hdmi_reg_req_length_set(td_u32 id, td_u8 req_length);
td_void hdmi_reg_stb_cs_sel_set(td_u32 id, td_u8 stb_cs_sel);
td_void hdmi_reg_fdsrcparam_unused_set(td_u32 id, td_u8 fdsrcparam_unused);
td_void hdmi_reg_fcg_en_set(td_u32 id, td_u8 up_fcg_en);
td_void hdmi_reg_fcg_dlf_en_set(td_u32 id, td_u8 up_fcg_dlf_en);
td_void hdmi_reg_fcg_dither_en_set(td_u32 id, td_u8 up_fcg_dither_en);
td_void hdmi_reg_fcg_lock_en_set(td_u32 id, td_u8 up_fcg_lock_en);
td_void hdmi_reg_tmds_cnt_val_set(td_u32 id, td_u16 up_tmds_cnt_val);
td_void hdmi_reg_cnt1_target_set(td_u32 id, td_u16 up_cnt1_target);
td_void hdmi_reg_lock_cnt_set(td_u32 id, td_u8 up_lock_cnt);
td_void hdmi_reg_lock_th_set(td_u32 id, td_u8 up_lock_th);
td_void hdmi_reg_ki_set(td_u32 id, td_u8 up_ki);
td_void hdmi_reg_lock_mode_set(td_u32 id, td_u8 up_lock_mode);
td_u8 hdmi_reg_dlf_lock_get(td_u32 id);
td_u8 hdmi_reg_dlf_ov_get(td_u32 id);
td_u8 hdmi_reg_dlf_un_get(td_u32 id);
td_void hdmi_reg_txfifoset0_unused_set(td_u32 id, td_u8 txfifoset0_unused);
td_void hdmi_reg_mode_en_set(td_u32 id, td_u8 reg_hdmi_mode_en);
td_void hdmi_reg_sw_reset_frl_clock_set(td_u32 id, td_u8 sw_reset_frl_clock);
td_void hdmi_reg_stb_delay2_set(td_u32 id, td_u8 stb_delay2);
td_void hdmi_reg_stb_delay1_set(td_u32 id, td_u8 stb_delay1);
td_void hdmi_reg_stb_delay0_set(td_u32 id, td_u8 stb_delay0);
td_void hdmi_reg_stb_acc_sel_set(td_u32 id, td_u8 stb_acc_sel);
td_u8 hdmi_reg_clk_ok_get(td_u32 id);
td_u8 hdmi_reg_done_get(td_u32 id);
td_u8 hdmi_reg_error_get(td_u32 id);
td_u16 hdmi_reg_tmds_cnt_val_get(td_u32 id);
#endif /* __HDMI_REG_DPHY_H__ */

