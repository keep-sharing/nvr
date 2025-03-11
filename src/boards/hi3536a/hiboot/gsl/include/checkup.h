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
void failure_process(void);

uint32_t set_key_slot(uint32_t slot);
uint32_t unset_key_slot(void);

uint32_t get_gsl_code_area_len();
uint32_t get_boot_image_int_ddr_addr();
uint32_t get_boot_image_total_len();
uint32_t get_bootloader_unchecked_area_len();
uint32_t get_ddr_decrypt_bootloader_addr();
int handle_bootloader_key_area(void);
int handle_bootloader_params_area(void);
int handle_bootloader_code_area(void);

void disable_sec_ddr_bypass(void);
void config_tee_img_non_sec_ddr(void);
void config_key_area_sec_ddr(void);
void config_tee_img_sec_ddr(void);
void config_non_sec_ddr(void);

int handle_tee_key_area(void);
int handle_atf_area(void);
int handle_tee_code_area(void);
