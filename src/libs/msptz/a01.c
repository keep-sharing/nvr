/*
 * a01.c
 *
 *  Created on: 2013-1-31
 *      Author: root
 */

#include "a01.h"

#define SPD_MAX		7
#define PAN_RIGHT	0x34
#define PAN_LEFT	0x33
#define TILT_UP		0x31
#define TILT_DOWN	0x32
#define	STOP_ALL	0x80
#define ZOOM_PLUS	0x38
#define ZOOM_MINUS	0x37
#define FOCUS_PLUS	0x41
#define FOCUS_MINUS	0x39
#define	IRIS_PLUS	0x35
#define	IRIS_MINUS	0x36
#define PRESET_SET	0x59
#define	PRESET_GOTO	0x4c

#define MAGIC_NUM	0x77

typedef unsigned char byte;
static unsigned char a01_msg[A01_MSG_LEN];

static unsigned char *a01_create_pkg(byte addr, byte data1, byte data2, byte data3, byte data4)
{
	a01_msg[0] = addr;
	a01_msg[1] = data1;
	a01_msg[2] = data2;
	a01_msg[3] = data3;
	a01_msg[4] = data4;
	return a01_msg;
}

unsigned char *a01_up(int addr, int tspd)
{
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	tspd += 0x0b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, TILT_UP, 0x00, (byte)tspd);
}

unsigned char *a01_down(int addr, int tspd)
{
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	tspd += 0x8b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, TILT_DOWN, 0x00, (byte)tspd);
}

unsigned char *a01_right(int addr, int pspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	pspd += 0x4b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, PAN_RIGHT, (byte)pspd, 0x00);
}

unsigned char *a01_left(int addr, int pspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	pspd += 0x0b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, PAN_LEFT, (byte)pspd, 0x00);
}

unsigned char *a01_left_up(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	pspd += 0x0b;
	tspd += 0x0b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, 0x33, (byte)pspd, (byte)tspd);
}

unsigned char *a01_left_down(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	pspd += 0x0b;
	tspd += 0x8b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, 0x33, (byte)pspd, (byte)tspd);
}

unsigned char *a01_right_up(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	pspd += 0x4b;
	tspd += 0x0b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, 0x34, (byte)pspd, (byte)tspd);
}

unsigned char *a01_right_down(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	pspd += 0x4b;
	tspd += 0x8b;
	return a01_create_pkg((byte)addr, MAGIC_NUM, 0x34, (byte)pspd, (byte)tspd);
}

unsigned char *a01_stop(int addr)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, STOP_ALL, 0x00, 0x00);
}

unsigned char *a01_zoom_plus(int addr)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, ZOOM_PLUS, 0x00, 0x00);
}

unsigned char *a01_zoom_minus(int addr)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, ZOOM_MINUS, 0x00, 0x00);
}

unsigned char *a01_focus_plus(int addr)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, FOCUS_PLUS, 0x00, 0x00);
}

unsigned char *a01_focus_minus(int addr)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, FOCUS_MINUS, 0x00, 0x00);
}

unsigned char *a01_iris_plus(int addr)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, IRIS_PLUS, 0x00, 0x00);
}

unsigned char *a01_iris_minus(int addr)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, IRIS_MINUS, 0x00, 0x00);
}

unsigned char *a01_preset_set(int addr, int preno)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, PRESET_SET, (byte)preno, 0x00);
}

unsigned char *a01_preset_goto(int addr, int preno)
{
	return a01_create_pkg((byte)addr, MAGIC_NUM, PRESET_GOTO, (byte)preno, 0x00);
}
