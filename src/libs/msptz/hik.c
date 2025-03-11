/*
 * hik.c
 *
 *  Created on: 2013-1-30
 *      Author: root
 */

#include "hik.h"

#define TILT_UP		0x06
#define TILT_DOWN	0x07
#define PAN_RIGHT	0x08
#define PAN_LEFT	0x09
#define LEFT_UP		0x0a
#define RIGHT_UP	0x0c
#define RIGHT_DOWN	0x0d
#define	LEFT_DOWN	0x0b
#define ZOOM_PLUS	0x13
#define ZOOM_MINUS	0x12
#define FOCUS_PLUS	0x10
#define FOCUS_MINUS	0x11
#define IRIS_PLUS	0x0f
#define IRIS_MINUS	0x0e
#define AUTO_SCAN	0x63
#define STOP_ALL	0x14
#define PRESET_SET	0x15
#define PRESET_GOTO	0x17
#define PATTERN_START	0x18
#define PATTERN_STOP	0x19
#define PATTERN_RUN		0x1a
#define MENU_ON		0x5f


#define STX 0xb5
typedef unsigned char byte;
static unsigned char hik_msg[HIK_MSG_LEN];

static unsigned char *hik_create_pkg(byte addr, byte data1, byte data2, byte data3, byte data4, byte data5)
{
	hik_msg[0] = STX;
	hik_msg[1] = addr;
	hik_msg[2] = data1;
	hik_msg[3] = data2;
	hik_msg[4] = data3;
	hik_msg[5] = data4;
	hik_msg[6] = data5;
	hik_msg[7] = STX + addr + data1 + data2 + data3 + data4 + data5;
	return hik_msg;
}

unsigned char *hik_up(int addr, int tspd)
{
	return hik_create_pkg((byte)addr, TILT_UP, (byte)tspd, 0x00, 0x00, 0x00);
}

unsigned char *hik_down(int addr, int tspd)
{
	return hik_create_pkg((byte)addr, TILT_DOWN, (byte)tspd, 0x00, 0x00, 0x00);
}

unsigned char *hik_left(int addr, int pspd)
{
	return hik_create_pkg((byte)addr, PAN_LEFT, 0x00, (byte)pspd, 0x00, 0x00);
}

unsigned char *hik_right(int addr, int pspd)
{
	return hik_create_pkg((byte)addr, PAN_RIGHT, 0x00, (byte)pspd, 0x00, 0x00);
}

unsigned char *hik_left_up(int addr, int pspd, int tspd)
{
	return hik_create_pkg((byte)addr, LEFT_UP, (byte)tspd, (byte)pspd, 0x00, 0x00);
}

unsigned char *hik_right_up(int addr, int pspd, int tspd)
{
	return hik_create_pkg((byte)addr, RIGHT_UP, (byte)tspd, (byte)pspd, 0x00, 0x00);
}

unsigned char *hik_left_down(int addr, int pspd, int tspd)
{
	return hik_create_pkg((byte)addr, LEFT_DOWN, (byte)tspd, (byte)pspd, 0x00, 0x00);
}

unsigned char *hik_right_down(int addr, int pspd, int tspd)
{
	return hik_create_pkg((byte)addr, RIGHT_DOWN, (byte)tspd, (byte)pspd, 0x00, 0x00);
}

unsigned char *hik_stop(int addr)
{
	return hik_create_pkg((byte)addr, STOP_ALL, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_auto_scan(int addr)
{
	return hik_create_pkg((byte)addr, PRESET_GOTO, AUTO_SCAN, 0x00, 0x00, 0x00);
}

unsigned char *hik_preset_set(int addr, int preno)
{
	return hik_create_pkg((byte)addr, PRESET_SET, (byte)preno, 0x00, 0x00, 0x00);
}

unsigned char *hik_preset_goto(int addr, int preno)
{
	return hik_create_pkg((byte)addr, PRESET_GOTO, (byte)preno, 0x00, 0x00, 0x00);
}

unsigned char *hik_menu_on(int addr)
{
	return hik_create_pkg((byte)addr, PRESET_GOTO, MENU_ON, 0x00, 0x00, 0x00);
}

unsigned char *hik_pattern_start(int addr)
{
	return hik_create_pkg((byte)addr, PATTERN_START, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_pattern_stop(int addr)
{
	return hik_create_pkg((byte)addr, PATTERN_STOP, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_pattern_run(int addr)
{
	return hik_create_pkg((byte)addr, PATTERN_RUN, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_zoom_plus(int addr)
{
	return hik_create_pkg((byte)addr, ZOOM_PLUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_zoom_minus(int addr)
{
	return hik_create_pkg((byte)addr, ZOOM_MINUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_iris_plus(int addr)
{
	return hik_create_pkg((byte)addr, IRIS_PLUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_iris_minus(int addr)
{
	return hik_create_pkg((byte)addr, IRIS_MINUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_focus_plus(int addr)
{
	return hik_create_pkg((byte)addr, FOCUS_PLUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_focus_minus(int addr)
{
	return hik_create_pkg((byte)addr, FOCUS_MINUS, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *hik_tour_clear(int addr, int preno)
{
	return hik_create_pkg((byte)addr, 0x0d, 0x01, (byte)preno, 0x00, 0x00);
}

unsigned char *hik_tour_run(int addr, int tourid)
{
	return hik_create_pkg((byte)addr, PRESET_GOTO, (byte)(0x23 + tourid), 0x00, 0x00, 0x00);
}

