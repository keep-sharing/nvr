/*
 * abd.c
 *
 *  Created on: 2013-2-1
 *      Author: root
 */

#include "abd.h"

#define STX			0xff
#define	PAN_RIGHT	0x02
#define	PAN_LEFT	0x04
#define	TILT_UP		0x08
#define	TILT_DOWN	0x10
#define ZOOM_PLUS	0x20
#define	ZOOM_MINUS	0x40
#define	FOCUS_PLUS	0x01
#define	FOCUS_MINUS	0x80
#define	IRIS_PLUS	0x02
#define	IRIS_MINUS	0x04
#define	PRESET_SET	0x03
#define	PRESET_GOTO	0x07

#define SPD_MAX	0x3f
#define PRENO_MAX	0x20

typedef unsigned char byte;
static unsigned char abd_msg[ABD_MSG_LEN];

static unsigned char *abd_create_pkg(byte addr, byte data1, byte data2, byte data3, byte data4)
{
	abd_msg[0] = STX;
	abd_msg[1] = addr;
	abd_msg[2] = data1;
	abd_msg[3] = data2;
	abd_msg[4] = data3;
	abd_msg[5] = data4;
	abd_msg[6] = addr + data1 + data2 + data3 + data4;
	return abd_msg;
}

unsigned char *abd_up(int addr, int tspd)
{
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	return abd_create_pkg((byte)addr, 0x00, TILT_UP, 0x00, (byte)tspd);
}

unsigned char *abd_down(int addr, int tspd)
{
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	return abd_create_pkg((byte)addr, 0x00, TILT_DOWN, 0x00, (byte)tspd);
}

unsigned char *abd_left(int addr, int pspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	return abd_create_pkg((byte)addr, 0x00, PAN_LEFT, (byte)pspd, 0x00);
}

unsigned char *abd_right(int addr, int pspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	return abd_create_pkg((byte)addr, 0x00, PAN_RIGHT, (byte)pspd, 0x00);
}

unsigned char *abd_left_up(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	return abd_create_pkg((byte)addr, 0x00, PAN_LEFT | TILT_UP, (byte)pspd, (byte)tspd);
}

unsigned char *abd_right_up(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	return abd_create_pkg((byte)addr, 0x00, PAN_RIGHT | TILT_UP, (byte)pspd, (byte)tspd);
}

unsigned char *abd_left_down(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	return abd_create_pkg((byte)addr, 0x00, PAN_LEFT | TILT_DOWN, (byte)pspd, (byte)tspd);
}

unsigned char *abd_right_down(int addr, int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	return abd_create_pkg((byte)addr, 0x00, PAN_RIGHT | TILT_DOWN, (byte)pspd, (byte)tspd);
}

unsigned char *abd_stop(int addr)
{
	return abd_create_pkg((byte)addr, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *abd_zoom_plus(int addr)
{
	return abd_create_pkg((byte)addr, 0x00, ZOOM_PLUS, 0x00, 0x00);
}

unsigned char *abd_zoom_minus(int addr)
{
	return abd_create_pkg((byte)addr, 0x00, ZOOM_MINUS, 0x00, 0x00);
}

unsigned char *abd_focus_plus(int addr)
{
	return abd_create_pkg((byte)addr, FOCUS_PLUS, 0x00, 0x00, 0x00);
}

unsigned char *abd_focus_minus(int addr)
{
	return abd_create_pkg((byte)addr, 0x00, FOCUS_MINUS, 0x00, 0x00);
}

unsigned char *abd_iris_plus(int addr)
{
	return abd_create_pkg((byte)addr,  IRIS_PLUS, 0x00, 0x00, 0x00);
}

unsigned char *abd_iris_minus(int addr)
{
	return abd_create_pkg((byte)addr, IRIS_MINUS, 0x00,  0x00, 0x00);
}

unsigned char *abd_preset_set(int addr, int preno)
{
	preno = preno > PRENO_MAX ? PRENO_MAX : preno;
	return abd_create_pkg((byte)addr, 0x00, PRESET_SET, 0x00, (byte)preno);
}

unsigned char *abd_preset_goto(int addr, int preno)
{
	preno = preno > PRENO_MAX ? PRENO_MAX : preno;
	return abd_create_pkg((byte)addr, 0x00, PRESET_GOTO, 0x00, (byte)preno);
}
