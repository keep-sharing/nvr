/*
*  Copyright (c) Shenshu Technologies Co., Ltd. 2021. All rights reserved.
*  Description :  OTP burning sample
*/
#include <../product/security_subsys/otp/include/ot_common_otp.h>
#include <../product/security_subsys/otp/include/ot_mpi_otp.h>
#include <common.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <securec.h>

#define array_size(x)	   (sizeof(x) / sizeof(x[0]))
#define OTP_LOCK TD_TRUE
#define INVALID_LOCK TD_FALSE  /* The lock enable bit does not lock the bit field of this attribute. */
#define OTP_UNLOCK TD_FALSE
#define USER_LEN 1024
#define DEBUG 1

typedef enum {
	NONE = 0,
	KEY = 1,
	FLAG = 2,
	VALUE = 3,
} otp_field_type;

typedef struct {
	td_char *field_name;
	otp_field_type type;
	td_u32 n_bits;
	td_bool lock_status;
	td_char *val_str;
} opt_burn_field;

typedef struct {
	const char *field_name;
	unsigned int offset;
	unsigned int length;
	td_bool lock_status;
	unsigned char data[USER_LEN];
} user_data_s;

#define OTP_INVAL_DATA 0XFFFF
#define BIT_8 8
#define BIT_4 4
#define CHAR_2 2

/*
 * Notice:
 * 	1. Lock all OTPs before the commercial use of mass production.
 * 	2. Lock status is invalid for items with "INVALID_LOCK".
 */

opt_burn_field g_otp_startup_burn_fields[] = {
	/* field_name			type	n_bits	lock_status 	val_str(requires 0x) */
// 	{"quick_boot",			FLAG,	4,	OTP_LOCK,	""},
// 	{"secure_boot_en",		FLAG,	8,	OTP_LOCK,	""},
// 	{"soc_tee_enable",		FLAG,	8,	OTP_LOCK,	""},
// 	{"gsl_dec_en",			FLAG,	4,	OTP_LOCK,	""},
// 	{"bload_dec_en",		FLAG,	1,	OTP_LOCK,	""},
// 	{"tee_owner_sel",		FLAG,	1,	OTP_LOCK,	""},
// 	{"uboot_redundance",		FLAG,	1,	OTP_LOCK,	""},
// 	{"oem_rk_deob_en",		FLAG,	1,	OTP_LOCK,	""},
// 	{"oem_root_public_key_sha256",	KEY,	256,	INVALID_LOCK,	""},
// 	{"oem_root_symc_key0",		KEY,	128,	INVALID_LOCK,	""},
// 	{"oem_root_symc_key0_flag",	FLAG,	32,	OTP_LOCK,	""},
// 	{"oem_msid",			FLAG,	32,	OTP_LOCK,	""},
// 	{"oem_version",			FLAG,	32,	INVALID_LOCK,	""},
// 	{"tee_msid",			FLAG,	32,	OTP_LOCK,	""},
//	{"tee_sec_version", 		FLAG,	128,	INVALID_LOCK,	""},
// 	{"double_sign_en",		FLAG,	4,	OTP_LOCK,	""},
// 	{"tp_root_public_key_sha256",	KEY,	256,	INVALID_LOCK,	""},
// 	{"third_party_msid",		FLAG,	32,	OTP_LOCK,	""},
// 	{"third_party_version",		FLAG,	32,	INVALID_LOCK,	""},
};

opt_burn_field g_otp_function_burn_fields[] = {
	/* field_name			type	n_bits	lock_status 	val_str(requires 0x) */
// 	{"oem_cw_crc_rd_disable",	FLAG,	8,	OTP_LOCK,	""},
//	{"oem_root_symc_key1",		KEY,	128,	INVALID_LOCK,	""},
//	{"oem_root_symc_key2",		KEY,	128,	INVALID_LOCK,	""},
//	{"oem_root_symc_key3",		KEY,	128,	INVALID_LOCK,	""},
// 	{"oem_root_symc_key1_flag",	FLAG,	32,	OTP_LOCK,	""},
// 	{"oem_root_symc_key2_flag",	FLAG,	32,	OTP_LOCK,	""},
// 	{"oem_root_symc_key3_flag",	FLAG,	32,	OTP_LOCK,	""},
// 	{"func_jtag_prt_mode",		FLAG,	8,	OTP_LOCK,	""},
// 	{"soc_jtag_prt_mode",		FLAG,	8,	OTP_LOCK,	""},
// 	{"jtag_key_sel0",		FLAG,	1,	OTP_LOCK,	""},
// 	{"jtag_key_sel1",		FLAG,	1,	OTP_LOCK,	""},
// 	{"sec_ds_enable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"acpu_ds_enable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"otp_pcie_disable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"otp_pcie_ep_boot_disable",	FLAG,	1,	OTP_LOCK,	""},
// 	{"update_from_uart_disable",	FLAG,	1,	OTP_LOCK,	""},
// 	{"update_from_sdio_disable",	FLAG,	1,	OTP_LOCK,	""},
// 	{"update_from_usbdev_disable",	FLAG,	1,	OTP_LOCK,	""},
// 	{"scs_dbg_disable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"uart0_disable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"uart1_disable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"uart2_disable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"uart3_disable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"uart4_disable",		FLAG,	1,	OTP_LOCK,	""},
// 	{"uart5_disable",		FLAG,	1,	OTP_LOCK,	""},
};

user_data_s g_wr_data[] = {
	/* field name,		offset_byte	length_byte	lock_status	data(requires 0x) */
// 	{"ree_user_data", 	0,		32,		OTP_LOCK,	""},
//	{"tee_user_data", 	0,		32,		OTP_LOCK,	""},
};

void print_str(const char *name, unsigned char *msg, int msg_len)
{
	unsigned int  i;

	if (msg_len == 0)
		return;

	if (name != NULL)
		printf("%s: \n", name);

	for (i = 0; i < msg_len; i++) {
		if (i != 0 && (i & 0x0f) == 0)
			printf("\n");
		printf("%02X ", msg[i]);
	}
	printf("\n");
}

unsigned char hextochar(unsigned char bchar)
{
	if (bchar >=0x30 && bchar <= 0x39)  /* ascii : '0' :0x30   '9' :0x30 */
		bchar -= 0x30;   /* ascii : '0' :0x30 */
	else if (bchar >= 0x41 && bchar <=0x46) /* ascii : 'A' :0x41   'F' :0x46 */
		bchar -= 0x37;  /* 0x37 : Converted to a hexadecimal number */
	else if (bchar >= 0x61 && bchar <=0x66) /* ascii : 'a' :0x61   'f' :0x66 */
		bchar -= 0x57;   /* 0x57 : Converted to a hexadecimal number */
	else
		bchar = 0xff;   /* 0xff : Invalid val */
	return bchar;
}

unsigned int check_arg(char *from_val)
{
	if (from_val[1] != 'x' && from_val[1] != 'X') {
		printf("Invalid key! \n");
		return OTP_INVAL_DATA;
	}

	return 0;
}
unsigned int padding_key_buf(td_u8 *to_buf, unsigned int bit_width, char *from_val)
{
	int i = 0;
	int j = 0;
	int count = 0;
	int length = 0;

	count = check_arg(from_val);
	if (count == OTP_INVAL_DATA)
		return TD_FAILURE;

	from_val +=CHAR_2 ;

	length = strlen(from_val);
	if (bit_width / BIT_8  > length / CHAR_2) {   /* 8: Byte conversion   2: */
		for (i = 0; i < (bit_width/BIT_4 - length)/CHAR_2; i++) {
			to_buf[i] = 0x00;
		}
		if ((bit_width/BIT_4 - length) % CHAR_2 != 0) {
			to_buf[i] = from_val[0] -'0';
			from_val ++;
			i ++;
		}
	}

	for (j = 0;  j < strlen(from_val) / CHAR_2 ;  j++) {
		to_buf[i] = (hextochar(from_val[j * CHAR_2 ]) << BIT_4) | (0x0f & (hextochar(from_val[j * CHAR_2 + 1])));
		i ++;
	}

	return TD_SUCCESS;
}

unsigned int padding_pv_buf(td_u8 *to_buf, unsigned int bit_width, char *from_val)
{
	int i = 0;
	int j = 0;
	int count = 0;
	int lrngth = 0;

	count = check_arg(from_val);
	if (count == OTP_INVAL_DATA)
		return TD_FAILURE;

	from_val +=CHAR_2 ;

	if (bit_width ==  1) {
		if (strlen(from_val)%CHAR_2  == 1)
			to_buf[0] = (hextochar(from_val[0]) & 0x01);
		else if (strlen(from_val)%CHAR_2  == 0)
			to_buf[0] = (hextochar(from_val[1]) & 0x01);
	} else if (bit_width == BIT_4) {
		if (strlen(from_val)%CHAR_2  == 1)
			to_buf[0] = (hextochar(from_val[0]) & 0x0f);
		else if (strlen(from_val)%CHAR_2  == 0)
			to_buf[0] = (hextochar(from_val[1]) & 0x0f);
	} else if (bit_width >=  BIT_8) {
		if (bit_width/BIT_8 > strlen(from_val)/CHAR_2) {
			length = strlen(from_val);
			for (i = 0; i < (bit_width/BIT_4 - length)/CHAR_2; i++)
				to_buf[(bit_width/BIT_8 -1) - i] = 0x00;

			if ((bit_width/BIT_4 - length) % CHAR_2 != 0) {
				to_buf[(bit_width/BIT_8 -1) - i] = from_val[0] -'0';
				from_val ++;
				i ++;
			}
		}

		for (j = 0;  j < strlen(from_val) / CHAR_2;  j++) {
			to_buf[(bit_width/BIT_8 -1) - i] =
				(hextochar(from_val[j * CHAR_2]) << BIT_4) | \
					(0x0f & (hextochar(from_val[j * CHAR_2 + 1])));
			i ++;
		}
	}

	return TD_SUCCESS;
}

unsigned int padding_user_buf(td_u8 *to_buf, unsigned int value_len, char *from_val)
{
	int i = 0;
	int j = 0;
	int count = 0;
	int length = 0;

	count = check_arg(from_val);
	if (count == OTP_INVAL_DATA)
		return TD_FAILURE;

	from_val += CHAR_2;

	lrngth = strlen(from_val);
	if (value_len > length/CHAR_2) {
		for (i = 0; i < (value_len * CHAR_2 - length) / CHAR_2; i++)
			to_buf[(value_len -1) - i] = 0x00;

		if ((value_len*CHAR_2 - length) % CHAR_2 != 0) {
			to_buf[(value_len -1) - i] = from_val[0] -'0';
			from_val ++;
			i ++;
		}
	}

	for (j = 0;  j < strlen(from_val) / CHAR_2;  j++) {
		to_buf[(value_len -1) - i] =
		(hextochar(from_val[j * CHAR_2]) << BIT_4) | (0x0f & (hextochar(from_val[j * CHAR_2 + 1])));
		i ++;
	}

	return TD_SUCCESS;
}

td_s32 sample_burn_single_pv(ot_otp_burn_pv_item *item, td_u32 item_num)
{
	td_s32 ret;
	td_u32 i;
	ot_otp_burn_pv_item *tmp_item = TD_NULL;
	td_bool sta = TD_TRUE;

	for (i = 0; i < item_num; i++) {
		if (strcmp(item[i].field_name, "tee_sec_version") != 0) {
			tmp_item = &item[i];
			printf("\nburn pv %s --->\n", tmp_item->field_name);
			udelay(5000); /* 5000us */
			ret = ot_mpi_otp_burn_product_pv(tmp_item, 1);
			if (ret != TD_SUCCESS) {
				printf("burn %s item failed!\n", tmp_item->field_name);
				sta = TD_FALSE;
			}
		}
	}

	if (sta == TD_TRUE)
		return TD_SUCCESS;
	else
		return TD_FAILURE;
}

td_s32 sample_write_user_data(td_void)
{
	td_s32 ret;
	td_u32 i, dat_num;
	user_data_s *usr_dat = TD_NULL;
	td_bool sta = TD_TRUE;

	dat_num = array_size(g_wr_data);

	for (i = 0; i < dat_num; i++) {
		usr_dat = &g_wr_data[i];
		printf("\nwrite data offset %u, length %u ", usr_dat->offset, usr_dat->length);
		udelay(5000); /* 5000us */
		ret = ot_mpi_otp_set_user_data(usr_dat->field_name, usr_dat->offset, usr_dat->data, usr_dat->length);
		if (ret != TD_SUCCESS) {
			printf("set user data failed\n");
			sta = TD_FALSE;
		}
		if (g_wr_data[i].lock_status == OTP_LOCK) {
			ret = ot_mpi_otp_set_user_data_lock(usr_dat->field_name, usr_dat->offset, usr_dat->length);
			if (ret != TD_SUCCESS) {
				printf("set user data lock status failed\n");
				sta = TD_FALSE;
			}
		}
	}

	if (sta == TD_TRUE) {
		return TD_SUCCESS;
	} else {
		return TD_FAILURE;
	}
}

int otp_val_str_to_value(td_char *val_str, td_u8 *to_buf, td_u32 num, otp_field_type type)
{
	int ret = -1;
	switch (type) {
	case KEY:
		ret = padding_key_buf(to_buf, num, val_str);
		break;
	case FLAG:
		ret = padding_pv_buf(to_buf, num, val_str);
		break;
	case VALUE:
		ret = padding_user_buf(to_buf, num, val_str);
		break;
	default:
		printf("set user data lock status failed\n");
		return ret;
	}

	return ret;
}

int burn_otp_fields(opt_burn_field *fields, size_t count)
{
	int i;
	td_s32 ret;
	ot_otp_burn_pv_item *pv_items = NULL;
	unsigned char tee_sec_version_buff[16] = {0};

	if (count == 0)
		return -1;

	pv_items = (ot_otp_burn_pv_item*)malloc(sizeof(ot_otp_burn_pv_item) * count);
	if (pv_items == NULL)
		return -1;

	memset_s(pv_items, count * sizeof(ot_otp_burn_pv_item), 0, count * sizeof(ot_otp_burn_pv_item));

	for (i = 0; i < count; i++) {
		pv_items[i].burn = TD_TRUE; /* TD_TURE: Burn OTP, TD_FALSE: Read OTP */
		pv_items[i].value_len = fields[i].n_bits;
		pv_items[i].lock = fields[i].lock_status;
		if (memcpy_s(pv_items[i].field_name, OT_OTP_PV_NAME_MAX_LEN,
			fields[i].field_name, strlen(fields[i].field_name)) != EOK)
			goto free;

		if (otp_val_str_to_value(fields[i].val_str, pv_items[i].value,
					fields[i].n_bits, fields[i].type) != 0)
			goto free;
#if DEBUG
		if (pv_items[i].value_len <= BIT_8)
			print_str(pv_items[i].field_name, pv_items[i].value, 1);
		else
			print_str(pv_items[i].field_name, pv_items[i].value, pv_items[i].value_len / BIT_8);
#endif
	}

	sample_burn_single_pv(pv_items, count);

	for (i = 0; i < count; i++) {
		if (strcmp(fields[i].field_name, "tee_sec_version") == 0) {
			if (otp_val_str_to_value(fields[i].val_str, tee_sec_version_buff, fields[i].n_bits, fields[i].type) != 0)
				goto free;
#if DEBUG
			print_str(fields[i].field_name, tee_sec_version_buff, fields[i].n_bits / BIT_8);
#endif
			udelay(5000); /* 5000us */
			ret = ot_mpi_otp_set_user_data(fields[i].field_name, 0, tee_sec_version_buff, fields[i].n_bits / BIT_8);
			if (ret != TD_SUCCESS) {
				printf("set user data failed\n");
				goto free;
			}
		}
	}

	free(pv_items);
	return 0;
free:
	free(pv_items);
	return -1;
}

int burn_otp_user_data(user_data_s *fields, size_t count)
{
	int i;
	char *buf = NULL;

	if (count == 0)
		return -1;

	buf = malloc(sizeof(fields[0].data));
	if (buf == NULL)
		return -1;

	memset_s(buf, sizeof(fields[0].data), 0, sizeof(fields[0].data));

	for (i = 0; i < count; i++) {
		if (memcpy_s(buf, sizeof(fields[0].data), fields[i].data, sizeof(fields[0].data)) != EOK) {
			free(buf);
			return -1;
		}

		if (otp_val_str_to_value(buf, fields[i].data, fields[i].length, VALUE) != 0) {
			free(buf);
			return -1;
		}
#if DEBUG
		print_str(fields[i].field_name, fields[i].data, 32); /* 32: msg_len */
#endif
	}

	sample_write_user_data();

	free(buf);
	return 0;
}

static td_s32 do_write_otp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ot_mpi_otp_init();

	burn_otp_fields(g_otp_startup_burn_fields, array_size(g_otp_startup_burn_fields));
	burn_otp_fields(g_otp_function_burn_fields, array_size(g_otp_function_burn_fields));

	burn_otp_user_data(g_wr_data, array_size(g_wr_data));

	ot_mpi_otp_deinit();

	printf("\n write otp end\n");
	return 0;
}
U_BOOT_CMD (
	write_otp, CONFIG_SYS_MAXARGS, 0, do_write_otp,
	"write otp operation",
	"write otp\n"
);
