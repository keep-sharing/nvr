/*
    Novatek protected header file of NT96660's driver.

    The header file for Novatek protected APIs of NT96660's driver.

    @file       efuse_protected.h
    @ingroup    mIDriver
    @note       For Novatek internal reference, don't export to agent or customer.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef __ARCH_EFUSE_PROTECTED_H
#define __ARCH_EFUSE_PROTECTED_H

#include <asm/nvt-common/nvt_types.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/rcw_macro.h>

typedef enum {
	PWM0_CORE_POWER 	= 0x0,
	PWM1_CPU_POWER,
} PWM_POWER_SET;

typedef enum {
	LEAKAGE_1 = 0x1,
	LEAKAGE_2,
	LEAKAGE_3,
	LEAKAGE_4,
	LEAKAGE_5,
	LEAKAGE_6,
	LEAKAGE_7,
	LEAKAGE_8,
	LEAKAGE_9,
	LEAKAGE_10,
	LEAKAGE_11,
	LEAKAGE_12,
	LEAKAGE_13,
	LEAKAGE_14,
	LEAKAGE_15,
	LEAKAGE_16,
	LEAKAGE_17,
	LEAKAGE_18,
	LEAKAGE_19,
	LEAKAGE_20,
	LEAKAGE_COUNT,

	LEAKAGE_MAX = 255,


} PWM_POWER_LEAKAGE;

typedef enum {
	SCE_NO_0_KEY_SET_FIELD = 0x0,
	SCE_NO_1_KEY_SET_FIELD,
	SCE_NO_2_KEY_SET_FIELD,
	SCE_NO_3_KEY_SET_FIELD,
	SCE_NO_4_KEY_SET_FIELD,
	SCE_NO_5_KEY_SET_FIELD,
	SCE_NO_6_KEY_SET_FIELD,
	SCE_NO_7_KEY_SET_FIELD,


	SCE_NO_8_KEY_SET_FIELD,
	SCE_NO_9_KEY_SET_FIELD,
	SCE_NO_10_KEY_SET_FIELD,
	SCE_NO_11_KEY_SET_FIELD,

	SCE_NO_12_KEY_SET_FIELD,
	SCE_NO_13_KEY_SET_FIELD,
	SCE_NO_14_KEY_SET_FIELD,
	SCE_NO_15_KEY_SET_FIELD,

	EFUSE_TOTAL_KEY_SET_FIELD,

	EFUSE_CRYPTO_ENGINE_KEY_CNT = SCE_NO_8_KEY_SET_FIELD,
	EFUSE_RSA_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,
	EFUSE_HASH_ENGINE_KEY_CNT = EFUSE_TOTAL_KEY_SET_FIELD,

	ENUM_DUMMY4WORD(SCE_KEY_SET_TO_OTP_FIELD)
} SCE_KEY_SET_TO_OTP_FIELD;


typedef enum {
	EFUSE_OTP_1ST_KEY_SET_FIELD 	= 0x0,        // This if for secure boot
	EFUSE_OTP_2ND_KEY_SET_FIELD 	= 0x1,
	EFUSE_OTP_3RD_KEY_SET_FIELD 	= 0x2,
	EFUSE_OTP_4TH_KEY_SET_FIELD 	= 0x3,
	EFUSE_OTP_5TH_KEY_SET_FIELD 	= 0x4,
	EFUSE_OTP_TOTAL_KEY_SET_FIELD 	= 0x5,
} EFUSE_OTP_KEY_SET_FIELD;

STATIC_ASSERT(EFUSE_OTP_TOTAL_KEY_SET_FIELD <= 5);

#define EFUSE_OTP_KEY_FIELD_CNT			4

typedef enum {
	OTP_KEY_MANAGER_NONE = 0x0,        			// Not process key manager
	OTP_KEY_MANAGER_CRYPTO,						// Key manager, destination is crypto engine(SCE)
	OTP_KEY_MANAGER_RSA,						// Key manager, destination is RSA engine
	OTP_KEY_MANAGER_HASH,						// Key manager, destination is HASH engine
	OTP_KEY_MANAGER_CNT,
} OTP_KEY_DESTINATION;

#define KEY_MANAGER_DESTINATION_ADDRESS		IOADDR_EFUSE_REG_BASE + 0x40
#define KEY_MANAGER_KEY_INDEX_ADDRESS		IOADDR_EFUSE_REG_BASE + 0x44

/**
    Crypto engine check
*/
typedef enum {
	SECUREBOOT_SECURE_EN = 0x00,       	///< Quary if secure enable or not
	SECUREBOOT_DATA_AREA_ENCRYPT,       ///< Quary if data area encrypt to cypher text or not
	SECUREBOOT_SIGN_RSA,				///< Quary if Signature methed is RSA or not(AES)
	SECUREBOOT_SIGN_RSA_CHK,			///< Quary if Signature hash checksum RSA key correct or not
	SECUREBOOT_JTAG_DISABLE_EN,			///< Quary if JTAG is disable or not(TRUE : disable)

	SECUREBOOT_1ST_KEY_SET_PROGRAMMED,	///< Quary if 1st key set programmed or not
	SECUREBOOT_2ND_KEY_SET_PROGRAMMED,	///< Quary if 2nd key set programmed or not
	SECUREBOOT_3RD_KEY_SET_PROGRAMMED,	///< Quary if 3rd key set programmed or not
	SECUREBOOT_4TH_KEY_SET_PROGRAMMED,	///< Quary if 4th key set programmed or not
	SECUREBOOT_5TH_KEY_SET_PROGRAMMED,	///< Quary if 5th key set programmed or not

	SECUREBOOT_1ST_KEY_SET_READ_LOCK,	///< Quary if 1st key read lock already or not
	SECUREBOOT_2ND_KEY_SET_READ_LOCK,	///< Quary if 2nd key read lock already or not
	SECUREBOOT_3RD_KEY_SET_READ_LOCK,	///< Quary if 3rd key read lock already or not
	SECUREBOOT_4TH_KEY_SET_READ_LOCK,	///< Quary if 4th key read lock already or not
	SECUREBOOT_5TH_KEY_SET_READ_LOCK,	///< Quary if 5th key read lock already or not

	SECUREBOOT_STATUS_NUM,

} SECUREBOOT_STATUS;

#ifndef TRUE
#define TRUE							1
#endif
#ifndef FALSE
#define FALSE							0
#endif
#define E_OK                            0
#define EFUSE_SUCCESS                E_OK
#define EFUSE_FREEZE_ERR             -1001          // Programmed already, only can read
#define EFUSE_INACTIVE_ERR           -1002          // This field is empty(not programmed yet)
#define EFUSE_INVALIDATE_ERR         -1003          // This field force invalidate already
#define EFUSE_UNKNOW_PARAM_ERR       -1004          // efuse param field not defined
#define EFUSE_OPS_ERR                -1005          // efuse operation error
#define EFUSE_SECURITY_ERR           -1006          // efuse under security mode => can not read back
#define EFUSE_PARAM_ERR              -1007          // efuse param error
#define EFUSE_CONTENT_ERR            -1008          // efuse operation error

#define OTP_HW_SECURE_EN			(1 << 0)
#define OTP_FW_SECURE_EN			(1 << 5)
#define OTP_DATA_ENCRYPT_EN			(1 << 7)
#define OTP_SIGNATURE_RSA			(1 << 1)
#define OTP_SIGNATURE_RSA_CHK_EN	(1 << 3)
#define OTP_JTAG_DISABLE_EN			(1 << 2)

#define OTP_1ST_KEY_PROGRAMMED_BIT	27
#define OTP_2ND_KEY_PROGRAMMED_BIT	28
#define OTP_3RD_KEY_PROGRAMMED_BIT	29
#define OTP_4TH_KEY_PROGRAMMED_BIT	30
#define OTP_5TH_KEY_PROGRAMMED_BIT	31

#define OTP_1ST_KEY_PROGRAMMED		(1 << OTP_1ST_KEY_PROGRAMMED_BIT)
#define OTP_2ND_KEY_PROGRAMMED		(1 << OTP_2ND_KEY_PROGRAMMED_BIT)
#define OTP_3RD_KEY_PROGRAMMED		(1 << OTP_3RD_KEY_PROGRAMMED_BIT)
#define OTP_4TH_KEY_PROGRAMMED		(1 << OTP_4TH_KEY_PROGRAMMED_BIT)
#define OTP_5TH_KEY_PROGRAMMED		(1 << OTP_5TH_KEY_PROGRAMMED_BIT)

#define OTP_1ST_KEY_READ_LOCK_BIT	22
#define OTP_2ND_KEY_READ_LOCK_BIT	23
#define OTP_3RD_KEY_READ_LOCK_BIT	24
#define OTP_4TH_KEY_READ_LOCK_BIT	25
#define OTP_5TH_KEY_READ_LOCK_BIT	26

#define OTP_1ST_KEY_READ_LOCK		(1 << OTP_1ST_KEY_READ_LOCK_BIT)
#define OTP_2ND_KEY_READ_LOCK		(1 << OTP_2ND_KEY_READ_LOCK_BIT)
#define OTP_3RD_KEY_READ_LOCK		(1 << OTP_3RD_KEY_READ_LOCK_BIT)
#define OTP_4TH_KEY_READ_LOCK		(1 << OTP_4TH_KEY_READ_LOCK_BIT)
#define OTP_5TH_KEY_READ_LOCK		(1 << OTP_5TH_KEY_READ_LOCK_BIT)




#define is_secure_enable()				quary_secure_boot(SECUREBOOT_SECURE_EN)
#define is_data_area_encrypted()		quary_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)
#define is_signature_rsa()				quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_signature_rsa_chsum_enable()	quary_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#define is_signature_aes()				!quary_secure_boot(SECUREBOOT_SIGN_RSA)
#define is_JTAG_DISABLE_en()			quary_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)

#define is_1st_key_programmed()			quary_secure_boot(SECUREBOOT_1ST_KEY_SET_PROGRAMMED)
#define is_2nd_key_programmed()			quary_secure_boot(SECUREBOOT_2ND_KEY_SET_PROGRAMMED)
#define is_3rd_key_programmed()			quary_secure_boot(SECUREBOOT_3RD_KEY_SET_PROGRAMMED)
#define is_4th_key_programmed()			quary_secure_boot(SECUREBOOT_4TH_KEY_SET_PROGRAMMED)
#define is_5th_key_programmed()			quary_secure_boot(SECUREBOOT_5TH_KEY_SET_PROGRAMMED)


#define is_1st_key_read_lock()			quary_secure_boot(SECUREBOOT_1ST_KEY_SET_READ_LOCK)
#define is_2nd_key_read_lock()			quary_secure_boot(SECUREBOOT_2ND_KEY_SET_READ_LOCK)
#define is_3rd_key_read_lock()			quary_secure_boot(SECUREBOOT_3RD_KEY_SET_READ_LOCK)
#define is_4th_key_read_lock()			quary_secure_boot(SECUREBOOT_4TH_KEY_SET_READ_LOCK)
#define is_5th_key_read_lock()			quary_secure_boot(SECUREBOOT_5TH_KEY_SET_READ_LOCK)


extern INT32 	otp_set_key_read_lock(EFUSE_OTP_KEY_SET_FIELD key_set_index);
extern INT32    otp_set_key_destination(EFUSE_OTP_KEY_SET_FIELD key_set_index);
/**
    otp_write_key

    Write specific key into specific key set (0~4)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @param[in] ucKey           key (16bytes)
    @return Description of data returned.
        - @b EFUSE_SUCCESS		: Success
        - @b EFUSE_OPS_ERR		: OTP operation error
        - @b EFUSE_FREEZE_ERR	: Key field already programmed
        - @b EFUSE_CONTENT_ERR	: Write key and read back and compare fail
*/
extern INT32    otp_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key);
extern BOOL 	quary_secure_boot(SECUREBOOT_STATUS scu_status);
extern BOOL 	enable_secure_boot(SECUREBOOT_STATUS scu_status);
extern UINT32 	otp_key_manager(UINT32 rowAddress);
extern ER 		efuse_get_unique_id(UINT32 * id_L, UINT32 * id_H);
extern BOOL 	extract_trim_valid(UINT32 code, UINT32 *pData);
extern UINT32 	extract_trim_to_pwm_duty(PWM_POWER_SET power_set, UINT32 data, UINT32 freq);




/**
    efuse_secure_en

    Enable secure boot
*/
#define efuse_secure_en()				enable_secure_boot(SECUREBOOT_SECURE_EN)
/**
    efuse_jtag_dis

    Disable JTAG

    @Note: Can not re enable once disabled
*/
#define efuse_jtag_dis()				enable_secure_boot(SECUREBOOT_JTAG_DISABLE_EN)
#define efuse_data_area_encrypt_en()	enable_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)
#define efuse_signature_rsa_en()		enable_secure_boot(SECUREBOOT_SIGN_RSA)
#define efuse_signature_rsa_chksum_en()	enable_secure_boot(SECUREBOOT_SIGN_RSA_CHK)
#endif /* __ARCH_EFUSE_PROTECTED_H */

