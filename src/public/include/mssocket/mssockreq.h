/* 
 * Filename:      	sockreq
 * Created at:    	2014.05.08
 * Description:   	dispatch the sock request
 *             
 * Author:        	bruce
 * Copyright (C)  	milesight
 * *******************************************************/
#ifndef _SOCK_REQUEST_H_
#define _SOCK_REQUEST_H_

#include "osa/osa_typedef.h"

#define MAX_REQUEST_COUNT  256

#ifdef __cplusplus
extern "C"
{
#endif

void dispatch_request(int req, int reqfrom, void* param, int size, UInt64 clientId);

struct sock_request_info
{
	int req;
	void (*handler)(int reqfrom, void* param, int size, UInt64 clientId);
};
void sock_request_register(struct sock_request_info* req, int count);
void sock_request_unregister(struct sock_request_info* req, int count);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif


#ifdef __cplusplus
}
#endif

#endif

