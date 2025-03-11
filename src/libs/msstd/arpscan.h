/* 
 * ***************************************************************
 * Filename:      	arpscan.h
 * Created at:    	2021.01.30
 * Description:   	arp scan.
 * Author:        	david
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __ARPSCAN_H__
#define __ARPSCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

////////////
//
////////////
int arpscan_ipaddr(const char *dev, const char *ipaddr, char *dstmac, int ndstsize);


#ifdef __cplusplus
}
#endif

#endif



