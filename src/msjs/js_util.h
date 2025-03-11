/* 
 * ***************************************************************
 * Filename:      	js_util.h
 * Created at:    	2017.2.17
 * Description:   	js common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __JS_UTIL_H__
#define __JS_UTIL_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char           U8;
typedef unsigned char           UCHAR;
typedef unsigned short          U16;
typedef unsigned int            U32;
typedef unsigned long           ULONG;
typedef unsigned long long      U64;

typedef signed char             S8;
typedef short                   S16;
typedef int                     S32;
typedef long                    SLONG;


#define RED 			"\033[0;31m"
#define GREEN 			"\033[0;32m"
#define CLRCOLOR		"\033[0;39m "

extern int g_jsdebug;
#define jsdebug(format, ...) \
    do{\
        if(g_jsdebug) \
        printf("[JSDEBUG] func:%s file:%s line:%d]" GREEN format CLRCOLOR"\n", \
		__func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)

#define jserror(format, ...) \
    do{\
		printf("[JSERROR] func:%s file:%s line:%d]" RED format CLRCOLOR"\n", \
		__func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)

U64 get_now_time_ms(void);

#ifdef __cplusplus
}
#endif


#endif


