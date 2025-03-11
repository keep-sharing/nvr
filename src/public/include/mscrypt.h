/***cryptico include aes ande rsa encrypt and decrypt***/
#ifndef __MS_CRYPT_H__
#define __MS_CRYPT_H__
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <time.h>


#define KEY_LEN 		33
#define INBUFF_LEN 		1024
#define PKEY_LEN 		256
#define RSA_BUFF_LEN 	128
#define IV_LEN 			17
#define SUCCESS_ACTIVE 	"Success"
#define FAIL_ACTIVE 	"Failed"
#define PWD_UNMATCH 	"Password Error"
#define DECRYPT_FAIL 	"Decrypt failed"
#define INACTIVE 		"Inactive"
#define ACTIVE 			"Active"
#define MAX_AES_SECOND 	300
#define PUBLIC_FILE		"/mnt/nand/public.pem"
#define PRIVATE_FILE	"/mnt/nand/private.pem"

//static const char *g_pPubFile = "/mnt/nand/public.pem";
//static const char *g_pPriFile = "/mnt/nand/private.pem";

typedef struct ms_aeskey_t	
{ 
	char aeskey[KEY_LEN];
	char keyid[KEY_LEN];
	long first_login_time;
}ms_aeskey_t;


int BIO_base64_decode(char *str,int str_len,char *decode,int decode_buffer_len); 
void aesEvpecb(char *key,int bit,char*Src,char *Des,int ed);
void testRAS(RSA *rsaKey);
//rsa dectypt
int kkrsa_public_encrypt(char *inStr,char *outData,RSA *r);
int kkrsa_private_decrypt(char *inStr,char *outData,RSA *r);
void testRSAnew(RSA *r);
void testRSA_EVP();
int testPubKey();
RSA * getRsaKey(char *pubkey);
void GetAes(int n,char *randkey,char *keyID);
int aeskey_list_create();

void aeskey_list_destory();
void aeskey_list_add(ms_list_t *aeskey_list, char *aeskey, char *keyID);
int aeskey_list_get(char *keyID, char *aeskey,int len);

void aeskey_list_unlock();
void aeskey_mutex_init();

void aeskey_mutex_uninit();

int ChallengeKey(char *pubkey,char *Des,char *keyID);
int aesDecryWeb(unsigned char *InBuff,unsigned char *Seed,unsigned char *decData,int len);
int makePublicKey(int *keySize);
int Dec(char *buffer, char *in, int inLen, char *out, int * outLen);
int EncodeAES(char *sKey, char *password, char *encodeData);


#endif

