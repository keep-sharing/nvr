/*
 * lilin.c
 *
 *  Created on: 2013-1-31
 *      Author: root
 */
#include "lilin.h"

typedef unsigned char byte;

static unsigned char lilin_msg[LILIN_MSG_LEN];

static unsigned char *lilin_create_pkg(byte addr, byte data1, byte data2)
{
	lilin_msg[0] = addr;
	lilin_msg[1] = data1;
	lilin_msg[2] = data2;
	return lilin_msg;
}

unsigned char *lilin_right(int addr)
{
	return lilin_create_pkg((byte)addr, 0x01, 0x83);
}

unsigned char *lilin_left(int addr)
{
	return lilin_create_pkg((byte)addr, 0x02, 0x83);
}

unsigned char *lilin_up(int addr)
{
	return lilin_create_pkg((byte)addr, 0x04, 0xb8);
}

unsigned char *lilin_down(int addr)
{
	return lilin_create_pkg((byte)addr, 0x08, 0xb8);
}

unsigned char *lilin_auto_scan(int addr)
{
	return lilin_create_pkg((byte)addr, 0x00, 0x0a);
}

unsigned char *lilin_stop(int addr)
{
	return lilin_create_pkg((byte)addr, 0x00, 0xff);
}

unsigned char *lilin_zoom_plus(int addr)
{
	return lilin_create_pkg((byte)addr, 0x10, 0xff);
}

unsigned char *lilin_zoom_minus(int addr)
{
	return lilin_create_pkg((byte)addr, 0x20, 0xff);
}

unsigned char *lilin_focus_plus(int addr)
{
	return lilin_create_pkg((byte)addr, 0x80, 0xff);
}

unsigned char *lilin_focus_minus(int addr)
{
	return lilin_create_pkg((byte)addr, 0x40, 0xff);
}

unsigned char *lilin_iris_plus(int addr)
{
	return lilin_create_pkg((byte)addr, 0x00, 0x00);
}

unsigned char *lilin_iris_minus(int addr)
{
	return lilin_create_pkg((byte)addr, 0x00, 0x01);
}

unsigned char *lilin_preset_set(int addr, int preno)
{
	addr += 0x80;
	return lilin_create_pkg((byte)addr, (byte)preno, 0x64);
}

unsigned char *lilin_preset_goto(int addr, int preno)
{
	addr += 0x40;
	return lilin_create_pkg((byte)addr, (byte)preno, 0x00);
}
