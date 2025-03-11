/**

    @file       crypto.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/crypto.h>
#include <asm/arch/efuse_protected.h>

#define INREG32(x)          (*((volatile UINT32*)(x)))          ///< Read 32bits IO register
#define OUTREG32(x, y)      (*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

#define REG_PMU_BASE						IOADDR_CG_REG_BASE

#define SYSC_MOD_RESET_REG0					(REG_PMU_BASE + 0x50)
#define SYSC_MOD_RESET_REG1					(REG_PMU_BASE + 0x54)
#define SYSC_APB_RESET_REG0					(REG_PMU_BASE + 0x58)
#define SYSC_APB_RESET_REG1					(REG_PMU_BASE + 0x5C)
#define SYSC_BUS_CLKGATING_REG1				(REG_PMU_BASE + 0x64)
#define SYSC_BUS_CLKGATING_REG2				(REG_PMU_BASE + 0x68)
#define SYSC_APB_CLKGATING_REG1				(REG_PMU_BASE + 0x74)
#define SYSC_APB_CLKGATING_REG2				(REG_PMU_BASE + 0x78)

#define CRYPTO_ENGINE_RESET_OFS				17
#define HASH_ENGINE_RESET_OFS				18
#define RSA_ENGINE_RESET_OFS				19

#define HASH_SCE_AXI_RESET_OFS				0

#define HASH_PCLK_GATING_OFS				2
#define RSA_PCLK_GATING_OFS					20
#define CRYPTO_PCLK_GATING_OFS				21

#define HASH_SCE_BUS_CLK_GATING_OFS			0

#define enableCryptoReset()         		CLRREG32(SYSC_MOD_RESET_REG0, (0x1<<CRYPTO_ENGINE_RESET_OFS))	//Reset Crypto(SCE)
#define enableHASHReset()         			CLRREG32(SYSC_MOD_RESET_REG0, (0x1<<HASH_ENGINE_RESET_OFS))	//Reset HASH
#define enableRSAReset()         			CLRREG32(SYSC_MOD_RESET_REG0, (0x1<<RSA_ENGINE_RESET_OFS))	//Reset RSA

#define disableCryptoReset()        		SETREG32(SYSC_MOD_RESET_REG0, (0x1<<CRYPTO_ENGINE_RESET_OFS))	//Release Crypto(SCE)
#define disableHASHReset()        			SETREG32(SYSC_MOD_RESET_REG0, (0x1<<HASH_ENGINE_RESET_OFS))	//Release HASH
#define disableRSAReset()        			SETREG32(SYSC_MOD_RESET_REG0, (0x1<<RSA_ENGINE_RESET_OFS))	//Release RSA


#define disableHASHSCEAXIReset()        	SETREG32(SYSC_MOD_RESET_REG1, (0x1<<HASH_SCE_AXI_RESET_OFS))	    			//Release SCE/HASH(axi)

//#define enableSCEPCLK()             		CLRREG32(SYSC_APB_CLKGATING_REG1, 0x300004)	   			//Release SCE/HASH/RSA PCLK gating
#define enableHASHPCLK()             		CLRREG32(SYSC_APB_CLKGATING_REG1, (1<<HASH_PCLK_GATING_OFS))	   			//Release SCE/HASH/RSA PCLK gating
#define enableRSAPCLK()             		CLRREG32(SYSC_APB_CLKGATING_REG1, (1<<RSA_PCLK_GATING_OFS))	   			//Release SCE/HASH/RSA PCLK gating
#define enableCRYPTOPCLK()             		CLRREG32(SYSC_APB_CLKGATING_REG1, (1<<CRYPTO_PCLK_GATING_OFS))	   			//Release SCE/HASH/RSA PCLK gating

#define disableHASHPCLK()             		SETREG32(SYSC_APB_CLKGATING_REG1, (1<<HASH_PCLK_GATING_OFS))	   			//Release SCE/HASH/RSA PCLK gating
#define disableRSAPCLK()             		SETREG32(SYSC_APB_CLKGATING_REG1, (1<<RSA_PCLK_GATING_OFS))	   			//Release SCE/HASH/RSA PCLK gating
#define disableCRYPTOPCLK()             	SETREG32(SYSC_APB_CLKGATING_REG1, (1<<CRYPTO_PCLK_GATING_OFS))	   			//Release SCE/HASH/RSA PCLK gating


#define enableHASHSCEBUSCLK()          		CLRREG32(SYSC_BUS_CLKGATING_REG2, (1<<HASH_SCE_BUS_CLK_GATING_OFS))	   				//Release Crypto bus gating
#define enableCryptoClock()         		CLRREG32(SYSC_BUS_CLKGATING_REG1, (0x07 << 0))  //Release SCE/HASH/RSA
#define disableCryptoClock()        		SETREG32(SYSC_BUS_CLKGATING_REG1, (0x07 << 0))  //gating SCE/HASH/RSA


#define CRYPTO_SETREG(ofs,value)    	OUTW(IOADDR_CRYPTO_REG_BASE+(ofs),(value))
#define CRYPTO_GETREG(ofs)          	INW(IOADDR_CRYPTO_REG_BASE+(ofs))

#define CRYPTO_REG_BASE_ADDR			IOADDR_CRYPTO_REG_BASE


#define CRYPTO_DMA_CONFIG_REG_OFS       0x08
#define CRYPTO_DMA_STS_REG_OFS          0x0C

#define CRYPTO_SRC_ADDR_REG_OFS         0x50
#define CRYPTO_DST_ADDR_REG_OFS         0xA4

#define CRYPTO_DMA_CONTROL_REG_OFS      0x04

#define CRYPTO_KEY000_REG_OFS           0x30
#define CRYPTO_KEY032_REG_OFS           0x34
#define CRYPTO_KEY064_REG_OFS           0x38
#define CRYPTO_KEY096_REG_OFS           0x3C

// Crypto
#define CRYPTO_CONFREG_SWRST          	(1<<0)
#define CRYPTO_CONFREG_CRYPTO_EN     	(1<<1)
#define CRYPTO_CONFREG_AES128         	(2<<4)
#define CRYPTO_CONFREG_DECRYPT        	(1<<8)
#define CRYPTO_PIO_DONE               	(1<<0)

#define dma_getPhyAddr(addr)    		((((uint32_t)(addr))>=0x60000000UL)?((uint32_t)(addr)-0x60000000UL):(uint32_t)(addr))
#define SCE_DES_TABLE_NUM        		(40)      //8(header DesTab) + 32(addr DesTab)
#define SCE_DES_WORD_SIZE        		(4)       //1 DesTab = 4 word
#define SCE_DES_KEY_OFS           		(0)
#define SCE_DES_IV_OFS           		(8)
#define SCE_DES_CNT_OFS          		(12)
#define SCE_DES_HAEDER_OFS       		(16)
#define SCE_DES_CV_OFS           		(20)
#define SCE_DES_S0_OFS           		(24)
#define SCE_DES_GHASH_OFS        		(28)
#define SCE_DES_BLOCK_CFG_OFS    		(32)

#define SCE_DMA_COUNT                   (1)
#define ROM_AES_SIZE                    (16)

#define _ALIGNED(x) __attribute__((aligned(x)))

static _ALIGNED(32) UINT32 vuiSCE_DesTab[SCE_DES_TABLE_NUM*SCE_DES_WORD_SIZE];

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

static u32 crypto_set_mode(u32 uiMode, u32 uiOPMode, u32 uiType)
{
	if (uiMode != CRYPTO_AES) {
		return -1;
	}

	if (uiOPMode >= CRYPTO_OPMODE_NUM) {
		return -1;
	}

	OUTREG32(CRYPTO_REG_BASE_ADDR, CRYPTO_CONFREG_CRYPTO_EN); // big endian, descrypt, AES-128, sw-reset
	return 0;
}

static void crypto_dma_enable(u32 uiMode, u32 uiOPMode, u32 uiType, u32 SrcAddr, u32 DstAddr, u32 Len, UINT8 *Key, UINT8 *IV)
{
	UINT32 	uiReg;
	UINT32 	header_config=0;
	UINT32  key_len=0;
	UINT32  iv_len=0;

	/* Disable interrupt enable */
	CRYPTO_SETREG(CRYPTO_DMA_CONFIG_REG_OFS, 0x0);

	/* clear DMA interrupt status */
	uiReg = CRYPTO_GETREG(CRYPTO_DMA_STS_REG_OFS);
	CRYPTO_SETREG(CRYPTO_DMA_STS_REG_OFS, uiReg);

    switch(uiMode) {
        case CRYPTO_AES:            ///< AES-128
            key_len = 16;
            iv_len  = 16;
            break;
        default:
            break;
    }

	/* Clear DMA Descriptor */
	memset((void*)&vuiSCE_DesTab[0], 0, SCE_DES_TABLE_NUM*SCE_DES_WORD_SIZE*4);

    /* Key */
    if (Key && key_len) {
        memcpy((void *)&vuiSCE_DesTab[SCE_DES_KEY_OFS], (void *)Key, key_len);
    }

    /* IV */
    if (IV && iv_len) {
        memcpy((void *)&vuiSCE_DesTab[SCE_DES_IV_OFS], (void *)IV, iv_len);
    }

    /* Header Config */
	header_config = (UINT32)((uiOPMode << 8 ) | (uiMode << 4 ) | uiType);
	if (!Key) {
	    header_config |= (0x1 << 12);     ///< key from key managament
	}
	vuiSCE_DesTab[SCE_DES_HAEDER_OFS] = header_config;

	/* Block Config */
    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS]    = dma_getPhyAddr(SrcAddr);
    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS+1]  = dma_getPhyAddr(DstAddr);
    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS+2]  = Len;
    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS+3]  = 1;                        ///< last block

  	/* Flush and Invalidate Cache */
   	flush_dcache_range((ulong)&vuiSCE_DesTab[0], (ulong)&vuiSCE_DesTab[0] + roundup(SCE_DES_TABLE_NUM *SCE_DES_WORD_SIZE*4, ARCH_DMA_MINALIGN));
   	flush_dcache_range(SrcAddr, SrcAddr + roundup(Len, ARCH_DMA_MINALIGN));
	invalidate_dcache_range(DstAddr, DstAddr + roundup(Len, ARCH_DMA_MINALIGN));

	/* Set Descriptor DMA CH#0 address */
	CRYPTO_SETREG(CRYPTO_SRC_ADDR_REG_OFS, dma_getPhyAddr((UINT32)&vuiSCE_DesTab[0]));

	/* Trigger DMA CH#0 Enable */
	CRYPTO_SETREG(CRYPTO_DMA_CONTROL_REG_OFS, (0x1<<4));

	/* Wait DMA CH#0 done */
	while (!(CRYPTO_GETREG(CRYPTO_DMA_STS_REG_OFS) & (0x1<<4)));

	/* Clear DMA CH#0 Status */
	CRYPTO_SETREG(CRYPTO_DMA_STS_REG_OFS, (0x1<<4));
}

s32 crypto_data_operation(EFUSE_OTP_KEY_SET_FIELD key_set, CRYPT_OP crypt_op_param)
{
	UINT32  ret;
	disableCryptoReset();
	disableHASHSCEAXIReset();
	enableCRYPTOPCLK();
	enableHASHSCEBUSCLK();
	enableCryptoClock();
	ret = crypto_set_mode(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt);
	if (ret < 0) {
		return ret;
	}
	otp_set_key_destination(key_set);
	crypto_dma_enable(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt, crypt_op_param.src_addr, crypt_op_param.dst_addr, crypt_op_param.length, NULL, NULL);
	enableCryptoReset();
	return 0;
}

s32 crypto_data_operation_by_key(UINT8 * key, CRYPT_OP crypt_op_param)
{
	s32  ret;
	disableCryptoReset();
	disableHASHSCEAXIReset();
	enableCRYPTOPCLK();
	enableHASHSCEBUSCLK();
	enableCryptoClock();
	ret = crypto_set_mode(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt);
	if (ret < 0) {
		return ret;
	}
	crypto_dma_enable(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt, crypt_op_param.src_addr, crypt_op_param.dst_addr, crypt_op_param.length, key, NULL);
	enableCryptoReset();
	return 0;
}

