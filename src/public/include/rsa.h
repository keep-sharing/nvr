/*
 * rsa.h
 *
 *  Created on: 2014-2-19
 *      Author: root
 */

#ifndef RSA_H_
#define RSA_H_

#include <openssl/rsa.h>

#define PUBLIC_KEYFILE	"/etc/public.rsa"

#define FILE_PUB	"/tmp/local.pub"
#define FILE_PRV	"/tmp/local.prv"
#define FILE_RMT	"/tmp/remote.pub"

#define KEY_SIZE	1024

enum keytype {
	KEY_PUBLIC = 0,
	KEY_PRIVATE,
};

int rsa_create(const char *pub_keyfile, const char *prv_keyfile);

int rsa_encrypt(const char *pub_keyfile, void *in, int in_size, void *out, int *out_size);

int rsa_decrypt(const char *prv_keyfile, void *in, int in_size, void *out, int *out_size);

int rsa_cal_ensize(int len);

int url_enc(const char *src, char *dst);

int url_dec(const char *src, char *dst);

#endif /* RSA_H_ */
