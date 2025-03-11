/*
 * ***************************************************************
 * Filename:        nt98323_gio_interface.h
 * Created at:      2015.12.21
 * Description:     nt98323 gio API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __NT98633_GIO_INTERFACE_H__
#define __NT98633_GIO_INTERFACE_H__

int nvt_gio_config(Uint32 gio_num, GPIO_DIR_E gio_dir);

int nvt_gio_write(Uint32 gio_num, int gio_level);

int nvt_gio_read(Uint32 gio_num);

int nvt_gio_mux(Uint32 mux, int value);

int nvt_gio_close(void);

#endif

