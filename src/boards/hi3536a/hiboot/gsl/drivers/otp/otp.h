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
#ifndef __OTP_DRV_H__
#define __OTP_DRV_H__

uint32_t is_tee_enable(void);
uint32_t get_tee_owner(void);
uint32_t get_oem_msid();
uint32_t get_oem_version();
uint32_t is_backup_image_enable(void);
uint32_t is_bload_dec_en_enable(void);
uint32_t is_secure_boot_en_enable(void);
uint32_t is_double_sign_en_enable(void);
uint32_t is_scs_dbg_enable(void);
uint32_t is_boot_info_lv_debug_enable(void);
uint32_t is_func_jtag_enable(void);
uint32_t get_tee_msid(void);
uint32_t get_tee_sec_version(uint32_t mask_ext);

#endif /* __OTP_DRV_H__ */