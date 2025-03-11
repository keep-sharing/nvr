/******************************************************************************
 *  Copyright (C) 2014 Hisilicon Technologies CO.,LTD.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Create By Cai Zhiyong 2014.12.22
 *
******************************************************************************/

#ifndef POLARSSL_RSA_H
#define POLARSSL_RSA_H

#include "hi_type.h"
#include "drv_cipher_define.h"
#include "drv_cipher_intf.h"
#include "drv_rsa_bignum.h"
#ifndef HI_MINIBOOT_SUPPORT
#include <common.h>
#else
#include "string.h"
#include "stdio.h"
#endif

#define CIPHER_MAX_RSA_KEY_LEN   512

#define POLARSSL_ERR_RSA_BAD_INPUT_DATA                    0x0100
#define POLARSSL_ERR_RSA_INVALID_PADDING                   0x0110
#define POLARSSL_ERR_RSA_KEY_GEN_FAILED                    0x0120
#define POLARSSL_ERR_RSA_KEY_CHECK_FAILED                  0x0130
#define POLARSSL_ERR_RSA_PUBLIC_FAILED                     0x0140
#define POLARSSL_ERR_RSA_PRIVATE_FAILED                    0x0150
#define POLARSSL_ERR_RSA_VERIFY_FAILED                     0x0160
#define POLARSSL_ERR_RSA_OUTPUT_TOO_LARGE                  0x0170
#define POLARSSL_ERR_RSA_RNG_FAILED                        0x0180

typedef struct {
	bignum N;  
    bignum D; 
	bignum E;
    
	bignum P;                      
	bignum Q;                      
	bignum DP;                     
	bignum DQ;                     
	bignum QP;                    
	bignum RN;                     
	bignum RP;                     
	bignum RQ;                     
} rsa_t;

#ifdef __cplusplus
extern "C" {
#endif

int hicap_rsa_init(rsa_t *rsa);
int hicap_rsa_gen_key( rsa_t *rsa, int (*f_rng)(void), int nbits, int exponent );
int hicap_rsa_public(rsa_t *rsa,const unsigned char *input, unsigned char *output);
int hicap_rsa_private(rsa_t *rsa,const unsigned char *input,unsigned char *output);
void hicap_rsa_free(rsa_t *rsa);

HI_S32 DRV_CIPHER_CalcRsa_SW(CIPHER_RSA_DATA_S *pCipherRsaData);

#ifdef __cplusplus
}
#endif

#endif /* rsa.h */
