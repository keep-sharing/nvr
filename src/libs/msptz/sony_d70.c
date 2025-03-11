/*
 * sony_d70.c
 *
 *  Created on: 2013-1-29
 *      Author: root
 */

#include "sony_d70.h"

#define PAN_SPD_MAX		18
#define TILT_SPD_MAX	17
typedef unsigned char byte;

static unsigned char sonyd70_msg[SONY_D70_MSG_LEN];

static unsigned char *sonyd70_create_pkg (byte addr, byte data5, byte data6, byte data7, byte data8)
{
	addr = addr > 7 ? 7 : addr;
	sonyd70_msg[0] = 0x80 + addr;
	sonyd70_msg[1] = 0x01;
	sonyd70_msg[2] = 0x06;
	sonyd70_msg[3] = 0x01;
	sonyd70_msg[4] = data5;
	sonyd70_msg[5] = data6;
	sonyd70_msg[6] = data7;
	sonyd70_msg[7] = data8;
	sonyd70_msg[8] = 0xff;

	return sonyd70_msg;
}

unsigned char *sonyd70_up(int addr, int tspd)
{
	tspd = tspd > TILT_SPD_MAX ? TILT_SPD_MAX : tspd;
	return sonyd70_create_pkg((byte)addr, 0x00, (byte)tspd, 0x03, 0x01);
}

unsigned char *sonyd70_down(int addr, int tspd)
{
	tspd = tspd > TILT_SPD_MAX ? TILT_SPD_MAX : tspd;
	return sonyd70_create_pkg((byte)addr, 0x00, (byte)tspd, 0x03, 0x02);
}

unsigned char *sonyd70_left(int addr, int pspd)
{
	pspd = pspd > PAN_SPD_MAX ? PAN_SPD_MAX : pspd;
	return sonyd70_create_pkg((byte)addr, (byte)pspd, 0x00, 0x01, 0x03);
}

unsigned char *sonyd70_right(int addr, int pspd)
{
	pspd = pspd > PAN_SPD_MAX ? PAN_SPD_MAX : pspd;
	return sonyd70_create_pkg((byte)addr, (byte)pspd, 0x00, 0x02, 0x03);
}

unsigned char *sonyd70_left_up(int addr, int pspd, int tspd)
{
	pspd = pspd > PAN_SPD_MAX ? PAN_SPD_MAX : pspd;
	tspd = tspd > TILT_SPD_MAX ? TILT_SPD_MAX : tspd;
	return sonyd70_create_pkg((byte)addr, (byte)pspd, (byte)tspd, 0x01, 0x06);
}

unsigned char *sonyd70_right_up(int addr,int pspd, int tspd)
{
	pspd = pspd > PAN_SPD_MAX ? PAN_SPD_MAX : pspd;
	tspd = tspd > TILT_SPD_MAX ? TILT_SPD_MAX : tspd;
	return sonyd70_create_pkg((byte)addr, (byte)pspd, (byte)tspd, 0x01, 0x05);
}

unsigned char *sonyd70_left_down(int addr, int pspd, int tspd)
{
	pspd = pspd > PAN_SPD_MAX ? PAN_SPD_MAX : pspd;
	tspd = tspd > TILT_SPD_MAX ? TILT_SPD_MAX : tspd;
	return sonyd70_create_pkg((byte)addr, (byte)pspd, (byte)tspd, 0x01, 0x0a);
}

unsigned char *sonyd70_right_down(int addr, int pspd, int tspd)
{
	pspd = pspd > PAN_SPD_MAX ? PAN_SPD_MAX : pspd;
	tspd = tspd > TILT_SPD_MAX ? TILT_SPD_MAX : tspd;
	return sonyd70_create_pkg((byte)addr, (byte)pspd, (byte)tspd, 0x01, 0x09);
}

unsigned char *sonyd70_stop(int addr)
{
	return sonyd70_create_pkg((byte)addr, 0x00, 0x00, 0x03, 0x03);
}
