/* 
 * ***************************************************************
 * Filename:      	hi3536_gio_interface.h
 * Created at:    	2015.12.21
 * Description:   	hi3536 gio API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __HI3536A_GIO_INTERFACE_H__
#define __HI3536A_GIO_INTERFACE_H__

int hi_gio_config(Uint32 gio_num, GPIO_DIR_E gio_dir);

int hi_gio_write(Uint32 gio_num, int gio_level);

int hi_gio_read(Uint32 gio_num);

int hi_gio_mux(Uint32 mux,int value);

int hi_gio_close(void);

#endif

