/*
 * panasonic.c
 *
 *  Created on: 2013-1-28
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "panasonic.h"

#define STX	0x02
#define ETX 0x03

#define SPD_MAX		7
#define PAN_RIGHT	"202136C:2022xx0"//xx是速度
#define PAN_LEFT	"2021368:2022xx0"
#define TILT_UP		"202136A:2022xx0"
#define TILT_DOWN	"202136E:2022xx0"
#define LEFT_UP		"2021369:2022xx0"
#define RIGHT_UP	"202136B:2022xx0"
#define LEFT_DOWN	"202136F:2022xx0"
#define RIGHT_DOWN	"202136D:2022xx0"
#define STOP_ALL	"2021364:2022xx0"
#define ZOOM_PLUS	"2021268"
#define ZOOM_MINUS	"202126C"
#define FOCUS_FAR	"202126A"
#define FOCUS_NEAR	"202126E"
#define IRIS_PLUS	"0021002"
#define IRIS_MINUS	"0021003"

typedef unsigned char byte;
static unsigned char pana_msg[PANA_MSG_LEN];

static void deal_with_head(void)
{
	pana_msg[0] = STX;
	pana_msg[1] = 'A';
	pana_msg[2] = 'D';
	pana_msg[5] = ';';
	pana_msg[6] = 'G';
	pana_msg[7] = 'C';
	pana_msg[8] = 'F';
	pana_msg[9] = ':';
	pana_msg[PANA_MSG_LEN - 1] = ETX;
}

static void deal_with_spd(int pspd, int tspd)
{
	pspd = pspd > SPD_MAX ? SPD_MAX : pspd;
	tspd = tspd > SPD_MAX ? SPD_MAX : tspd;
	pspd += '0';
	tspd += '0';
	pana_msg[PANA_MSG_LEN - 4] = (byte)pspd;
	pana_msg[PANA_MSG_LEN - 3] = (byte)tspd;
}

static void deal_with_addr(int addr)
{
	if (addr >= 10) {
		pana_msg[3] = addr/10 + '0';
		addr /= 10;
	}
	pana_msg[4] = addr + '0';
}

static void deal_with_short(void)
{
	pana_msg[8] = '7';
	pana_msg[PANA_MSG_LEN_SHORT - 1] = ETX;
}

unsigned char *pana_up(int addr, int tspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)TILT_UP, strlen(TILT_UP));
	deal_with_spd(0, tspd);
	return pana_msg;
}

unsigned char *pana_down(int addr, int tspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)TILT_DOWN, strlen(TILT_DOWN));
	deal_with_spd(0, tspd);
	return pana_msg;
}

unsigned char *pana_left(int addr, int pspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)PAN_LEFT, strlen(PAN_LEFT));
	deal_with_spd(pspd, 0);
	return pana_msg;
}

unsigned char *pana_right(int addr, int pspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)PAN_RIGHT, strlen(PAN_RIGHT));
	deal_with_spd(pspd, 0);
	return pana_msg;
}

unsigned char *pana_left_up(int addr, int pspd, int tspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)LEFT_UP, strlen(LEFT_UP));
	deal_with_spd(pspd, tspd);
	return pana_msg;
}

unsigned char *pana_left_down(int addr, int pspd, int tspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)LEFT_DOWN, strlen(LEFT_DOWN));
	deal_with_spd(pspd, tspd);
	return pana_msg;
}

unsigned char *pana_right_up(int addr, int pspd, int tspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)RIGHT_UP, strlen(RIGHT_UP));
	deal_with_spd(pspd, tspd);
	return pana_msg;
}

unsigned char *pana_right_down(int addr, int pspd, int tspd)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)RIGHT_DOWN, strlen(RIGHT_DOWN));
	deal_with_spd(pspd, tspd);
	return pana_msg;
}

unsigned char *pana_stop(int addr)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)STOP_ALL, strlen(STOP_ALL));
	deal_with_spd(0, 0);
	return pana_msg;
}

unsigned char *pana_zoom_plus(int addr)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)ZOOM_PLUS, strlen(ZOOM_PLUS));
	deal_with_short();
	return pana_msg;
}

unsigned char *pana_zoom_minus(int addr)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)ZOOM_MINUS, strlen(ZOOM_MINUS));
	deal_with_short();
	return pana_msg;
}

unsigned char *pana_iris_plus(int addr)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)IRIS_PLUS, strlen(IRIS_PLUS));
	deal_with_short();
	return pana_msg;
}

unsigned char *pana_iris_minus(int addr)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)IRIS_MINUS, strlen(IRIS_MINUS));
	deal_with_short();
	return pana_msg;
}

unsigned char *pana_focus_plus(int addr)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)FOCUS_NEAR, strlen(FOCUS_NEAR));
	deal_with_short();
	return pana_msg;
}

unsigned char *pana_focus_minus(int addr)
{
	deal_with_head();
	deal_with_addr(addr);
	unsigned char *dst = pana_msg;
	strncpy((char *)dst + 10, (char *)FOCUS_FAR, strlen(FOCUS_FAR));
	deal_with_short();
	return pana_msg;
}
