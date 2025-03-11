/* 
 * ***************************************************************
 * Filename:      	rs485.h
 * Created at:    	2016.01.14
 * Description:   	common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */


#ifndef __RS485_H__
#define __RS485_H__

int rs485OpenDev(char *Dev);
void rs485CloseDev(int fd);
int rs485SetPortAttr (int fd,int baudrate, int databit, int stopbit, char parity);

// full duplex 
int rs485FullWrite(int fd, void *data, int len);
int rs485FullRead(int fd, void *data, int len);

//half duplex
int rs485HalfWrite(int fd, void *data, int len);
int rs485HalfRead(int fd, void *data, int len);

#endif

