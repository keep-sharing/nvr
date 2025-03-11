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

#ifndef POLARSSL_BIGNUM_H
#define POLARSSL_BIGNUM_H

#define MUL_ADDC_INIT                            \
	asm("ldr    r0, %0         " :: "m" (s));  \
	asm("ldr    r1, %0         " :: "m" (d));  \
	asm("ldr    r2, %0         " :: "m" (c));  \
	asm("ldr    r3, %0         " :: "m" (b));

#define MUL_ADDC_CORE                            \
	asm("ldr    r4, [r0], #4   ");            \
	asm("mov    r5, #0         ");            \
	asm("ldr    r6, [r1]       ");            \
	asm("umlal  r2, r5, r3, r4 ");            \
	asm("adds   r7, r6, r2     ");            \
	asm("adc    r2, r5, #0     ");            \
	asm("str    r7, [r1], #4   ");

#define MUL_ADDC_STOP                            \
	asm("str    r2, %0         " : "=m" (c));  \
	asm("str    r1, %0         " : "=m" (d));  \
	asm("str    r0, %0         " : "=m" (s) :: \
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");


#define BIGNUM_FILE_IO_ERROR                     0x0002
#define BIGNUM_BAD_INPUT_DATA                    0x0004
#define BIGNUM_INVALID_CHARACTER                 0x0006
#define BIGNUM_BUFFER_TOO_SMALL                  0x0008
#define BIGNUM_NEGATIVE_VALUE                    0x000A
#define BIGNUM_DIVISION_BY_ZERO                  0x000C
#define BIGNUM_NOT_ACCEPTABLE                    0x000E


#define BIG_CHK(r) if((ret = r) != 0) goto _exit

typedef struct {
    unsigned int *buf;  
    int u32size;
	int u32sign;                                    
} bignum;

#ifdef __cplusplus
extern "C" {
#endif


void hicap_bignum_init(bignum *X, ...);
void hicap_bignum_free(bignum *X, ...);
int hicap_bignum_grow(bignum *X, int nblimbs);
int hicap_bignum_copy(bignum *X, const bignum *Y);
int hicap_bignum_msb(const bignum *X);
void hicap_bignum_swap( bignum *X, bignum *Y );
int hicap_bignum_read_binary(bignum *X, const unsigned char *buf, int buflen);
int hicap_bignum_write_binary(const bignum *X, unsigned char *buf, int buflen);
int hicap_bignum_shift_l(bignum *X, int count);
int hicap_bignum_shift_r(bignum *X, int count);
int hicap_bignum_cmp_abs(const bignum *X, const bignum *Y);
int hicap_bignum_cmp_bignum(const bignum *X, const bignum *Y);
int hicap_bignum_cmp_int(const bignum *X, int z);
int hicap_bignum_add_abs(bignum *X, const bignum *A, const bignum *B);
int hicap_bignum_sub_abs(bignum *X, const bignum *A, const bignum *B);
int hicap_bignum_add_bignum(bignum *X, const bignum *A, const bignum *B);
int hicap_bignum_sub_bignum(bignum *X, const bignum *A, const bignum *B);
int hicap_bignum_add_int(bignum *X, const bignum *A, int b);
int hicap_bignum_sub_int(bignum *X, const bignum *A, int b);
int hicap_bignum_mul_bignum(bignum *X, const bignum *A, const bignum *B);
int hicap_bignum_mul_int(bignum *X, const bignum *A, unsigned int b);
int hicap_bignum_div_bignum(bignum *Q, bignum *R, const bignum *A, const bignum *B);
int hicap_bignum_mod_bignum(bignum *R, const bignum *A, const bignum *B);
int hicap_bignum_mod_int(unsigned int *r, const bignum *A, int b);
int hicap_bignum_exp_mod(bignum *X, const bignum *A, const bignum *E, const bignum *N, bignum *_RR);
int hicap_bignum_gcd(bignum *G, const bignum *A, const bignum *B);
int hicap_bignum_is_prime(bignum *X, int (*f_rng)(void));
int hicap_bignum_inv_mod( bignum *X, const bignum *A, const bignum *N );
int hicap_bignum_gen_prime( bignum *X, int nbits, int dh_flag,               int (*f_rng)(void));
int hicap_bignum_lset(bignum *X, int z);
int hicap_bignum_lsb(const bignum *X);
int hicap_bignum_size(const bignum *X);
void *hicap_vmalloc(unsigned int size);
void hicap_vfree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* bignum.h */
