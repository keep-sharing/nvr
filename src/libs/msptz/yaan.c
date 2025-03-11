/*
 * yaan.c
 *
 *  Created on: 2013-1-28
 *      Author: root
 */
#include <stdio.h>
#include "yaan.h"

#define CMD_START	0x02


// cmd1
#define PAN_RIGHT	0x01
#define PAN_LEFT	0x02
#define TILT_UP		0x04
#define TILT_DOWN	0x08
#define ZOOM_TELE	0x10
#define ZOOM_WIDE	0x20
#define FOCUS_NEAR	0x40
#define FOCUS_FAR	0x80

#define PAN_AUTO	0x03
#define PTZF_SET	0x01
#define	SPD_SET		0x0e
#define AUTO_SCAN	0x03
#define PRESET_SET	0x05
#define PRESET_GOTO	0x02
#define	TOUR_ADD	0x06
#define TOUR_CLEAR	0x08
#define TOUR_TIME_SET	0x0b
#define LINE_SCAN_SET	0x0f
//cmd2
#define IRIS_ON		0x01
#define IRIS_OFF	0x02

typedef unsigned char byte;

static unsigned char yaan_msg[YAAN_MSG_LEN];

static unsigned char *yaan_create_pkg(byte addr, byte cmd, byte data1, byte data2);

unsigned char *yaan_right(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, PAN_RIGHT, 0x00);
}

unsigned char *yaan_left(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, PAN_LEFT, 0x00);
}

unsigned char *yaan_up(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, TILT_UP, 0x00);
}

unsigned char *yaan_down(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, TILT_DOWN, 0x00);
}

unsigned char *yaan_right_up(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, PAN_RIGHT | TILT_UP, 0x00);
}

unsigned char *yaan_right_down(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, PAN_RIGHT | TILT_DOWN, 0x00);
}

unsigned char *yaan_left_up(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, PAN_LEFT | TILT_UP, 0x00);
}

unsigned char *yaan_left_down(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, PAN_LEFT | TILT_DOWN, 0x00);
}

unsigned char *yaan_auto_scan(int addr)
{
	return yaan_create_pkg((byte)addr, LINE_SCAN_SET, PAN_AUTO, 0x00);
}

unsigned char *yaan_zoom_plus(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, ZOOM_TELE, 0x00);
}

unsigned char *yaan_zoom_minus(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, ZOOM_WIDE, 0x00);
}

unsigned char *yaan_focus_plus(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, FOCUS_NEAR, 0x00);
}

unsigned char *yaan_focus_minus(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, FOCUS_FAR, 0x00);
}

unsigned char *yaan_iris_plus(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, 0x00, IRIS_ON);
}

unsigned char *yaan_iris_minus(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, 0x00, IRIS_OFF);
}

unsigned char *yaan_stop(int addr)
{
	return yaan_create_pkg((byte)addr, PTZF_SET, 0x00, 0x00);
}

unsigned char *yaan_set_spd(int addr, int pan, int tilt)
{
	return yaan_create_pkg((byte)addr, SPD_SET, (byte)pan, (byte)tilt);
}

unsigned char *yaan_stop_spd(int addr)
{
	return yaan_create_pkg((byte)addr, SPD_SET, 0x00, 0x00);
}

/*********************************PRESET & TOUR*****************************/
unsigned char *yaan_preset_set(int addr, int preno)
{
	return yaan_create_pkg((byte)addr, PRESET_SET, preno, 0x00);
}

unsigned char *yaan_preset_goto(int addr, int preno)
{
	return yaan_create_pkg((byte)addr, PRESET_GOTO, preno, 0x00);
}

unsigned char *yaan_tour_add(int addr, int preno)
{
	return yaan_create_pkg((byte)addr, TOUR_ADD, preno, 0x00);
}

unsigned char *yaan_tour_time_set(int addr, int time)
{
	return yaan_create_pkg((byte)addr, TOUR_TIME_SET, (byte)time, 0x00);
}

unsigned char *yaan_tour_clear(int addr)
{
	return yaan_create_pkg((byte)addr, TOUR_CLEAR, 0x00, 0x00);
}

unsigned char *yaan_tour_auto_scan(int addr)
{
	return yaan_create_pkg((byte)addr, AUTO_SCAN, 0x00, 0x00);
}

unsigned char *yaan_line_scan_set_left_lmt(int addr)
{
	return yaan_create_pkg((byte)addr, LINE_SCAN_SET, 0x02, 0x01);
}

unsigned char *yaan_line_scan_set_right_lmt(int addr)
{
	return yaan_create_pkg((byte)addr, LINE_SCAN_SET, 0x02, 0x02);
}

unsigned char *yaan_set_auto_scan_spd(int addr, int spd)
{
	return yaan_create_pkg((byte)addr, LINE_SCAN_SET, 0x01, (byte)spd);
}

static unsigned char *yaan_create_pkg(byte addr, byte cmd, byte data1, byte data2)
{
	yaan_msg[0] = CMD_START;
	yaan_msg[1] = addr;
	yaan_msg[2] = cmd;
	yaan_msg[3] = data1;
	yaan_msg[4] = data2;
	yaan_msg[5] = CMD_START + addr + cmd + data1 + data2 ;
	return yaan_msg;
}
/*********************************PRESET & TOUR END*************************/
