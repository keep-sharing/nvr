/**
    nvt-opt key manager
    This file will Enable and disable SRAM shutdown
    @file       nvt-otp.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/arch/efuse_protected.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#include "nvt_otp_reg.h"

#ifndef CHKPNT
#define CHKPNT    printf("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printf("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printf("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif

#define OUTREG32(x, y)      writel(y, x)
#define INREG32(x)			readl(x)
#define SETREG32(x, y)      OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x) & ~(y))
extern s32 local_load(u32 uiRowAddress);
extern s32 local_store(u32 uiRowAddress);

#define OTP_120_TIMING              0xFF6050
#define DBL_BIT_EN                  0x3

static const UINT32   core_power_leakage[LEAKAGE_COUNT] = {
	9,          //0.98(V)       //0x0 => 0000
	17,         //0.97(V)       //0x1 => 0001
	26,         //0.96(V)       //0x2 => 0010
	35,         //0.95(V)       //0x3 => 0011

	43,         //0.94(V)       //0x4 => 0100
	52,         //0.93(V)       //0x5 => 0101
	60,         //0.92(V)       //0x6 => 0110
	69,         //0.91(V)       //0x7 => 0111

	//===========all treat as 0.99V==========
	1,          //0.99(V)       //0x8 => 1xxx
	1,          //0.99(V)       //0x9 => 1xxx
	1,          //0.99(V)       //0xa => 1xxx
	1,          //0.99(V)       //0xb => 1xxx

	1,          //0.99(V)       //0xc => 1xxx
	1,          //0.99(V)       //0xd => 1xxx
	1,          //0.99(V)       //0xe => 1xxx
	1,          //0.99(V)       //0xf => 1xxx
};

void otp_init(void)
{
	UINT32  uiReg;

	DBG_DUMP("otp_init[1.00.001]\r\n");
	DBG_DUMP("120MHz\r\n");
	OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, OTP_120_TIMING);

	uiReg = INREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS);
	DBG_DUMP("otp_timing_reg= 0x%x\r\n", (int)uiReg);
#ifdef CONFIG_PWM_NVT
	prepare_power_trim(PWM0_CORE_POWER);
#endif
}

static s32 key_program_bit(u32 rol, u32 col)
{
	return local_store(rol | (col << 5) | (DBL_BIT_EN << 11));
}

static s32 key_write_data(u32 addr, u32 data)
{
	u32  ui_bits;
	u32  ui_data;

	if (addr < 8) {
		DBG_ERR("key_write_data addr = 0x%08x error(need < 8)\r\n", (int)addr);
		return EFUSE_PARAM_ERR;
	}

	ui_data = data;

	ui_bits = 0;
	SETREG32(KEY_MANAGER_CONTROL_ADDRESS, 0x1);
	while ((INREG32(KEY_MANAGER_STATUS_ADDRESS) & 0x1) != 0x1) {};
	while (ui_data) {
		ui_bits = __builtin_ctz(ui_data);
		ui_data &= ~(1 << ui_bits);
		if (key_program_bit(addr, ui_bits) != E_OK) {
			DBG_ERR("%s,eFuse program addr[%02x] bit[%2d] fail\r\n", __func__, (unsigned long)addr, (int)ui_bits);
			return EFUSE_OPS_ERR;
		} else {
			udelay(100);
		}
	}
	CLRREG32(KEY_MANAGER_CONTROL_ADDRESS, 0x1);
	while ((INREG32(KEY_MANAGER_STATUS_ADDRESS) & 0x1) == 0x1) {};

	return EFUSE_SUCCESS;
}

#define KEY_MANAGER_READ_PARAM()        otp_key_manager(8)
#define KEY_MANAGER_WRITE_PARAM(m)      key_write_data(8, m)


/*
     efuse_set_key_destination

     Destination of efuse get key field value

     @return IC revision of specific package revision
        - @b   NT9666X_VER_A    NT9666X version A
        - @b   UNKNOWN_DIE_VER  Unknown IC die version(system must halt)
*/
static ER set_key_destination(OTP_KEY_DESTINATION key_dst, SCE_KEY_SET_TO_OTP_FIELD key_field_set)
{
	if (key_dst >= OTP_KEY_MANAGER_CNT) {
		DBG_ERR("key_dst out of range %d\r\n", key_dst);
		return EFUSE_PARAM_ERR;
	}

	if (key_dst == OTP_KEY_MANAGER_CRYPTO &&  key_field_set >= EFUSE_CRYPTO_ENGINE_KEY_CNT) {
		DBG_ERR("Dest[Crypto]=>key_field_set out of range %d > %d\r\n", key_field_set, EFUSE_CRYPTO_ENGINE_KEY_CNT);
		return EFUSE_PARAM_ERR;
	}

	if ((key_dst == OTP_KEY_MANAGER_RSA || key_dst == OTP_KEY_MANAGER_HASH) &&  key_field_set >= EFUSE_TOTAL_KEY_SET_FIELD) {
		DBG_ERR("Dest[RSA] or [HASH] =>key_field_set out of range %d > %d\r\n", key_field_set, EFUSE_CRYPTO_ENGINE_KEY_CNT);
		return EFUSE_PARAM_ERR;
	}

	OUTREG32(KEY_MANAGER_DESTINATION_ADDRESS, key_dst | (key_field_set<<4));

	//OUTREG32(KEY_MANAGER_KEY_INDEX_ADDRESS, key_field_set);

	return E_OK;
}


/*
     efuse_get_unique_id

     efuse get unique id (56bits)

     @param[out]     id_L   LSB of ID (32bit)
     @param[out]     id_H   MSB of ID (17bit)

     @return IC success or fail
        - @b   	EFUSE_SUCCESS      	success
        - @b	EFUSE_OPS_ERR		fail
*/
ER efuse_get_unique_id(UINT32 *id_L, UINT32 *id_H)
{
	UINT64  value;
	u32		xy;
	UINT64  dec6 = 0x81BF1000ul;
	UINT64  dec5 = 0x39AA400ul;
	union T_EFUSE_LOT_ID1_REG lot_id_1_reg;
	union T_EFUSE_LOT_ID2_REG lot_id_2_reg;

    lot_id_1_reg.Reg = otp_key_manager(EFUSE_LOT_ID1);
    lot_id_2_reg.Reg = otp_key_manager(EFUSE_LOT_ID2);

#if 0
	lot_id_1_reg.Bit.lot_id_no_1_5_0 = 36;
	lot_id_1_reg.Bit.lot_id_no_2_5_0 = 36;
	lot_id_1_reg.Bit.lot_id_no_3_5_0 = 36;
	lot_id_1_reg.Bit.lot_id_no_4_5_0 = 36;
	lot_id_1_reg.Bit.lot_id_no_5_5_0 = 36;
	lot_id_2_reg.Bit.wafer_id = 26;
#endif
	//catch 36bit
	value  = (INT64)(lot_id_1_reg.Bit.lot_id_no_1_5_0);
	DBG_IND("Unique ID value=[0x%llx]\r\n", value);
	value += (INT64)(lot_id_1_reg.Bit.lot_id_no_2_5_0 * 36);
	DBG_IND("Unique ID value=[0x%llx]\r\n", value);
	value += (INT64)(lot_id_1_reg.Bit.lot_id_no_3_5_0 * 1296);
	DBG_IND("Unique ID value=[0x%llx]\r\n", value);
	value += (INT64)(lot_id_1_reg.Bit.lot_id_no_4_5_0 * 46656);
	DBG_IND("Unique ID value=[0x%llx]\r\n", value);
	value += (INT64)(lot_id_1_reg.Bit.lot_id_no_5_5_0 * 1679616);
	DBG_IND("Unique ID value=[0x%llx]\r\n", value);

	value += (UINT64)(((lot_id_2_reg.Bit.lot_id_no_6_5_2 << 2) | lot_id_1_reg.Bit.lot_id_no_6_1_0) * dec5);
	value += (INT64)(lot_id_2_reg.Bit.wafer_id * dec6);

	xy  = lot_id_2_reg.Bit.x_coordinate;
	xy += (lot_id_2_reg.Bit.y_coordinate * 80);

	* id_L = (u32)value;
	* id_H = ((((value >> 32) & 0xF) | (xy << 4)) & 0x1FFFF);

	if ((*id_L + *id_H) == 0) {
		return EFUSE_OPS_ERR;
	} else {
		return EFUSE_SUCCESS;
	}
}


/**
    otp_write_key

    Write specific key into specific key set (0~3)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~3)
    @param[in] ucKey           key (16bytes)
    @return Description of data returned.
        - @b E_OK:   Success
        - @b E_SYS:  Fail
*/
s32 otp_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key)
{
	s32     result = EFUSE_SUCCESS;
	u32     key_data;
	u8       *u32_key = (u8 *)uc_key;
	u32     data[4];
	u32     key_field_start_index = 16;
	u32     index_cnt;
	u32     enable_secure_number = 32;
	u32     uiProgram2ndBit = 0;

	u32     key_field_index;
	switch (key_set_index) {
	//Note: >>>1st Key set is dedicate for secure boot usage<<<
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_field_start_index = 16;
		if (is_1st_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_1ST_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_field_start_index = 20;
		if (is_2nd_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_2ND_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_field_start_index = 24;
		if (is_3rd_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_3RD_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:	//28,29,30,31 ---> 28,09,10,11
		key_field_start_index = 28;
		if (is_4th_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_4TH_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_field_start_index = 12;
		if (is_5th_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_5TH_KEY_SET_PROGRAMMED;
		}
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index + 1);
		result = EFUSE_OPS_ERR;
		break;
	}

	if (result != EFUSE_SUCCESS) {
		if (result == EFUSE_FREEZE_ERR) {
			DBG_ERR("key set[%d], key field not enpty\r\n", (int)key_set_index);
		}
		return result;
	}
	//Need porting
	data[0] = *(u32 *)(u32_key + 0);

	data[1] = *(u32 *)(u32_key + 4);

	data[2] = *(u32 *)(u32_key + 8);

	data[3] = *(u32 *)(u32_key + 12);

	for (index_cnt = 0; index_cnt < EFUSE_OTP_KEY_FIELD_CNT; index_cnt++) {
		key_field_index = key_field_start_index + index_cnt;
		//DBG_DUMP("cnt[%d][0x%08x]\r\n", key_field_index, data[index_cnt]);
		result = key_write_data(key_field_index, data[index_cnt]);

		if (result < 0) {
			DBG_ERR("[%d]set key => write addr[%2d][0x%08x] fail\r\n", (int)(key_set_index + 1), (int)key_field_index, (int)data[index_cnt]);
			break;
		} else {
			key_data = otp_key_manager(key_field_index);
			if (key_data != data[index_cnt]) {
				//Re program 2nd if 1st programmed not complete.
				uiProgram2ndBit = key_data ^ data[index_cnt];
				result = key_write_data(key_field_index, uiProgram2ndBit);
				if (result < 0) {
					DBG_ERR("[%d]set key => write addr[%2d][0x%08x] fail\r\n", (int)(key_set_index + 1), (int)(key_field_index), (int)data[index_cnt]);
					break;
				} else {
					key_data = otp_key_manager(key_field_index);
					if (key_data != data[index_cnt]) {
						DBG_ERR("[%d]2nd set key => write addr[%2d][0x%08x] fail\r\n", (int)(key_set_index + 1), (int)key_field_start_index + index_cnt, (int)uiProgram2ndBit);
						result = EFUSE_CONTENT_ERR;
						break;
					} else {
						result = EFUSE_SUCCESS;
					}
				}
			}
		}
	}
	if (result == EFUSE_SUCCESS) {
		enable_secure_boot(enable_secure_number);
	}
	return result;
}

/**
(v)
    otp_set_key_destination

    Durung encrypt or decrypt, configure specific key set as AES key(0~3) to crypto engine

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b E_OK:   Success
*/
s32 otp_set_key_destination(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	u32  key_field_start_index = 16;
	//u32		param;

	//param 	= KEY_MANAGER_READ_PARAM();

	switch (key_set_index) {
	//Note: >>>1st Key set is dedicate for secure boot usage<<<
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_field_start_index = 16;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_field_start_index = 20;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_field_start_index = 24;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_field_start_index = 28;
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_field_start_index = 12;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index);
		return EFUSE_OPS_ERR;
	}
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 0);
	otp_key_manager(key_field_start_index);
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 1);
	otp_key_manager(key_field_start_index + 1);
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 2);
	otp_key_manager(key_field_start_index + 2);
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 3);
	otp_key_manager(key_field_start_index + 3);
	return E_OK;
}

/**
    otp_set_key_read_lock

    Once otp_set_key_read_lock set, this key set field will not allow read value by CPU

    (Only can operate by key manager)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b E_OK:   Success
*/
s32 otp_set_key_read_lock(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	u32  key_set_read_lock_index = SECUREBOOT_1ST_KEY_SET_READ_LOCK;
	switch (key_set_index) {
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_1ST_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_2ND_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_3RD_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_4TH_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_5TH_KEY_SET_READ_LOCK;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index);
		return EFUSE_OPS_ERR;
	}

	enable_secure_boot(key_set_read_lock_index);

	return E_OK;
}


/**
    otp_set_key_engine_access_right

    Once otp_set_key_engine_access_right set, this key set field will not transfer key to destination engine(RSA/HASH/Crypto)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b E_OK:   Success
*/
s32 otp_set_key_engine_access_right(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	u32	key_set_engine_access_right_index;
	switch (key_set_index) {
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_1ST_KEY_BIT_START;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_2ND_KEY_BIT_START;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_3RD_KEY_BIT_START;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_4TH_KEY_BIT_START;
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_5TH_KEY_BIT_START;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index);
		return EFUSE_OPS_ERR;
	}
	EFUSE_SET_ENGINE_ACCESS_RIGHT(key_set_engine_access_right_index);

	return EFUSE_SUCCESS;
}

//Bit[0] & Bit[5] == 1
/**
    quary_secure_boot

    Quary now is what kind of secure boot type

    @Note: key set 0 is for secure boot use

    @param[in] scu_status   status
    @return Description of data returned.
        - @b  TRUE:   enabled
        - @b FALSE:   disabled
*/
BOOL quary_secure_boot(SECUREBOOT_STATUS scu_status)
{
	u32  sec;
	BOOL    result = FALSE;

	sec = KEY_MANAGER_READ_PARAM();

	switch (scu_status) {
	case SECUREBOOT_SECURE_EN:
		if ((sec & (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) == (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_DATA_AREA_ENCRYPT:
		if ((sec & OTP_DATA_ENCRYPT_EN) == OTP_DATA_ENCRYPT_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_SIGN_RSA:
		if ((sec & OTP_SIGNATURE_RSA) == OTP_SIGNATURE_RSA) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_SIGN_RSA_CHK:
		if ((sec & OTP_SIGNATURE_RSA_CHK_EN) == OTP_SIGNATURE_RSA_CHK_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_JTAG_DISABLE_EN:
		if ((sec & OTP_JTAG_DISABLE_EN) == OTP_JTAG_DISABLE_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_1ST_KEY_SET_PROGRAMMED:
		if ((sec & OTP_1ST_KEY_PROGRAMMED) == OTP_1ST_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_2ND_KEY_SET_PROGRAMMED:
		if ((sec & OTP_2ND_KEY_PROGRAMMED) == OTP_2ND_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_3RD_KEY_SET_PROGRAMMED:
		if ((sec & OTP_3RD_KEY_PROGRAMMED) == OTP_3RD_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_4TH_KEY_SET_PROGRAMMED:
		if ((sec & OTP_4TH_KEY_PROGRAMMED) == OTP_4TH_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_5TH_KEY_SET_PROGRAMMED:
		if ((sec & OTP_5TH_KEY_PROGRAMMED) == OTP_5TH_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_1ST_KEY_SET_READ_LOCK:
		if ((sec & OTP_1ST_KEY_READ_LOCK) == OTP_1ST_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
		if ((sec & OTP_2ND_KEY_READ_LOCK) == OTP_2ND_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		if ((sec & OTP_3RD_KEY_READ_LOCK) == OTP_3RD_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_4TH_KEY_SET_READ_LOCK:
		if ((sec & OTP_4TH_KEY_READ_LOCK) == OTP_4TH_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_5TH_KEY_SET_READ_LOCK:
		if ((sec & OTP_5TH_KEY_READ_LOCK) == OTP_5TH_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_HDMI_AVAILABLE:
		if ((sec & EFUSE_HDMI_USAGE_BIT_MASK) == HDMI_1ST_AVAILABLE) {
			result = TRUE;
		} else if ((sec & EFUSE_HDMI_USAGE_BIT_MASK) == HDMI_2ND_AVAILABLE) {
			result = TRUE;
		} else if ((sec & EFUSE_HDMI_USAGE_BIT_MASK) == HDMI_1ST_DISABLE) {
			result = FALSE;
		} else if ((sec & EFUSE_HDMI_USAGE_BIT_MASK) == HDMI_2ND_DISABLE) {
			result = FALSE;
		} else {
			result = FALSE;
		}
		break;

	case SECUREBOOT_CN_NT_OVR_CLK:
		if ((sec & OTP_CN_NT_OVR_CLK) == OTP_CN_NT_OVR_CLK) {
			result = TRUE;
		}
		break;
/*
	case SECUREBOOT_NEW_KEY_RULE:
		//if ((sec & OTP_NEW_KEY_RULE_ID) == OTP_NEW_KEY_RULE_ID) {
		//	result = TRUE;
		//}
		result = FALSE;
		break;
*/
	default:
		break;
	}
	return result;
}

/**
    set_secure_boot

    Quary now is what kind of secure boot type

    @Note: key set 0 is for secure boot use

    @param[in] scu_status   status
    @return Description of data returned.
        - @b  TRUE:   enabled
        - @b FALSE:   something wrong(already configured)
*/
BOOL enable_secure_boot(SECUREBOOT_STATUS scu_status)
{
	u32  sec;
	BOOL    result = FALSE;
	sec = KEY_MANAGER_READ_PARAM();

	switch (scu_status) {
	case SECUREBOOT_SECURE_EN:
		if ((sec & (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) != 0x0) {
			DBG_ERR("Secure already enabled ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM((OTP_HW_SECURE_EN | OTP_FW_SECURE_EN));
		}
		break;

	case SECUREBOOT_DATA_AREA_ENCRYPT:
		if ((sec & OTP_DATA_ENCRYPT_EN) != 0x0) {
			DBG_ERR("Data area encrypted bit already set ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_DATA_ENCRYPT_EN);
		}
		break;

	case SECUREBOOT_SIGN_RSA:
		if ((sec & OTP_SIGNATURE_RSA) != 0x0) {
			DBG_ERR("Signature use RSA bit already set ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_SIGNATURE_RSA);
		}
		break;

	case SECUREBOOT_SIGN_RSA_CHK:

		if ((sec & OTP_SIGNATURE_RSA_CHK_EN) != 0x0) {
			DBG_ERR("Signature use RSA and process hash checksum bit already set ...\r\n");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_SIGNATURE_RSA_CHK_EN);
		}
		break;

	case SECUREBOOT_JTAG_DISABLE_EN:
		KEY_MANAGER_WRITE_PARAM(OTP_JTAG_DISABLE_EN);
		result = TRUE;
		break;

	case SECUREBOOT_1ST_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_1ST_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_2ND_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_2ND_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_3RD_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_3RD_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_4TH_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_4TH_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_5TH_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_5TH_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_1ST_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_1ST_KEY_READ_LOCK);
		result = TRUE;
		break;

		//2ND & 3RD key set can not configured as read lock(because of RSA checksum)
#if 0
	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_2ND_KEY_READ_LOCK);
		result = TRUE;
		break;

	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_3RD_KEY_READ_LOCK);
		result = TRUE;
		break;
#endif

	case SECUREBOOT_4TH_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_4TH_KEY_READ_LOCK);
		result = TRUE;
		break;

	case SECUREBOOT_5TH_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_5TH_KEY_READ_LOCK);
		result = TRUE;
		break;
	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		DBG_ERR("2ND & 3RD key set can not configured as read lock(RSA checksum)");
	default:
		result = FALSE;
		break;
	}
	return result;
}

/*
    otp read data

    Read specific eFuse addressing(32bits/per time)
     ^
     |-- (row address)
    @param[in] rowAddress       row address

    @return read half-word efuse data
        - @b  Positive: Valid data
        - @b    E_SYS : Invalid data
*/
u32 otp_key_manager(u32 rowAddress)
{
	s32  uiData;

	uiData = local_load((rowAddress & 0x1F) | (DBL_BIT_EN << 11));

	return uiData;
}

/*
    exteact trim data valid

    Parsing trim data is valid or not

    @param [in] code	trim raw data
    @param[out] pData	Point of valid trim data

    @return read half-word efuse data
        - @b  TRUE	: Valid data
        - @b  FALSE : Invalid data
*/
BOOL extract_trim_valid(u32 code, u32 *pData)
{
	u32 i;
	u32 ret = 0;
	BOOL is_found = FALSE;

	for (i = 0; i < 2; i++) {
		if (code & (1 << 14)) {
			// do nothing: skip this setting
		} else if (code & (1 << 15)) {
			ret = code & 0x3FFF;
			*pData = ret;
			is_found = TRUE;
			break;
		} else {
			// bit[15..14] = 0x0: efuse never programmed
			break;
		}

		code >>= 16;
	}

	return is_found;
}

/*
    exteact 2 set trim data valid(1st/2nd)

    Parsing trim data is valid or not

    @param [in] code    trim raw data
    @param[out] pData   Point of valid trim data

    @return read half-word efuse data
        - @b  TRUE  : Valid data
        - @b  FALSE : Invalid data
*/
BOOL extract_2_set_type_trim_valid(TRIM_FIELD_SET trim_set, u32 code, u32 *pData)
{
	u32 i;
	u32 ret = 0;
	BOOL is_found = FALSE;

	for (i = 0; i < 2; i++) {
		if (trim_set == TRIM_1ST_FIELD) {
			if (code & (1 << 6)) {
				// do nothing: skip this setting
			} else if (code & (1 << 7)) {
				ret = code & 0x3F;
				*pData = ret;
				is_found = TRUE;
				break;
			} else {
				// bit[7..6] = 0x0: efuse never programmed
				break;
			}
		} else if (trim_set == TRIM_2ND_FIELD) {
			if (code & (1 << 14)) {
				// do nothing: skip this setting
			} else if (code & (1 << 15)) {
				ret = code & 0x3F00;
				*pData = ret;
				is_found = TRUE;
				break;
			} else {
				// bit[15..14] = 0x0: efuse never programmed
				break;
			}
		} else {

		}

		code >>= 16;
	}

	return is_found;
}
/*
    exteact trim data to PWM duty cycle

    Parsing trim data and transfer to pwm duty cycle

    @param [in] data    trim data
    @param [in] freq    CPU freq
    @return read half-word efuse data
        - @b  UINT32: PWM dufy
*/
//Default = 0.98V
#define CORE_DEFAULT_DUTY 9

u32 extract_trim_to_pwm_duty(PWM_POWER_SET power_set, UINT32 data)
{
	u32 duty = 0;

	if (data < LEAKAGE_COUNT) {
		duty = core_power_leakage[data];
	} else {    //leakage >= 21 =>
		duty = CORE_DEFAULT_DUTY;
	}
	return duty;
}
