/*
 * ssl.c
 *
 *  Created on: 2014-2-18
 *      Author: root
 */

#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <unistd.h>
#include "rsa.h"

#define PRIVATE_KEYFILE	"/etc/private.rsa"
#define KEY_EXPONENT	RSA_F4

#define PACK_SIZE (1024 / 8)
#define DATA_SIZE (PACK_SIZE - 11)

#define RSA_DEBUG 0
#if RSA_DEBUG
#define DEBUG(msg, args...) printf("[%s,%d]"msg, __FUNCTION__, __LINE__, ##args)
#else
#define DEBUG(msg, args...)
#endif
static int create_pem(const char *pub_key, const char *prv_key)
{
	int ret = 0;
	BIO *pub = NULL, *pri = NULL;
	RSA *rsa = NULL;
    BIGNUM *bne = NULL;
    rsa = RSA_new();
    bne = BN_new();
	do {
		if ((pub = BIO_new_file(pub_key, "w")) == NULL)
			break;
		if ((pri = BIO_new_file(prv_key, "w")) == NULL)
			break;
        if (BN_set_word(bne, KEY_EXPONENT) != 1) {
            break;
        }
		if ((ret = RSA_generate_key_ex(rsa, KEY_SIZE, bne, NULL)) != 1) {
			break;
        }
		if (!PEM_write_bio_RSAPublicKey(pub, rsa))
			break;
		if (!PEM_write_bio_RSAPrivateKey(pri, rsa, NULL, NULL, 0, NULL, NULL))
			break;
		ret = 0;
	} while (0);
	if (pub) {
//		BIO_CLOSE(pub);
		BIO_free(pub);
	}
	if (pri) {
//		BIO_CLOSE(pri);
		BIO_free(pri);
	}
	if (rsa)
		RSA_free(rsa);
    BN_free(bne);
	return ret;
}

int rsa_create(const char *pub_keyfile, const char *prv_keyfile)
{
//	if (access(pub_keyfile, F_OK) || access(prv_keyfile, F_OK)) {
		if (create_pem(pub_keyfile, prv_keyfile)) {
			DEBUG("create key file error\n");
			return -1;
		}
//	}
	return 0;
}

int rsa_encrypt(const char *pub_keyfile, void *in, int in_size, void *out, int *out_size)
{
	RSA *rsa = NULL;
	FILE *fp;
	int len = 0, ret = -1;
	int realsize, dealsize;
	*out_size = 0;
	int sum = 0;

	if (in_size <= 0)
		return 0;
	if ((fp = fopen(pub_keyfile, "r")) == NULL) return -1;
	if (!PEM_read_RSAPublicKey(fp, &rsa, NULL, NULL)) return -1;
	len = RSA_size(rsa) - 11;

	while (in_size) {
		realsize = in_size >= len? len:in_size;
		in_size -= realsize;
		dealsize = RSA_public_encrypt(realsize, (unsigned char *) in + sum, (unsigned char *) out + *out_size, rsa, RSA_PKCS1_PADDING);
		if(dealsize < 0)
			break;
		*out_size += dealsize;
		sum += realsize;
		ret = 0;
	};

	if (fp) fclose(fp);
	if (rsa) RSA_free(rsa);
	rsa = NULL;

	return ret;
}

int rsa_decrypt(const char *prv_keyfile, void *in, int in_size, void *out, int *out_size)
{
	RSA *rsa = NULL;
	FILE *fp;
	int len = 0, ret = -1;
	int realsize, dealsize;
	*out_size = 0;
	int sum = 0;

	if (in_size <= 0)
		return 0;
	if ((fp = fopen(prv_keyfile, "r")) == NULL) return -1;
	if (!PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL)) return -1;
	len = RSA_size(rsa);

	while (in_size) {
		realsize = in_size >= len? len:in_size;
		in_size -= realsize;
		dealsize = RSA_private_decrypt(realsize, (unsigned char *) in + sum, (unsigned char *) out + *out_size, rsa, RSA_PKCS1_PADDING);
		if (dealsize < 0)
			break;
		*out_size += dealsize;
		sum += realsize;
		ret = 0;
	};

	if (fp) fclose(fp);
	if (rsa) RSA_free(rsa);
	rsa = NULL;

	return ret;
}

int rsa_cal_ensize(int len)
{
	int cnt = 0;

	cnt = len / DATA_SIZE;
	if (len % DATA_SIZE)
		cnt++;
	return (cnt * PACK_SIZE);
}

/**
 * @brief 对base64的url中的"/、+和＝"转换
 * \ '/' -> '-'
 * \ '+' -> '*'
 * \ '=' -> '!'
 * @param src
 * @param dst
 * @param dst_len
 * @return
 */
int url_enc(const char *src, char *dst)
{
	int i,len = strlen(src);

	for (i = 0; i < len; i++) {
		switch (src[i]) {
		case '/': dst[i] = '-';break;
		case '+': dst[i] = '*'; break;
		case '=': dst[i] = '$'; break;
//		case '=': dst[i++] = '%';dst[i++] = '3'; dst[i] = 'D'; break;
		default:
			dst[i] = src[i];
			break;
		}
	}
	dst[i] = '\0';
	return 0;
}

int url_dec(const char *src, char *dst)
{
	int i,len = strlen(src);

	for (i = 0; i < len; i++) {
		switch (src[i]) {
		case '-': dst[i] = '/';break;
		case '*': dst[i] = '+'; break;
		case '$': dst[i] = '='; break;
		default:
			dst[i] = src[i];
			break;
		}
	}
	dst[i] = '\0';
	return 0;
}
#if RSA_DEBUG
static void print(const char *buf)
{
	int i = 0;

	for (;buf[i]; i++)
		printf("%x", buf[i]);
	printf("\n");
}

int main(int argc, char *argv[])
{
#if 0
	if (argc != 2) {
		DEBUG("usage: %s [string]\n", argv[0]);
		return -1;
	}
	char buf[256] = {0}, debuf[256] = {0};
	if (create_pem()) {
		DEBUG("create key file error\n");
		return -1;
	}
	DEBUG("#############before\n");
	print(argv[1]);
#endif
	struct test{
		int a;
		int b;
		char buf[1234];
		int c;
	};

	struct test tmp = {
		.a = 1,
		.b = 2,
		.c = 3,
	};

	int i;
	for(i = 0;i < sizeof(tmp.buf);i ++)
		tmp.buf[i] = 'a';
	tmp.buf[sizeof(tmp.buf) - 1] = 0;
	
	rsa_create("public.key", "private.key");


	char debuf[20480] = {0};
	int in_size = 0, out_size = 0;
	rsa_encrypt("private.key", KEY_PRIVATE, &tmp, sizeof(tmp), debuf, &out_size);

	struct test tmpout = {0};
	rsa_decrypt("public.key", KEY_PUBLIC, debuf, out_size, &tmpout, &in_size);

	printf("########## a = %d, b = %d, c = %d\n", tmpout.a, tmpout.b, tmpout.c);
//	char buf[256] = {0}, debuf[256] = {0};
//	snprintf(buf, sizeof(buf), "%s", "hello");
//	rsa_encrypt(PUBLIC_KEYFILE, KEY_PUBLIC, buf, debuf);
//	printf("encrypt done\n");
//	memset(buf, 0, sizeof(buf));
//	rsa_decrypt(PRIVATE_KEYFILE, KEY_PRIVATE, debuf, buf);
//	printf("after decrypt\n");
//	printf("buf is : %s\n", buf);
	return 0;
}

#endif
