/*
* Copyright (c) 2005-2014 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#ifndef __DRV_CIPHER_H__
#define __DRV_CIPHER_H__

/* add include here */
#include "drv_cipher_intf.h"

#include "hal_cipher.h"
#include "drv_cipher_define.h"
//#include "common_mem.h"


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/
#define CIPHER_DEFAULT_INT_NUM    1
#define CIPHER_SOFT_CHAN_NUM      CIPHER_CHAN_NUM
#define CIPHER_INVALID_CHN        (0xffffffff)

typedef HI_VOID (*funcCipherCallback)(HI_U32);

HI_S32 DRV_Cipher_OpenChn(HI_U32 softChnId);
HI_S32 DRV_Cipher_CloseChn(HI_U32 softChnId);
HI_S32 DRV_Cipher_ConfigChn(HI_U32 softChnId, HI_UNF_CIPHER_CTRL_S *pConfig, funcCipherCallback fnCallBack);
HI_S32 DRV_Cipher_CreatTask(HI_U32 softChnId, HI_DRV_CIPHER_TASK_S *pTask, HI_U32 *pKey, HI_U32 *pIV);
HI_S32 DRV_CipherDataDoneMultiPkg(HI_U32 chnId);

HI_S32 DRV_Cipher_CreatMultiPkgTask(HI_U32 softChnId, HI_DRV_CIPHER_DATA_INFO_S *pBuf2Process, HI_U32 pkgNum, HI_U32 callBackArg);
HI_S32 DRV_Cipher_Stat(HI_U32 chnId, HI_U32 *pCipherStat);

HI_S32 DRV_Cipher_Init(HI_VOID);
HI_VOID DRV_Cipher_DeInit(HI_VOID);

HI_S32 DRV_Cipher_CalcHashInit(CIPHER_HASH_DATA_S *pCipherHashData);
HI_S32 DRV_Cipher_CalcHashUpdate(CIPHER_HASH_DATA_S *pCipherHashData);
HI_S32 DRV_Cipher_CalcHashFinal(CIPHER_HASH_DATA_S *pCipherHashData);
HI_S32 DRV_Cipher_AuthCbcMac(HI_U8 *pu8RefCbcMac, HI_U32 u32AppLen);

HI_S32 DRV_CIPHER_CalcRsa(CIPHER_RSA_DATA_S *pCipherRsaData);
#ifdef __cplusplus
}
#endif
#endif /* __DRV_CIPHER_H__ */

