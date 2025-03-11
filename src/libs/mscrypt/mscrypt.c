/***cryptico include aes ande rsa encrypt and decrypt***/
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <time.h>
#include <string.h>
#include "msstd.h"
#include "mscrypt.h"
#include "rsa.h"

ms_list_t* aeskeyList = NULL;
MUTEX_OBJECT aesMutex;

int BIO_base64_decode(char *str,int str_len,char *decode,int decode_buffer_len)
{
    int len=0;
    BIO *b64,*bmem;
    b64=BIO_new(BIO_f_base64());
    bmem=BIO_new_mem_buf(str,str_len);
    bmem=BIO_push(b64,bmem);
    len=BIO_read(bmem,decode,str_len);
    decode[len]=0;
    BIO_free_all(bmem);
    return 0;
} 
void aesEvpecb(char *key,int bit,char*Src,char *Des,int ed)
{
	unsigned char iv[17] = {0};
	memset(iv, 0, sizeof(iv));  
	strcpy((char *)iv, "6BC1BEE22E409F96");  
	
	int inLen = strlen(Src);
	int encLen = 0;
    int outlen = 0;
	if(ed==1){
		EVP_CIPHER_CTX *ctx;
    	ctx = EVP_CIPHER_CTX_new();
		EVP_CipherInit_ex(ctx, EVP_aes_256_ecb(), NULL, (const unsigned char *)key, iv, 1);
	    EVP_CipherUpdate(ctx, (unsigned char *)Des, &outlen, (const unsigned char *)Src, inLen);
	    encLen = outlen;
	    EVP_CipherFinal(ctx, (unsigned char *)Des+outlen, &outlen);
	    encLen += outlen;
	    EVP_CIPHER_CTX_free(ctx);
		//msprintf("decrypt: %s\n",Des);
	}else{
		int decLen = 0;
	    outlen = 0;
	    EVP_CIPHER_CTX *ctx2;
	    ctx2 = EVP_CIPHER_CTX_new();
	    EVP_CipherInit_ex(ctx2, EVP_aes_256_ecb(), NULL, (const unsigned char *)key, iv, 0);
	    EVP_CipherUpdate(ctx2, (unsigned char *)Des, &outlen, (const unsigned char *)Src, encLen);
	    decLen = outlen;
	    EVP_CipherFinal(ctx2, (unsigned char *)Des+outlen, &outlen);
	    decLen += outlen;
	    EVP_CIPHER_CTX_free(ctx2);
		//msprintf("decrypt: %s\n",Des);
	}
}
void testEvp()
{
	unsigned char key[33] = {1};
    unsigned char iv[17] = {0};
	memset(key, 0, sizeof(key));  
	memset(iv, 0, sizeof(iv));  
    strcpy((char *)key, "");  
	strcpy((char *)iv, "");
	
	char buf[7] = "123456";
    unsigned char *inStr = (unsigned char *)buf;
    int inLen = strlen(buf);
    int encLen = 0;
    int outlen = 0;
    unsigned char encData[1024];
    
    //msprintf("source: %s\n",inStr);
    
    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    
    EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, 1);
    EVP_CipherUpdate(ctx, (unsigned char *)encData, &outlen, (const unsigned char *)inStr, inLen);
    encLen = outlen;
    EVP_CipherFinal(ctx, encData+outlen, &outlen);
    encLen += outlen;
    EVP_CIPHER_CTX_free(ctx);
    int decLen = 0;
    outlen = 0;
    unsigned char decData[1024];
    EVP_CIPHER_CTX *ctx2;
    ctx2 = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx2, EVP_aes_256_cbc(), NULL, key, iv, 0);
    EVP_CipherUpdate(ctx2, (unsigned char *)decData, &outlen, (const unsigned char *)encData, encLen);
    decLen = outlen;
    EVP_CipherFinal(ctx2, decData+outlen, &outlen);
    decLen += outlen;
    EVP_CIPHER_CTX_free(ctx2);
    
    decData[decLen] = '\0';
    //msprintf("decrypt: %s\n",decData);
}
//rsa encrypt
void testRAS(RSA *rsaKey)
{
	RSA_size(rsaKey);  
	  
	char fData[]="aaabbbccdskjkfd";  
	char tData[128]; 	  
	int flen = strlen(fData);	
	RSA_public_encrypt(flen, (unsigned char *)fData, (unsigned char *)tData, rsaKey,  RSA_PKCS1_PADDING);  
	RSA_private_decrypt(128, (unsigned char *)tData, (unsigned char *)fData, rsaKey, RSA_PKCS1_PADDING);	
	RSA_free(rsaKey);  
	return;	  

}
//rsa dectypt
int kkrsa_public_encrypt(char *inStr,char *outData,RSA *r)
{
    int encRet = 0;
    unsigned long inLen = strlen(inStr);
    int pdBlock = RSA_size(r)-11;
    unsigned int eCount = (inLen / pdBlock) +1;
	int i = 0;
    for (i = 0; i < eCount; i++) 
	{
        RSA_public_encrypt(inLen > pdBlock?pdBlock:inLen, (const unsigned char *)inStr, (unsigned char *)outData, r, RSA_PKCS1_PADDING);
        inStr += pdBlock;
        outData+=RSA_size(r);
        encRet+=RSA_size(r);
        inLen -= pdBlock;
    }
    return encRet;
}
int kkrsa_private_decrypt(char *inStr,char *outData,RSA *r)
{
    int decRet = 0;
    unsigned long inLen = strlen(inStr);
    int pdBlock = RSA_size(r);
    unsigned int dCount = inLen / pdBlock;
	int i = 0;
    for(i = 0;i < dCount; i++) 
	{
        int ret = RSA_private_decrypt(pdBlock, (const unsigned char *)inStr, (unsigned char *)outData, r, RSA_PKCS1_PADDING);
        inStr += pdBlock;
        outData+=ret;
        decRet+=ret;
    }
    return decRet;
}


void testRSAnew(RSA *r)
{
	char *src = "this is test encrypt data use RSA_PKCS1_PADDING";
	char *encDat = malloc(1024);
	char *decDat = malloc(1024);

	kkrsa_public_encrypt(src, encDat, r);
	kkrsa_private_decrypt(encDat, decDat, r);

	free(encDat);
	free(decDat);
}

void testRSA_EVP()
{
	RSA *r = RSA_new();
    int bits = 512;
    BIGNUM *e = BN_new();
    BN_set_word(e, 65537);
    RSA_generate_key_ex(r, bits, e, NULL);

    EVP_PKEY *key;
    key = EVP_PKEY_new();
    EVP_PKEY_set1_RSA(key, r);
	 //Ĭ��ʹ�õ��� RSA_PKCS1_PADDING�������������ܿ�Ϊ64-11=53,����������Ҫ���鴦��
    char *srcStr = "01234567890123456789012345678901234567890123456789123";
    //char *srcStr = "hello world";
    int enclen = 0;
    char encData[1024] = {0};
    char decData[1024] = {0};
    int declen = 0;
    //msprintf("src=%s\n",srcStr);
    
    
    //����
    EVP_PKEY_CTX *ectx;
    ectx = EVP_PKEY_CTX_new(key, NULL);
    EVP_PKEY_encrypt_init(ectx);
    EVP_PKEY_encrypt(ectx, (unsigned char *)encData, (size_t *)&enclen, (const unsigned char *)srcStr, strlen(srcStr));
	//msprintf("src=%s\n",encData);
    
    
    //����
    EVP_PKEY_CTX *dctx;
    dctx = EVP_PKEY_CTX_new(key, NULL);
    EVP_PKEY_decrypt_init(dctx);
    EVP_PKEY_decrypt(dctx, (unsigned char *)decData, (size_t *)&declen, (const unsigned char *)encData, enclen);
    //msprintf("dec=%s\n",decData);

    EVP_PKEY_CTX_free(ectx);
    EVP_PKEY_CTX_free(dctx);

    
    EVP_PKEY_free(key);
    BN_free(e);
    RSA_free(r);
}

int testPubKey()
{
    RSA *ClientRsa = NULL;
    BIGNUM *bne = NULL;
    bne = BN_new();
    if (BN_set_word(bne, 65537) != 1) {
        BN_free(bne);
        return -1;
    }

    ClientRsa = RSA_new();
    if (RSA_generate_key_ex(ClientRsa, 1024, bne, NULL) != 1) {
        BN_free(bne);
        RSA_free(ClientRsa);
        return -1;
    }
      
    unsigned char PublicKey[1024];
    unsigned char *PKey = PublicKey; // ע�����ָ�벻�Ƕ��࣬������Ҫ�������ģ�
    int PublicKeyLen = i2d_RSA_PUBKEY(ClientRsa, &PKey);
      
    PKey = PublicKey;
    RSA *EncryptRsa = d2i_RSA_PUBKEY(NULL, (const unsigned char**)&PKey, PublicKeyLen);
      
    unsigned char InBuff[128], OutBuff[128]; 
      
    strcpy((char *)InBuff, "1234567890abcdefghiklmnopqrstuvwxyz.");
    RSA_public_encrypt(128, (const unsigned char*)InBuff, OutBuff, EncryptRsa, RSA_NO_PADDING); // ����
      
    memset(InBuff, 0, sizeof(InBuff));
    RSA_private_decrypt(128, (const unsigned char*)OutBuff, InBuff, ClientRsa, RSA_NO_PADDING); // ����
    //msprintf("RSADecrypt OK: %s \n", InBuff);
      
    unsigned char Seed[33]; 
    strcpy((char *)Seed, "2B7E151628AED2A6ABF7158809CF4F3C");
    AES_KEY AESEncryptKey, AESDecryptKey;
    AES_set_encrypt_key(Seed, 256, &AESEncryptKey);
    AES_set_decrypt_key(Seed, 256, &AESDecryptKey);
      
    strcpy((char *)InBuff, "a1b2c3d4e5f6g7h8?");
    AES_encrypt(InBuff, OutBuff, &AESEncryptKey);
      
    memset(InBuff, 0, sizeof(InBuff));
    AES_decrypt(OutBuff, InBuff, &AESDecryptKey);
    //msprintf("AESDecrypt OK: %s \n", InBuff);
      
    RSA_free(ClientRsa);
    RSA_free(EncryptRsa);
    BN_free(bne);
    
    return(0);
}

RSA * getRsaKey(char *pubkey)
{
	//msprintf("pubkey=%s",pubkey);
	BIO *bio;  
	bio = BIO_new(BIO_s_mem());  
    BIO_puts(bio, pubkey); 
	RSA *rsa = RSA_new();
	rsa = PEM_read_bio_RSAPublicKey(bio, NULL,NULL,NULL);

	RSA_size(rsa);
	return rsa;
}

static char randchars[16] = "0123456789ABCDEF";
void GetAes(int n,char *randkey,char *keyID)
{
	int i = 0;
	srand((int) time(0));
	long timesecond;
	timesecond = time(NULL);
	for(i = 0; i < n;i++)
	{
		 int rnd=rand()%15;
		 randkey[i] = randchars[rnd];
	}
	memset(keyID,0,n);
	snprintf(keyID,n,"%ld",timesecond);
	aeskey_list_add(aeskeyList, randkey, keyID);
	//msprintf("aeskey=%s========keyID=%s\n",randkey,keyID);
}
int aeskey_list_create()
{
	aeskeyList = (ms_list_t*)ms_malloc(sizeof(ms_list_t));
	if(!aeskeyList)
		return -1;
	memset(aeskeyList, 0, sizeof(ms_list_t));
	return 0;
}

void aeskey_list_destory()
{
	ms_mutex_lock(&aesMutex);
	ms_list_ofchar_free(aeskeyList);
	ms_mutex_unlock(&aesMutex);
}

void aeskey_list_add(ms_list_t *aeskey_list, char *aeskey, char *keyID)
{
	long timesecond;
	timesecond = time(NULL);
	ms_aeskey_t* aesKey = (ms_aeskey_t *)ms_malloc(sizeof(ms_aeskey_t));
	if(!aesKey) return;
	aesKey->first_login_time = timesecond;
	snprintf(aesKey->aeskey, sizeof(aesKey->aeskey), "%s", aeskey);
	snprintf(aesKey->keyid, sizeof(aesKey->keyid), "%s", keyID);
	ms_mutex_lock(&aesMutex);
	ms_list_add(aeskey_list, (void *)aesKey, -1);		
	ms_mutex_unlock(&aesMutex);
}

int aeskey_list_get(char *keyID, char *aeskey,int len)
{
	ms_list_iterator_t iterator;	
	ms_mutex_lock(&aesMutex);
	ms_aeskey_t* c_aes = (ms_aeskey_t*)ms_list_get_first(aeskeyList, &iterator);
	while(ms_list_iterator_has_elem(iterator))
	{
		if(!strcmp(c_aes->keyid, keyID))
		{     
		   	  //msprintf("###########get aeskey lock %d#############\n",len);
		   	  snprintf(aeskey, len, "%s", c_aes->aeskey);
		   	  ms_mutex_unlock(&aesMutex);
			  return 1;                				  						
		}	
		c_aes = (ms_aeskey_t*)ms_list_get_next(&iterator);
	}
	ms_mutex_unlock(&aesMutex);
	return 0;	
}

void aeskey_list_unlock()
{
	long cur_time = time(NULL); 		
	ms_list_iterator_t iterator;
	ms_mutex_lock(&aesMutex);
	ms_aeskey_t*  c_aes = (ms_aeskey_t*)ms_list_get_first(aeskeyList, &iterator);
	while(ms_list_iterator_has_elem(iterator))
	{
								
		if(cur_time - c_aes->first_login_time > MAX_AES_SECOND)//5min
		{
			c_aes = (ms_aeskey_t*)ms_list_iterator_remove(&iterator);
			continue;
		}						
		c_aes = (ms_aeskey_t*)ms_list_get_next(&iterator);
	}
	ms_mutex_unlock(&aesMutex);
}

void aeskey_mutex_init()
{
	ms_mutex_init(&aesMutex);
}

void aeskey_mutex_uninit()
{
	ms_mutex_uninit(&aesMutex);
}

int ChallengeKey(char *pubkey,char *Des,char *keyID)
{
	char *chPublicKey=pubkey;
	BIO *bio;  
	bio = BIO_new(BIO_s_mem());  
    BIO_puts(bio, chPublicKey); 
	RSA *rsa = RSA_new();
	rsa = PEM_read_bio_RSAPublicKey(bio, NULL,NULL,NULL);
	if(!rsa)
		return 0;
	int keySize = RSA_size(rsa);
	unsigned char InBuff[RSA_BUFF_LEN]={0},OutBuff[RSA_BUFF_LEN]={0}; 
    GetAes(KEY_LEN - 1,(char *)InBuff,keyID);
	//msprintf("RSAput OK: %d keyID %s\n", keySize,keyID);
    RSA_public_encrypt(keySize-11, (const unsigned char*)InBuff, OutBuff, rsa, RSA_PKCS1_PADDING);
	//msprintf("RSAput OK2: %s \n", InBuff);
	RSA_free(rsa);
	BIO_free(bio);
	memset(Des, 0, PKEY_LEN);
	ms_base64_encode(OutBuff,Des,128);
	return 1;
}
int aesDecryWeb(unsigned char *InBuff,unsigned char *Seed,unsigned char *decData,int len)
{
	unsigned char ivec[]=""; 
	memset(decData,0,INBUFF_LEN);
	int decLen = 0,outlen = 0;
    EVP_CIPHER_CTX *ctx2;
    ctx2 = EVP_CIPHER_CTX_new();
	//msprintf("ecryt====%s,%s=====================\n",Seed,ivec);
	int sus=0;
    sus=EVP_CipherInit_ex(ctx2, EVP_aes_256_cbc(), NULL, Seed, ivec, 0);
    sus=EVP_CipherUpdate(ctx2, (unsigned char *)decData, &outlen, (const unsigned char *)InBuff, len);
    decLen = outlen;
    sus=EVP_CipherFinal_ex(ctx2, decData+outlen, &outlen);
	if(sus!=1)ERR_print_errors_fp (stderr);
    decLen += outlen;
    EVP_CIPHER_CTX_free(ctx2);
    decData[decLen] = '\0';
	//msprintf("OutBuff===%s=====%d,%d\n",decData,decLen,len);
	return decLen;
}

int makePublicKey(int *keySize)
{
    RSA *pRsa = NULL;
    BIGNUM *bne = NULL;
    bne = BN_new();
    if (BN_set_word(bne, RSA_F4) != 1) {
        BN_free(bne);
        return -1;
    }

    pRsa = RSA_new();
    if (RSA_generate_key_ex(pRsa, 1024, bne, NULL) != 1) {
        BN_free(bne);
        RSA_free(pRsa);
        return -1;
    }
    
	if (pRsa == NULL)
	{
		msprintf("rsa_generate_key error");
		return -1;
	}
	BIO *pBio = BIO_new_file(PUBLIC_FILE, "wb");

	if (pBio == NULL)
	{
		msprintf("BIO_new_file = %s  error", PUBLIC_FILE);
		return -2;
	}
	*keySize = RSA_size(pRsa);
	if (PEM_write_bio_RSAPublicKey(pBio, pRsa) == 0)
	{
		msprintf("write public key error");
		return -3;
	}
	BIO_free_all(pBio);


	pBio = BIO_new_file(PRIVATE_FILE, "w");
	if (pBio == NULL)
	{
		msprintf("BIO_new_file %s error ", PRIVATE_FILE);
		return -4;
	}
	if (PEM_write_bio_RSAPrivateKey(pBio, pRsa, NULL, NULL, 0, NULL, NULL) == 0)
	{
		msprintf("write private key error");
		return -5;
	}
	BIO_free_all(pBio);
	RSA_free(pRsa);

	return 0;
}

int Dec(char *buffer, char *in, int inLen, char *out, int * outLen)
{
	BIO *pBio = BIO_new_mem_buf(buffer, -1);
	RSA *pRsa = PEM_read_bio_RSAPrivateKey(pBio, NULL, NULL, NULL);
	BIO_free_all(pBio);
	//msprintf("outLen:%d==000\n",*outLen);
	*outLen = RSA_private_decrypt(
		inLen,
		(unsigned char *)(in),
		(unsigned char *)(out),
		pRsa,
		RSA_PKCS1_PADDING);
	//msprintf("outLen:%d==111\n",*outLen);
	RSA_free(pRsa);
	if (*outLen >= 0)
		return 0;
	return -1;
}

int EncodeAES(char *sKey, char *password, char *encodeData)
{
	unsigned char key[33] = {1};
	unsigned char iv[17] = {0};
	memset(key, 0, sizeof(key));
	memset(iv, 0, sizeof(iv));
	strcpy((char *)key, sKey);
	strcpy((char *)iv, "");

	int encLen = 0;
	int outlen = 0;
	unsigned char encData[1024];
	char Source[1024];

	memset(Source, 0x00, sizeof(Source));
	strcpy(Source, password);
	int inLen = strlen(Source);

	EVP_CIPHER_CTX *ctx;
	ctx = EVP_CIPHER_CTX_new();

	EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, 1);
	EVP_CipherUpdate(ctx, encData, &outlen, (unsigned char *)Source, inLen);
	encLen = outlen;
	EVP_CipherFinal(ctx, encData + outlen, &outlen);
	encLen += outlen;
	EVP_CIPHER_CTX_free(ctx);


	char szBase64[1024] = { 0 };
	ms_base64_encode(encData, szBase64, encLen);
	memcpy(encodeData, szBase64, sizeof(szBase64));

	return 0;

}


