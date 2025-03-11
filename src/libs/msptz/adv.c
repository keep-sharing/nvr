/*
 * adv.c
 *
 *  Created on: 2013-2-2
 *      Author: root
 */

#include "adv.h"

#define STX	0x02
#define	ETX	0x03

#define TILT_UP		0x88
#define	TILT_DOWN	0x8c
#define	PAN_LEFT	0xa0
#define	PAN_RIGHT	0xb0
#define	ZOOM_PLUS	0xa0
#define	ZOOM_MINUS	0xb0
#define	FOCUS_PLUS	0x83
#define	FOCUS_MINUS	0x82
#define	IRIS_PLUS	0x88
#define	IRIS_MINUS	0x8c
#define MAGIC_NUM	0x59

typedef unsigned char byte;

static unsigned char adv_msg[ADV_MSG_LEN];

static unsigned char *adv_create_pkg(byte addr, byte data1, byte data2, byte data3, byte data4, byte data5)
{
	adv_msg[0] = STX;
	adv_msg[1] = addr;
	adv_msg[2] = data1;
	adv_msg[3] = data2;
	adv_msg[4] = data3;
	adv_msg[5] = data4;
	adv_msg[6] = data5;
	adv_msg[7] = ETX;
	return adv_msg;
}

unsigned char *adv_up(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, TILT_UP, 0x00, 0x00);
}

unsigned char *adv_down(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, TILT_DOWN, 0x00, 0x00);
}

unsigned char *adv_left(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, PAN_LEFT, 0x00, 0x00);
}

unsigned char *adv_right(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, PAN_RIGHT, 0x00, 0x00);
}

unsigned char *adv_left_up(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, PAN_LEFT | TILT_UP, 0x00, 0x00);
}

unsigned char *adv_left_down(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, PAN_LEFT | TILT_DOWN, 0x00, 0x00);
}

unsigned char *adv_right_up(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, PAN_RIGHT | TILT_UP, 0x00, 0x00);
}

unsigned char *adv_right_down(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, PAN_RIGHT | TILT_DOWN, 0x00, 0x00);
}

unsigned char *adv_stop(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *adv_zoom_plus(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, 0x00, ZOOM_PLUS, 0x00);
}

unsigned char *adv_zoom_minus(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, 0x00, ZOOM_MINUS, 0x00);
}

unsigned char *adv_focus_plus(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, 0x00, FOCUS_PLUS, 0x00);
}

unsigned char *adv_focus_minus(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, 0x00, FOCUS_MINUS, 0x00);
}

unsigned char *adv_iris_plus(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, 0x00, IRIS_PLUS, 0x00);
}

unsigned char *adv_iris_minus(int addr)
{
	return adv_create_pkg((byte)addr, MAGIC_NUM, 0x00, 0x00, IRIS_MINUS, 0x00);
}
