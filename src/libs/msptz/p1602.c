/*
 * p1602.c
 *
 *  Created on: 2013-1-31
 *      Author: root
 */

#include "p1602.h"

#define STX		0x5a
#define ETX		0x5f

#define PAN_RIGHT		0x0a
#define PAN_LEFT		0x0c
#define	TILT_UP			0x06
#define TILT_DOWN		0x08
#define STOP_ALL		0x0e
#define	ZOOM_PLUS		0x01
#define ZOOM_MINUS		0x00
#define	FOCUS_PLUS		0x05
#define	FOCUS_MINUS		0x04
#define	IRIS_PLUS		0x03
#define	IRIS_MINUS		0x07

typedef unsigned char byte;
static unsigned char p1602_msg[P1602_MSG_LEN];

static unsigned char *p1602_create_pkg(byte addr, byte data1, byte data2, byte data3,
							byte data4, byte data5, byte data6, byte data7)
{
	p1602_msg[0] = STX;
	p1602_msg[1] = addr;
	p1602_msg[2] = data1;
	p1602_msg[3] = data2;
	p1602_msg[4] = data3;
	p1602_msg[5] = data4;
	p1602_msg[6] = data5;
	p1602_msg[7] = data6;
	p1602_msg[8] = data7;
	p1602_msg[9] = ETX;
	p1602_msg[10] = addr + data1 + data2 + data3 + data4 + data5 + data6 + data7;
	return p1602_msg;
}

unsigned char *p1602_up(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, TILT_UP, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_down(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, TILT_DOWN, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_left(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, PAN_LEFT, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_right(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, PAN_RIGHT, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_left_up(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, PAN_LEFT | TILT_UP, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_right_up(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, PAN_RIGHT | TILT_UP, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_left_down(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, PAN_LEFT | TILT_DOWN, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_right_down(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, PAN_RIGHT | TILT_DOWN, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_zoom_plus(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, ZOOM_PLUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_zoom_minus(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, ZOOM_MINUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_focus_plus(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, FOCUS_PLUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_focus_minus(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, FOCUS_MINUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_iris_plus(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, IRIS_PLUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_iris_minus(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, IRIS_MINUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *p1602_stop(int addr)
{
	return p1602_create_pkg((byte)addr, 0x01, 0x00, STOP_ALL, 0x00, 0x00, 0x00, 0x00);
}
