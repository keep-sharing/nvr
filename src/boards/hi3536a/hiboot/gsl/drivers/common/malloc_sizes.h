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
/***********************************************************************************
* [bootRam Space] start: 0x04020000, end: 0x0403a000, len: 0x1a000(104k).
* [bss + stack]   start: 0x04020000, end: 0x04020a00, len: 0xa00.
* [ddr init or key area] start: 0x04020a00, end: 0x04034a00, len: 0x14000.
* [malloc] start: 0x04030a00, end: 0x04038800, len: 0x3e00.
***********************************************************************************/
/* [ddr init or key area] start: 0x04020a00, end: 0x04034a00, len: 0x14000. */
cache(0xa000,   2)	/* ddr init or key area reserved */

/* [malloc] start: 0x04034a00, end: 0x04038800 , len: 0x3e00. */
cache(0x100,	5)	/* rsa pss */
cache(0x200,	5)	/* rsa decrypt buf for n=4096 */
cache(0x400,	3)	/* rsa_exp_mod (RSA_KEY_LEN_4096*2) */
cache(0xb00,    1)  	/* reserved */
cache(0xc00,    2)  	/* reserved */
/**********************************************************************************/
/**********************************************************************************/
