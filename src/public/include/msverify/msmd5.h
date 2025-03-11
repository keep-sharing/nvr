#ifndef __MS_MD5_H__
#define __MS_MD5_H__

////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

/*
 ** 参数1：mskey为需要比对的字符串，keysize为长度
 ** 参数2：msrad为需要加密的随机数，radsize为长度
 ** 返回值：与strcmp返回值对应
*/
int check_uboot_auth(char *mskey, int keysize, char *msrad, int radsize);

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////
#endif
