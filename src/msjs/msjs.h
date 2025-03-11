/* 
 * ***************************************************************
 * Filename:      	msjs.h
 * Created at:    	2018.03.28
 * Description:   	milesight joystick api.
 * Author:        	goli
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#ifndef __MSJS_H__
#define __MSJS_H__

#ifdef __cplusplus
extern "C" {
#endif

int ms_js_init();
int ms_js_uninit();
int js_core_init();
int js_cmd_handle(char *data);

#ifdef __cplusplus
}
#endif

#endif

