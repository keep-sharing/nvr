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
#ifndef __KEY_CTRL_REG_KS_H__
#define __KEY_CTRL_REG_KS_H__
/* define the union u_kc_teecpu_lock_cmd */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    tee_key_slot_num      : 10; /* [9..0]  */
		unsigned int    reserved_0            : 10; /* [19..10]  */
		unsigned int    tee_lock_cmd          : 1; /* [20]  */
		unsigned int    reserved_1            : 11; /* [31..21]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_tee_lock_cmd;

/* define the union kc_ree_lock_cmd */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    ree_key_slot_num      : 10; /* [9..0]  */
		unsigned int    reserved_0            : 10; /* [19..10]  */
		unsigned int    ree_lock_cmd          : 1; /* [20]  */
		unsigned int    reserved_1            : 11; /* [31..21]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_ree_lock_cmd;

/* define the union kc_tee_flush_busy */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    tee_flush_busy     : 1; /* [0]  */
		unsigned int    tee_flush_fail     : 1; /* [1]  */
		unsigned int    reserved_0         : 30; /* [31..2]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_tee_flush_busy;

/* define the union kc_ree_flush_busy */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    ree_flush_busy     : 1; /* [0]  */
		unsigned int    ree_flush_fail     : 1; /* [1]  */
		unsigned int    reserved_0         : 30; /* [31..2]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_ree_flush_busy;

/* define the union kc_send_dbg */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    reserved_0            : 4; /* [3..0]  */
		unsigned int    kc_cur_st             : 4; /* [7..4]  */
		unsigned int    reserved_1            : 24; /* [31..8]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_send_dbg;

/* define the union kc_rob_alarm */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    kc_rob_alarm          : 4; /* [3..0]  */
		unsigned int    reserved_0            : 28; /* [31..4]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_rob_alarm;

/* define the union kc_rd_slot_num */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    slot_num_cfg          : 10; /* [9..0]  */
		unsigned int    reserved_0            : 6; /* [15..10]  */
		unsigned int    tscipher_slot_ind     : 1; /* [16]  */
		unsigned int    reserved_1            : 15; /* [31..17]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_rd_slot_num;

/* define the union kc_rd_lock_status */
typedef union {
	/* define the struct bits */
	struct {
		unsigned int    rd_lock_status    : 3; /* [2..0]  */
		unsigned int    reserved_0        : 29; /* [31..3]  */
	} bits;

	/* define an unsigned member */
	unsigned int    u32;
} kc_rd_lock_status;

#endif /* __KEY_CTRL_REG_KS_H__ */