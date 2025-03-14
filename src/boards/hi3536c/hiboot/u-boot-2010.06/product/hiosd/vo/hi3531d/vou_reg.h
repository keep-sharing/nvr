/*
* Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#ifndef __VOU_REG_H__
#define __VOU_REG_H__

#include "hi_type.h"
	
#ifdef __cplusplus
#if __cplusplus
	extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
/* Define the union U_VOCTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    m0_arb_mode           : 4   ; /* [3..0]  */
        unsigned int    m1_arb_mode           : 4   ; /* [7..4]  */
        unsigned int    reserved_0            : 19  ; /* [26..8]  */
        unsigned int    chn2_select           : 1   ; /* [27]  */
        unsigned int    one_sync2_en          : 1   ; /* [28]  */
        unsigned int    reserved_1            : 1   ; /* [29]  */
        unsigned int    chk_sum_en            : 1   ; /* [30]  */
        unsigned int    vo_ck_gt_en           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOCTRL;

/* Define the union U_VOINTSTA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0vtthd1_int        : 1   ; /* [0]  */
        unsigned int    dhd0vtthd2_int        : 1   ; /* [1]  */
        unsigned int    dhd0vtthd3_int        : 1   ; /* [2]  */
        unsigned int    dhd0uf_int            : 1   ; /* [3]  */
        unsigned int    dhd1vtthd1_int        : 1   ; /* [4]  */
        unsigned int    dhd1vtthd2_int        : 1   ; /* [5]  */
        unsigned int    dhd1vtthd3_int        : 1   ; /* [6]  */
        unsigned int    dhd1uf_int            : 1   ; /* [7]  */
        unsigned int    gwbc0_vte_int         : 1   ; /* [8]  */
        unsigned int    dwbc0_vte_int         : 1   ; /* [9]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    vdac0_load_int        : 1   ; /* [12]  */
        unsigned int    vdac1_load_int        : 1   ; /* [13]  */
        unsigned int    vdac2_load_int        : 1   ; /* [14]  */
        unsigned int    vdac3_load_int        : 1   ; /* [15]  */
        unsigned int    dsd0vtthd1_int        : 1   ; /* [16]  */
        unsigned int    dsd0uf_int            : 1   ; /* [17]  */
        unsigned int    v0rr_int              : 1   ; /* [18]  */
        unsigned int    v1rr_int              : 1   ; /* [19]  */
        unsigned int    v3rr_int              : 1   ; /* [20]  */
        unsigned int    v4rr_int              : 1   ; /* [21]  */
        unsigned int    g0rr_int              : 1   ; /* [22]  */
        unsigned int    g1rr_int              : 1   ; /* [23]  */
        unsigned int    g2rr_int              : 1   ; /* [24]  */
        unsigned int    g3rr_int              : 1   ; /* [25]  */
        unsigned int    g4rr_int              : 1   ; /* [26]  */
        unsigned int    g5rr_int              : 1   ; /* [27]  */
        unsigned int    wbcdhd_partfns_int    : 1   ; /* [28]  */
        unsigned int    ut_end_int            : 1   ; /* [29]  */
        unsigned int    m0_be_int             : 1   ; /* [30]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOINTSTA;

/* Define the union U_VOMSKINTSTA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0vtthd1_clr        : 1   ; /* [0]  */
        unsigned int    dhd0vtthd2_clr        : 1   ; /* [1]  */
        unsigned int    dhd0vtthd3_clr        : 1   ; /* [2]  */
        unsigned int    dhd0uf_clr            : 1   ; /* [3]  */
        unsigned int    dhd1vtthd1_clr        : 1   ; /* [4]  */
        unsigned int    dhd1vtthd2_clr        : 1   ; /* [5]  */
        unsigned int    dhd1vtthd3_clr        : 1   ; /* [6]  */
        unsigned int    dhd1uf_clr            : 1   ; /* [7]  */
        unsigned int    gwbc0_vte_clr         : 1   ; /* [8]  */
        unsigned int    dwbc0_vte_clr         : 1   ; /* [9]  */
        unsigned int    g0wbc_vte_clr         : 1   ; /* [10]  */
        unsigned int    g4wbc_vte_clr         : 1   ; /* [11]  */
        unsigned int    vdac0_load_clr        : 1   ; /* [12]  */
        unsigned int    vdac1_load_clr        : 1   ; /* [13]  */
        unsigned int    vdac2_load_clr        : 1   ; /* [14]  */
        unsigned int    vdac3_load_clr        : 1   ; /* [15]  */
        unsigned int    dsd0vtthd1_clr        : 1   ; /* [16]  */
        unsigned int    dsd0uf_clr            : 1   ; /* [17]  */
        unsigned int    v0rr_clr              : 1   ; /* [18]  */
        unsigned int    v1rr_clr              : 1   ; /* [19]  */
        unsigned int    v3rr_clr              : 1   ; /* [20]  */
        unsigned int    v4rr_clr              : 1   ; /* [21]  */
        unsigned int    g0rr_clr              : 1   ; /* [22]  */
        unsigned int    g1rr_clr              : 1   ; /* [23]  */
        unsigned int    g2rr_clr              : 1   ; /* [24]  */
        unsigned int    g3rr_clr              : 1   ; /* [25]  */
        unsigned int    g4rr_clr              : 1   ; /* [26]  */
        unsigned int    g5rr_clr              : 1   ; /* [27]  */
        unsigned int    wbcdhd_partfns_clr    : 1   ; /* [28]  */
        unsigned int    ut_end_clr            : 1   ; /* [29]  */
        unsigned int    m0_be_clr             : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOMSKINTSTA;

/* Define the union U_VOINTMSK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0vtthd1_intmsk     : 1   ; /* [0]  */
        unsigned int    dhd0vtthd2_intmsk     : 1   ; /* [1]  */
        unsigned int    dhd0vtthd3_intmsk     : 1   ; /* [2]  */
        unsigned int    dhd0uf_intmsk         : 1   ; /* [3]  */
        unsigned int    dhd1vtthd1_intmsk     : 1   ; /* [4]  */
        unsigned int    dhd1vtthd2_intmsk     : 1   ; /* [5]  */
        unsigned int    dhd1vtthd3_intmsk     : 1   ; /* [6]  */
        unsigned int    dhd1uf_intmsk         : 1   ; /* [7]  */
        unsigned int    gwbc0_vte_intmsk      : 1   ; /* [8]  */
        unsigned int    dwbc0_vte_intmsk      : 1   ; /* [9]  */
        unsigned int    g0wbc_vte_intmsk      : 1   ; /* [10]  */
        unsigned int    g4wbc_vte_intmsk      : 1   ; /* [11]  */
        unsigned int    vdac0_load_intmask    : 1   ; /* [12]  */
        unsigned int    vdac1_load_intmask    : 1   ; /* [13]  */
        unsigned int    vdac2_load_intmask    : 1   ; /* [14]  */
        unsigned int    vdac3_load_intmask    : 1   ; /* [15]  */
        unsigned int    dsd0vtthd1_intmsk     : 1   ; /* [16]  */
        unsigned int    dsd0uf_intmsk         : 1   ; /* [17]  */
        unsigned int    v0rr_intmsk           : 1   ; /* [18]  */
        unsigned int    v1rr_intmsk           : 1   ; /* [19]  */
        unsigned int    v3rr_intmsk           : 1   ; /* [20]  */
        unsigned int    v4rr_intmsk           : 1   ; /* [21]  */
        unsigned int    g0rr_intmsk           : 1   ; /* [22]  */
        unsigned int    g1rr_intmsk           : 1   ; /* [23]  */
        unsigned int    g2rr_intmsk           : 1   ; /* [24]  */
        unsigned int    g3rr_intmsk           : 1   ; /* [25]  */
        unsigned int    g4rr_intmsk           : 1   ; /* [26]  */
        unsigned int    g5rr_intmsk           : 1   ; /* [27]  */
        unsigned int    wbcdhd_partfns_intmsk : 1   ; /* [28]  */
        unsigned int    ut_end_intmsk         : 1   ; /* [29]  */
        unsigned int    m0_be_intmsk          : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOINTMSK;

/* Define the union U_VDPVERSION1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vdpversion1            : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_VDPVERSION1;
/* Define the union U_VDPVERSION2 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vdpversion2            : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_VDPVERSION2;
/* Define the union U_VODEBUG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rm_en_chn             : 4   ; /* [3..0]  */
        unsigned int    dhd0_ff_info          : 2   ; /* [5..4]  */
        unsigned int    dhd1_ff_info          : 2   ; /* [7..6]  */
        unsigned int    dsd0_ff_info          : 2   ; /* [9..8]  */
        unsigned int    bfm_vga_en            : 1   ; /* [10]  */
        unsigned int    bfm_cvbs_en           : 1   ; /* [11]  */
        unsigned int    bfm_lcd_en            : 1   ; /* [12]  */
        unsigned int    bfm_bt1120_en         : 1   ; /* [13]  */
        unsigned int    wbc2_ff_info          : 2   ; /* [15..14]  */
        unsigned int    wbc_mode              : 4   ; /* [19..16]  */
        unsigned int    node_num              : 4   ; /* [23..20]  */
        unsigned int    wbc_cmp_mode          : 2   ; /* [25..24]  */
        unsigned int    bfm_mode              : 3   ; /* [28..26]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VODEBUG;

/* Define the union U_VOINTSTA1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0vtthd1_int        : 1   ; /* [0]  */
        unsigned int    dhd0vtthd2_int        : 1   ; /* [1]  */
        unsigned int    dhd0vtthd3_int        : 1   ; /* [2]  */
        unsigned int    dhd0uf_int            : 1   ; /* [3]  */
        unsigned int    dhd1vtthd1_int        : 1   ; /* [4]  */
        unsigned int    dhd1vtthd2_int        : 1   ; /* [5]  */
        unsigned int    dhd1vtthd3_int        : 1   ; /* [6]  */
        unsigned int    dhd1uf_int            : 1   ; /* [7]  */
        unsigned int    gwbc0_vte_int         : 1   ; /* [8]  */
        unsigned int    dwbc0_vte_int         : 1   ; /* [9]  */
        unsigned int    g0wbc_vte_int         : 1   ; /* [10]  */
        unsigned int    g4wbc_vte_int         : 1   ; /* [11]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    dsd0vtthd1_int        : 1   ; /* [16]  */
        unsigned int    dsd0uf_int            : 1   ; /* [17]  */
        unsigned int    v0rr_int              : 1   ; /* [18]  */
        unsigned int    v1rr_int              : 1   ; /* [19]  */
        unsigned int    v3rr_int              : 1   ; /* [20]  */
        unsigned int    v4rr_int              : 1   ; /* [21]  */
        unsigned int    g0rr_int              : 1   ; /* [22]  */
        unsigned int    g1rr_int              : 1   ; /* [23]  */
        unsigned int    g2rr_int              : 1   ; /* [24]  */
        unsigned int    g3rr_int              : 1   ; /* [25]  */
        unsigned int    g4rr_int              : 1   ; /* [26]  */
        unsigned int    g5rr_int              : 1   ; /* [27]  */
        unsigned int    wbcdhd_partfns_int    : 1   ; /* [28]  */
        unsigned int    ut_end_int            : 1   ; /* [29]  */
        unsigned int    m0_be_int             : 1   ; /* [30]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOINTSTA1;

/* Define the union U_VOMSKINTSTA1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0vtthd1_clr        : 1   ; /* [0]  */
        unsigned int    dhd0vtthd2_clr        : 1   ; /* [1]  */
        unsigned int    dhd0vtthd3_clr        : 1   ; /* [2]  */
        unsigned int    dhd0uf_clr            : 1   ; /* [3]  */
        unsigned int    dhd1vtthd1_clr        : 1   ; /* [4]  */
        unsigned int    dhd1vtthd2_clr        : 1   ; /* [5]  */
        unsigned int    dhd1vtthd3_clr        : 1   ; /* [6]  */
        unsigned int    dhd1uf_clr            : 1   ; /* [7]  */
        unsigned int    gwbc0_vte_clr         : 1   ; /* [8]  */
        unsigned int    dwbc0_vte_clr         : 1   ; /* [9]  */
        unsigned int    g0wbc_vte_clr         : 1   ; /* [10]  */
        unsigned int    g4wbc_vte_clr         : 1   ; /* [11]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    dsd0vtthd1_clr        : 1   ; /* [16]  */
        unsigned int    dsd0uf_clr            : 1   ; /* [17]  */
        unsigned int    v0rr_clr              : 1   ; /* [18]  */
        unsigned int    v1rr_clr              : 1   ; /* [19]  */
        unsigned int    v3rr_clr              : 1   ; /* [20]  */
        unsigned int    v4rr_clr              : 1   ; /* [21]  */
        unsigned int    g0rr_clr              : 1   ; /* [22]  */
        unsigned int    g1rr_clr              : 1   ; /* [23]  */
        unsigned int    g2rr_clr              : 1   ; /* [24]  */
        unsigned int    g3rr_clr              : 1   ; /* [25]  */
        unsigned int    g4rr_clr              : 1   ; /* [26]  */
        unsigned int    g5rr_clr              : 1   ; /* [27]  */
        unsigned int    wbcdhd_partfns_clr    : 1   ; /* [28]  */
        unsigned int    ut_end_clr            : 1   ; /* [29]  */
        unsigned int    m0_be_clr             : 1   ; /* [30]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOMSKINTSTA1;

/* Define the union U_VOINTMSK1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0vtthd1_intmsk     : 1   ; /* [0]  */
        unsigned int    dhd0vtthd2_intmsk     : 1   ; /* [1]  */
        unsigned int    dhd0vtthd3_intmsk     : 1   ; /* [2]  */
        unsigned int    dhd0uf_intmsk         : 1   ; /* [3]  */
        unsigned int    dhd1vtthd1_intmsk     : 1   ; /* [4]  */
        unsigned int    dhd1vtthd2_intmsk     : 1   ; /* [5]  */
        unsigned int    dhd1vtthd3_intmsk     : 1   ; /* [6]  */
        unsigned int    dhd1uf_intmsk         : 1   ; /* [7]  */
        unsigned int    gwbc0_vte_intmsk      : 1   ; /* [8]  */
        unsigned int    dwbc0_vte_intmsk      : 1   ; /* [9]  */
        unsigned int    g0wbc_vte_intmsk      : 1   ; /* [10]  */
        unsigned int    g4wbc_vte_intmsk      : 1   ; /* [11]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    dsd0vtthd1_intmsk     : 1   ; /* [16]  */
        unsigned int    dsd0uf_intmsk         : 1   ; /* [17]  */
        unsigned int    v0rr_intmsk           : 1   ; /* [18]  */
        unsigned int    v1rr_intmsk           : 1   ; /* [19]  */
        unsigned int    v3rr_intmsk           : 1   ; /* [20]  */
        unsigned int    v4rr_intmsk           : 1   ; /* [21]  */
        unsigned int    g0rr_intmsk           : 1   ; /* [22]  */
        unsigned int    g1rr_intmsk           : 1   ; /* [23]  */
        unsigned int    g2rr_intmsk           : 1   ; /* [24]  */
        unsigned int    g3rr_intmsk           : 1   ; /* [25]  */
        unsigned int    g4rr_intmsk           : 1   ; /* [26]  */
        unsigned int    g5rr_intmsk           : 1   ; /* [27]  */
        unsigned int    wbcdhd_partfns_intmsk : 1   ; /* [28]  */
        unsigned int    ut_end_intmsk         : 1   ; /* [29]  */
        unsigned int    m0_be_intmsk          : 1   ; /* [30]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOINTMSK1;

/* Define the union U_VOAXISEL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_axi_sel            : 1   ; /* [0]  */
        unsigned int    v1_axi_sel            : 1   ; /* [1]  */
        unsigned int    reserved_0            : 1   ; /* [2]  */
        unsigned int    v3_axi_sel            : 1   ; /* [3]  */
        unsigned int    v4_axi_sel            : 1   ; /* [4]  */
        unsigned int    reserved_1            : 5   ; /* [9..5]  */
        unsigned int    g0_axi_sel            : 1   ; /* [10]  */
        unsigned int    g1_axi_sel            : 1   ; /* [11]  */
        unsigned int    g2_axi_sel            : 1   ; /* [12]  */
        unsigned int    g3_axi_sel            : 1   ; /* [13]  */
        unsigned int    g4_axi_sel            : 1   ; /* [14]  */
        unsigned int    reserved_2            : 5   ; /* [19..15]  */
        unsigned int    wbc_dhd_axi_sel       : 1   ; /* [20]  */
        unsigned int    wbc_g0_axi_sel        : 1   ; /* [21]  */
        unsigned int    reserved_3            : 1   ; /* [22]  */
        unsigned int    wbc_g4_axi_sel        : 1   ; /* [23]  */
        unsigned int    reserved_4            : 3   ; /* [26..24]  */
        unsigned int    para_axi_sel          : 1   ; /* [27]  */
        unsigned int    reserved_5            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOAXISEL;

/* Define the union U_VOAXICTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    m0_outstd_rid0        : 4   ; /* [3..0]  */
        unsigned int    m0_outstd_rid1        : 4   ; /* [7..4]  */
        unsigned int    m0_wr_ostd            : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 3   ; /* [14..12]  */
        unsigned int    m0_id_sel             : 1   ; /* [15]  */
        unsigned int    m1_outstd_rid0        : 4   ; /* [19..16]  */
        unsigned int    m1_outstd_rid1        : 4   ; /* [23..20]  */
        unsigned int    m1_wr_ostd            : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 3   ; /* [30..28]  */
        unsigned int    m1_id_sel             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOAXICTRL;

/* Define the union U_VOWBCARB0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    prio0                 : 3   ; /* [2..0]  */
        unsigned int    prio1                 : 3   ; /* [5..3]  */
        unsigned int    prio2                 : 3   ; /* [8..6]  */
        unsigned int    prio3                 : 3   ; /* [11..9]  */
        unsigned int    prio4                 : 3   ; /* [14..12]  */
        unsigned int    prio5                 : 3   ; /* [17..15]  */
        unsigned int    prio6                 : 3   ; /* [20..18]  */
        unsigned int    reserved_0            : 7   ; /* [27..21]  */
        unsigned int    w_arb_mode            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOWBCARB0;

/* Define the union U_VOWBCARB1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    prio0                 : 3   ; /* [2..0]  */
        unsigned int    prio1                 : 3   ; /* [5..3]  */
        unsigned int    prio2                 : 3   ; /* [8..6]  */
        unsigned int    prio3                 : 3   ; /* [11..9]  */
        unsigned int    prio4                 : 3   ; /* [14..12]  */
        unsigned int    prio5                 : 3   ; /* [17..15]  */
        unsigned int    prio6                 : 3   ; /* [20..18]  */
        unsigned int    reserved_0            : 7   ; /* [27..21]  */
        unsigned int    w_arb_mode            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOWBCARB1;

/* Define the union U_VOUFSTA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_uf_sta             : 1   ; /* [0]  */
        unsigned int    v1_uf_sta             : 1   ; /* [1]  */
        unsigned int    reserved_0            : 1   ; /* [2]  */
        unsigned int    v3_uf_sta             : 1   ; /* [3]  */
        unsigned int    reserved_1            : 4   ; /* [7..4]  */
        unsigned int    g0_uf_sta             : 1   ; /* [8]  */
        unsigned int    g1_uf_sta             : 1   ; /* [9]  */
        unsigned int    g2_uf_sta             : 1   ; /* [10]  */
        unsigned int    g3_uf_sta             : 1   ; /* [11]  */
        unsigned int    g4_uf_sta             : 1   ; /* [12]  */
        unsigned int    reserved_2            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOUFSTA;

/* Define the union U_VOUFCLR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_uf_clr             : 1   ; /* [0]  */
        unsigned int    v1_uf_clr             : 1   ; /* [1]  */
        unsigned int    reserved_0            : 1   ; /* [2]  */
        unsigned int    v3_uf_clr             : 1   ; /* [3]  */
        unsigned int    reserved_1            : 4   ; /* [7..4]  */
        unsigned int    g0_uf_clr             : 1   ; /* [8]  */
        unsigned int    g1_uf_clr             : 1   ; /* [9]  */
        unsigned int    g2_uf_clr             : 1   ; /* [10]  */
        unsigned int    g3_uf_clr             : 1   ; /* [11]  */
        unsigned int    g4_uf_clr             : 1   ; /* [12]  */
        unsigned int    reserved_2            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOUFCLR;

/* Define the union U_VOINTPROC_TIM */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vointproc_time        : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VOINTPROC_TIM;

/* Define the union U_VO_MUX */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sddate_sel            : 4   ; /* [3..0]  */
        unsigned int    hddate_sel            : 4   ; /* [7..4]  */
        unsigned int    vga_sel               : 4   ; /* [11..8]  */
        unsigned int    hdmi_sel              : 4   ; /* [15..12]  */
        unsigned int    lcd_sel               : 4   ; /* [19..16]  */
        unsigned int    bt1120_sel            : 4   ; /* [23..20]  */
        unsigned int    bt656_sel             : 4   ; /* [27..24]  */
        unsigned int    digital_sel           : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_MUX;

/* Define the union U_VO_MUX_DAC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dac0_sel              : 4   ; /* [3..0]  */
        unsigned int    dac1_sel              : 4   ; /* [7..4]  */
        unsigned int    dac2_sel              : 4   ; /* [11..8]  */
        unsigned int    dac3_sel              : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_MUX_DAC;

/* Define the union U_VO_MUX_TESTSYNC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_dv               : 1   ; /* [0]  */
        unsigned int    test_hsync            : 1   ; /* [1]  */
        unsigned int    test_vsync            : 1   ; /* [2]  */
        unsigned int    test_field            : 1   ; /* [3]  */
        unsigned int    reserved_0            : 27  ; /* [30..4]  */
        unsigned int    vo_test_en            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_MUX_TESTSYNC;

/* Define the union U_VO_MUX_TESTDATA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_data             : 30  ; /* [29..0]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_MUX_TESTDATA;

/* Define the union U_VO_DAC_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dac_reg_rev           : 16  ; /* [15..0]  */
        unsigned int    enctr                 : 4   ; /* [19..16]  */
        unsigned int    enextref              : 1   ; /* [20]  */
        unsigned int    pdchopper             : 1   ; /* [21]  */
        unsigned int    envbg                 : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DAC_CTRL;

/* Define the union U_VO_DAC_C_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cablectr              : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    dacgc                 : 6   ; /* [9..4]  */
        unsigned int    reserved_1            : 21  ; /* [30..10]  */
        unsigned int    dac_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DAC_C_CTRL;

/* Define the union U_VO_DAC_R_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cablectr              : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    dacgc                 : 6   ; /* [9..4]  */
        unsigned int    reserved_1            : 21  ; /* [30..10]  */
        unsigned int    dac_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DAC_R_CTRL;

/* Define the union U_VO_DAC_G_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cablectr              : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    dacgc                 : 6   ; /* [9..4]  */
        unsigned int    reserved_1            : 21  ; /* [30..10]  */
        unsigned int    dac_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DAC_G_CTRL;

/* Define the union U_VO_DAC_B_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cablectr              : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    dacgc                 : 6   ; /* [9..4]  */
        unsigned int    reserved_1            : 21  ; /* [30..10]  */
        unsigned int    dac_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DAC_B_CTRL;

/* Define the union U_VO_DAC_STAT0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dout_rct              : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DAC_STAT0;

/* Define the union U_VO_DAC_STAT1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctbl_out_c            : 8   ; /* [7..0]  */
        unsigned int    ctrl_out_b            : 8   ; /* [15..8]  */
        unsigned int    ctrl_out_g            : 8   ; /* [23..16]  */
        unsigned int    ctrl_out_r            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DAC_STAT1;

/* Define the union U_WBC_DHD_LOCATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbc_dhd0_locate       : 1   ; /* [0]  */
        unsigned int    wbc_dhd1_locate       : 1   ; /* [1]  */
        unsigned int    wbc_dsd0_locate       : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD_LOCATE;

/* Define the union U_WBC_OFL_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbc0_ofl_en           : 1   ; /* [0]  */
        unsigned int    wbc0_ofl_pro          : 1   ; /* [1]  */
        unsigned int    wbc1_ofl_en           : 1   ; /* [2]  */
        unsigned int    wbc1_ofl_pro          : 1   ; /* [3]  */
        unsigned int    wbc2_ofl_en           : 1   ; /* [4]  */
        unsigned int    wbc2_ofl_pro          : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_OFL_EN;

/* Define the union U_VHD_CORRESP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 4   ; /* [3..0]  */
        unsigned int    v1_corresp            : 4   ; /* [7..4]  */
        unsigned int    reserved_1            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VHD_CORRESP;

/* Define the union U_GDC_CORRESP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [7..0]  */
        unsigned int    g2_corresp            : 4   ; /* [11..8]  */
        unsigned int    g3_corresp            : 4   ; /* [15..12]  */
        unsigned int    reserved_1            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GDC_CORRESP;

/* Define the union U_WBC_CORRESP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbc_corresp           : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_CORRESP;

/* Define the union U_COEF_DATA */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_data              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_COEF_DATA;
/* Define the union U_V0_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_hlcoef_rd          : 1   ; /* [0]  */
        unsigned int    v0_hccoef_rd          : 1   ; /* [1]  */
        unsigned int    v0_vlcoef_rd          : 1   ; /* [2]  */
        unsigned int    v0_vccoef_rd          : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_PARARD;

/* Define the union U_V1_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v1_hlcoef_rd          : 1   ; /* [0]  */
        unsigned int    v1_hccoef_rd          : 1   ; /* [1]  */
        unsigned int    v1_vlcoef_rd          : 1   ; /* [2]  */
        unsigned int    v1_vccoef_rd          : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V1_PARARD;

/* Define the union U_V3_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v3_hlcoef_rd          : 1   ; /* [0]  */
        unsigned int    v3_hccoef_rd          : 1   ; /* [1]  */
        unsigned int    v3_vlcoef_rd          : 1   ; /* [2]  */
        unsigned int    v3_vccoef_rd          : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V3_PARARD;

/* Define the union U_VP0_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vp0_acmlut_rd         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_PARARD;

/* Define the union U_GP0_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gp0_hlcoef_rd         : 1   ; /* [0]  */
        unsigned int    gp0_hccoef_rd         : 1   ; /* [1]  */
        unsigned int    gp0_vlcoef_rd         : 1   ; /* [2]  */
        unsigned int    gp0_vccoef_rd         : 1   ; /* [3]  */
        unsigned int    gp0_gti_hlcoef_rd     : 1   ; /* [4]  */
        unsigned int    gp0_gti_hccoef_rd     : 1   ; /* [5]  */
        unsigned int    gp0_gti_vlcoef_rd     : 1   ; /* [6]  */
        unsigned int    gp0_gti_vccoef_rd     : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_PARARD;

/* Define the union U_GP1_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gp1_hlcoef_rd         : 1   ; /* [0]  */
        unsigned int    gp1_hccoef_rd         : 1   ; /* [1]  */
        unsigned int    gp1_vlcoef_rd         : 1   ; /* [2]  */
        unsigned int    gp1_vccoef_rd         : 1   ; /* [3]  */
        unsigned int    gp1_gti_hlcoef_rd     : 1   ; /* [4]  */
        unsigned int    gp1_gti_hccoef_rd     : 1   ; /* [5]  */
        unsigned int    gp1_gti_vlcoef_rd     : 1   ; /* [6]  */
        unsigned int    gp1_gti_vccoef_rd     : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP1_PARARD;

/* Define the union U_WBCDHD_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbcdhd_hlcoef_rd      : 1   ; /* [0]  */
        unsigned int    wbcdhd_hccoef_rd      : 1   ; /* [1]  */
        unsigned int    wbcdhd_vlcoef_rd      : 1   ; /* [2]  */
        unsigned int    wbcdhd_vccoef_rd      : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBCDHD_PARARD;

/* Define the union U_DHD0_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0_gmmr_rd          : 1   ; /* [0]  */
        unsigned int    dhd0_gmmg_rd          : 1   ; /* [1]  */
        unsigned int    dhd0_gmmb_rd          : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_PARARD;

/* Define the union U_DHD1_PARARD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd1_gmmr_rd          : 1   ; /* [0]  */
        unsigned int    dhd1_gmmg_rd          : 1   ; /* [1]  */
        unsigned int    dhd1_gmmb_rd          : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD1_PARARD;

/* Define the union U_V0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ifmt                  : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 1   ; /* [4]  */
        unsigned int    req_ctrl              : 3   ; /* [7..5]  */
        unsigned int    dcmp_en               : 1   ; /* [8]  */
        unsigned int    reserved_1            : 1   ; /* [9]  */
        unsigned int    nosec_flag            : 1   ; /* [10]  */
        unsigned int    uv_order              : 1   ; /* [11]  */
        unsigned int    chm_rmode             : 2   ; /* [13..12]  */
        unsigned int    lm_rmode              : 2   ; /* [15..14]  */
        unsigned int    reserved_2            : 1   ; /* [16]  */
        unsigned int    vup_mode              : 1   ; /* [17]  */
        unsigned int    ifir_mode             : 2   ; /* [19..18]  */
        unsigned int    reserved_3            : 6   ; /* [25..20]  */
        unsigned int    flip_en               : 1   ; /* [26]  */
        unsigned int    mute_en               : 1   ; /* [27]  */
        unsigned int    reserved_4            : 3   ; /* [30..28]  */
        unsigned int    surface_en            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CTRL;

/* Define the union U_V0_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_UPD;

/* Define the union U_V0_PRERD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [30..0]  */
        unsigned int    pre_rd_en             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_PRERD;

/* Define the union U_V0_IRESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    iw                    : 12  ; /* [11..0]  */
        unsigned int    ih                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_IRESO;

/* Define the union U_V0_ORESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ow                    : 12  ; /* [11..0]  */
        unsigned int    oh                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_ORESO;

/* Define the union U_V0_CBMPARA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    galpha                : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CBMPARA;

/* Define the union U_V0_PARAUP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_hlcoef_upd         : 1   ; /* [0]  */
        unsigned int    v0_hccoef_upd         : 1   ; /* [1]  */
        unsigned int    v0_vlcoef_upd         : 1   ; /* [2]  */
        unsigned int    v0_vccoef_upd         : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_PARAUP;

/* Define the union U_V0_CPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    src_xfpos             : 12  ; /* [11..0]  */
        unsigned int    src_xlpos             : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CPOS;

/* Define the union U_V0_DRAWMODE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    draw_mode             : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_DRAWMODE;

/* Define the union U_V0_HLCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_HLCOEFAD;
/* Define the union U_V0_HCCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_HCCOEFAD;
/* Define the union U_V0_VLCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_VLCOEFAD;
/* Define the union U_V0_VCCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_VCCOEFAD;
/* Define the union U_V0_CSC_IDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 11  ; /* [10..0]  */
        unsigned int    cscidc1               : 11  ; /* [21..11]  */
        unsigned int    csc_en                : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_IDC;

/* Define the union U_V0_CSC_ODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 11  ; /* [10..0]  */
        unsigned int    cscodc1               : 11  ; /* [21..11]  */
        unsigned int    csc_sign_mode         : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_ODC;

/* Define the union U_V0_CSC_IODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc2               : 11  ; /* [10..0]  */
        unsigned int    cscodc2               : 11  ; /* [21..11]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_IODC;

/* Define the union U_V0_CSC_P0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_P0;

/* Define the union U_V0_CSC_P1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_P1;

/* Define the union U_V0_CSC_P2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_P2;

/* Define the union U_V0_CSC_P3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_P3;

/* Define the union U_V0_CSC_P4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_CSC_P4;

/* Define the union U_V0_HSP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hratio                : 24  ; /* [23..0]  */
        unsigned int    hfir_order            : 1   ; /* [24]  */
        unsigned int    hchfir_en             : 1   ; /* [25]  */
        unsigned int    hlfir_en              : 1   ; /* [26]  */
        unsigned int    non_lnr_en            : 1   ; /* [27]  */
        unsigned int    hchmid_en             : 1   ; /* [28]  */
        unsigned int    hlmid_en              : 1   ; /* [29]  */
        unsigned int    hchmsc_en             : 1   ; /* [30]  */
        unsigned int    hlmsc_en              : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_HSP;

/* Define the union U_V0_HLOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hor_loffset           : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_HLOFFSET;

/* Define the union U_V0_HCOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hor_coffset           : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_HCOFFSET;

/* Define the union U_V0_VSP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 19  ; /* [18..0]  */
        unsigned int    zme_in_fmt            : 2   ; /* [20..19]  */
        unsigned int    zme_out_fmt           : 2   ; /* [22..21]  */
        unsigned int    vchfir_en             : 1   ; /* [23]  */
        unsigned int    vlfir_en              : 1   ; /* [24]  */
        unsigned int    reserved_1            : 1   ; /* [25]  */
        unsigned int    vsc_chroma_tap        : 1   ; /* [26]  */
        unsigned int    reserved_2            : 1   ; /* [27]  */
        unsigned int    vchmid_en             : 1   ; /* [28]  */
        unsigned int    vlmid_en              : 1   ; /* [29]  */
        unsigned int    vchmsc_en             : 1   ; /* [30]  */
        unsigned int    vlmsc_en              : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_VSP;

/* Define the union U_V0_VSR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vratio                : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_VSR;

/* Define the union U_V0_VOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vchroma_offset        : 16  ; /* [15..0]  */
        unsigned int    vluma_offset          : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_VOFFSET;

/* Define the union U_V0_VBOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbchroma_offset       : 16  ; /* [15..0]  */
        unsigned int    vbluma_offset         : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_VBOFFSET;

/* Define the union U_V0_DFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xfpos            : 12  ; /* [11..0]  */
        unsigned int    disp_yfpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_DFPOS;

/* Define the union U_V0_DLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xlpos            : 12  ; /* [11..0]  */
        unsigned int    disp_ylpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_DLPOS;

/* Define the union U_V0_VFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xfpos           : 12  ; /* [11..0]  */
        unsigned int    video_yfpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_VFPOS;

/* Define the union U_V0_VLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xlpos           : 12  ; /* [11..0]  */
        unsigned int    video_ylpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_VLPOS;

/* Define the union U_V0_BK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_cr                : 10  ; /* [9..0]  */
        unsigned int    vbk_cb                : 10  ; /* [19..10]  */
        unsigned int    vbk_y                 : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_BK;

/* Define the union U_V0_ALPHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_alpha             : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_ALPHA;

/* Define the union U_V0_RIMWIDTH */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_rim_width          : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_RIMWIDTH;

/* Define the union U_V0_RIMCOL0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_rim_v0             : 10  ; /* [9..0]  */
        unsigned int    v0_rim_u0             : 10  ; /* [19..10]  */
        unsigned int    v0_rim_y0             : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_RIMCOL0;

/* Define the union U_V0_RIMCOL1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_rim_v1             : 10  ; /* [9..0]  */
        unsigned int    v0_rim_u1             : 10  ; /* [19..10]  */
        unsigned int    v0_rim_y1             : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_RIMCOL1;

/* Define the union U_V0_IFIRCOEF01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef0                 : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef1                 : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_IFIRCOEF01;

/* Define the union U_V0_IFIRCOEF23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef2                 : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef3                 : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_IFIRCOEF23;

/* Define the union U_V0_IFIRCOEF45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef4                 : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef5                 : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_IFIRCOEF45;

/* Define the union U_V0_IFIRCOEF67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef6                 : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef7                 : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_IFIRCOEF67;

/* Define the union U_V0_P0RESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    w                     : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_P0RESO;

/* Define the union U_V0_P0LADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_addr           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_P0LADDR;
/* Define the union U_V0_P0CADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_addr           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_P0CADDR;
/* Define the union U_V0_P0STRIDE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    surface_stride        : 16  ; /* [15..0]  */
        unsigned int    surface_cstride       : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_P0STRIDE;

/* Define the union U_V0_P0VFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xfpos           : 12  ; /* [11..0]  */
        unsigned int    video_yfpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_P0VFPOS;

/* Define the union U_V0_P0VLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xlpos           : 12  ; /* [11..0]  */
        unsigned int    video_ylpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_P0VLPOS;

/* Define the union U_V0_P0CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    p0_rim_col_mod        : 1   ; /* [0]  */
        unsigned int    p0_rim_en             : 1   ; /* [1]  */
        unsigned int    p0_c_loss_en          : 1   ; /* [2]  */
        unsigned int    p0_l_loss_en          : 1   ; /* [3]  */
        unsigned int    p0_dcmp_en            : 1   ; /* [4]  */
        unsigned int    p0_en                 : 1   ; /* [5]  */
        unsigned int    mute_en               : 1   ; /* [6]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_P0CTRL;

/* Define the union U_V0_NADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_naddr          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_NADDR;
/* Define the union U_V0_NCADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_ncaddr         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_NCADDR;
/* Define the union U_V0_MULTI_MODE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mrg_mode              : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_MULTI_MODE;

/* Define the union U_V0_LADDROFFSET */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int laddr_offset           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_LADDROFFSET;
/* Define the union U_V0_CADDROFFSET */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int caddr_offset           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_CADDROFFSET;
/* Define the union U_V0_FDRFIFOTHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ff_thd                : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_FDRFIFOTHD;

/* Define the union U_V0_DCMP_LSTATE0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int dcmp_l_state0          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_DCMP_LSTATE0;
/* Define the union U_V0_DCMP_LSTATE1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int dcmp_l_state1          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_DCMP_LSTATE1;
/* Define the union U_V0_DCMP_CSTATE0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int dcmp_c_state0          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_DCMP_CSTATE0;
/* Define the union U_V0_DCMP_CSTATE1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int dcmp_c_state1          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_V0_DCMP_CSTATE1;
/* Define the union U_VO_DCMPERRCLR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dcmp_l_errclr         : 1   ; /* [0]  */
        unsigned int    dcmp_c_errclr         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_DCMPERRCLR;

/* Define the union U_V0_DCMP_ERR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dcmp_l_wrong          : 1   ; /* [0]  */
        unsigned int    dcmp_c_wrong          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_DCMP_ERR;

/* Define the union U_VO_MRGERRCLR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mrg_errclr            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VO_MRGERRCLR;

/* Define the union U_V0_MRG_ERR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mrg_wrong             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_V0_MRG_ERR;

/* Define the union U_VP0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vp_galpha             : 8   ; /* [7..0]  */
        unsigned int    mute_en               : 1   ; /* [8]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CTRL;

/* Define the union U_VP0_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_UPD;

/* Define the union U_VP0_ACC_CAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_VP0_ACC_CAD;
/* Define the union U_VP0_ACM_CAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_VP0_ACM_CAD;
/* Define the union U_VP0_PARAUP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v0_acmcoef_upd        : 1   ; /* [0]  */
        unsigned int    v0_acccoef_upd        : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_PARAUP;

/* Define the union U_VP0_IRESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    iw                    : 12  ; /* [11..0]  */
        unsigned int    ih                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_IRESO;

/* Define the union U_VP0_DOF_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [30..0]  */
        unsigned int    dof_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_DOF_CTRL;

/* Define the union U_VP0_DOF_STEP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    left_step             : 8   ; /* [7..0]  */
        unsigned int    right_step            : 8   ; /* [15..8]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_DOF_STEP;

/* Define the union U_VP0_ACCTHD1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    thd_low               : 10  ; /* [9..0]  */
        unsigned int    thd_high              : 10  ; /* [19..10]  */
        unsigned int    thd_med_low           : 10  ; /* [29..20]  */
        unsigned int    acc_mode              : 1   ; /* [30]  */
        unsigned int    acc_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCTHD1;

/* Define the union U_VP0_ACCTHD2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    thd_med_high          : 10  ; /* [9..0]  */
        unsigned int    acc_multiple          : 8   ; /* [17..10]  */
        unsigned int    acc_rst               : 1   ; /* [18]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCTHD2;

/* Define the union U_VP0_ACCLOWN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    table_data1n          : 10  ; /* [9..0]  */
        unsigned int    table_data2n          : 10  ; /* [19..10]  */
        unsigned int    table_data3n          : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCLOWN;

/* Define the union U_VP0_ACCMEDN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    table_data1n          : 10  ; /* [9..0]  */
        unsigned int    table_data2n          : 10  ; /* [19..10]  */
        unsigned int    table_data3n          : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCMEDN;

/* Define the union U_VP0_ACCHIGHN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    table_data1n          : 10  ; /* [9..0]  */
        unsigned int    table_data2n          : 10  ; /* [19..10]  */
        unsigned int    table_data3n          : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCHIGHN;

/* Define the union U_VP0_ACCMLN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    table_data1n          : 10  ; /* [9..0]  */
        unsigned int    table_data2n          : 10  ; /* [19..10]  */
        unsigned int    table_data3n          : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCMLN;

/* Define the union U_VP0_ACCMHN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    table_data1n          : 10  ; /* [9..0]  */
        unsigned int    table_data2n          : 10  ; /* [19..10]  */
        unsigned int    table_data3n          : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCMHN;

/* Define the union U_VP0_ACC3LOW */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cnt3_low              : 21  ; /* [20..0]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACC3LOW;

/* Define the union U_VP0_ACC3MED */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cnt3_med              : 21  ; /* [20..0]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACC3MED;

/* Define the union U_VP0_ACC3HIGH */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cnt3_high             : 21  ; /* [20..0]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACC3HIGH;

/* Define the union U_VP0_ACC8MLOW */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cnt8_med_low          : 21  ; /* [20..0]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACC8MLOW;

/* Define the union U_VP0_ACC8MHIGH */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cnt8_med_high         : 21  ; /* [20..0]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACC8MHIGH;

/* Define the union U_VP0_ACCTOTAL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    acc_pix_total         : 21  ; /* [20..0]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACCTOTAL;

/* Define the union U_VP0_ACM_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    acm_cbcrthr           : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 19  ; /* [26..8]  */
        unsigned int    acm_cliporwrap        : 1   ; /* [27]  */
        unsigned int    acm_cliprange         : 1   ; /* [28]  */
        unsigned int    acm_stretch           : 1   ; /* [29]  */
        unsigned int    acm_dbg_en            : 1   ; /* [30]  */
        unsigned int    acm_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACM_CTRL;

/* Define the union U_VP0_ACM_ADJ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    acm_gain2             : 10  ; /* [9..0]  */
        unsigned int    acm_gain1             : 10  ; /* [19..10]  */
        unsigned int    acm_gain0             : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ACM_ADJ;

/* Define the union U_VP0_DFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xfpos            : 12  ; /* [11..0]  */
        unsigned int    disp_yfpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_DFPOS;

/* Define the union U_VP0_DLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xlpos            : 12  ; /* [11..0]  */
        unsigned int    disp_ylpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_DLPOS;

/* Define the union U_VP0_VFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xfpos           : 12  ; /* [11..0]  */
        unsigned int    video_yfpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_VFPOS;

/* Define the union U_VP0_VLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xlpos           : 12  ; /* [11..0]  */
        unsigned int    video_ylpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_VLPOS;

/* Define the union U_VP0_BK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_cr                : 10  ; /* [9..0]  */
        unsigned int    vbk_cb                : 10  ; /* [19..10]  */
        unsigned int    vbk_y                 : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_BK;

/* Define the union U_VP0_ALPHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_alpha             : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_ALPHA;

/* Define the union U_VP0_CSC0_IDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 11  ; /* [10..0]  */
        unsigned int    cscidc1               : 11  ; /* [21..11]  */
        unsigned int    csc_en                : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_IDC;

/* Define the union U_VP0_CSC0_ODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 11  ; /* [10..0]  */
        unsigned int    cscodc1               : 11  ; /* [21..11]  */
        unsigned int    csc_sign_mode         : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_ODC;

/* Define the union U_VP0_CSC0_IODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc2               : 11  ; /* [10..0]  */
        unsigned int    cscodc2               : 11  ; /* [21..11]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_IODC;

/* Define the union U_VP0_CSC0_P0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_P0;

/* Define the union U_VP0_CSC0_P1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_P1;

/* Define the union U_VP0_CSC0_P2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_P2;

/* Define the union U_VP0_CSC0_P3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_P3;

/* Define the union U_VP0_CSC0_P4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC0_P4;

/* Define the union U_VP0_CSC1_IDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 11  ; /* [10..0]  */
        unsigned int    cscidc1               : 11  ; /* [21..11]  */
        unsigned int    csc_en                : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_IDC;

/* Define the union U_VP0_CSC1_ODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 11  ; /* [10..0]  */
        unsigned int    cscodc1               : 11  ; /* [21..11]  */
        unsigned int    csc_sign_mode         : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_ODC;

/* Define the union U_VP0_CSC1_IODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc2               : 11  ; /* [10..0]  */
        unsigned int    cscodc2               : 11  ; /* [21..11]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_IODC;

/* Define the union U_VP0_CSC1_P0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_P0;

/* Define the union U_VP0_CSC1_P1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_P1;

/* Define the union U_VP0_CSC1_P2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_P2;

/* Define the union U_VP0_CSC1_P3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_P3;

/* Define the union U_VP0_CSC1_P4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VP0_CSC1_P4;

/* Define the union U_G0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ifmt                  : 8   ; /* [7..0]  */
        unsigned int    bitext                : 2   ; /* [9..8]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    req_ctrl              : 3   ; /* [14..12]  */
        unsigned int    reserved_1            : 6   ; /* [20..15]  */
        unsigned int    cmp_mode              : 1   ; /* [21]  */
        unsigned int    lossless_a            : 1   ; /* [22]  */
        unsigned int    lossless              : 1   ; /* [23]  */
        unsigned int    flip_en               : 1   ; /* [24]  */
        unsigned int    dcmp_en               : 1   ; /* [25]  */
        unsigned int    read_mode             : 1   ; /* [26]  */
        unsigned int    upd_mode              : 1   ; /* [27]  */
        unsigned int    mute_en               : 1   ; /* [28]  */
        unsigned int    reserved_2            : 1   ; /* [29]  */
        unsigned int    nosec_flag            : 1   ; /* [30]  */
        unsigned int    surface_en            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CTRL;

/* Define the union U_G0_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_UPD;

/* Define the union U_G0_ADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_addr           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_G0_ADDR;
/* Define the union U_G0_NADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_addr           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_G0_NADDR;
/* Define the union U_G0_STRIDE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    surface_stride        : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_STRIDE;

/* Define the union U_G0_IRESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    iw                    : 12  ; /* [11..0]  */
        unsigned int    ih                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_IRESO;

/* Define the union U_G0_SFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    src_xfpos             : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_SFPOS;

/* Define the union U_G0_CBMPARA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    galpha                : 8   ; /* [7..0]  */
        unsigned int    palpha_range          : 1   ; /* [8]  */
        unsigned int    reserved_0            : 1   ; /* [9]  */
        unsigned int    vedge_p               : 1   ; /* [10]  */
        unsigned int    hedge_p               : 1   ; /* [11]  */
        unsigned int    palpha_en             : 1   ; /* [12]  */
        unsigned int    premult_en            : 1   ; /* [13]  */
        unsigned int    key_en                : 1   ; /* [14]  */
        unsigned int    key_mode              : 1   ; /* [15]  */
        unsigned int    reserved_1            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CBMPARA;

/* Define the union U_G0_CKEYMAX */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    keyb_max              : 8   ; /* [7..0]  */
        unsigned int    keyg_max              : 8   ; /* [15..8]  */
        unsigned int    keyr_max              : 8   ; /* [23..16]  */
        unsigned int    va0                   : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CKEYMAX;

/* Define the union U_G0_CKEYMIN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    keyb_min              : 8   ; /* [7..0]  */
        unsigned int    keyg_min              : 8   ; /* [15..8]  */
        unsigned int    keyr_min              : 8   ; /* [23..16]  */
        unsigned int    va1                   : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CKEYMIN;

/* Define the union U_G0_CMASK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    kmsk_b                : 8   ; /* [7..0]  */
        unsigned int    kmsk_g                : 8   ; /* [15..8]  */
        unsigned int    kmsk_r                : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CMASK;

/* Define the union U_G0_PARAADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_addr           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_G0_PARAADDR;
/* Define the union U_G0_PARAUP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gdc_paraup            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_PARAUP;

/* Define the union U_G0_FIFOTHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    buf_wr_thd            : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 21  ; /* [30..10]  */
        unsigned int    gdc_buf_en            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_FIFOTHD;

/* Define the union U_G0_DCMP_ADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_addr           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_G0_DCMP_ADDR;
/* Define the union U_G0_DCMP_NADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int surface_addr           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_G0_DCMP_NADDR;
/* Define the union U_G0_DCMP_OFFSET */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int offset                 : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_G0_DCMP_OFFSET;
/* Define the union U_G0_DCMP_DBG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int dcmp_dbg               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_G0_DCMP_DBG;
/* Define the union U_G0_DCMP_INTER */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gdccerrclr            : 1   ; /* [0]  */
        unsigned int    widthlen_crr          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_DCMP_INTER;

/* Define the union U_G0_DFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xfpos            : 12  ; /* [11..0]  */
        unsigned int    disp_yfpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_DFPOS;

/* Define the union U_G0_DLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xlpos            : 12  ; /* [11..0]  */
        unsigned int    disp_ylpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_DLPOS;

/* Define the union U_G0_VFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xfpos           : 12  ; /* [11..0]  */
        unsigned int    video_yfpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_VFPOS;

/* Define the union U_G0_VLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xlpos           : 12  ; /* [11..0]  */
        unsigned int    video_ylpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_VLPOS;

/* Define the union U_G0_BK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_cr                : 10  ; /* [9..0]  */
        unsigned int    vbk_cb                : 10  ; /* [19..10]  */
        unsigned int    vbk_y                 : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_BK;

/* Define the union U_G0_ALPHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_alpha             : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_ALPHA;

/* Define the union U_G0_DOF_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [30..0]  */
        unsigned int    dof_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_DOF_CTRL;

/* Define the union U_G0_DOF_STEP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    left_step             : 8   ; /* [7..0]  */
        unsigned int    right_step            : 8   ; /* [15..8]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_DOF_STEP;

/* Define the union U_G0_CSC_IDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 11  ; /* [10..0]  */
        unsigned int    cscidc1               : 11  ; /* [21..11]  */
        unsigned int    csc_en                : 1   ; /* [22]  */
        unsigned int    csc_mode              : 3   ; /* [25..23]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_IDC;

/* Define the union U_G0_CSC_ODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 11  ; /* [10..0]  */
        unsigned int    cscodc1               : 11  ; /* [21..11]  */
        unsigned int    csc_sign_mode         : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_ODC;

/* Define the union U_G0_CSC_IODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc2               : 11  ; /* [10..0]  */
        unsigned int    cscodc2               : 11  ; /* [21..11]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_IODC;

/* Define the union U_G0_CSC_P0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_P0;

/* Define the union U_G0_CSC_P1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_P1;

/* Define the union U_G0_CSC_P2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_P2;

/* Define the union U_G0_CSC_P3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_P3;

/* Define the union U_G0_CSC_P4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_G0_CSC_P4;

/* Define the union U_GP0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mux1_sel              : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    mux2_sel              : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    aout_sel              : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 2   ; /* [11..10]  */
        unsigned int    bout_sel              : 2   ; /* [13..12]  */
        unsigned int    reserved_3            : 12  ; /* [25..14]  */
        unsigned int    ffc_sel               : 1   ; /* [26]  */
        unsigned int    reserved_4            : 3   ; /* [29..27]  */
        unsigned int    mute_en               : 1   ; /* [30]  */
        unsigned int    read_mode             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CTRL;

/* Define the union U_GP0_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_UPD;

/* Define the union U_GP0_ORESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ow                    : 12  ; /* [11..0]  */
        unsigned int    oh                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ORESO;

/* Define the union U_GP0_IRESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    iw                    : 12  ; /* [11..0]  */
        unsigned int    ih                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_IRESO;

/* Define the union U_GP0_HCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_GP0_HCOEFAD;
/* Define the union U_GP0_VCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_GP0_VCOEFAD;
/* Define the union U_GP0_PARAUP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gp0_hcoef_upd         : 1   ; /* [0]  */
        unsigned int    gp0_vcoef_upd         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_PARAUP;

/* Define the union U_GP0_GALPHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    galpha                : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_GALPHA;

/* Define the union U_GP0_DFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xfpos            : 12  ; /* [11..0]  */
        unsigned int    disp_yfpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_DFPOS;

/* Define the union U_GP0_DLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    disp_xlpos            : 12  ; /* [11..0]  */
        unsigned int    disp_ylpos            : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_DLPOS;

/* Define the union U_GP0_VFPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xfpos           : 12  ; /* [11..0]  */
        unsigned int    video_yfpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_VFPOS;

/* Define the union U_GP0_VLPOS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_xlpos           : 12  ; /* [11..0]  */
        unsigned int    video_ylpos           : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_VLPOS;

/* Define the union U_GP0_BK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_cr                : 10  ; /* [9..0]  */
        unsigned int    vbk_cb                : 10  ; /* [19..10]  */
        unsigned int    vbk_y                 : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_BK;

/* Define the union U_GP0_ALPHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbk_alpha             : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ALPHA;

/* Define the union U_GP0_CSC_IDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 11  ; /* [10..0]  */
        unsigned int    cscidc1               : 11  ; /* [21..11]  */
        unsigned int    csc_en                : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_IDC;

/* Define the union U_GP0_CSC_ODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 11  ; /* [10..0]  */
        unsigned int    cscodc1               : 11  ; /* [21..11]  */
        unsigned int    csc_sign_mode         : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_ODC;

/* Define the union U_GP0_CSC_IODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc2               : 11  ; /* [10..0]  */
        unsigned int    cscodc2               : 11  ; /* [21..11]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_IODC;

/* Define the union U_GP0_CSC_P0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_P0;

/* Define the union U_GP0_CSC_P1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_P1;

/* Define the union U_GP0_CSC_P2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_P2;

/* Define the union U_GP0_CSC_P3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_P3;

/* Define the union U_GP0_CSC_P4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_CSC_P4;

/* Define the union U_GP0_ZME_HSP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hratio                : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 3   ; /* [18..16]  */
        unsigned int    hfir_order            : 1   ; /* [19]  */
        unsigned int    reserved_1            : 3   ; /* [22..20]  */
        unsigned int    hafir_en              : 1   ; /* [23]  */
        unsigned int    hfir_en               : 1   ; /* [24]  */
        unsigned int    reserved_2            : 3   ; /* [27..25]  */
        unsigned int    hchmid_en             : 1   ; /* [28]  */
        unsigned int    hlmid_en              : 1   ; /* [29]  */
        unsigned int    hamid_en              : 1   ; /* [30]  */
        unsigned int    hsc_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_HSP;

/* Define the union U_GP0_ZME_HOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hor_coffset           : 16  ; /* [15..0]  */
        unsigned int    hor_loffset           : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_HOFFSET;

/* Define the union U_GP0_ZME_VSP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 23  ; /* [22..0]  */
        unsigned int    vafir_en              : 1   ; /* [23]  */
        unsigned int    vfir_en               : 1   ; /* [24]  */
        unsigned int    reserved_1            : 2   ; /* [26..25]  */
        unsigned int    vsc_luma_tap          : 1   ; /* [27]  */
        unsigned int    vchmid_en             : 1   ; /* [28]  */
        unsigned int    vlmid_en              : 1   ; /* [29]  */
        unsigned int    vamid_en              : 1   ; /* [30]  */
        unsigned int    vsc_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_VSP;

/* Define the union U_GP0_ZME_VSR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vratio                : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_VSR;

/* Define the union U_GP0_ZME_VOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbtm_offset           : 16  ; /* [15..0]  */
        unsigned int    vtp_offset            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_VOFFSET;

/* Define the union U_GP0_ZME_LTICTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    lhpass_coef4          : 8   ; /* [7..0]  */
        unsigned int    lmixing_ratio         : 8   ; /* [15..8]  */
        unsigned int    lgain_ratio           : 12  ; /* [27..16]  */
        unsigned int    reserved_0            : 3   ; /* [30..28]  */
        unsigned int    lti_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_LTICTRL;

/* Define the union U_GP0_ZME_LHPASSCOEF */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    lhpass_coef0          : 8   ; /* [7..0]  */
        unsigned int    lhpass_coef1          : 8   ; /* [15..8]  */
        unsigned int    lhpass_coef2          : 8   ; /* [23..16]  */
        unsigned int    lhpass_coef3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_LHPASSCOEF;

/* Define the union U_GP0_ZME_LTITHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    lover_swing           : 10  ; /* [9..0]  */
        unsigned int    lunder_swing          : 10  ; /* [19..10]  */
        unsigned int    lcoring_thd           : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_LTITHD;

/* Define the union U_GP0_ZME_LHFREQTHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    lhfreq_thd0           : 16  ; /* [15..0]  */
        unsigned int    lhfreq_thd1           : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_LHFREQTHD;

/* Define the union U_GP0_ZME_LGAINCOEF */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    lgain_coef0           : 8   ; /* [7..0]  */
        unsigned int    lgain_coef1           : 8   ; /* [15..8]  */
        unsigned int    lgain_coef2           : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_LGAINCOEF;

/* Define the union U_GP0_ZME_CTICTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [7..0]  */
        unsigned int    cmixing_ratio         : 8   ; /* [15..8]  */
        unsigned int    cgain_ratio           : 12  ; /* [27..16]  */
        unsigned int    reserved_1            : 3   ; /* [30..28]  */
        unsigned int    cti_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_CTICTRL;

/* Define the union U_GP0_ZME_CHPASSCOEF */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    chpass_coef0          : 8   ; /* [7..0]  */
        unsigned int    chpass_coef1          : 8   ; /* [15..8]  */
        unsigned int    chpass_coef2          : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_CHPASSCOEF;

/* Define the union U_GP0_ZME_CTITHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cover_swing           : 10  ; /* [9..0]  */
        unsigned int    cunder_swing          : 10  ; /* [19..10]  */
        unsigned int    ccoring_thd           : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GP0_ZME_CTITHD;

/* Define the union U_WBC_G0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    req_interval          : 10  ; /* [9..0]  */
        unsigned int    auto_stop_en          : 1   ; /* [10]  */
        unsigned int    reserved_0            : 15  ; /* [25..11]  */
        unsigned int    format_out            : 2   ; /* [27..26]  */
        unsigned int    reserved_1            : 3   ; /* [30..28]  */
        unsigned int    wbc_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_CTRL;

/* Define the union U_WBC_G0_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_UPD;

/* Define the union U_WBC_G0_CMP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmp_lossy_en          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    cmp_drr               : 4   ; /* [7..4]  */
        unsigned int    reserved_1            : 23  ; /* [30..8]  */
        unsigned int    cmp_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_CMP;

/* Define the union U_WBC_G0_ADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int wbcaddr                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_G0_ADDR;
/* Define the union U_WBC_G0_GB_ADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int wbccaddr               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_G0_GB_ADDR;
/* Define the union U_WBC_G0_STRIDE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbcstride             : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_STRIDE;

/* Define the union U_WBC_G0_OFFSET */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int wbcoffset              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_G0_OFFSET;
/* Define the union U_WBC_G0_ORESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ow                    : 12  ; /* [11..0]  */
        unsigned int    oh                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_ORESO;

/* Define the union U_WBC_G0_YCROP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    yfcrop                : 12  ; /* [11..0]  */
        unsigned int    ylcrop                : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_YCROP;

/* Define the union U_WBC_G0_FCROP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wfcrop                : 12  ; /* [11..0]  */
        unsigned int    hfcrop                : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_FCROP;

/* Define the union U_WBC_G0_LCROP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wlcrop                : 12  ; /* [11..0]  */
        unsigned int    hlcrop                : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_LCROP;

/* Define the union U_WBC_G0_CSTR_ERR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    str_err_flag          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_CSTR_ERR;

/* Define the union U_WBC_G0_CLR_CSTR_ERR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clr_err_flag          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_CLR_CSTR_ERR;

/* Define the union U_WBC_G0_GLB_INFO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    is_lossless           : 1   ; /* [0]  */
        unsigned int    is_lossless_alpha     : 1   ; /* [1]  */
        unsigned int    cmp_mode              : 1   ; /* [2]  */
        unsigned int    osd_mode              : 2   ; /* [4..3]  */
        unsigned int    partition_en          : 1   ; /* [5]  */
        unsigned int    part_num              : 3   ; /* [8..6]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_GLB_INFO;

/* Define the union U_WBC_G0_FRAME_SIZE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    frame_width           : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    frame_height          : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_FRAME_SIZE;

/* Define the union U_WBC_G0_RC_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    budget_bits_mb        : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    min_mb_bits           : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG0;

/* Define the union U_WBC_G0_RC_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    budget_bits_mb_cap    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG1;

/* Define the union U_WBC_G0_RC_CFG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    max_qp                : 4   ; /* [3..0]  */
        unsigned int    smth_qp               : 4   ; /* [7..4]  */
        unsigned int    sad_bits_ngain        : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    rc_smth_ngain         : 3   ; /* [18..16]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    special_bit_gain      : 4   ; /* [27..24]  */
        unsigned int    reserved_2            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG2;

/* Define the union U_WBC_G0_RC_CFG3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    max_sad_thr           : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 9   ; /* [15..7]  */
        unsigned int    min_sad_thr           : 7   ; /* [22..16]  */
        unsigned int    reserved_1            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG3;

/* Define the union U_WBC_G0_RC_CFG4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    smth_thr              : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 1   ; /* [7]  */
        unsigned int    still_thr             : 7   ; /* [14..8]  */
        unsigned int    reserved_1            : 1   ; /* [15]  */
        unsigned int    big_grad_thr          : 10  ; /* [25..16]  */
        unsigned int    reserved_2            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG4;

/* Define the union U_WBC_G0_RC_CFG5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    smth_pix_num_thr      : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    still_pix_num_thr     : 6   ; /* [13..8]  */
        unsigned int    reserved_1            : 2   ; /* [15..14]  */
        unsigned int    noise_pix_num_thr     : 6   ; /* [21..16]  */
        unsigned int    reserved_2            : 2   ; /* [23..22]  */
        unsigned int    large_smth_pix_num_thr : 6   ; /* [29..24]  */
        unsigned int    reserved_3            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG5;

/* Define the union U_WBC_G0_RC_CFG6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    noise_sad             : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 9   ; /* [15..7]  */
        unsigned int    pix_diff_thr          : 9   ; /* [24..16]  */
        unsigned int    reserved_1            : 7   ; /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG6;

/* Define the union U_WBC_G0_RC_CFG7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    adj_sad_bits_thr      : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 1   ; /* [7]  */
        unsigned int    max_trow_bits         : 8   ; /* [15..8]  */
        unsigned int    reserved_1            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG7;

/* Define the union U_WBC_G0_RC_CFG8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    qp_inc1_bits_thr      : 8   ; /* [7..0]  */
        unsigned int    qp_dec1_bits_thr      : 8   ; /* [15..8]  */
        unsigned int    qp_dec2_bits_thr      : 8   ; /* [23..16]  */
        unsigned int    qp_dec3_bits_thr      : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG8;

/* Define the union U_WBC_G0_RC_CFG9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    force_qp_thr          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    force_qp_thr_cap      : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_RC_CFG9;

/* Define the union U_WBC_G0_MAX_ROW_LEN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    max_row_len           : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_G0_MAX_ROW_LEN;

/* Define the union U_WBC_GP0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    req_interval          : 10  ; /* [9..0]  */
        unsigned int    auto_stop_en          : 1   ; /* [10]  */
        unsigned int    reserved_0            : 1   ; /* [11]  */
        unsigned int    wbc_vtthd_mode        : 1   ; /* [12]  */
        unsigned int    reserved_1            : 5   ; /* [17..13]  */
        unsigned int    three_d_mode          : 2   ; /* [19..18]  */
        unsigned int    reserved_2            : 4   ; /* [23..20]  */
        unsigned int    format_out            : 4   ; /* [27..24]  */
        unsigned int    mode_out              : 2   ; /* [29..28]  */
        unsigned int    uv_order              : 1   ; /* [30]  */
        unsigned int    wbc_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_CTRL;

/* Define the union U_WBC_GP0_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_UPD;

/* Define the union U_WBC_GP0_YADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int wbcaddr                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_GP0_YADDR;
/* Define the union U_WBC_GP0_CADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int wbccaddr               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_GP0_CADDR;
/* Define the union U_WBC_GP0_STRIDE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbclstride            : 16  ; /* [15..0]  */
        unsigned int    wbccstride            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_STRIDE;

/* Define the union U_WBC_GP0_ORESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ow                    : 12  ; /* [11..0]  */
        unsigned int    oh                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_ORESO;

/* Define the union U_WBC_GP0_FCROP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wfcrop                : 12  ; /* [11..0]  */
        unsigned int    hfcrop                : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_FCROP;

/* Define the union U_WBC_GP0_LCROP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wlcrop                : 12  ; /* [11..0]  */
        unsigned int    hlcrop                : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_LCROP;

/* Define the union U_WBC_GP0_DITHER_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [28..0]  */
        unsigned int    dither_md             : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_DITHER_CTRL;

/* Define the union U_WBC_GP0_DITHER_COEF0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef0          : 8   ; /* [7..0]  */
        unsigned int    dither_coef1          : 8   ; /* [15..8]  */
        unsigned int    dither_coef2          : 8   ; /* [23..16]  */
        unsigned int    dither_coef3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_DITHER_COEF0;

/* Define the union U_WBC_GP0_DITHER_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef4          : 8   ; /* [7..0]  */
        unsigned int    dither_coef5          : 8   ; /* [15..8]  */
        unsigned int    dither_coef6          : 8   ; /* [23..16]  */
        unsigned int    dither_coef7          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_GP0_DITHER_COEF1;

/* Define the union U_WBC_DHD0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    req_interval          : 10  ; /* [9..0]  */
        unsigned int    auto_stop_en          : 1   ; /* [10]  */
        unsigned int    histogam_en           : 1   ; /* [11]  */
        unsigned int    wbc_vtthd_mode        : 1   ; /* [12]  */
        unsigned int    reserved_0            : 5   ; /* [17..13]  */
        unsigned int    three_d_mode          : 2   ; /* [19..18]  */
        unsigned int    reserved_1            : 4   ; /* [23..20]  */
        unsigned int    format_out            : 4   ; /* [27..24]  */
        unsigned int    mode_out              : 2   ; /* [29..28]  */
        unsigned int    uv_order              : 1   ; /* [30]  */
        unsigned int    wbc_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CTRL;

/* Define the union U_WBC_DHD0_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_UPD;

/* Define the union U_WBC_DHD0_YADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int wbcaddr                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_YADDR;
/* Define the union U_WBC_DHD0_CADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int wbccaddr               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_CADDR;
/* Define the union U_WBC_DHD0_STRIDE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbclstride            : 16  ; /* [15..0]  */
        unsigned int    wbccstride            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_STRIDE;

/* Define the union U_WBC_DHD0_ORESO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ow                    : 12  ; /* [11..0]  */
        unsigned int    oh                    : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ORESO;

/* Define the union U_WBC_DHD0_FCROP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wfcrop                : 12  ; /* [11..0]  */
        unsigned int    hfcrop                : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_FCROP;

/* Define the union U_WBC_DHD0_LCROP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wlcrop                : 12  ; /* [11..0]  */
        unsigned int    hlcrop                : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_LCROP;

/* Define the union U_WBC_DHD0_LOWDLYCTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wb_per_line_num       : 12  ; /* [11..0]  */
        unsigned int    partfns_line_num      : 12  ; /* [23..12]  */
        unsigned int    reserved_0            : 6   ; /* [29..24]  */
        unsigned int    lowdly_test           : 1   ; /* [30]  */
        unsigned int    lowdly_en             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_LOWDLYCTRL;

/* Define the union U_WBC_DHD0_TUNLADDR */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int tunl_cell_addr         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_TUNLADDR;
/* Define the union U_WBC_DHD0_LOWDLYSTA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [30..0]  */
        unsigned int    part_finish           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_LOWDLYSTA;

/* Define the union U_WBC_DHD0_PARAUP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wbc_hlcoef_upd        : 1   ; /* [0]  */
        unsigned int    wbc_hccoef_upd        : 1   ; /* [1]  */
        unsigned int    wbc_vlcoef_upd        : 1   ; /* [2]  */
        unsigned int    wbc_vccoef_upd        : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_PARAUP;

/* Define the union U_WBC_DHD0_HLCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_HLCOEFAD;
/* Define the union U_WBC_DHD0_HCCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_HCCOEFAD;
/* Define the union U_WBC_DHD0_VLCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_VLCOEFAD;
/* Define the union U_WBC_DHD0_VCCOEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_VCCOEFAD;
/* Define the union U_WBC_DHD0_HISTOGRAM0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM0;

/* Define the union U_WBC_DHD0_HISTOGRAM1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM1;

/* Define the union U_WBC_DHD0_HISTOGRAM2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM2;

/* Define the union U_WBC_DHD0_HISTOGRAM3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM3;

/* Define the union U_WBC_DHD0_HISTOGRAM4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM4;

/* Define the union U_WBC_DHD0_HISTOGRAM5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM5;

/* Define the union U_WBC_DHD0_HISTOGRAM6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM6;

/* Define the union U_WBC_DHD0_HISTOGRAM7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM7;

/* Define the union U_WBC_DHD0_HISTOGRAM8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM8;

/* Define the union U_WBC_DHD0_HISTOGRAM9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM9;

/* Define the union U_WBC_DHD0_HISTOGRAM10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM10;

/* Define the union U_WBC_DHD0_HISTOGRAM11 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM11;

/* Define the union U_WBC_DHD0_HISTOGRAM12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM12;

/* Define the union U_WBC_DHD0_HISTOGRAM13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM13;

/* Define the union U_WBC_DHD0_HISTOGRAM14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM14;

/* Define the union U_WBC_DHD0_HISTOGRAM15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hgram_cnt0            : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HISTOGRAM15;

/* Define the union U_WBC_DHD0_CHECKSUM_Y */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int reserved_0             : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_CHECKSUM_Y;
/* Define the union U_WBC_DHD0_CHECKSUM_C */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int reserved_0             : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_WBC_DHD0_CHECKSUM_C;
/* Define the union U_WBC_DHD0_DITHER_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [28..0]  */
        unsigned int    dither_md             : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_DITHER_CTRL;

/* Define the union U_WBC_DHD0_DITHER_COEF0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef0          : 8   ; /* [7..0]  */
        unsigned int    dither_coef1          : 8   ; /* [15..8]  */
        unsigned int    dither_coef2          : 8   ; /* [23..16]  */
        unsigned int    dither_coef3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_DITHER_COEF0;

/* Define the union U_WBC_DHD0_DITHER_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef4          : 8   ; /* [7..0]  */
        unsigned int    dither_coef5          : 8   ; /* [15..8]  */
        unsigned int    dither_coef6          : 8   ; /* [23..16]  */
        unsigned int    dither_coef7          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_DITHER_COEF1;

/* Define the union U_WBC_DHD0_HCDS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [28..0]  */
        unsigned int    hchfir_en             : 1   ; /* [29]  */
        unsigned int    hchmid_en             : 1   ; /* [30]  */
        unsigned int    hcds_en               : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HCDS;

/* Define the union U_WBC_DHD0_HCDS_COEF0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef0                 : 10  ; /* [9..0]  */
        unsigned int    coef1                 : 10  ; /* [19..10]  */
        unsigned int    coef2                 : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HCDS_COEF0;

/* Define the union U_WBC_DHD0_HCDS_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef3                 : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_HCDS_COEF1;

/* Define the union U_WBC_DHD0_ZME_HSP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hratio                : 24  ; /* [23..0]  */
        unsigned int    hfir_order            : 1   ; /* [24]  */
        unsigned int    hchfir_en             : 1   ; /* [25]  */
        unsigned int    hlfir_en              : 1   ; /* [26]  */
        unsigned int    non_lnr_en            : 1   ; /* [27]  */
        unsigned int    hchmid_en             : 1   ; /* [28]  */
        unsigned int    hlmid_en              : 1   ; /* [29]  */
        unsigned int    hchmsc_en             : 1   ; /* [30]  */
        unsigned int    hlmsc_en              : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ZME_HSP;

/* Define the union U_WBC_DHD0_ZME_HLOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hor_loffset           : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ZME_HLOFFSET;

/* Define the union U_WBC_DHD0_ZME_HCOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hor_coffset           : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ZME_HCOFFSET;

/* Define the union U_WBC_DHD0_ZME_VSP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 19  ; /* [18..0]  */
        unsigned int    zme_in_fmt            : 2   ; /* [20..19]  */
        unsigned int    zme_out_fmt           : 2   ; /* [22..21]  */
        unsigned int    vchfir_en             : 1   ; /* [23]  */
        unsigned int    vlfir_en              : 1   ; /* [24]  */
        unsigned int    reserved_1            : 1   ; /* [25]  */
        unsigned int    vsc_chroma_tap        : 1   ; /* [26]  */
        unsigned int    reserved_2            : 1   ; /* [27]  */
        unsigned int    vchmid_en             : 1   ; /* [28]  */
        unsigned int    vlmid_en              : 1   ; /* [29]  */
        unsigned int    vchmsc_en             : 1   ; /* [30]  */
        unsigned int    vlmsc_en              : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ZME_VSP;

/* Define the union U_WBC_DHD0_ZME_VSR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vratio                : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ZME_VSR;

/* Define the union U_WBC_DHD0_ZME_VOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vchroma_offset        : 16  ; /* [15..0]  */
        unsigned int    vluma_offset          : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ZME_VOFFSET;

/* Define the union U_WBC_DHD0_ZME_VBOFFSET */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vbchroma_offset       : 16  ; /* [15..0]  */
        unsigned int    vbluma_offset         : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_ZME_VBOFFSET;

/* Define the union U_WBC_DHD0_CSCIDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 9   ; /* [8..0]  */
        unsigned int    cscidc1               : 9   ; /* [17..9]  */
        unsigned int    cscidc2               : 9   ; /* [26..18]  */
        unsigned int    csc_en                : 1   ; /* [27]  */
        unsigned int    csc_mode              : 3   ; /* [30..28]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CSCIDC;

/* Define the union U_WBC_DHD0_CSCODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 9   ; /* [8..0]  */
        unsigned int    cscodc1               : 9   ; /* [17..9]  */
        unsigned int    cscodc2               : 9   ; /* [26..18]  */
        unsigned int    reserved_0            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CSCODC;

/* Define the union U_WBC_DHD0_CSCP0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CSCP0;

/* Define the union U_WBC_DHD0_CSCP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CSCP1;

/* Define the union U_WBC_DHD0_CSCP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CSCP2;

/* Define the union U_WBC_DHD0_CSCP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CSCP3;

/* Define the union U_WBC_DHD0_CSCP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_WBC_DHD0_CSCP4;

/* Define the union U_MIXV0_BKG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_bkgcr           : 10  ; /* [9..0]  */
        unsigned int    mixer_bkgcb           : 10  ; /* [19..10]  */
        unsigned int    mixer_bkgy            : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MIXV0_BKG;

/* Define the union U_MIXV0_MIX */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_prio0           : 4   ; /* [3..0]  */
        unsigned int    mixer_prio1           : 4   ; /* [7..4]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MIXV0_MIX;

/* Define the union U_MIXG0_BKG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_bkgcr           : 10  ; /* [9..0]  */
        unsigned int    mixer_bkgcb           : 10  ; /* [19..10]  */
        unsigned int    mixer_bkgy            : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MIXG0_BKG;

/* Define the union U_MIXG0_BKALPHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_alpha           : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MIXG0_BKALPHA;

/* Define the union U_MIXG0_MIX */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_prio0           : 4   ; /* [3..0]  */
        unsigned int    mixer_prio1           : 4   ; /* [7..4]  */
        unsigned int    mixer_prio2           : 4   ; /* [11..8]  */
        unsigned int    mixer_prio3           : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MIXG0_MIX;

/* Define the union U_CBM_BKG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cbm_bkgcr1            : 10  ; /* [9..0]  */
        unsigned int    cbm_bkgcb1            : 10  ; /* [19..10]  */
        unsigned int    cbm_bkgy1             : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CBM_BKG1;

/* Define the union U_CBM_MIX1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_prio0           : 4   ; /* [3..0]  */
        unsigned int    mixer_prio1           : 4   ; /* [7..4]  */
        unsigned int    mixer_prio2           : 4   ; /* [11..8]  */
        unsigned int    mixer_prio3           : 4   ; /* [15..12]  */
        unsigned int    mixer_prio4           : 4   ; /* [19..16]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CBM_MIX1;

/* Define the union U_CBM_BKG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cbm_bkgcr2            : 10  ; /* [9..0]  */
        unsigned int    cbm_bkgcb2            : 10  ; /* [19..10]  */
        unsigned int    cbm_bkgy2             : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CBM_BKG2;

/* Define the union U_CBM_MIX2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_prio0           : 4   ; /* [3..0]  */
        unsigned int    mixer_prio1           : 4   ; /* [7..4]  */
        unsigned int    mixer_prio2           : 4   ; /* [11..8]  */
        unsigned int    mixer_prio3           : 4   ; /* [15..12]  */
        unsigned int    mixer_prio4           : 4   ; /* [19..16]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CBM_MIX2;

/* Define the union U_CBM_ATTR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sur_attr0             : 1   ; /* [0]  */
        unsigned int    sur_attr1             : 1   ; /* [1]  */
        unsigned int    sur_attr2             : 1   ; /* [2]  */
        unsigned int    sur_attr3             : 1   ; /* [3]  */
        unsigned int    sur_attr4             : 1   ; /* [4]  */
        unsigned int    sur_attr5             : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CBM_ATTR;

/* Define the union U_MIXDSD_BKG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_bkgcr           : 10  ; /* [9..0]  */
        unsigned int    mixer_bkgcb           : 10  ; /* [19..10]  */
        unsigned int    mixer_bkgy            : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MIXDSD_BKG;

/* Define the union U_MIXDSD_MIX */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mixer_prio0           : 4   ; /* [3..0]  */
        unsigned int    mixer_prio1           : 4   ; /* [7..4]  */
        unsigned int    mixer_prio2           : 4   ; /* [11..8]  */
        unsigned int    mixer_prio3           : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MIXDSD_MIX;

/* Define the union U_DHD0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    disp_mode             : 3   ; /* [3..1]  */
        unsigned int    iop                   : 1   ; /* [4]  */
        unsigned int    reserved_0            : 7   ; /* [11..5]  */
        unsigned int    gmm_mode              : 1   ; /* [12]  */
        unsigned int    gmm_en                : 1   ; /* [13]  */
        unsigned int    hdmi_mode             : 1   ; /* [14]  */
        unsigned int    reserved_1            : 3   ; /* [17..15]  */
        unsigned int    sin_en                : 1   ; /* [18]  */
        unsigned int    trigger_en            : 1   ; /* [19]  */
        unsigned int    fpga_lmt_width        : 7   ; /* [26..20]  */
        unsigned int    fpga_lmt_en           : 1   ; /* [27]  */
        unsigned int    p2i_en                : 1   ; /* [28]  */
        unsigned int    cbar_sel              : 1   ; /* [29]  */
        unsigned int    cbar_en               : 1   ; /* [30]  */
        unsigned int    intf_en               : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CTRL;

/* Define the union U_DHD0_VSYNC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vact                  : 12  ; /* [11..0]  */
        unsigned int    vbb                   : 8   ; /* [19..12]  */
        unsigned int    vfb                   : 8   ; /* [27..20]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_VSYNC;

/* Define the union U_DHD0_HSYNC1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hact                  : 16  ; /* [15..0]  */
        unsigned int    hbb                   : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSYNC1;

/* Define the union U_DHD0_HSYNC2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hfb                   : 16  ; /* [15..0]  */
        unsigned int    hmid                  : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSYNC2;

/* Define the union U_DHD0_VPLUS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    bvact                 : 12  ; /* [11..0]  */
        unsigned int    bvbb                  : 8   ; /* [19..12]  */
        unsigned int    bvfb                  : 8   ; /* [27..20]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_VPLUS;

/* Define the union U_DHD0_PWR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hpw                   : 16  ; /* [15..0]  */
        unsigned int    vpw                   : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_PWR;

/* Define the union U_DHD0_VTTHD3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vtmgthd3              : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 2   ; /* [14..13]  */
        unsigned int    thd3_mode             : 1   ; /* [15]  */
        unsigned int    vtmgthd4              : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 2   ; /* [30..29]  */
        unsigned int    thd4_mode             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_VTTHD3;

/* Define the union U_DHD0_VTTHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vtmgthd1              : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 2   ; /* [14..13]  */
        unsigned int    thd1_mode             : 1   ; /* [15]  */
        unsigned int    vtmgthd2              : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 2   ; /* [30..29]  */
        unsigned int    thd2_mode             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_VTTHD;

/* Define the union U_DHD0_SYNC_INV */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    lcd_dv_inv            : 1   ; /* [0]  */
        unsigned int    lcd_hs_inv            : 1   ; /* [1]  */
        unsigned int    lcd_vs_inv            : 1   ; /* [2]  */
        unsigned int    reserved_0            : 1   ; /* [3]  */
        unsigned int    vga_dv_inv            : 1   ; /* [4]  */
        unsigned int    vga_hs_inv            : 1   ; /* [5]  */
        unsigned int    vga_vs_inv            : 1   ; /* [6]  */
        unsigned int    reserved_1            : 1   ; /* [7]  */
        unsigned int    hdmi_dv_inv           : 1   ; /* [8]  */
        unsigned int    hdmi_hs_inv           : 1   ; /* [9]  */
        unsigned int    hdmi_vs_inv           : 1   ; /* [10]  */
        unsigned int    hdmi_f_inv            : 1   ; /* [11]  */
        unsigned int    date_dv_inv           : 1   ; /* [12]  */
        unsigned int    date_hs_inv           : 1   ; /* [13]  */
        unsigned int    date_vs_inv           : 1   ; /* [14]  */
        unsigned int    date_f_inv            : 1   ; /* [15]  */
        unsigned int    reserved_2            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_SYNC_INV;

/* Define the union U_DHD0_DATA_SEL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    date_data_sel         : 1   ; /* [0]  */
        unsigned int    vga_data_sel          : 1   ; /* [1]  */
        unsigned int    lcd_data_sel          : 1   ; /* [2]  */
        unsigned int    hdmi_data_sel         : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DATA_SEL;

/* Define the union U_DHD0_AFFTHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd_aff_thd           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_AFFTHD;

/* Define the union U_DHD0_ABUFTHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd_buf_thd           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_ABUFTHD;

/* Define the union U_DHD0_DACDET1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vdac_det_high         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    det_line              : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DACDET1;

/* Define the union U_DHD0_DACDET2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    det_pixel_sta         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    det_pixel_wid         : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 4   ; /* [30..27]  */
        unsigned int    vdac_det_en           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DACDET2;

/* Define the union U_DHD0_CSC_IDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 11  ; /* [10..0]  */
        unsigned int    cscidc1               : 11  ; /* [21..11]  */
        unsigned int    csc_en                : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_IDC;

/* Define the union U_DHD0_CSC_ODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 11  ; /* [10..0]  */
        unsigned int    cscodc1               : 11  ; /* [21..11]  */
        unsigned int    csc_sign_mode         : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_ODC;

/* Define the union U_DHD0_CSC_IODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc2               : 11  ; /* [10..0]  */
        unsigned int    cscodc2               : 11  ; /* [21..11]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_IODC;

/* Define the union U_DHD0_CSC_P0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_P0;

/* Define the union U_DHD0_CSC_P1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_P1;

/* Define the union U_DHD0_CSC_P2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_P2;

/* Define the union U_DHD0_CSC_P3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_P3;

/* Define the union U_DHD0_CSC_P4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CSC_P4;

/* Define the union U_DHD0_DITHER0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [28..0]  */
        unsigned int    dither_md             : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DITHER0_CTRL;

/* Define the union U_DHD0_DITHER0_COEF0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef0          : 8   ; /* [7..0]  */
        unsigned int    dither_coef1          : 8   ; /* [15..8]  */
        unsigned int    dither_coef2          : 8   ; /* [23..16]  */
        unsigned int    dither_coef3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DITHER0_COEF0;

/* Define the union U_DHD0_DITHER0_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef4          : 8   ; /* [7..0]  */
        unsigned int    dither_coef5          : 8   ; /* [15..8]  */
        unsigned int    dither_coef6          : 8   ; /* [23..16]  */
        unsigned int    dither_coef7          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DITHER0_COEF1;

/* Define the union U_DHD0_DITHER1_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [28..0]  */
        unsigned int    dither_md             : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DITHER1_CTRL;

/* Define the union U_DHD0_DITHER1_COEF0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef0          : 8   ; /* [7..0]  */
        unsigned int    dither_coef1          : 8   ; /* [15..8]  */
        unsigned int    dither_coef2          : 8   ; /* [23..16]  */
        unsigned int    dither_coef3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DITHER1_COEF0;

/* Define the union U_DHD0_DITHER1_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef4          : 8   ; /* [7..0]  */
        unsigned int    dither_coef5          : 8   ; /* [15..8]  */
        unsigned int    dither_coef6          : 8   ; /* [23..16]  */
        unsigned int    dither_coef7          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DITHER1_COEF1;

/* Define the union U_DHD0_CLIP0_L */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_cl0              : 10  ; /* [9..0]  */
        unsigned int    clip_cl1              : 10  ; /* [19..10]  */
        unsigned int    clip_cl2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP0_L;

/* Define the union U_DHD0_CLIP0_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_ch0              : 10  ; /* [9..0]  */
        unsigned int    clip_ch1              : 10  ; /* [19..10]  */
        unsigned int    clip_ch2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP0_H;

/* Define the union U_DHD0_CLIP1_L */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_cl0              : 10  ; /* [9..0]  */
        unsigned int    clip_cl1              : 10  ; /* [19..10]  */
        unsigned int    clip_cl2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP1_L;

/* Define the union U_DHD0_CLIP1_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_ch0              : 10  ; /* [9..0]  */
        unsigned int    clip_ch1              : 10  ; /* [19..10]  */
        unsigned int    clip_ch2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP1_H;

/* Define the union U_DHD0_CLIP2_L */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_cl0              : 10  ; /* [9..0]  */
        unsigned int    clip_cl1              : 10  ; /* [19..10]  */
        unsigned int    clip_cl2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP2_L;

/* Define the union U_DHD0_CLIP2_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_ch0              : 10  ; /* [9..0]  */
        unsigned int    clip_ch1              : 10  ; /* [19..10]  */
        unsigned int    clip_ch2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP2_H;

/* Define the union U_DHD0_CLIP3_L */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_cl0              : 10  ; /* [9..0]  */
        unsigned int    clip_cl1              : 10  ; /* [19..10]  */
        unsigned int    clip_cl2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP3_L;

/* Define the union U_DHD0_CLIP3_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_ch0              : 10  ; /* [9..0]  */
        unsigned int    clip_ch1              : 10  ; /* [19..10]  */
        unsigned int    clip_ch2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP3_H;

/* Define the union U_DHD0_CLIP4_L */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_cl0              : 10  ; /* [9..0]  */
        unsigned int    clip_cl1              : 10  ; /* [19..10]  */
        unsigned int    clip_cl2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP4_L;

/* Define the union U_DHD0_CLIP4_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_ch0              : 10  ; /* [9..0]  */
        unsigned int    clip_ch1              : 10  ; /* [19..10]  */
        unsigned int    clip_ch2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CLIP4_H;

/* Define the union U_DHD0_PARATHD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    para_thd              : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 25  ; /* [30..6]  */
        unsigned int    dfs_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_PARATHD;

/* Define the union U_DHD0_START_POS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    start_pos             : 8   ; /* [7..0]  */
        unsigned int    timing_start_pos      : 8   ; /* [15..8]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_START_POS;

/* Define the union U_DHD0_CCDOIMGMOD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    img_mode              : 7   ; /* [6..0]  */
        unsigned int    img_right             : 1   ; /* [7]  */
        unsigned int    img_id                : 2   ; /* [9..8]  */
        unsigned int    reserved_0            : 1   ; /* [10]  */
        unsigned int    ccd_en                : 1   ; /* [11]  */
        unsigned int    reserved_1            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CCDIIMGMOD;

/* Define the union U_DHD0_CCDOIMGMOD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    img_mode              : 7   ; /* [6..0]  */
        unsigned int    img_right             : 1   ; /* [7]  */
        unsigned int    img_id                : 2   ; /* [9..8]  */
        unsigned int    slave_mode            : 1   ; /* [10]  */
        unsigned int    ccd_en                : 1   ; /* [11]  */
        unsigned int    reserved_1            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CCDOIMGMOD;

/* Define the union U_DHD0_CCDOPOSMSKH */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    p32_en                : 1   ; /* [0]  */
        unsigned int    p33_en                : 1   ; /* [1]  */
        unsigned int    p34_en                : 1   ; /* [2]  */
        unsigned int    p35_en                : 1   ; /* [3]  */
        unsigned int    p36_en                : 1   ; /* [4]  */
        unsigned int    p37_en                : 1   ; /* [5]  */
        unsigned int    p38_en                : 1   ; /* [6]  */
        unsigned int    p39_en                : 1   ; /* [7]  */
        unsigned int    p40_en                : 1   ; /* [8]  */
        unsigned int    p41_en                : 1   ; /* [9]  */
        unsigned int    p42_en                : 1   ; /* [10]  */
        unsigned int    p43_en                : 1   ; /* [11]  */
        unsigned int    p44_en                : 1   ; /* [12]  */
        unsigned int    p45_en                : 1   ; /* [13]  */
        unsigned int    p46_en                : 1   ; /* [14]  */
        unsigned int    p47_en                : 1   ; /* [15]  */
        unsigned int    p48_en                : 1   ; /* [16]  */
        unsigned int    p49_en                : 1   ; /* [17]  */
        unsigned int    p50_en                : 1   ; /* [18]  */
        unsigned int    p51_en                : 1   ; /* [19]  */
        unsigned int    p52_en                : 1   ; /* [20]  */
        unsigned int    p53_en                : 1   ; /* [21]  */
        unsigned int    p54_en                : 1   ; /* [22]  */
        unsigned int    p55_en                : 1   ; /* [23]  */
        unsigned int    p56_en                : 1   ; /* [24]  */
        unsigned int    p57_en                : 1   ; /* [25]  */
        unsigned int    p58_en                : 1   ; /* [26]  */
        unsigned int    p59_en                : 1   ; /* [27]  */
        unsigned int    p60_en                : 1   ; /* [28]  */
        unsigned int    p61_en                : 1   ; /* [29]  */
        unsigned int    p62_en                : 1   ; /* [30]  */
        unsigned int    p63_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CCDOPOSMSKH;

/* Define the union U_DHD0_CCDOPOSMSKL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    p0_en                 : 1   ; /* [0]  */
        unsigned int    p1_en                 : 1   ; /* [1]  */
        unsigned int    p2_en                 : 1   ; /* [2]  */
        unsigned int    p3_en                 : 1   ; /* [3]  */
        unsigned int    p4_en                 : 1   ; /* [4]  */
        unsigned int    p5_en                 : 1   ; /* [5]  */
        unsigned int    p6_en                 : 1   ; /* [6]  */
        unsigned int    p7_en                 : 1   ; /* [7]  */
        unsigned int    p8_en                 : 1   ; /* [8]  */
        unsigned int    p9_en                 : 1   ; /* [9]  */
        unsigned int    p10_en                : 1   ; /* [10]  */
        unsigned int    p11_en                : 1   ; /* [11]  */
        unsigned int    p12_en                : 1   ; /* [12]  */
        unsigned int    p13_en                : 1   ; /* [13]  */
        unsigned int    p14_en                : 1   ; /* [14]  */
        unsigned int    p15_en                : 1   ; /* [15]  */
        unsigned int    p16_en                : 1   ; /* [16]  */
        unsigned int    p17_en                : 1   ; /* [17]  */
        unsigned int    p18_en                : 1   ; /* [18]  */
        unsigned int    p19_en                : 1   ; /* [19]  */
        unsigned int    p20_en                : 1   ; /* [20]  */
        unsigned int    p21_en                : 1   ; /* [21]  */
        unsigned int    p22_en                : 1   ; /* [22]  */
        unsigned int    p23_en                : 1   ; /* [23]  */
        unsigned int    p24_en                : 1   ; /* [24]  */
        unsigned int    p25_en                : 1   ; /* [25]  */
        unsigned int    p26_en                : 1   ; /* [26]  */
        unsigned int    p27_en                : 1   ; /* [27]  */
        unsigned int    p28_en                : 1   ; /* [28]  */
        unsigned int    p29_en                : 1   ; /* [29]  */
        unsigned int    p30_en                : 1   ; /* [30]  */
        unsigned int    p31_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_CCDOPOSMSKL;

/* Define the union U_DHD0_LOCKCFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    measure_en            : 1   ; /* [0]  */
        unsigned int    lock_cnt_en           : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_LOCKCFG;

/* Define the union U_DHD0_LOCK_STATE1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cap_frm_cnt           : 26  ; /* [25..0]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_LOCK_STATE1;

/* Define the union U_DHD0_LOCK_STATE2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vsync_cap_vdp_init    : 26  ; /* [25..0]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_LOCK_STATE2;

/* Define the union U_DHD0_LOCK_STATE3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vsync_cap_vdp_cnt     : 26  ; /* [25..0]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_LOCK_STATE3;

/* Define the union U_DHD0_GMM_COEFAD */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef_addr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DHD0_GMM_COEFAD;
/* Define the union U_DHD0_PARAUP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dhd0_gmm_upd          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_PARAUP;

/* Define the union U_DHD0_STATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vback_blank           : 1   ; /* [0]  */
        unsigned int    vblank                : 1   ; /* [1]  */
        unsigned int    bottom_field          : 1   ; /* [2]  */
        unsigned int    vcnt                  : 13  ; /* [15..3]  */
        unsigned int    count_int             : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_STATE;

/* Define the union U_DHD0_DEBUG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pix_h                 : 16  ; /* [15..0]  */
        unsigned int    pix_v                 : 12  ; /* [27..16]  */
        unsigned int    pix_src               : 3   ; /* [30..28]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DEBUG;

/* Define the union U_DHD0_DEBUG_STATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pixel_value           : 30  ; /* [29..0]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_DEBUG_STATE;

/* Define the union U_DHD0_HSPCFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_tmp0          : 8   ; /* [7..0]  */
        unsigned int    hsp_hf0_tmp1          : 8   ; /* [15..8]  */
        unsigned int    hsp_hf0_tmp2          : 8   ; /* [23..16]  */
        unsigned int    hsp_hf0_tmp3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG0;

/* Define the union U_DHD0_HSPCFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_coring        : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 23  ; /* [30..8]  */
        unsigned int    hsp_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG1;

/* Define the union U_DHD0_HSPCFG5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_gainpos       : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    hsp_hf0_gainneg       : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG5;

/* Define the union U_DHD0_HSPCFG6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_overth        : 8   ; /* [7..0]  */
        unsigned int    hsp_hf0_underth       : 8   ; /* [15..8]  */
        unsigned int    hsp_hf0_mixratio      : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 4   ; /* [27..24]  */
        unsigned int    hsp_hf0_winsize       : 3   ; /* [30..28]  */
        unsigned int    hsp_hf0_adpshoot_en   : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG6;

/* Define the union U_DHD0_HSPCFG7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_tmp0          : 8   ; /* [7..0]  */
        unsigned int    hsp_hf1_tmp1          : 8   ; /* [15..8]  */
        unsigned int    hsp_hf1_tmp2          : 8   ; /* [23..16]  */
        unsigned int    hsp_hf1_tmp3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG7;

/* Define the union U_DHD0_HSPCFG8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_coring        : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG8;

/* Define the union U_DHD0_HSPCFG12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_gainpos       : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    hsp_hf1_gainneg       : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG12;

/* Define the union U_DHD0_HSPCFG13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_overth        : 8   ; /* [7..0]  */
        unsigned int    hsp_hf1_underth       : 8   ; /* [15..8]  */
        unsigned int    hsp_hf1_mixratio      : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 4   ; /* [27..24]  */
        unsigned int    hsp_hf1_winsize       : 3   ; /* [30..28]  */
        unsigned int    hsp_hf1_adpshoot_en   : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG13;

/* Define the union U_DHD0_HSPCFG14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_cdti_gain         : 8   ; /* [7..0]  */
        unsigned int    hsp_ldti_gain         : 8   ; /* [15..8]  */
        unsigned int    hsp_lti_ratio         : 8   ; /* [23..16]  */
        unsigned int    hsp_hf_shootdiv       : 3   ; /* [26..24]  */
        unsigned int    reserved_0            : 1   ; /* [27]  */
        unsigned int    hsp_ctih_en           : 1   ; /* [28]  */
        unsigned int    hsp_ltih_en           : 1   ; /* [29]  */
        unsigned int    hsp_h1_en             : 1   ; /* [30]  */
        unsigned int    hsp_h0_en             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG14;

/* Define the union U_DHD0_HSPCFG15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_glb_underth       : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 1   ; /* [9]  */
        unsigned int    hsp_glb_overth        : 9   ; /* [18..10]  */
        unsigned int    reserved_1            : 1   ; /* [19]  */
        unsigned int    hsp_peak_ratio        : 8   ; /* [27..20]  */
        unsigned int    reserved_2            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DHD0_HSPCFG15;

/* Define the union U_INTF_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [28..0]  */
        unsigned int    yc_mode               : 1   ; /* [29]  */
        unsigned int    hdmi_420_mode         : 1   ; /* [30]  */
        unsigned int    hdmi_mode             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CTRL;

/* Define the union U_INTF_UPD */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    regup                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_UPD;

/* Define the union U_INTF_SYNC_INV */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dv_inv                : 1   ; /* [0]  */
        unsigned int    hs_inv                : 1   ; /* [1]  */
        unsigned int    vs_inv                : 1   ; /* [2]  */
        unsigned int    f_inv                 : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_SYNC_INV;

/* Define the union U_INTF_CLIP0_L */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_cl0              : 10  ; /* [9..0]  */
        unsigned int    clip_cl1              : 10  ; /* [19..10]  */
        unsigned int    clip_cl2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CLIP0_L;

/* Define the union U_INTF_CLIP0_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_ch0              : 10  ; /* [9..0]  */
        unsigned int    clip_ch1              : 10  ; /* [19..10]  */
        unsigned int    clip_ch2              : 10  ; /* [29..20]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CLIP0_H;

/* Define the union U_INTF_CSC_IDC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc0               : 11  ; /* [10..0]  */
        unsigned int    cscidc1               : 11  ; /* [21..11]  */
        unsigned int    csc_en                : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_IDC;

/* Define the union U_INTF_CSC_ODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscodc0               : 11  ; /* [10..0]  */
        unsigned int    cscodc1               : 11  ; /* [21..11]  */
        unsigned int    csc_sign_mode         : 1   ; /* [22]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_ODC;

/* Define the union U_INTF_CSC_IODC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscidc2               : 11  ; /* [10..0]  */
        unsigned int    cscodc2               : 11  ; /* [21..11]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_IODC;

/* Define the union U_INTF_CSC_P0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp00                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp01                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_P0;

/* Define the union U_INTF_CSC_P1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp02                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp10                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_P1;

/* Define the union U_INTF_CSC_P2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp11                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp12                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_P2;

/* Define the union U_INTF_CSC_P3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp20                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 1   ; /* [15]  */
        unsigned int    cscp21                : 15  ; /* [30..16]  */
        unsigned int    reserved_1            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_P3;

/* Define the union U_INTF_CSC_P4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cscp22                : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CSC_P4;

/* Define the union U_INTF_HSPCFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_tmp0          : 8   ; /* [7..0]  */
        unsigned int    hsp_hf0_tmp1          : 8   ; /* [15..8]  */
        unsigned int    hsp_hf0_tmp2          : 8   ; /* [23..16]  */
        unsigned int    hsp_hf0_tmp3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG0;

/* Define the union U_INTF_HSPCFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_coring        : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 23  ; /* [30..8]  */
        unsigned int    hsp_en                : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG1;

/* Define the union U_INTF_HSPCFG5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_gainpos       : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    hsp_hf0_gainneg       : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG5;

/* Define the union U_INTF_HSPCFG6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf0_overth        : 8   ; /* [7..0]  */
        unsigned int    hsp_hf0_underth       : 8   ; /* [15..8]  */
        unsigned int    hsp_hf0_mixratio      : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 4   ; /* [27..24]  */
        unsigned int    hsp_hf0_winsize       : 3   ; /* [30..28]  */
        unsigned int    hsp_hf0_adpshoot_en   : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG6;

/* Define the union U_INTF_HSPCFG7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_tmp0          : 8   ; /* [7..0]  */
        unsigned int    hsp_hf1_tmp1          : 8   ; /* [15..8]  */
        unsigned int    hsp_hf1_tmp2          : 8   ; /* [23..16]  */
        unsigned int    hsp_hf1_tmp3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG7;

/* Define the union U_INTF_HSPCFG8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_coring        : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG8;

/* Define the union U_INTF_HSPCFG12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_gainpos       : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    hsp_hf1_gainneg       : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG12;

/* Define the union U_INTF_HSPCFG13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_hf1_overth        : 8   ; /* [7..0]  */
        unsigned int    hsp_hf1_underth       : 8   ; /* [15..8]  */
        unsigned int    hsp_hf1_mixratio      : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 4   ; /* [27..24]  */
        unsigned int    hsp_hf1_winsize       : 3   ; /* [30..28]  */
        unsigned int    hsp_hf1_adpshoot_en   : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG13;

/* Define the union U_INTF_HSPCFG14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_cdti_gain         : 8   ; /* [7..0]  */
        unsigned int    hsp_ldti_gain         : 8   ; /* [15..8]  */
        unsigned int    hsp_lti_ratio         : 8   ; /* [23..16]  */
        unsigned int    hsp_hf_shootdiv       : 3   ; /* [26..24]  */
        unsigned int    reserved_0            : 1   ; /* [27]  */
        unsigned int    hsp_ctih_en           : 1   ; /* [28]  */
        unsigned int    hsp_ltih_en           : 1   ; /* [29]  */
        unsigned int    hsp_h1_en             : 1   ; /* [30]  */
        unsigned int    hsp_h0_en             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG14;

/* Define the union U_INTF_HSPCFG15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsp_glb_underth       : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 1   ; /* [9]  */
        unsigned int    hsp_glb_overth        : 9   ; /* [18..10]  */
        unsigned int    reserved_1            : 1   ; /* [19]  */
        unsigned int    hsp_peak_ratio        : 8   ; /* [27..20]  */
        unsigned int    reserved_2            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_HSPCFG15;

/* Define the union U_INTF_DITHER0_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [28..0]  */
        unsigned int    dither_md             : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_DITHER0_CTRL;

/* Define the union U_INTF_DITHER0_COEF0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef0          : 8   ; /* [7..0]  */
        unsigned int    dither_coef1          : 8   ; /* [15..8]  */
        unsigned int    dither_coef2          : 8   ; /* [23..16]  */
        unsigned int    dither_coef3          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_DITHER0_COEF0;

/* Define the union U_INTF_DITHER0_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dither_coef4          : 8   ; /* [7..0]  */
        unsigned int    dither_coef5          : 8   ; /* [15..8]  */
        unsigned int    dither_coef6          : 8   ; /* [23..16]  */
        unsigned int    dither_coef7          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_DITHER0_COEF1;

/* Define the union U_INTF_CHKSUM_Y_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    check_sum             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CHKSUM_Y_H;

/* Define the union U_INTF_CHKSUM_Y_L */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int check_sum              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_INTF_CHKSUM_Y_L;
/* Define the union U_INTF_CHKSUM_U_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    check_sum             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CHKSUM_U_H;

/* Define the union U_INTF_CHKSUM_U_L */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int check_sum              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_INTF_CHKSUM_U_L;
/* Define the union U_INTF_CHKSUM_V_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    check_sum             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_INTF_CHKSUM_V_H;

/* Define the union U_INTF_CHKSUM_V_L */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int check_sum              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_INTF_CHKSUM_V_L;
/* Define the union U_HDATE_VERSION */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int hdate_version          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_HDATE_VERSION;
/* Define the union U_HDATE_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hd_en                 : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_EN;

/* Define the union U_HDATE_POLA_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hsync_in_pola         : 1   ; /* [0]  */
        unsigned int    vsync_in_pola         : 1   ; /* [1]  */
        unsigned int    fid_in_pola           : 1   ; /* [2]  */
        unsigned int    hsync_out_pola        : 1   ; /* [3]  */
        unsigned int    vsync_out_pola        : 1   ; /* [4]  */
        unsigned int    fid_out_pola          : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_POLA_CTRL;

/* Define the union U_HDATE_VIDEO_FORMAT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_ft              : 4   ; /* [3..0]  */
        unsigned int    sync_add_ctrl         : 3   ; /* [6..4]  */
        unsigned int    video_out_ctrl        : 2   ; /* [8..7]  */
        unsigned int    csc_ctrl              : 3   ; /* [11..9]  */
        unsigned int    csc_round_disable     : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_VIDEO_FORMAT;

/* Define the union U_HDATE_STATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    line_len              : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    frame_len             : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 1   ; /* [27]  */
        unsigned int    mv_en_pin             : 1   ; /* [28]  */
        unsigned int    reserved_2            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_STATE;

/* Define the union U_HDATE_OUT_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vsync_sel             : 2   ; /* [1..0]  */
        unsigned int    hsync_sel             : 2   ; /* [3..2]  */
        unsigned int    video3_sel            : 2   ; /* [5..4]  */
        unsigned int    video2_sel            : 2   ; /* [7..6]  */
        unsigned int    video1_sel            : 2   ; /* [9..8]  */
        unsigned int    src_ctrl              : 2   ; /* [11..10]  */
        unsigned int    sync_lpf_en           : 1   ; /* [12]  */
        unsigned int    sd_sel                : 1   ; /* [13]  */
        unsigned int    src_round_disable     : 1   ; /* [14]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_OUT_CTRL;

/* Define the union U_HDATE_SRC_13_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap1_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap1_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF1;

/* Define the union U_HDATE_SRC_13_COEF2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap2_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap2_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF2;

/* Define the union U_HDATE_SRC_13_COEF3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap3_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap3_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF3;

/* Define the union U_HDATE_SRC_13_COEF4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap4_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap4_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF4;

/* Define the union U_HDATE_SRC_13_COEF5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap5_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap5_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF5;

/* Define the union U_HDATE_SRC_13_COEF6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap6_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap6_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF6;

/* Define the union U_HDATE_SRC_13_COEF7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap7_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap7_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF7;

/* Define the union U_HDATE_SRC_13_COEF8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap8_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap8_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF8;

/* Define the union U_HDATE_SRC_13_COEF9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap9_1           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap9_3           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF9;

/* Define the union U_HDATE_SRC_13_COEF10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap10_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap10_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF10;

/* Define the union U_HDATE_SRC_13_COEF11 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap11_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap11_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF11;

/* Define the union U_HDATE_SRC_13_COEF12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap12_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap12_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF12;

/* Define the union U_HDATE_SRC_13_COEF13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap13_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap13_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF13;

/* Define the union U_HDATE_SRC_24_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap1_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap1_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF1;

/* Define the union U_HDATE_SRC_24_COEF2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap2_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap2_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF2;

/* Define the union U_HDATE_SRC_24_COEF3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap3_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap3_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF3;

/* Define the union U_HDATE_SRC_24_COEF4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap4_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap4_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF4;

/* Define the union U_HDATE_SRC_24_COEF5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap5_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap5_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF5;

/* Define the union U_HDATE_SRC_24_COEF6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap6_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap6_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF6;

/* Define the union U_HDATE_SRC_24_COEF7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap7_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap7_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF7;

/* Define the union U_HDATE_SRC_24_COEF8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap8_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap8_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF8;

/* Define the union U_HDATE_SRC_24_COEF9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap9_2           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap9_4           : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF9;

/* Define the union U_HDATE_SRC_24_COEF10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap10_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap10_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF10;

/* Define the union U_HDATE_SRC_24_COEF11 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap11_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap11_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF11;

/* Define the union U_HDATE_SRC_24_COEF12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap12_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap12_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF12;

/* Define the union U_HDATE_SRC_24_COEF13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap13_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap13_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF13;

/* Define the union U_HDATE_CSC_COEF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    csc_coef_r_y          : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    csc_coef_r_cb         : 12  ; /* [27..16]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CSC_COEF1;

/* Define the union U_HDATE_CSC_COEF2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    csc_coef_r_cr         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    csc_coef_g_y          : 12  ; /* [27..16]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CSC_COEF2;

/* Define the union U_HDATE_CSC_COEF3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    csc_coef_g_cb         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    csc_coef_g_cr         : 12  ; /* [27..16]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CSC_COEF3;

/* Define the union U_HDATE_CSC_COEF4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    csc_coef_b_y          : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 4   ; /* [15..12]  */
        unsigned int    csc_coef_b_cb         : 12  ; /* [27..16]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CSC_COEF4;

/* Define the union U_HDATE_CSC_COEF5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    csc_coef_b_cr         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CSC_COEF5;

/* Define the union U_HDATE_TEST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_data             : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_TEST;

/* Define the union U_HDATE_VBI_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cgmsb_add_en          : 1   ; /* [0]  */
        unsigned int    cgmsa_add_en          : 1   ; /* [1]  */
        unsigned int    mv_en                 : 1   ; /* [2]  */
        unsigned int    vbi_lpf_en            : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_VBI_CTRL;

/* Define the union U_HDATE_CGMSA_DATA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cgmsa_data            : 20  ; /* [19..0]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CGMSA_DATA;

/* Define the union U_HDATE_CGMSB_H */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hdate_cgmsb_h         : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CGMSB_H;

/* Define the union U_HDATE_CGMSB_DATA1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int cgmsb_data1            : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_HDATE_CGMSB_DATA1;
/* Define the union U_HDATE_CGMSB_DATA2 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int cgmsb_data2            : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_HDATE_CGMSB_DATA2;
/* Define the union U_HDATE_CGMSB_DATA3 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int cgmsb_data3            : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_HDATE_CGMSB_DATA3;
/* Define the union U_HDATE_CGMSB_DATA4 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int cgmsb_data4            : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_HDATE_CGMSB_DATA4;
/* Define the union U_HDATE_DACDET1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vdac_det_high         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    det_line              : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_DACDET1;

/* Define the union U_HDATE_DACDET2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    det_pixel_sta         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    det_pixel_wid         : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 4   ; /* [30..27]  */
        unsigned int    vdac_det_en           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_DACDET2;

/* Define the union U_HDATE_SRC_13_COEF14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap14_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap14_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF14;

/* Define the union U_HDATE_SRC_13_COEF15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap15_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap15_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF15;

/* Define the union U_HDATE_SRC_13_COEF16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap16_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap16_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF16;

/* Define the union U_HDATE_SRC_13_COEF17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap17_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap17_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF17;

/* Define the union U_HDATE_SRC_13_COEF18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap18_1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap18_3          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_13_COEF18;

/* Define the union U_HDATE_SRC_24_COEF14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap14_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap14_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF14;

/* Define the union U_HDATE_SRC_24_COEF15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap15_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap15_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF15;

/* Define the union U_HDATE_SRC_24_COEF16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap16_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap16_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF16;

/* Define the union U_HDATE_SRC_24_COEF17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap17_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap17_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF17;

/* Define the union U_HDATE_SRC_24_COEF18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_tap18_2          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    coef_tap18_4          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_SRC_24_COEF18;

/* Define the union U_HDATE_CLIP */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clip_thdl             : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    clip_disable          : 1   ; /* [15]  */
        unsigned int    clip_fb               : 8   ; /* [23..16]  */
        unsigned int    clip_bb               : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HDATE_CLIP;

/* Define the union U_DATE_COEFF0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tt_seq                : 1   ; /* [0]  */
        unsigned int    chgain_en             : 1   ; /* [1]  */
        unsigned int    sylp_en               : 1   ; /* [2]  */
        unsigned int    chlp_en               : 1   ; /* [3]  */
        unsigned int    oversam2_en           : 1   ; /* [4]  */
        unsigned int    lunt_en               : 1   ; /* [5]  */
        unsigned int    oversam_en            : 2   ; /* [7..6]  */
        unsigned int    reserved_0            : 1   ; /* [8]  */
        unsigned int    luma_dl               : 4   ; /* [12..9]  */
        unsigned int    agc_amp_sel           : 1   ; /* [13]  */
        unsigned int    length_sel            : 1   ; /* [14]  */
        unsigned int    sync_mode_scart       : 1   ; /* [15]  */
        unsigned int    sync_mode_sel         : 2   ; /* [17..16]  */
        unsigned int    style_sel             : 4   ; /* [21..18]  */
        unsigned int    fm_sel                : 1   ; /* [22]  */
        unsigned int    vbi_lpf_en            : 1   ; /* [23]  */
        unsigned int    rgb_en                : 1   ; /* [24]  */
        unsigned int    scanline              : 1   ; /* [25]  */
        unsigned int    pbpr_lpf_en           : 1   ; /* [26]  */
        unsigned int    pal_half_en           : 1   ; /* [27]  */
        unsigned int    reserved_1            : 1   ; /* [28]  */
        unsigned int    dis_ire               : 1   ; /* [29]  */
        unsigned int    clpf_sel              : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF0;

/* Define the union U_DATE_COEFF1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dac_test              : 10  ; /* [9..0]  */
        unsigned int    date_test_mode        : 2   ; /* [11..10]  */
        unsigned int    date_test_en          : 1   ; /* [12]  */
        unsigned int    amp_outside           : 10  ; /* [22..13]  */
        unsigned int    c_limit_en            : 1   ; /* [23]  */
        unsigned int    cc_seq                : 1   ; /* [24]  */
        unsigned int    cgms_seq              : 1   ; /* [25]  */
        unsigned int    vps_seq               : 1   ; /* [26]  */
        unsigned int    wss_seq               : 1   ; /* [27]  */
        unsigned int    cvbs_limit_en         : 1   ; /* [28]  */
        unsigned int    c_gain                : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF1;

/* Define the union U_DATE_COEFF2 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int coef02                 : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF2;
/* Define the union U_DATE_COEFF3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef03                : 26  ; /* [25..0]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF3;

/* Define the union U_DATE_COEFF4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef04                : 30  ; /* [29..0]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF4;

/* Define the union U_DATE_COEFF5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef05                : 29  ; /* [28..0]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF5;

/* Define the union U_DATE_COEFF6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef06_1              : 23  ; /* [22..0]  */
        unsigned int    reserved_0            : 8   ; /* [30..23]  */
        unsigned int    coef06_0              : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF6;

/* Define the union U_DATE_COEFF7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tt07_enf2             : 1   ; /* [0]  */
        unsigned int    tt08_enf2             : 1   ; /* [1]  */
        unsigned int    tt09_enf2             : 1   ; /* [2]  */
        unsigned int    tt10_enf2             : 1   ; /* [3]  */
        unsigned int    tt11_enf2             : 1   ; /* [4]  */
        unsigned int    tt12_enf2             : 1   ; /* [5]  */
        unsigned int    tt13_enf2             : 1   ; /* [6]  */
        unsigned int    tt14_enf2             : 1   ; /* [7]  */
        unsigned int    tt15_enf2             : 1   ; /* [8]  */
        unsigned int    tt16_enf2             : 1   ; /* [9]  */
        unsigned int    tt17_enf2             : 1   ; /* [10]  */
        unsigned int    tt18_enf2             : 1   ; /* [11]  */
        unsigned int    tt19_enf2             : 1   ; /* [12]  */
        unsigned int    tt20_enf2             : 1   ; /* [13]  */
        unsigned int    tt21_enf2             : 1   ; /* [14]  */
        unsigned int    tt22_enf2             : 1   ; /* [15]  */
        unsigned int    tt07_enf1             : 1   ; /* [16]  */
        unsigned int    tt08_enf1             : 1   ; /* [17]  */
        unsigned int    tt09_enf1             : 1   ; /* [18]  */
        unsigned int    tt10_enf1             : 1   ; /* [19]  */
        unsigned int    tt11_enf1             : 1   ; /* [20]  */
        unsigned int    tt12_enf1             : 1   ; /* [21]  */
        unsigned int    tt13_enf1             : 1   ; /* [22]  */
        unsigned int    tt14_enf1             : 1   ; /* [23]  */
        unsigned int    tt15_enf1             : 1   ; /* [24]  */
        unsigned int    tt16_enf1             : 1   ; /* [25]  */
        unsigned int    tt17_enf1             : 1   ; /* [26]  */
        unsigned int    tt18_enf1             : 1   ; /* [27]  */
        unsigned int    tt19_enf1             : 1   ; /* [28]  */
        unsigned int    tt20_enf1             : 1   ; /* [29]  */
        unsigned int    tt21_enf1             : 1   ; /* [30]  */
        unsigned int    tt22_enf1             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF7;

/* Define the union U_DATE_COEFF8 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int tt_staddr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF8;
/* Define the union U_DATE_COEFF9 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int tt_edaddr              : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF9;
/* Define the union U_DATE_COEFF10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tt_pktoff             : 8   ; /* [7..0]  */
        unsigned int    tt_mode               : 2   ; /* [9..8]  */
        unsigned int    tt_highest            : 1   ; /* [10]  */
        unsigned int    full_page             : 1   ; /* [11]  */
        unsigned int    nabts_100ire          : 1   ; /* [12]  */
        unsigned int    reserved_0            : 18  ; /* [30..13]  */
        unsigned int    tt_ready              : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF10;

/* Define the union U_DATE_COEFF11 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    date_clf2             : 10  ; /* [9..0]  */
        unsigned int    date_clf1             : 10  ; /* [19..10]  */
        unsigned int    cc_enf2               : 1   ; /* [20]  */
        unsigned int    cc_enf1               : 1   ; /* [21]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF11;

/* Define the union U_DATE_COEFF12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cc_f2data             : 16  ; /* [15..0]  */
        unsigned int    cc_f1data             : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF12;

/* Define the union U_DATE_COEFF13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cg_f1data             : 20  ; /* [19..0]  */
        unsigned int    cg_enf2               : 1   ; /* [20]  */
        unsigned int    cg_enf1               : 1   ; /* [21]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF13;

/* Define the union U_DATE_COEFF14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cg_f2data             : 20  ; /* [19..0]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF14;

/* Define the union U_DATE_COEFF15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wss_data              : 14  ; /* [13..0]  */
        unsigned int    wss_en                : 1   ; /* [14]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF15;

/* Define the union U_DATE_COEFF16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vps_data              : 24  ; /* [23..0]  */
        unsigned int    vps_en                : 1   ; /* [24]  */
        unsigned int    reserved_0            : 7   ; /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF16;

/* Define the union U_DATE_COEFF17 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vps_data               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF17;
/* Define the union U_DATE_COEFF18 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vps_data               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF18;
/* Define the union U_DATE_COEFF19 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vps_data              : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF19;

/* Define the union U_DATE_COEFF20 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tt05_enf2             : 1   ; /* [0]  */
        unsigned int    tt06_enf2             : 1   ; /* [1]  */
        unsigned int    tt06_enf1             : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF20;

/* Define the union U_DATE_COEFF21 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dac0_in_sel           : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 1   ; /* [3]  */
        unsigned int    dac1_in_sel           : 3   ; /* [6..4]  */
        unsigned int    reserved_1            : 1   ; /* [7]  */
        unsigned int    dac2_in_sel           : 3   ; /* [10..8]  */
        unsigned int    reserved_2            : 1   ; /* [11]  */
        unsigned int    dac3_in_sel           : 3   ; /* [14..12]  */
        unsigned int    reserved_3            : 1   ; /* [15]  */
        unsigned int    dac4_in_sel           : 3   ; /* [18..16]  */
        unsigned int    reserved_4            : 1   ; /* [19]  */
        unsigned int    dac5_in_sel           : 3   ; /* [22..20]  */
        unsigned int    reserved_5            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF21;

/* Define the union U_DATE_COEFF22 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    video_phase_delta     : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF22;

/* Define the union U_DATE_COEFF23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dac0_out_dly          : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 1   ; /* [3]  */
        unsigned int    dac1_out_dly          : 3   ; /* [6..4]  */
        unsigned int    reserved_1            : 1   ; /* [7]  */
        unsigned int    dac2_out_dly          : 3   ; /* [10..8]  */
        unsigned int    reserved_2            : 1   ; /* [11]  */
        unsigned int    dac3_out_dly          : 3   ; /* [14..12]  */
        unsigned int    reserved_3            : 1   ; /* [15]  */
        unsigned int    dac4_out_dly          : 3   ; /* [18..16]  */
        unsigned int    reserved_4            : 1   ; /* [19]  */
        unsigned int    dac5_out_dly          : 3   ; /* [22..20]  */
        unsigned int    reserved_5            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF23;

/* Define the union U_DATE_COEFF24 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int burst_start            : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF24;
/* Define the union U_DATE_COEFF25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    x_n_coef              : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    x_n_1_coef            : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF25;

/* Define the union U_DATE_COEFF26 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    x_n_1_coef            : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF26;

/* Define the union U_DATE_COEFF27 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    y_n_coef              : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    y_n_1_coef            : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF27;

/* Define the union U_DATE_COEFF28 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pixel_begin1          : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    pixel_begin2          : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF28;

/* Define the union U_DATE_COEFF29 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pixel_end             : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF29;

/* Define the union U_DATE_COEFF30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    g_secam               : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF30;

/* Define the union U_DATE_ISRMASK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tt_mask               : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_ISRMASK;

/* Define the union U_DATE_ISRSTATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tt_status             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_ISRSTATE;

/* Define the union U_DATE_ISR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tt_int                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_ISR;

/* Define the union U_DATE_VERSION */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int reserved_0             : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_VERSION;
/* Define the union U_DATE_COEFF37 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fir_y1_coeff0         : 8   ; /* [7..0]  */
        unsigned int    fir_y1_coeff1         : 8   ; /* [15..8]  */
        unsigned int    fir_y1_coeff2         : 8   ; /* [23..16]  */
        unsigned int    fir_y1_coeff3         : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF37;

/* Define the union U_DATE_COEFF38 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fir_y2_coeff0         : 16  ; /* [15..0]  */
        unsigned int    fir_y2_coeff1         : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF38;

/* Define the union U_DATE_COEFF39 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fir_y2_coeff2         : 16  ; /* [15..0]  */
        unsigned int    fir_y2_coeff3         : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF39;

/* Define the union U_DATE_COEFF40 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fir_c1_coeff0         : 8   ; /* [7..0]  */
        unsigned int    fir_c1_coeff1         : 8   ; /* [15..8]  */
        unsigned int    fir_c1_coeff2         : 8   ; /* [23..16]  */
        unsigned int    fir_c1_coeff3         : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF40;

/* Define the union U_DATE_COEFF41 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fir_c2_coeff0         : 16  ; /* [15..0]  */
        unsigned int    fir_c2_coeff1         : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF41;

/* Define the union U_DATE_COEFF42 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fir_c2_coeff2         : 16  ; /* [15..0]  */
        unsigned int    fir_c2_coeff3         : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF42;

/* Define the union U_DATE_DACDET1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vdac_det_high         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    det_line              : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_DACDET1;

/* Define the union U_DATE_DACDET2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    det_pixel_sta         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    det_pixel_wid         : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 4   ; /* [30..27]  */
        unsigned int    vdac_det_en           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_DACDET2;

/* Define the union U_DATE_COEFF50 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ovs_coeff0            : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    ovs_coeff1            : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF50;

/* Define the union U_DATE_COEFF51 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ovs_coeff0            : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    ovs_coeff1            : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF51;

/* Define the union U_DATE_COEFF52 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ovs_coeff0            : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    ovs_coeff1            : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF52;

/* Define the union U_DATE_COEFF53 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ovs_coeff0            : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    ovs_coeff1            : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF53;

/* Define the union U_DATE_COEFF54 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ovs_coeff0            : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    ovs_coeff1            : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF54;

/* Define the union U_DATE_COEFF55 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ovs_coeff0            : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    ovs_coeff1            : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF55;

/* Define the union U_DATE_COEFF56 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    oversam2_round_en     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF56;

/* Define the union U_DATE_COEFF57 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    v_gain                : 8   ; /* [7..0]  */
        unsigned int    u_gain                : 8   ; /* [15..8]  */
        unsigned int    ycvbs_gain            : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 7   ; /* [30..24]  */
        unsigned int    cvbs_gain_en          : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF57;

/* Define the union U_DATE_COEFF58 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pr_gain               : 8   ; /* [7..0]  */
        unsigned int    pb_gain               : 8   ; /* [15..8]  */
        unsigned int    ycomp_gain            : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 7   ; /* [30..24]  */
        unsigned int    comp_gain_en          : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF58;

/* Define the union U_DATE_COEFF59 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ynotch_clip_fullrange : 1   ; /* [0]  */
        unsigned int    clpf_clip_fullrange   : 1   ; /* [1]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    y_os_clip_fullrange   : 1   ; /* [4]  */
        unsigned int    reserved_1            : 3   ; /* [7..5]  */
        unsigned int    u_os_clip_fullrange   : 1   ; /* [8]  */
        unsigned int    v_os_clip_fullrange   : 1   ; /* [9]  */
        unsigned int    reserved_2            : 2   ; /* [11..10]  */
        unsigned int    cb_os_clip_fullrange  : 1   ; /* [12]  */
        unsigned int    cr_os_clip_fullrange  : 1   ; /* [13]  */
        unsigned int    reserved_3            : 2   ; /* [15..14]  */
        unsigned int    cb_gain_polar         : 1   ; /* [16]  */
        unsigned int    reserved_4            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF59;

/* Define the union U_DATE_COEFF60 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    line_960h_en          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF60;

/* Define the union U_DATE_COEFF61 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    burst_start_ovrd      : 1   ; /* [0]  */
        unsigned int    burst_dura_ovrd       : 1   ; /* [1]  */
        unsigned int    cb_bound_ovrd         : 1   ; /* [2]  */
        unsigned int    pal_half_ovrd         : 1   ; /* [3]  */
        unsigned int    rgb_acrive_ovrd       : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF61;

/* Define the union U_DATE_COEFF62 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int burst_dura_coeff       : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF62;
/* Define the union U_DATE_COEFF63 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int cb_bound_coeff         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF63;
/* Define the union U_DATE_COEFF64 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int pal_half_coeff         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF64;
/* Define the union U_DATE_COEFF65 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int rgb_active_coeff       : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF65;
/* Define the union U_DATE_COEFF66 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    htotal_ovrd           : 1   ; /* [0]  */
        unsigned int    wid_sync_ovrd         : 1   ; /* [1]  */
        unsigned int    wid_hfb_ovrd          : 1   ; /* [2]  */
        unsigned int    wid_act_ovrd          : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF66;

/* Define the union U_DATE_COEFF67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    htotal_sw             : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF67;

/* Define the union U_DATE_COEFF68 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wid_sync_sw           : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF68;

/* Define the union U_DATE_COEFF69 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wid_hfb_sw            : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF69;

/* Define the union U_DATE_COEFF70 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wid_act_sw            : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF70;

/* Define the union U_DATE_COEFF71 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    phase_shift_ovrd      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF71;

/* Define the union U_DATE_COEFF72 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int phase_shift_coeff      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DATE_COEFF72;
/* Define the union U_DATE_COEFF73 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_notch_ovrd       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF73;

/* Define the union U_DATE_COEFF74 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_notch_1          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef_notch_2          : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF74;

/* Define the union U_DATE_COEFF75 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_notch_3          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef_notch_4          : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF75;

/* Define the union U_DATE_COEFF76 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_notch_5          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef_notch_6          : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF76;

/* Define the union U_DATE_COEFF77 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_notch_7          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef_notch_8          : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF77;

/* Define the union U_DATE_COEFF78 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_notch_9          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef_notch_10         : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF78;

/* Define the union U_DATE_COEFF79 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_notch_11         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coef_notch_12         : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF79;

/* Define the union U_DATE_COEFF80 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_chra_lpf_ovrd    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF80;

/* Define the union U_DATE_COEFF81 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_chra_lpf_1       : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 7   ; /* [15..9]  */
        unsigned int    coef_chra_lpf_2       : 9   ; /* [24..16]  */
        unsigned int    reserved_1            : 7   ; /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF81;

/* Define the union U_DATE_COEFF82 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_chra_lpf_3       : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 7   ; /* [15..9]  */
        unsigned int    coef_chra_lpf_4       : 9   ; /* [24..16]  */
        unsigned int    reserved_1            : 7   ; /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF82;

/* Define the union U_DATE_COEFF83 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_chra_lpf_5       : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 7   ; /* [15..9]  */
        unsigned int    coef_chra_lpf_6       : 9   ; /* [24..16]  */
        unsigned int    reserved_1            : 7   ; /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF83;

/* Define the union U_DATE_COEFF84 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coef_chra_lpf_7       : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DATE_COEFF84;

//==============================================================================
/* Define the global struct */
typedef struct
{
    U_VOCTRL               VOCTRL                            ; /* 0x0 */
    U_VOINTSTA             VOINTSTA                          ; /* 0x4 */
    U_VOMSKINTSTA          VOMSKINTSTA                       ; /* 0x8 */
    U_VOINTMSK             VOINTMSK                          ; /* 0xc */
    U_VDPVERSION1          VDPVERSION1                       ; /* 0x10 */
    U_VDPVERSION2          VDPVERSION2                       ; /* 0x14 */
    unsigned int           reserved_0[2]                     ; /* 0x18~0x1c */
    U_VODEBUG              VODEBUG                           ; /* 0x20 */
    U_VOINTSTA1            VOINTSTA1                         ; /* 0x24 */
    U_VOMSKINTSTA1         VOMSKINTSTA1                      ; /* 0x28 */
    U_VOINTMSK1            VOINTMSK1                         ; /* 0x2c */
    U_VOAXISEL             VOAXISEL                          ; /* 0x30 */
    U_VOAXICTRL            VOAXICTRL                         ; /* 0x34 */
    U_VOWBCARB0            VOWBCARB0                         ; /* 0x38 */
    U_VOWBCARB1            VOWBCARB1                         ; /* 0x3c */
    U_VOUFSTA              VOUFSTA                           ; /* 0x40 */
    U_VOUFCLR              VOUFCLR                           ; /* 0x44 */
    unsigned int           reserved_1[2]                     ; /* 0x48~0x4c */
    U_VOINTPROC_TIM        VOINTPROC_TIM                     ; /* 0x50 */
    unsigned int           reserved_2[43]                    ; /* 0x54~0xfc */
    U_VO_MUX               VO_MUX                            ; /* 0x100 */
    U_VO_MUX_DAC           VO_MUX_DAC                        ; /* 0x104 */
    U_VO_MUX_TESTSYNC      VO_MUX_TESTSYNC                   ; /* 0x108 */
    U_VO_MUX_TESTDATA      VO_MUX_TESTDATA                   ; /* 0x10c */
    unsigned int           reserved_3[4]                     ; /* 0x110~0x11c */
    U_VO_DAC_CTRL          VO_DAC_CTRL                       ; /* 0x120 */
    unsigned int           reserved_4[3]                     ; /* 0x124~0x12c */
    U_VO_DAC_C_CTRL        VO_DAC_C_CTRL                     ; /* 0x130 */
    U_VO_DAC_R_CTRL        VO_DAC_R_CTRL                     ; /* 0x134 */
    U_VO_DAC_G_CTRL        VO_DAC_G_CTRL                     ; /* 0x138 */
    U_VO_DAC_B_CTRL        VO_DAC_B_CTRL                     ; /* 0x13c */
    U_VO_DAC_STAT0         VO_DAC_STAT0                      ; /* 0x140 */
    U_VO_DAC_STAT1         VO_DAC_STAT1                      ; /* 0x144 */
    unsigned int           reserved_5[110]                   ; /* 0x148~0x2fc */
    U_WBC_DHD_LOCATE       WBC_DHD_LOCATE                    ; /* 0x300 */
    U_WBC_OFL_EN           WBC_OFL_EN                        ; /* 0x304 */
    U_VHD_CORRESP          VHD_CORRESP                       ; /* 0x308 */
    U_GDC_CORRESP          GDC_CORRESP                       ; /* 0x30c */
    U_WBC_CORRESP          WBC_CORRESP                       ; /* 0x310 */
    unsigned int           reserved_6[59]                    ; /* 0x314~0x3fc */
    U_COEF_DATA            COEF_DATA                         ; /* 0x400 */
    unsigned int           reserved_7[3]                     ; /* 0x404~0x40c */
    U_V0_PARARD            V0_PARARD                         ; /* 0x410 */
    U_V1_PARARD            V1_PARARD                         ; /* 0x414 */
    unsigned int           reserved_8                        ; /* 0x418 */
    U_V3_PARARD            V3_PARARD                         ; /* 0x41c */
    unsigned int           reserved_9[8]                     ; /* 0x420~0x43c */
    U_VP0_PARARD           VP0_PARARD                        ; /* 0x440 */
    unsigned int           reserved_10[19]                   ; /* 0x444~0x48c */
    U_GP0_PARARD           GP0_PARARD                        ; /* 0x490 */
    U_GP1_PARARD           GP1_PARARD                        ; /* 0x494 */
    unsigned int           reserved_11[10]                   ; /* 0x498~0x4bc */
    U_WBCDHD_PARARD        WBCDHD_PARARD                     ; /* 0x4c0 */
    unsigned int           reserved_12[11]                   ; /* 0x4c4~0x4ec */
    U_DHD0_PARARD          DHD0_PARARD                       ; /* 0x4f0 */
    U_DHD1_PARARD          DHD1_PARARD                       ; /* 0x4f4 */
    unsigned int           reserved_13[194]                  ; /* 0x4f8~0x7fc */
    U_V0_CTRL              V0_CTRL                           ; /* 0x800 */
    U_V0_UPD               V0_UPD                            ; /* 0x804 */
    unsigned int           reserved_14[6]                    ; /* 0x808~0x81c */
    U_V0_PRERD             V0_PRERD                          ; /* 0x820 */
    unsigned int           reserved_15                       ; /* 0x824 */
    U_V0_IRESO             V0_IRESO                          ; /* 0x828 */
    U_V0_ORESO             V0_ORESO                          ; /* 0x82c */
    unsigned int           reserved_16[2]                    ; /* 0x830~0x834 */
    U_V0_CBMPARA           V0_CBMPARA                        ; /* 0x838 */
    unsigned int           reserved_17                       ; /* 0x83c */
    U_V0_PARAUP            V0_PARAUP                         ; /* 0x840 */
    U_V0_CPOS              V0_CPOS                           ; /* 0x844 */
    U_V0_DRAWMODE          V0_DRAWMODE                       ; /* 0x848 */
    unsigned int           reserved_18                       ; /* 0x84c */
    U_V0_HLCOEFAD          V0_HLCOEFAD                       ; /* 0x850 */
    U_V0_HCCOEFAD          V0_HCCOEFAD                       ; /* 0x854 */
    U_V0_VLCOEFAD          V0_VLCOEFAD                       ; /* 0x858 */
    U_V0_VCCOEFAD          V0_VCCOEFAD                       ; /* 0x85c */
    unsigned int           reserved_19[8]                    ; /* 0x860~0x87c */
    U_V0_CSC_IDC           V0_CSC_IDC                        ; /* 0x880 */
    U_V0_CSC_ODC           V0_CSC_ODC                        ; /* 0x884 */
    U_V0_CSC_IODC          V0_CSC_IODC                       ; /* 0x888 */
    U_V0_CSC_P0            V0_CSC_P0                         ; /* 0x88c */
    U_V0_CSC_P1            V0_CSC_P1                         ; /* 0x890 */
    U_V0_CSC_P2            V0_CSC_P2                         ; /* 0x894 */
    U_V0_CSC_P3            V0_CSC_P3                         ; /* 0x898 */
    U_V0_CSC_P4            V0_CSC_P4                         ; /* 0x89c */
    unsigned int           reserved_20[8]                    ; /* 0x8a0~0x8bc */
    U_V0_HSP               V0_HSP                            ; /* 0x8c0 */
    U_V0_HLOFFSET          V0_HLOFFSET                       ; /* 0x8c4 */
    U_V0_HCOFFSET          V0_HCOFFSET                       ; /* 0x8c8 */
    unsigned int           reserved_21[3]                    ; /* 0x8cc~0x8d4 */
    U_V0_VSP               V0_VSP                            ; /* 0x8d8 */
    U_V0_VSR               V0_VSR                            ; /* 0x8dc */
    U_V0_VOFFSET           V0_VOFFSET                        ; /* 0x8e0 */
    U_V0_VBOFFSET          V0_VBOFFSET                       ; /* 0x8e4 */
    unsigned int           reserved_22[6]                    ; /* 0x8e8~0x8fc */
    U_V0_DFPOS             V0_DFPOS                          ; /* 0x900 */
    U_V0_DLPOS             V0_DLPOS                          ; /* 0x904 */
    U_V0_VFPOS             V0_VFPOS                          ; /* 0x908 */
    U_V0_VLPOS             V0_VLPOS                          ; /* 0x90c */
    U_V0_BK                V0_BK                             ; /* 0x910 */
    U_V0_ALPHA             V0_ALPHA                          ; /* 0x914 */
    U_V0_RIMWIDTH          V0_RIMWIDTH                       ; /* 0x918 */
    U_V0_RIMCOL0           V0_RIMCOL0                        ; /* 0x91c */
    U_V0_RIMCOL1           V0_RIMCOL1                        ; /* 0x920 */
    unsigned int           reserved_23[23]                   ; /* 0x924~0x97c */
    U_V0_IFIRCOEF01        V0_IFIRCOEF01                     ; /* 0x980 */
    U_V0_IFIRCOEF23        V0_IFIRCOEF23                     ; /* 0x984 */
    U_V0_IFIRCOEF45        V0_IFIRCOEF45                     ; /* 0x988 */
    U_V0_IFIRCOEF67        V0_IFIRCOEF67                     ; /* 0x98c */
    unsigned int           reserved_24[28]                   ; /* 0x990~0x9fc */
    U_V0_P0RESO            V0_P0RESO                         ; /* 0xa00 */
    U_V0_P0LADDR           V0_P0LADDR                        ; /* 0xa04 */
    U_V0_P0CADDR           V0_P0CADDR                        ; /* 0xa08 */
    U_V0_P0STRIDE          V0_P0STRIDE                       ; /* 0xa0c */
    U_V0_P0VFPOS           V0_P0VFPOS                        ; /* 0xa10 */
    U_V0_P0VLPOS           V0_P0VLPOS                        ; /* 0xa14 */
    U_V0_P0CTRL            V0_P0CTRL                         ; /* 0xa18 */
    unsigned int           reserved_25[505]                  ; /* 0xa1c~0x11fc */
    U_V0_NADDR             V0_NADDR                          ; /* 0x1200 */
    U_V0_NCADDR            V0_NCADDR                         ; /* 0x1204 */
    unsigned int           reserved_26[10]                   ; /* 0x1208~0x122c */
    U_V0_MULTI_MODE        V0_MULTI_MODE                     ; /* 0x1230 */
    unsigned int           reserved_27[3]                    ; /* 0x1234~0x123c */
    U_V0_LADDROFFSET       V0_LADDROFFSET                    ; /* 0x1240 */
    U_V0_CADDROFFSET       V0_CADDROFFSET                    ; /* 0x1244 */
    unsigned int           reserved_28[2]                    ; /* 0x1248~0x124c */
    U_V0_FDRFIFOTHD        V0_FDRFIFOTHD                     ; /* 0x1250 */
    unsigned int           reserved_29[51]                   ; /* 0x1254~0x131c */
    U_V0_DCMP_LSTATE0      V0_DCMP_LSTATE0                   ; /* 0x1320 */
    U_V0_DCMP_LSTATE1      V0_DCMP_LSTATE1                   ; /* 0x1324 */
    U_V0_DCMP_CSTATE0      V0_DCMP_CSTATE0                   ; /* 0x1328 */
    U_V0_DCMP_CSTATE1      V0_DCMP_CSTATE1                   ; /* 0x132c */
    U_VO_DCMPERRCLR        VO_DCMPERRCLR                     ; /* 0x1330 */
    U_V0_DCMP_ERR          V0_DCMP_ERR                       ; /* 0x1334 */
    unsigned int           reserved_30[6]                    ; /* 0x1338~0x134c */
    U_VO_MRGERRCLR         VO_MRGERRCLR                      ; /* 0x1350 */
    U_V0_MRG_ERR           V0_MRG_ERR                        ; /* 0x1354 */
    unsigned int           reserved_31[4394]                 ; /* 0x1358~0x57fc */
    U_VP0_CTRL             VP0_CTRL                          ; /* 0x5800 */
    U_VP0_UPD              VP0_UPD                           ; /* 0x5804 */
    U_VP0_ACC_CAD          VP0_ACC_CAD                       ; /* 0x5808 */
    U_VP0_ACM_CAD          VP0_ACM_CAD                       ; /* 0x580c */
    U_VP0_PARAUP           VP0_PARAUP                        ; /* 0x5810 */
    U_VP0_IRESO            VP0_IRESO                         ; /* 0x5814 */
    U_VP0_DOF_CTRL         VP0_DOF_CTRL                      ; /* 0x5818 */
    U_VP0_DOF_STEP         VP0_DOF_STEP                      ; /* 0x581c */
    U_VP0_ACCTHD1          VP0_ACCTHD1                       ; /* 0x5820 */
    U_VP0_ACCTHD2          VP0_ACCTHD2                       ; /* 0x5824 */
    U_VP0_ACCLOWN          VP0_ACCLOWN[3]                    ; /* 0x5828~0x5830 */
    U_VP0_ACCMEDN          VP0_ACCMEDN[3]                    ; /* 0x5834~0x583c */
    U_VP0_ACCHIGHN         VP0_ACCHIGHN[3]                   ; /* 0x5840~0x5848 */
    U_VP0_ACCMLN           VP0_ACCMLN[3]                     ; /* 0x584c~0x5854 */
    U_VP0_ACCMHN           VP0_ACCMHN[3]                     ; /* 0x5858~0x5860 */
    U_VP0_ACC3LOW          VP0_ACC3LOW                       ; /* 0x5864 */
    U_VP0_ACC3MED          VP0_ACC3MED                       ; /* 0x5868 */
    U_VP0_ACC3HIGH         VP0_ACC3HIGH                      ; /* 0x586c */
    U_VP0_ACC8MLOW         VP0_ACC8MLOW                      ; /* 0x5870 */
    U_VP0_ACC8MHIGH        VP0_ACC8MHIGH                     ; /* 0x5874 */
    U_VP0_ACCTOTAL         VP0_ACCTOTAL                      ; /* 0x5878 */
    U_VP0_ACM_CTRL         VP0_ACM_CTRL                      ; /* 0x587c */
    U_VP0_ACM_ADJ          VP0_ACM_ADJ                       ; /* 0x5880 */
    U_VP0_DFPOS            VP0_DFPOS                         ; /* 0x5884 */
    U_VP0_DLPOS            VP0_DLPOS                         ; /* 0x5888 */
    U_VP0_VFPOS            VP0_VFPOS                         ; /* 0x588c */
    U_VP0_VLPOS            VP0_VLPOS                         ; /* 0x5890 */
    U_VP0_BK               VP0_BK                            ; /* 0x5894 */
    U_VP0_ALPHA            VP0_ALPHA                         ; /* 0x5898 */
    U_VP0_CSC0_IDC         VP0_CSC0_IDC                      ; /* 0x589c */
    unsigned int           reserved_32[24]                   ; /* 0x58a0~0x58fc */
    U_VP0_CSC0_ODC         VP0_CSC0_ODC                      ; /* 0x5900 */
    U_VP0_CSC0_IODC        VP0_CSC0_IODC                     ; /* 0x5904 */
    U_VP0_CSC0_P0          VP0_CSC0_P0                       ; /* 0x5908 */
    U_VP0_CSC0_P1          VP0_CSC0_P1                       ; /* 0x590c */
    U_VP0_CSC0_P2          VP0_CSC0_P2                       ; /* 0x5910 */
    U_VP0_CSC0_P3          VP0_CSC0_P3                       ; /* 0x5914 */
    U_VP0_CSC0_P4          VP0_CSC0_P4                       ; /* 0x5918 */
    U_VP0_CSC1_IDC         VP0_CSC1_IDC                      ; /* 0x591c */
    U_VP0_CSC1_ODC         VP0_CSC1_ODC                      ; /* 0x5920 */
    U_VP0_CSC1_IODC        VP0_CSC1_IODC                     ; /* 0x5924 */
    U_VP0_CSC1_P0          VP0_CSC1_P0                       ; /* 0x5928 */
    U_VP0_CSC1_P1          VP0_CSC1_P1                       ; /* 0x592c */
    U_VP0_CSC1_P2          VP0_CSC1_P2                       ; /* 0x5930 */
    U_VP0_CSC1_P3          VP0_CSC1_P3                       ; /* 0x5934 */
    U_VP0_CSC1_P4          VP0_CSC1_P4                       ; /* 0x5938 */
    unsigned int           reserved_33[433]                  ; /* 0x593c~0x5ffc */
    U_G0_CTRL              G0_CTRL                           ; /* 0x6000 */
    U_G0_UPD               G0_UPD                            ; /* 0x6004 */
    unsigned int           reserved_34[2]                    ; /* 0x6008~0x600c */
    U_G0_ADDR              G0_ADDR                           ; /* 0x6010 */
    unsigned int           reserved_35                       ; /* 0x6014 */
    U_G0_NADDR             G0_NADDR                          ; /* 0x6018 */
    U_G0_STRIDE            G0_STRIDE                         ; /* 0x601c */
    U_G0_IRESO             G0_IRESO                          ; /* 0x6020 */
    U_G0_SFPOS             G0_SFPOS                          ; /* 0x6024 */
    unsigned int           reserved_36[2]                    ; /* 0x6028~0x602c */
    U_G0_CBMPARA           G0_CBMPARA                        ; /* 0x6030 */
    U_G0_CKEYMAX           G0_CKEYMAX                        ; /* 0x6034 */
    U_G0_CKEYMIN           G0_CKEYMIN                        ; /* 0x6038 */
    U_G0_CMASK             G0_CMASK                          ; /* 0x603c */
    U_G0_PARAADDR          G0_PARAADDR                       ; /* 0x6040 */
    U_G0_PARAUP            G0_PARAUP                         ; /* 0x6044 */
    U_G0_FIFOTHD           G0_FIFOTHD                        ; /* 0x6048 */
    unsigned int           reserved_37                       ; /* 0x604c */
    U_G0_DCMP_ADDR         G0_DCMP_ADDR                      ; /* 0x6050 */
    U_G0_DCMP_NADDR        G0_DCMP_NADDR                     ; /* 0x6054 */
    U_G0_DCMP_OFFSET       G0_DCMP_OFFSET                    ; /* 0x6058 */
    unsigned int           reserved_38                       ; /* 0x605c */
    U_G0_DCMP_DBG          G0_DCMP_DBG                       ; /* 0x6060 */
    unsigned int           reserved_39[3]                    ; /* 0x6064~0x606c */
    U_G0_DCMP_INTER        G0_DCMP_INTER                     ; /* 0x6070 */
    unsigned int           reserved_40[3]                    ; /* 0x6074~0x607c */
    U_G0_DFPOS             G0_DFPOS                          ; /* 0x6080 */
    U_G0_DLPOS             G0_DLPOS                          ; /* 0x6084 */
    U_G0_VFPOS             G0_VFPOS                          ; /* 0x6088 */
    U_G0_VLPOS             G0_VLPOS                          ; /* 0x608c */
    U_G0_BK                G0_BK                             ; /* 0x6090 */
    U_G0_ALPHA             G0_ALPHA                          ; /* 0x6094 */
    unsigned int           reserved_41[2]                    ; /* 0x6098~0x609c */
    U_G0_DOF_CTRL          G0_DOF_CTRL                       ; /* 0x60a0 */
    U_G0_DOF_STEP          G0_DOF_STEP                       ; /* 0x60a4 */
    unsigned int           reserved_42[6]                    ; /* 0x60a8~0x60bc */
    U_G0_CSC_IDC           G0_CSC_IDC                        ; /* 0x60c0 */
    U_G0_CSC_ODC           G0_CSC_ODC                        ; /* 0x60c4 */
    U_G0_CSC_IODC          G0_CSC_IODC                       ; /* 0x60c8 */
    U_G0_CSC_P0            G0_CSC_P0                         ; /* 0x60cc */
    U_G0_CSC_P1            G0_CSC_P1                         ; /* 0x60d0 */
    U_G0_CSC_P2            G0_CSC_P2                         ; /* 0x60d4 */
    U_G0_CSC_P3            G0_CSC_P3                         ; /* 0x60d8 */
    U_G0_CSC_P4            G0_CSC_P4                         ; /* 0x60dc */
    unsigned int           reserved_43[3016]                 ; /* 0x60e0~0x8ffc */
    U_GP0_CTRL             GP0_CTRL                          ; /* 0x9000 */
    U_GP0_UPD              GP0_UPD                           ; /* 0x9004 */
    U_GP0_ORESO            GP0_ORESO                         ; /* 0x9008 */
    U_GP0_IRESO            GP0_IRESO                         ; /* 0x900c */
    U_GP0_HCOEFAD          GP0_HCOEFAD                       ; /* 0x9010 */
    U_GP0_VCOEFAD          GP0_VCOEFAD                       ; /* 0x9014 */
    U_GP0_PARAUP           GP0_PARAUP                        ; /* 0x9018 */
    unsigned int           reserved_44                       ; /* 0x901c */
    U_GP0_GALPHA           GP0_GALPHA                        ; /* 0x9020 */
    unsigned int           reserved_45[55]                   ; /* 0x9024~0x90fc */
    U_GP0_DFPOS            GP0_DFPOS                         ; /* 0x9100 */
    U_GP0_DLPOS            GP0_DLPOS                         ; /* 0x9104 */
    U_GP0_VFPOS            GP0_VFPOS                         ; /* 0x9108 */
    U_GP0_VLPOS            GP0_VLPOS                         ; /* 0x910c */
    U_GP0_BK               GP0_BK                            ; /* 0x9110 */
    U_GP0_ALPHA            GP0_ALPHA                         ; /* 0x9114 */
    unsigned int           reserved_46[2]                    ; /* 0x9118~0x911c */
    U_GP0_CSC_IDC          GP0_CSC_IDC                       ; /* 0x9120 */
    U_GP0_CSC_ODC          GP0_CSC_ODC                       ; /* 0x9124 */
    U_GP0_CSC_IODC         GP0_CSC_IODC                      ; /* 0x9128 */
    U_GP0_CSC_P0           GP0_CSC_P0                        ; /* 0x912c */
    U_GP0_CSC_P1           GP0_CSC_P1                        ; /* 0x9130 */
    U_GP0_CSC_P2           GP0_CSC_P2                        ; /* 0x9134 */
    U_GP0_CSC_P3           GP0_CSC_P3                        ; /* 0x9138 */
    U_GP0_CSC_P4           GP0_CSC_P4                        ; /* 0x913c */
    U_GP0_ZME_HSP          GP0_ZME_HSP                       ; /* 0x9140 */
    U_GP0_ZME_HOFFSET      GP0_ZME_HOFFSET                   ; /* 0x9144 */
    U_GP0_ZME_VSP          GP0_ZME_VSP                       ; /* 0x9148 */
    U_GP0_ZME_VSR          GP0_ZME_VSR                       ; /* 0x914c */
    U_GP0_ZME_VOFFSET      GP0_ZME_VOFFSET                   ; /* 0x9150 */
    unsigned int           reserved_47[3]                    ; /* 0x9154~0x915c */
    U_GP0_ZME_LTICTRL      GP0_ZME_LTICTRL                   ; /* 0x9160 */
    U_GP0_ZME_LHPASSCOEF   GP0_ZME_LHPASSCOEF                ; /* 0x9164 */
    U_GP0_ZME_LTITHD       GP0_ZME_LTITHD                    ; /* 0x9168 */
    unsigned int           reserved_48                       ; /* 0x916c */
    U_GP0_ZME_LHFREQTHD    GP0_ZME_LHFREQTHD                 ; /* 0x9170 */
    U_GP0_ZME_LGAINCOEF    GP0_ZME_LGAINCOEF                 ; /* 0x9174 */
    unsigned int           reserved_49[2]                    ; /* 0x9178~0x917c */
    U_GP0_ZME_CTICTRL      GP0_ZME_CTICTRL                   ; /* 0x9180 */
    U_GP0_ZME_CHPASSCOEF   GP0_ZME_CHPASSCOEF                ; /* 0x9184 */
    U_GP0_ZME_CTITHD       GP0_ZME_CTITHD                    ; /* 0x9188 */
    unsigned int           reserved_50[925]                  ; /* 0x918c~0x9ffc */
    U_WBC_G0_CTRL          WBC_G0_CTRL                       ; /* 0xa000 */
    U_WBC_G0_UPD           WBC_G0_UPD                        ; /* 0xa004 */
    U_WBC_G0_CMP           WBC_G0_CMP                        ; /* 0xa008 */
    unsigned int           reserved_51                       ; /* 0xa00c */
    U_WBC_G0_ADDR          WBC_G0_ADDR                       ; /* 0xa010 */
    U_WBC_G0_GB_ADDR       WBC_G0_GB_ADDR                    ; /* 0xa014 */
    U_WBC_G0_STRIDE        WBC_G0_STRIDE                     ; /* 0xa018 */
    U_WBC_G0_OFFSET        WBC_G0_OFFSET                     ; /* 0xa01c */
    U_WBC_G0_ORESO         WBC_G0_ORESO                      ; /* 0xa020 */
    U_WBC_G0_YCROP         WBC_G0_YCROP                      ; /* 0xa024 */
    U_WBC_G0_FCROP         WBC_G0_FCROP                      ; /* 0xa028 */
    unsigned int           reserved_52                       ; /* 0xa02c */
    U_WBC_G0_LCROP         WBC_G0_LCROP                      ; /* 0xa030 */
    U_WBC_G0_CSTR_ERR      WBC_G0_CSTR_ERR                   ; /* 0xa034 */
    U_WBC_G0_CLR_CSTR_ERR  WBC_G0_CLR_CSTR_ERR              ; /* 0xa038 */
    unsigned int           reserved_53[17]                   ; /* 0xa03c~0xa07c */
    U_WBC_G0_GLB_INFO      WBC_G0_GLB_INFO                   ; /* 0xa080 */
    U_WBC_G0_FRAME_SIZE    WBC_G0_FRAME_SIZE                 ; /* 0xa084 */
    U_WBC_G0_RC_CFG0       WBC_G0_RC_CFG0                    ; /* 0xa088 */
    U_WBC_G0_RC_CFG1       WBC_G0_RC_CFG1                    ; /* 0xa08c */
    U_WBC_G0_RC_CFG2       WBC_G0_RC_CFG2                    ; /* 0xa090 */
    U_WBC_G0_RC_CFG3       WBC_G0_RC_CFG3                    ; /* 0xa094 */
    U_WBC_G0_RC_CFG4       WBC_G0_RC_CFG4                    ; /* 0xa098 */
    U_WBC_G0_RC_CFG5       WBC_G0_RC_CFG5                    ; /* 0xa09c */
    U_WBC_G0_RC_CFG6       WBC_G0_RC_CFG6                    ; /* 0xa0a0 */
    U_WBC_G0_RC_CFG7       WBC_G0_RC_CFG7                    ; /* 0xa0a4 */
    U_WBC_G0_RC_CFG8       WBC_G0_RC_CFG8                    ; /* 0xa0a8 */
    U_WBC_G0_RC_CFG9       WBC_G0_RC_CFG9                    ; /* 0xa0ac */
    U_WBC_G0_MAX_ROW_LEN   WBC_G0_MAX_ROW_LEN                ; /* 0xa0b0 */
    unsigned int           reserved_54[467]                  ; /* 0xa0b4~0xa7fc */
    U_WBC_GP0_CTRL         WBC_GP0_CTRL                      ; /* 0xa800 */
    U_WBC_GP0_UPD          WBC_GP0_UPD                       ; /* 0xa804 */
    unsigned int           reserved_55[2]                    ; /* 0xa808~0xa80c */
    U_WBC_GP0_YADDR        WBC_GP0_YADDR                     ; /* 0xa810 */
    U_WBC_GP0_CADDR        WBC_GP0_CADDR                     ; /* 0xa814 */
    U_WBC_GP0_STRIDE       WBC_GP0_STRIDE                    ; /* 0xa818 */
    unsigned int           reserved_56                       ; /* 0xa81c */
    U_WBC_GP0_ORESO        WBC_GP0_ORESO                     ; /* 0xa820 */
    U_WBC_GP0_FCROP        WBC_GP0_FCROP                     ; /* 0xa824 */
    U_WBC_GP0_LCROP        WBC_GP0_LCROP                     ; /* 0xa828 */
    unsigned int           reserved_57[53]                   ; /* 0xa82c~0xa8fc */
    U_WBC_GP0_DITHER_CTRL   WBC_GP0_DITHER_CTRL              ; /* 0xa900 */
    U_WBC_GP0_DITHER_COEF0   WBC_GP0_DITHER_COEF0            ; /* 0xa904 */
    U_WBC_GP0_DITHER_COEF1   WBC_GP0_DITHER_COEF1            ; /* 0xa908 */
    unsigned int           reserved_58[189]                  ; /* 0xa90c~0xabfc */
    U_WBC_DHD0_CTRL        WBC_DHD0_CTRL                     ; /* 0xac00 */
    U_WBC_DHD0_UPD         WBC_DHD0_UPD                      ; /* 0xac04 */
    unsigned int           reserved_59[2]                    ; /* 0xac08~0xac0c */
    U_WBC_DHD0_YADDR       WBC_DHD0_YADDR                    ; /* 0xac10 */
    U_WBC_DHD0_CADDR       WBC_DHD0_CADDR                    ; /* 0xac14 */
    U_WBC_DHD0_STRIDE      WBC_DHD0_STRIDE                   ; /* 0xac18 */
    unsigned int           reserved_60                       ; /* 0xac1c */
    U_WBC_DHD0_ORESO       WBC_DHD0_ORESO                    ; /* 0xac20 */
    U_WBC_DHD0_FCROP       WBC_DHD0_FCROP                    ; /* 0xac24 */
    U_WBC_DHD0_LCROP       WBC_DHD0_LCROP                    ; /* 0xac28 */
    unsigned int           reserved_61                       ; /* 0xac2c */
    U_WBC_DHD0_LOWDLYCTRL   WBC_DHD0_LOWDLYCTRL              ; /* 0xac30 */
    U_WBC_DHD0_TUNLADDR    WBC_DHD0_TUNLADDR                 ; /* 0xac34 */
    U_WBC_DHD0_LOWDLYSTA   WBC_DHD0_LOWDLYSTA                ; /* 0xac38 */
    unsigned int           reserved_62                       ; /* 0xac3c */
    U_WBC_DHD0_PARAUP      WBC_DHD0_PARAUP                   ; /* 0xac40 */
    unsigned int           reserved_63[3]                    ; /* 0xac44~0xac4c */
    U_WBC_DHD0_HLCOEFAD    WBC_DHD0_HLCOEFAD                 ; /* 0xac50 */
    U_WBC_DHD0_HCCOEFAD    WBC_DHD0_HCCOEFAD                 ; /* 0xac54 */
    U_WBC_DHD0_VLCOEFAD    WBC_DHD0_VLCOEFAD                 ; /* 0xac58 */
    U_WBC_DHD0_VCCOEFAD    WBC_DHD0_VCCOEFAD                 ; /* 0xac5c */
    unsigned int           reserved_64[16]                   ; /* 0xac60~0xac9c */
    U_WBC_DHD0_HISTOGRAM0   WBC_DHD0_HISTOGRAM0              ; /* 0xaca0 */
    U_WBC_DHD0_HISTOGRAM1   WBC_DHD0_HISTOGRAM1              ; /* 0xaca4 */
    U_WBC_DHD0_HISTOGRAM2   WBC_DHD0_HISTOGRAM2              ; /* 0xaca8 */
    U_WBC_DHD0_HISTOGRAM3   WBC_DHD0_HISTOGRAM3              ; /* 0xacac */
    U_WBC_DHD0_HISTOGRAM4   WBC_DHD0_HISTOGRAM4              ; /* 0xacb0 */
    U_WBC_DHD0_HISTOGRAM5   WBC_DHD0_HISTOGRAM5              ; /* 0xacb4 */
    U_WBC_DHD0_HISTOGRAM6   WBC_DHD0_HISTOGRAM6              ; /* 0xacb8 */
    U_WBC_DHD0_HISTOGRAM7   WBC_DHD0_HISTOGRAM7              ; /* 0xacbc */
    U_WBC_DHD0_HISTOGRAM8   WBC_DHD0_HISTOGRAM8              ; /* 0xacc0 */
    U_WBC_DHD0_HISTOGRAM9   WBC_DHD0_HISTOGRAM9              ; /* 0xacc4 */
    U_WBC_DHD0_HISTOGRAM10   WBC_DHD0_HISTOGRAM10            ; /* 0xacc8 */
    U_WBC_DHD0_HISTOGRAM11   WBC_DHD0_HISTOGRAM11            ; /* 0xaccc */
    U_WBC_DHD0_HISTOGRAM12   WBC_DHD0_HISTOGRAM12            ; /* 0xacd0 */
    U_WBC_DHD0_HISTOGRAM13   WBC_DHD0_HISTOGRAM13            ; /* 0xacd4 */
    U_WBC_DHD0_HISTOGRAM14   WBC_DHD0_HISTOGRAM14            ; /* 0xacd8 */
    U_WBC_DHD0_HISTOGRAM15   WBC_DHD0_HISTOGRAM15            ; /* 0xacdc */
    unsigned int           reserved_65[4]                    ; /* 0xace0~0xacec */
    U_WBC_DHD0_CHECKSUM_Y   WBC_DHD0_CHECKSUM_Y              ; /* 0xacf0 */
    U_WBC_DHD0_CHECKSUM_C   WBC_DHD0_CHECKSUM_C              ; /* 0xacf4 */
    unsigned int           reserved_66[2]                    ; /* 0xacf8~0xacfc */
    U_WBC_DHD0_DITHER_CTRL   WBC_DHD0_DITHER_CTRL            ; /* 0xad00 */
    U_WBC_DHD0_DITHER_COEF0   WBC_DHD0_DITHER_COEF0          ; /* 0xad04 */
    U_WBC_DHD0_DITHER_COEF1   WBC_DHD0_DITHER_COEF1          ; /* 0xad08 */
    unsigned int           reserved_67[65]                   ; /* 0xad0c~0xae0c */
    U_WBC_DHD0_HCDS        WBC_DHD0_HCDS                     ; /* 0xae10 */
    U_WBC_DHD0_HCDS_COEF0   WBC_DHD0_HCDS_COEF0              ; /* 0xae14 */
    U_WBC_DHD0_HCDS_COEF1   WBC_DHD0_HCDS_COEF1              ; /* 0xae18 */
    unsigned int           reserved_68[41]                   ; /* 0xae1c~0xaebc */
    U_WBC_DHD0_ZME_HSP     WBC_DHD0_ZME_HSP                  ; /* 0xaec0 */
    U_WBC_DHD0_ZME_HLOFFSET   WBC_DHD0_ZME_HLOFFSET          ; /* 0xaec4 */
    U_WBC_DHD0_ZME_HCOFFSET   WBC_DHD0_ZME_HCOFFSET          ; /* 0xaec8 */
    unsigned int           reserved_69[3]                    ; /* 0xaecc~0xaed4 */
    U_WBC_DHD0_ZME_VSP     WBC_DHD0_ZME_VSP                  ; /* 0xaed8 */
    U_WBC_DHD0_ZME_VSR     WBC_DHD0_ZME_VSR                  ; /* 0xaedc */
    U_WBC_DHD0_ZME_VOFFSET   WBC_DHD0_ZME_VOFFSET            ; /* 0xaee0 */
    U_WBC_DHD0_ZME_VBOFFSET   WBC_DHD0_ZME_VBOFFSET          ; /* 0xaee4 */
    unsigned int           reserved_70[6]                    ; /* 0xaee8~0xaefc */
    U_WBC_DHD0_CSCIDC      WBC_DHD0_CSCIDC                   ; /* 0xaf00 */
    U_WBC_DHD0_CSCODC      WBC_DHD0_CSCODC                   ; /* 0xaf04 */
    U_WBC_DHD0_CSCP0       WBC_DHD0_CSCP0                    ; /* 0xaf08 */
    U_WBC_DHD0_CSCP1       WBC_DHD0_CSCP1                    ; /* 0xaf0c */
    U_WBC_DHD0_CSCP2       WBC_DHD0_CSCP2                    ; /* 0xaf10 */
    U_WBC_DHD0_CSCP3       WBC_DHD0_CSCP3                    ; /* 0xaf14 */
    U_WBC_DHD0_CSCP4       WBC_DHD0_CSCP4                    ; /* 0xaf18 */
    unsigned int           reserved_71[57]                   ; /* 0xaf1c~0xaffc */
    U_MIXV0_BKG            MIXV0_BKG                         ; /* 0xb000 */
    unsigned int           reserved_72                       ; /* 0xb004 */
    U_MIXV0_MIX            MIXV0_MIX                         ; /* 0xb008 */
    unsigned int           reserved_73[125]                  ; /* 0xb00c~0xb1fc */
    U_MIXG0_BKG            MIXG0_BKG                         ; /* 0xb200 */
    U_MIXG0_BKALPHA        MIXG0_BKALPHA                     ; /* 0xb204 */
    U_MIXG0_MIX            MIXG0_MIX                         ; /* 0xb208 */
    unsigned int           reserved_74[125]                  ; /* 0xb20c~0xb3fc */
    U_CBM_BKG1             CBM_BKG1                          ; /* 0xb400 */
    unsigned int           reserved_75                       ; /* 0xb404 */
    U_CBM_MIX1             CBM_MIX1                          ; /* 0xb408 */
    unsigned int           reserved_76[5]                    ; /* 0xb40c~0xb41c */
    U_CBM_BKG2             CBM_BKG2                          ; /* 0xb420 */
    unsigned int           reserved_77                       ; /* 0xb424 */
    U_CBM_MIX2             CBM_MIX2                          ; /* 0xb428 */
    unsigned int           reserved_78[5]                    ; /* 0xb42c~0xb43c */
    U_CBM_ATTR             CBM_ATTR                          ; /* 0xb440 */
    unsigned int           reserved_79[111]                  ; /* 0xb444~0xb5fc */
    U_MIXDSD_BKG           MIXDSD_BKG                        ; /* 0xb600 */
    unsigned int           reserved_80                       ; /* 0xb604 */
    U_MIXDSD_MIX           MIXDSD_MIX                        ; /* 0xb608 */
    unsigned int           reserved_81[637]                  ; /* 0xb60c~0xbffc */
    U_DHD0_CTRL            DHD0_CTRL                         ; /* 0xc000 */
    U_DHD0_VSYNC           DHD0_VSYNC                        ; /* 0xc004 */
    U_DHD0_HSYNC1          DHD0_HSYNC1                       ; /* 0xc008 */
    U_DHD0_HSYNC2          DHD0_HSYNC2                       ; /* 0xc00c */
    U_DHD0_VPLUS           DHD0_VPLUS                        ; /* 0xc010 */
    U_DHD0_PWR             DHD0_PWR                          ; /* 0xc014 */
    U_DHD0_VTTHD3          DHD0_VTTHD3                       ; /* 0xc018 */
    U_DHD0_VTTHD           DHD0_VTTHD                        ; /* 0xc01c */
    U_DHD0_SYNC_INV        DHD0_SYNC_INV                     ; /* 0xc020 */
    unsigned int           reserved_82[2]                    ; /* 0xc024~0xc028 */
    U_DHD0_DATA_SEL        DHD0_DATA_SEL                     ; /* 0xc02c */
    U_DHD0_AFFTHD          DHD0_AFFTHD                       ; /* 0xc030 */
    U_DHD0_ABUFTHD         DHD0_ABUFTHD                      ; /* 0xc034 */
    U_DHD0_DACDET1         DHD0_DACDET1                      ; /* 0xc038 */
    U_DHD0_DACDET2         DHD0_DACDET2                      ; /* 0xc03c */
    U_DHD0_CSC_IDC         DHD0_CSC_IDC                      ; /* 0xc040 */
    U_DHD0_CSC_ODC         DHD0_CSC_ODC                      ; /* 0xc044 */
    U_DHD0_CSC_IODC        DHD0_CSC_IODC                     ; /* 0xc048 */
    U_DHD0_CSC_P0          DHD0_CSC_P0                       ; /* 0xc04c */
    U_DHD0_CSC_P1          DHD0_CSC_P1                       ; /* 0xc050 */
    U_DHD0_CSC_P2          DHD0_CSC_P2                       ; /* 0xc054 */
    U_DHD0_CSC_P3          DHD0_CSC_P3                       ; /* 0xc058 */
    U_DHD0_CSC_P4          DHD0_CSC_P4                       ; /* 0xc05c */
    U_DHD0_DITHER0_CTRL    DHD0_DITHER0_CTRL                 ; /* 0xc060 */
    U_DHD0_DITHER0_COEF0   DHD0_DITHER0_COEF0                ; /* 0xc064 */
    U_DHD0_DITHER0_COEF1   DHD0_DITHER0_COEF1                ; /* 0xc068 */
    unsigned int           reserved_83                       ; /* 0xc06c */
    U_DHD0_DITHER1_CTRL    DHD0_DITHER1_CTRL                 ; /* 0xc070 */
    U_DHD0_DITHER1_COEF0   DHD0_DITHER1_COEF0                ; /* 0xc074 */
    U_DHD0_DITHER1_COEF1   DHD0_DITHER1_COEF1                ; /* 0xc078 */
    unsigned int           reserved_84                       ; /* 0xc07c */
    U_DHD0_CLIP0_L         DHD0_CLIP0_L                      ; /* 0xc080 */
    U_DHD0_CLIP0_H         DHD0_CLIP0_H                      ; /* 0xc084 */
    U_DHD0_CLIP1_L         DHD0_CLIP1_L                      ; /* 0xc088 */
    U_DHD0_CLIP1_H         DHD0_CLIP1_H                      ; /* 0xc08c */
    U_DHD0_CLIP2_L         DHD0_CLIP2_L                      ; /* 0xc090 */
    U_DHD0_CLIP2_H         DHD0_CLIP2_H                      ; /* 0xc094 */
    U_DHD0_CLIP3_L         DHD0_CLIP3_L                      ; /* 0xc098 */
    U_DHD0_CLIP3_H         DHD0_CLIP3_H                      ; /* 0xc09c */  
    U_DHD0_CCDIIMGMOD      DHD0_CCDIIMGMOD                   ; /* 0xc0a0 */
    U_DHD0_CCDOPOSMSKH     DHD0_CCDIPOSMSKH                  ; /* 0xc0a4 */
    U_DHD0_CCDOPOSMSKL     DHD0_CCDIPOSMSKL                  ; /* 0xc0a8 */
    unsigned int           reserved_85                       ; /* 0xc0ac */
    //U_DHD0_CLIP4_L         DHD0_CLIP4_L                      ; /* 0xc0a0 */
    //U_DHD0_CLIP4_H         DHD0_CLIP4_H                      ; /* 0xc0a4 */
    //unsigned int           reserved_85[2]                    ; /* 0xc0a8~0xc0ac */
    U_DHD0_PARATHD         DHD0_PARATHD                      ; /* 0xc0b0 */
    U_DHD0_START_POS       DHD0_START_POS                    ; /* 0xc0b4 */
    unsigned int           reserved_86[2]                    ; /* 0xc0b8~0xc0bc */
    U_DHD0_CCDOIMGMOD      DHD0_CCDOIMGMOD                   ; /* 0xc0c0 */
    U_DHD0_CCDOPOSMSKH     DHD0_CCDOPOSMSKH                  ; /* 0xc0c4 */
    U_DHD0_CCDOPOSMSKL     DHD0_CCDOPOSMSKL                  ; /* 0xc0c8 */
    unsigned int           reserved_87                       ; /* 0xc0cc */
    U_DHD0_LOCKCFG         DHD0_LOCKCFG                      ; /* 0xc0d0 */
    U_DHD0_LOCK_STATE1     DHD0_LOCK_STATE1                  ; /* 0xc0d4 */
    U_DHD0_LOCK_STATE2     DHD0_LOCK_STATE2                  ; /* 0xc0d8 */
    U_DHD0_LOCK_STATE3     DHD0_LOCK_STATE3                  ; /* 0xc0dc */
    U_DHD0_GMM_COEFAD      DHD0_GMM_COEFAD                   ; /* 0xc0e0 */
    unsigned int           reserved_88[2]                    ; /* 0xc0e4~0xc0e8 */
    U_DHD0_PARAUP          DHD0_PARAUP                       ; /* 0xc0ec */
    U_DHD0_STATE           DHD0_STATE                        ; /* 0xc0f0 */
    unsigned int           reserved_89                       ; /* 0xc0f4 */
    U_DHD0_DEBUG           DHD0_DEBUG                        ; /* 0xc0f8 */
    U_DHD0_DEBUG_STATE     DHD0_DEBUG_STATE                  ; /* 0xc0fc */
    U_DHD0_HSPCFG0         DHD0_HSPCFG0                      ; /* 0xc100 */
    U_DHD0_HSPCFG1         DHD0_HSPCFG1                      ; /* 0xc104 */
    unsigned int           reserved_90[3]                     ; /* 0xc108~0xc110 */
    U_DHD0_HSPCFG5         DHD0_HSPCFG5                      ; /* 0xc114 */
    U_DHD0_HSPCFG6         DHD0_HSPCFG6                      ; /* 0xc118 */
    U_DHD0_HSPCFG7         DHD0_HSPCFG7                      ; /* 0xc11c */
    U_DHD0_HSPCFG8         DHD0_HSPCFG8                      ; /* 0xc120 */
    unsigned int           reserved_91[3]                    ; /* 0xc124~0xc12c */
    U_DHD0_HSPCFG12        DHD0_HSPCFG12                     ; /* 0xc130 */
    U_DHD0_HSPCFG13        DHD0_HSPCFG13                     ; /* 0xc134 */
    U_DHD0_HSPCFG14        DHD0_HSPCFG14                     ; /* 0xc138 */
    U_DHD0_HSPCFG15        DHD0_HSPCFG15                     ; /* 0xc13c */
    unsigned int           reserved_92[944]                     ; /* 0xc140~0xcffc */
    U_INTF_CTRL            INTF_CTRL                         ; /* 0xd000 */
    U_INTF_UPD             INTF_UPD                          ; /* 0xd004 */
    U_INTF_SYNC_INV        INTF_SYNC_INV                     ; /* 0xd008 */
    unsigned int           reserved_93                       ; /* 0xd00c */
    U_INTF_CLIP0_L         INTF_CLIP0_L                      ; /* 0xd010 */
    U_INTF_CLIP0_H         INTF_CLIP0_H                      ; /* 0xd014 */
    unsigned int           reserved_94[2]                    ; /* 0xd018~0xd01c */
    U_INTF_CSC_IDC         INTF_CSC_IDC                      ; /* 0xd020 */
    U_INTF_CSC_ODC         INTF_CSC_ODC                      ; /* 0xd024 */
    U_INTF_CSC_IODC        INTF_CSC_IODC                     ; /* 0xd028 */
    U_INTF_CSC_P0          INTF_CSC_P0                       ; /* 0xd02c */
    U_INTF_CSC_P1          INTF_CSC_P1                       ; /* 0xd030 */
    U_INTF_CSC_P2          INTF_CSC_P2                       ; /* 0xd034 */
    U_INTF_CSC_P3          INTF_CSC_P3                       ; /* 0xd038 */
    U_INTF_CSC_P4          INTF_CSC_P4                       ; /* 0xd03c */
    U_INTF_HSPCFG0         INTF_HSPCFG0                      ; /* 0xd040 */
    U_INTF_HSPCFG1         INTF_HSPCFG1                      ; /* 0xd044 */
    unsigned int           reserved_95[3]                    ; /* 0xd048~0xd050 */
    U_INTF_HSPCFG5         INTF_HSPCFG5                      ; /* 0xd054 */
    U_INTF_HSPCFG6         INTF_HSPCFG6                      ; /* 0xd058 */
    U_INTF_HSPCFG7         INTF_HSPCFG7                      ; /* 0xd05c */
    U_INTF_HSPCFG8         INTF_HSPCFG8                      ; /* 0xd060 */
    unsigned int           reserved_96[3]                    ; /* 0xd064~0xd06c */
    U_INTF_HSPCFG12        INTF_HSPCFG12                     ; /* 0xd070 */
    U_INTF_HSPCFG13        INTF_HSPCFG13                     ; /* 0xd074 */
    U_INTF_HSPCFG14        INTF_HSPCFG14                     ; /* 0xd078 */
    U_INTF_HSPCFG15        INTF_HSPCFG15                     ; /* 0xd07c */
    U_INTF_DITHER0_CTRL    INTF_DITHER0_CTRL                 ; /* 0xd080 */
    U_INTF_DITHER0_COEF0   INTF_DITHER0_COEF0                ; /* 0xd084 */
    U_INTF_DITHER0_COEF1   INTF_DITHER0_COEF1                ; /* 0xd088 */
    unsigned int           reserved_97[21]                   ; /* 0xd08c~0xd0dc */
    U_INTF_CHKSUM_Y_H      INTF_CHKSUM_Y_H                   ; /* 0xd0e0 */
    U_INTF_CHKSUM_Y_L      INTF_CHKSUM_Y_L                   ; /* 0xd0e4 */
    U_INTF_CHKSUM_U_H      INTF_CHKSUM_U_H                   ; /* 0xd0e8 */
    U_INTF_CHKSUM_U_L      INTF_CHKSUM_U_L                   ; /* 0xd0ec */
    U_INTF_CHKSUM_V_H      INTF_CHKSUM_V_H                   ; /* 0xd0f0 */
    U_INTF_CHKSUM_V_L      INTF_CHKSUM_V_L                   ; /* 0xd0f4 */
    unsigned int           reserved_98[1986]                 ; /* 0xd0f8~0xeffc */
    U_HDATE_VERSION        HDATE_VERSION                     ; /* 0xf000 */
    U_HDATE_EN             HDATE_EN                          ; /* 0xf004 */
    U_HDATE_POLA_CTRL      HDATE_POLA_CTRL                   ; /* 0xf008 */
    U_HDATE_VIDEO_FORMAT   HDATE_VIDEO_FORMAT                ; /* 0xf00c */
    U_HDATE_STATE          HDATE_STATE                       ; /* 0xf010 */
    U_HDATE_OUT_CTRL       HDATE_OUT_CTRL                    ; /* 0xf014 */
    U_HDATE_SRC_13_COEF1   HDATE_SRC_13_COEF1                ; /* 0xf018 */
    U_HDATE_SRC_13_COEF2   HDATE_SRC_13_COEF2                ; /* 0xf01c */
    U_HDATE_SRC_13_COEF3   HDATE_SRC_13_COEF3                ; /* 0xf020 */
    U_HDATE_SRC_13_COEF4   HDATE_SRC_13_COEF4                ; /* 0xf024 */
    U_HDATE_SRC_13_COEF5   HDATE_SRC_13_COEF5                ; /* 0xf028 */
    U_HDATE_SRC_13_COEF6   HDATE_SRC_13_COEF6                ; /* 0xf02c */
    U_HDATE_SRC_13_COEF7   HDATE_SRC_13_COEF7                ; /* 0xf030 */
    U_HDATE_SRC_13_COEF8   HDATE_SRC_13_COEF8                ; /* 0xf034 */
    U_HDATE_SRC_13_COEF9   HDATE_SRC_13_COEF9                ; /* 0xf038 */
    U_HDATE_SRC_13_COEF10   HDATE_SRC_13_COEF10              ; /* 0xf03c */
    U_HDATE_SRC_13_COEF11   HDATE_SRC_13_COEF11              ; /* 0xf040 */
    U_HDATE_SRC_13_COEF12   HDATE_SRC_13_COEF12              ; /* 0xf044 */
    U_HDATE_SRC_13_COEF13   HDATE_SRC_13_COEF13              ; /* 0xf048 */
    U_HDATE_SRC_24_COEF1   HDATE_SRC_24_COEF1                ; /* 0xf04c */
    U_HDATE_SRC_24_COEF2   HDATE_SRC_24_COEF2                ; /* 0xf050 */
    U_HDATE_SRC_24_COEF3   HDATE_SRC_24_COEF3                ; /* 0xf054 */
    U_HDATE_SRC_24_COEF4   HDATE_SRC_24_COEF4                ; /* 0xf058 */
    U_HDATE_SRC_24_COEF5   HDATE_SRC_24_COEF5                ; /* 0xf05c */
    U_HDATE_SRC_24_COEF6   HDATE_SRC_24_COEF6                ; /* 0xf060 */
    U_HDATE_SRC_24_COEF7   HDATE_SRC_24_COEF7                ; /* 0xf064 */
    U_HDATE_SRC_24_COEF8   HDATE_SRC_24_COEF8                ; /* 0xf068 */
    U_HDATE_SRC_24_COEF9   HDATE_SRC_24_COEF9                ; /* 0xf06c */
    U_HDATE_SRC_24_COEF10   HDATE_SRC_24_COEF10              ; /* 0xf070 */
    U_HDATE_SRC_24_COEF11   HDATE_SRC_24_COEF11              ; /* 0xf074 */
    U_HDATE_SRC_24_COEF12   HDATE_SRC_24_COEF12              ; /* 0xf078 */
    U_HDATE_SRC_24_COEF13   HDATE_SRC_24_COEF13              ; /* 0xf07c */
    U_HDATE_CSC_COEF1      HDATE_CSC_COEF1                   ; /* 0xf080 */
    U_HDATE_CSC_COEF2      HDATE_CSC_COEF2                   ; /* 0xf084 */
    U_HDATE_CSC_COEF3      HDATE_CSC_COEF3                   ; /* 0xf088 */
    U_HDATE_CSC_COEF4      HDATE_CSC_COEF4                   ; /* 0xf08c */
    U_HDATE_CSC_COEF5      HDATE_CSC_COEF5                   ; /* 0xf090 */
    unsigned int           reserved_99[3]                    ; /* 0xf094~0xf09c */
    U_HDATE_TEST           HDATE_TEST                        ; /* 0xf0a0 */
    U_HDATE_VBI_CTRL       HDATE_VBI_CTRL                    ; /* 0xf0a4 */
    U_HDATE_CGMSA_DATA     HDATE_CGMSA_DATA                  ; /* 0xf0a8 */
    U_HDATE_CGMSB_H        HDATE_CGMSB_H                     ; /* 0xf0ac */
    U_HDATE_CGMSB_DATA1    HDATE_CGMSB_DATA1                 ; /* 0xf0b0 */
    U_HDATE_CGMSB_DATA2    HDATE_CGMSB_DATA2                 ; /* 0xf0b4 */
    U_HDATE_CGMSB_DATA3    HDATE_CGMSB_DATA3                 ; /* 0xf0b8 */
    U_HDATE_CGMSB_DATA4    HDATE_CGMSB_DATA4                 ; /* 0xf0bc */
    U_HDATE_DACDET1        HDATE_DACDET1                     ; /* 0xf0c0 */
    U_HDATE_DACDET2        HDATE_DACDET2                     ; /* 0xf0c4 */
    U_HDATE_SRC_13_COEF14   HDATE_SRC_13_COEF14              ; /* 0xf0c8 */
    U_HDATE_SRC_13_COEF15   HDATE_SRC_13_COEF15              ; /* 0xf0cc */
    U_HDATE_SRC_13_COEF16   HDATE_SRC_13_COEF16              ; /* 0xf0d0 */
    U_HDATE_SRC_13_COEF17   HDATE_SRC_13_COEF17              ; /* 0xf0d4 */
    U_HDATE_SRC_13_COEF18   HDATE_SRC_13_COEF18              ; /* 0xf0d8 */
    U_HDATE_SRC_24_COEF14   HDATE_SRC_24_COEF14              ; /* 0xf0dc */
    U_HDATE_SRC_24_COEF15   HDATE_SRC_24_COEF15              ; /* 0xf0e0 */
    U_HDATE_SRC_24_COEF16   HDATE_SRC_24_COEF16              ; /* 0xf0e4 */
    U_HDATE_SRC_24_COEF17   HDATE_SRC_24_COEF17              ; /* 0xf0e8 */
    U_HDATE_SRC_24_COEF18   HDATE_SRC_24_COEF18              ; /* 0xf0ec */
    U_HDATE_CLIP           HDATE_CLIP                        ; /* 0xf0f0 */
    unsigned int           reserved_100[67]                  ; /* 0xf0f4~0xf1fc */
    U_DATE_COEFF0          DATE_COEFF0                       ; /* 0xf200 */
    U_DATE_COEFF1          DATE_COEFF1                       ; /* 0xf204 */
    U_DATE_COEFF2          DATE_COEFF2                       ; /* 0xf208 */
    U_DATE_COEFF3          DATE_COEFF3                       ; /* 0xf20c */
    U_DATE_COEFF4          DATE_COEFF4                       ; /* 0xf210 */
    U_DATE_COEFF5          DATE_COEFF5                       ; /* 0xf214 */
    U_DATE_COEFF6          DATE_COEFF6                       ; /* 0xf218 */
    U_DATE_COEFF7          DATE_COEFF7                       ; /* 0xf21c */
    U_DATE_COEFF8          DATE_COEFF8                       ; /* 0xf220 */
    U_DATE_COEFF9          DATE_COEFF9                       ; /* 0xf224 */
    U_DATE_COEFF10         DATE_COEFF10                      ; /* 0xf228 */
    U_DATE_COEFF11         DATE_COEFF11                      ; /* 0xf22c */
    U_DATE_COEFF12         DATE_COEFF12                      ; /* 0xf230 */
    U_DATE_COEFF13         DATE_COEFF13                      ; /* 0xf234 */
    U_DATE_COEFF14         DATE_COEFF14                      ; /* 0xf238 */
    U_DATE_COEFF15         DATE_COEFF15                      ; /* 0xf23c */
    U_DATE_COEFF16         DATE_COEFF16                      ; /* 0xf240 */
    U_DATE_COEFF17         DATE_COEFF17                      ; /* 0xf244 */
    U_DATE_COEFF18         DATE_COEFF18                      ; /* 0xf248 */
    U_DATE_COEFF19         DATE_COEFF19                      ; /* 0xf24c */
    U_DATE_COEFF20         DATE_COEFF20                      ; /* 0xf250 */
    U_DATE_COEFF21         DATE_COEFF21                      ; /* 0xf254 */
    U_DATE_COEFF22         DATE_COEFF22                      ; /* 0xf258 */
    U_DATE_COEFF23         DATE_COEFF23                      ; /* 0xf25c */
    U_DATE_COEFF24         DATE_COEFF24                      ; /* 0xf260 */
    U_DATE_COEFF25         DATE_COEFF25                      ; /* 0xf264 */
    U_DATE_COEFF26         DATE_COEFF26                      ; /* 0xf268 */
    U_DATE_COEFF27         DATE_COEFF27                      ; /* 0xf26c */
    U_DATE_COEFF28         DATE_COEFF28                      ; /* 0xf270 */
    U_DATE_COEFF29         DATE_COEFF29                      ; /* 0xf274 */
    U_DATE_COEFF30         DATE_COEFF30                      ; /* 0xf278 */
    unsigned int           reserved_101                     ; /* 0xf27c */
    U_DATE_ISRMASK         DATE_ISRMASK                      ; /* 0xf280 */
    U_DATE_ISRSTATE        DATE_ISRSTATE                     ; /* 0xf284 */
    U_DATE_ISR             DATE_ISR                          ; /* 0xf288 */
    unsigned int           reserved_102                     ; /* 0xf28c */
    U_DATE_VERSION         DATE_VERSION                      ; /* 0xf290 */
    U_DATE_COEFF37         DATE_COEFF37                      ; /* 0xf294 */
    U_DATE_COEFF38         DATE_COEFF38                      ; /* 0xf298 */
    U_DATE_COEFF39         DATE_COEFF39                      ; /* 0xf29c */
    U_DATE_COEFF40         DATE_COEFF40                      ; /* 0xf2a0 */
    U_DATE_COEFF41         DATE_COEFF41                      ; /* 0xf2a4 */
    U_DATE_COEFF42         DATE_COEFF42                      ; /* 0xf2a8 */
    unsigned int           reserved_103[5]                     ; /* 0xf2ac~0xf2bc */
    U_DATE_DACDET1         DATE_DACDET1                      ; /* 0xf2c0 */
    U_DATE_DACDET2         DATE_DACDET2                      ; /* 0xf2c4 */
    U_DATE_COEFF50         DATE_COEFF50                      ; /* 0xf2c8 */
    U_DATE_COEFF51         DATE_COEFF51                      ; /* 0xf2cc */
    U_DATE_COEFF52         DATE_COEFF52                      ; /* 0xf2d0 */
    U_DATE_COEFF53         DATE_COEFF53                      ; /* 0xf2d4 */
    U_DATE_COEFF54         DATE_COEFF54                      ; /* 0xf2d8 */
    U_DATE_COEFF55         DATE_COEFF55                      ; /* 0xf2dc */
    U_DATE_COEFF56         DATE_COEFF56                      ; /* 0xf2e0 */
    U_DATE_COEFF57         DATE_COEFF57                      ; /* 0xf2e4 */
    U_DATE_COEFF58         DATE_COEFF58                      ; /* 0xf2e8 */
    U_DATE_COEFF59         DATE_COEFF59                      ; /* 0xf2ec */
    U_DATE_COEFF60         DATE_COEFF60                      ; /* 0xf2f0 */
    U_DATE_COEFF61         DATE_COEFF61                      ; /* 0xf2f4 */
    U_DATE_COEFF62         DATE_COEFF62                      ; /* 0xf2f8 */
    U_DATE_COEFF63         DATE_COEFF63                      ; /* 0xf2fc */
    U_DATE_COEFF64         DATE_COEFF64                      ; /* 0xf300 */
    U_DATE_COEFF65         DATE_COEFF65                      ; /* 0xf304 */
    U_DATE_COEFF66         DATE_COEFF66                      ; /* 0xf308 */
    U_DATE_COEFF67         DATE_COEFF67                      ; /* 0xf30c */
    U_DATE_COEFF68         DATE_COEFF68                      ; /* 0xf310 */
    U_DATE_COEFF69         DATE_COEFF69                      ; /* 0xf314 */
    U_DATE_COEFF70         DATE_COEFF70                      ; /* 0xf318 */
    U_DATE_COEFF71         DATE_COEFF71                      ; /* 0xf31c */
    U_DATE_COEFF72         DATE_COEFF72                      ; /* 0xf320 */
    U_DATE_COEFF73         DATE_COEFF73                      ; /* 0xf324 */
    U_DATE_COEFF74         DATE_COEFF74                      ; /* 0xf328 */
    U_DATE_COEFF75         DATE_COEFF75                      ; /* 0xf32c */
    U_DATE_COEFF76         DATE_COEFF76                      ; /* 0xf330 */
    U_DATE_COEFF77         DATE_COEFF77                      ; /* 0xf334 */
    U_DATE_COEFF78         DATE_COEFF78                      ; /* 0xf338 */
    U_DATE_COEFF79         DATE_COEFF79                      ; /* 0xf33c */
    U_DATE_COEFF80         DATE_COEFF80                      ; /* 0xf340 */
    U_DATE_COEFF81         DATE_COEFF81                      ; /* 0xf344 */
    U_DATE_COEFF82         DATE_COEFF82                      ; /* 0xf348 */
    U_DATE_COEFF83         DATE_COEFF83                      ; /* 0xf34c */
    U_DATE_COEFF84         DATE_COEFF84                      ; /* 0xf350 */

} VOU_REGS_S;

/* Declare the struct pointor of the module VDP */
#ifdef __cplusplus
#if __cplusplus
	extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __VOU_REG_H__ */
