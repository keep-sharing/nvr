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

#define IOADDR_EFUSE_EP_REG_BASE	0x620A90000

#define OUTREG32(x, y)      		writel(y, x)
#define INREG32(x)					readl(x)

#define KEY_MANAGER_READ_PARAM()    INREG32(IOADDR_EFUSE_EP_REG_BASE+0x50)    

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
BOOL quary_secure_boot_ep(SECUREBOOT_STATUS scu_status)
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

	case SECUREBOOT_NEW_KEY_RULE:
		if ((sec & OTP_NEW_KEY_RULE_ID) == OTP_NEW_KEY_RULE_ID) {
			result = TRUE;
		}
		break;

	default:
		break;
	}
	return result;
}
