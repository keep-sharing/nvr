/*
 *
 * Copyright (c) 2020-2021 Shenshu Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _RKP_COM_REG_H__
#define _RKP_COM_REG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define RKP_BASE                    0x10111000

#define RKP_SEC_CFG                 0x074
#define RKP_SW_REG            	    0x078
#define RKP_CALC_CTRL               0x07c
#define RKP_ERROR                   0x080

/* define the union u_rkp_module_id_0 */
typedef union {
	/* define an unsigned member */
	rkp_module_id_0_tpp tpp;
	rkp_module_id_0_tee tee;
	rkp_module_id_0_ree ree;
	rkp_module_id_0_hpp hpp;
	unsigned int u32;
} rkp_module_id_0;

/* define the union rkp_module_id_1 */
typedef union {
	/* define an unsigned member */
	rkp_module_id_1_tpp tpp;
	rkp_module_id_1_tee tee;
	rkp_module_id_1_ree ree;
	rkp_module_id_1_hpp hpp;
	unsigned int u32;
} rkp_module_id_1;

/* define the union rkp_module_id_2 */
typedef union {
	/* define an unsigned member */
	rkp_module_id_2_tpp tpp;
	rkp_module_id_2_tee tee;
	rkp_module_id_2_ree ree;
	rkp_module_id_2_hpp hpp;
	unsigned int u32;
} rkp_module_id_2;

typedef union {
	unsigned int u32;
} rkp_module_id_3;

/* define the union rkp_rk_info_s */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    ca_vendor_id             : 8; /* [7..0]  */
		unsigned int    csa2_check               : 1; /* [8]  */
		unsigned int    reserved_1               : 23; /* [31..9]  */
	} bits;
	/* define an unsigned member */
	unsigned int    u32;
} rkp_rk_info;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* __drv_rkp_h__ */

