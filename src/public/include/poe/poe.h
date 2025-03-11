/* 
 * ***************************************************************
 * Filename:      	poe.h
 * Created at:    	2016.8.25
 * Description:   	poe api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __POE_H__
#define __POE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "msdefs.h"

typedef enum {
    POE_FALSE = 0,
    POE_TRUE = 1,
}POE_BOOL;

#define POE_SUCCESS  0
#define POE_FAILURE  (-1)

#define POE_1004UPC  "/dev/hi_switch"

//in : pMac--mac format: xx:xx:xx:xx:xx:xx
int 
poe_get_net_port(char *pMac);

POE_BOOL 
poe_is_net_link_up(int portID);

int 
poe_set_net_enable(int portID, POE_BOOL bEnable);

POE_BOOL 
poe_is_power_link_up(int portID);

float 
poe_get_power_rate(int portID);

int 
poe_set_power_enable(int portID, POE_BOOL bEnable);

int 
poe_init(char *devName, int poeNum);

int 
poe_uninit();

#ifdef __cplusplus
}
#endif

#endif

