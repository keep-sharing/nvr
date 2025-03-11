/* 
 * ***************************************************************
 * Filename:      	mskb.h
 * Created at:    	2017.01.17
 * Description:   	milesight keyboard api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#ifndef __MSKB_H__
#define __MSKB_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int g_kb_version;// 0: previous versions ;   1:new version

int ms_kb_init();

int ms_kb_uinit();

#ifdef __cplusplus
}
#endif

#endif

