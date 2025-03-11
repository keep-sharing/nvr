/* 
 * ***************************************************************
 * Filename:      	vapi_comm.h
 * Created at:    	2015.10.21
 * Description:   	common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __POE_UTIL_H__
#define __POE_UTIL_H__

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void string_to_mac(unsigned char *mac, char* s);

#ifdef __cplusplus
}
#endif


#endif

