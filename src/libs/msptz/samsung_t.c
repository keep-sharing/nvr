/*
 * samsung_t.c
 *
 *  Created on: 2012-11-30
 *      Author: chimmu
 */

//Important!: The Zoom and Focus commands cannot be transmitted simultaneously.

#include "samsung_t.h"

#define PTZ_STX			0xa0
#define PTZ_ETX			0xaf
#define HOST_ID			0x00

//cmd1
#define FOCUS_FAR		0x01
#define FOCUS_NEAR		0x02
#define IRIS_OFF		0x08
#define IRIS_ON         0x10

//cmd2
#define PAN_RIGHT		0x02
#define PAN_LEFT		0x04
#define	TILT_UP			0x08
#define TILT_DOWN		0x10
#define	ZOOM_TELE		0x20
#define ZOOM_WIDE		0x40
#define MENU_MODE		0xb1
#define PRESET_SET		0x03
#define PRESET_GOTO		0x07
#define PRESET_CLEAR	0x05

//data1
#define PAN_SPEED_MAX	0x3f
#define PAN_SPEED_MIN	0x00
#define MENU_ON			0x00
#define MENU_OFF		0x01
#define MENU_ENTER		0x06
#define MENU_CANCEL		0x07

//data2
#define TILT_SPEED_MAX	0x3f
#define TILT_SPEED_MIN	0x00

//data3
#define ZOOM_SPEED_MAX	0x08
#define ZOOM_SPEED_MIN	0x01
#define FOCUS_SPEED_MAX	0x80
#define FOCUS_SPEED_MIN	0x10

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef unsigned char BYTE;
unsigned char SamsungT_Msg[SAMSUNG_T_MSG_LEN];

static unsigned char *SamsungT_Create(unsigned char addr, unsigned char cmd1, unsigned char cmd2,
                                unsigned char data1, unsigned char data2,unsigned char data3, unsigned char data4);

unsigned char *SamsungT_Left(int addr, int speed)
{
	if (speed < PAN_SPEED_MIN)
		speed = PAN_SPEED_MIN;
	else if (speed > PAN_SPEED_MAX)
		speed = PAN_SPEED_MAX;
	return SamsungT_Create((BYTE)addr, 0x00, PAN_LEFT, (BYTE)speed, 0x00, 0x00,0x00);
}

unsigned char *SamsungT_Right(int addr, int speed)
{
	if (speed < PAN_SPEED_MIN)
		speed = PAN_SPEED_MIN;
	else if (speed > PAN_SPEED_MAX)
		speed = PAN_SPEED_MAX;
	return SamsungT_Create((BYTE)addr, 0x00, PAN_RIGHT, (BYTE)speed, 0x00, 0x00,0x00);
}

unsigned char *SamsungT_Up(int addr, int speed)
{
	if (speed < TILT_SPEED_MIN)
		speed = TILT_SPEED_MIN;
	else if (speed > TILT_SPEED_MAX)
		speed = TILT_SPEED_MAX;
	return SamsungT_Create((BYTE)addr, 0x00, TILT_UP, 0x00, (BYTE)speed, 0x00,0x00);
}

unsigned char *SamsungT_Down(int addr, int speed)
{
	if (speed < TILT_SPEED_MIN)
		speed = TILT_SPEED_MIN;
	else if (speed > TILT_SPEED_MAX)
		speed = TILT_SPEED_MAX;
	return SamsungT_Create((BYTE)addr, 0x00, TILT_DOWN, 0x00, (BYTE)speed, 0x00,0x00);
}

unsigned char *SamsungT_TopRight(int addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN : pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX : pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungT_Create((BYTE)addr, 0x00, TILT_UP | PAN_RIGHT, (BYTE)pspeed, (BYTE)tspeed, 0x00,0x00);
}

unsigned char *SamsungT_TopLeft(int addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN : pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX : pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungT_Create((BYTE)addr, 0x00, TILT_UP | PAN_LEFT, (BYTE)pspeed, (BYTE)tspeed, 0x00,0x00);
}

unsigned char *SamsungT_BottomLeft(int addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN : pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX : pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungT_Create((BYTE)addr, 0x00, TILT_DOWN | PAN_LEFT, (BYTE)pspeed, (BYTE)tspeed, 0x00,0x00);
}

unsigned char *SamsungT_BottomRight(int addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN : pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX : pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungT_Create((BYTE)addr, 0x00, TILT_DOWN | PAN_RIGHT, (BYTE)pspeed, (BYTE)tspeed, 0x00,0x00);
}

unsigned char *SamsungT_Noop(int addr)
{
	return SamsungT_Create((BYTE)addr, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_ZoomPlus(int addr, int zmspd)
{
    return SamsungT_Create((BYTE)addr, 0x00, ZOOM_TELE, 0x00, 0x00, (BYTE)zmspd, 0x00);
}

unsigned char *SamsungT_ZoomMinus(int addr, int zmspd)
{
    return SamsungT_Create((BYTE)addr, 0x00, ZOOM_WIDE, 0x00, 0x00, (BYTE)zmspd, 0x00);
}

unsigned char *SamsungT_IrisPlus(int addr)
{
    return SamsungT_Create((BYTE)addr, IRIS_ON, 0x00, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_IrisMinus(int addr)
{
    return SamsungT_Create((BYTE)addr, IRIS_OFF, 0x00, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_FocusPlus(int addr, int fcspd)
{
    return SamsungT_Create((BYTE)addr, FOCUS_NEAR, 0x00, 0x00, 0x00, (BYTE)fcspd, 0x00);
}

unsigned char *SamsungT_FocusMinus(int addr, int fcspd)
{
    return SamsungT_Create((BYTE)addr, FOCUS_FAR, 0x00, 0x00, 0x00, (BYTE)fcspd, 0x00);
}

unsigned char *SamsungT_PresetSet(int addr, int pre_no)
{
    return SamsungT_Create((BYTE)addr, 0x00, PRESET_SET, (BYTE)pre_no, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_PresetGoto(int addr, int pre_no)
{
    return SamsungT_Create((BYTE)addr, 0x00, PRESET_GOTO, (BYTE)pre_no, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_PresetClear(int addr, int pre_no)
{
    return SamsungT_Create((BYTE)addr, 0x00, PRESET_CLEAR, (BYTE)pre_no, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_MenuOn(int addr)
{
    return SamsungT_Create((BYTE)addr, 0x00, MENU_MODE, MENU_ON, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_MenuOff(int addr)
{
    return SamsungT_Create((BYTE)addr, 0x00, MENU_MODE, MENU_OFF, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_MenuEnter(int addr)
{
    return SamsungT_Create((BYTE)addr, 0x01, 0x00, MENU_ENTER, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_MenuCancel(int addr)
{
    return SamsungT_Create((BYTE)addr, 0x02, 0x00, MENU_CANCEL, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_PanTiltPosInit(int addr)
{
    return SamsungT_Create((BYTE)addr, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

unsigned char *SamsungT_ZoomPosInit(int addr)
{
    return SamsungT_Create((BYTE)addr, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00);
}


static unsigned char *SamsungT_Create(unsigned char addr, unsigned char cmd1, unsigned char cmd2,
                                unsigned char data1, unsigned char data2,unsigned char data3, unsigned char data4)
{
	SamsungT_Msg[0] = PTZ_STX;
	SamsungT_Msg[1] = addr;
	SamsungT_Msg[2] = HOST_ID;
	SamsungT_Msg[3] = cmd1;
	SamsungT_Msg[4] = cmd2;
	SamsungT_Msg[5] = data1;
	SamsungT_Msg[6] = data2;
	SamsungT_Msg[7] = data3;
	SamsungT_Msg[8] = data4;
	SamsungT_Msg[9] = PTZ_ETX;

    SamsungT_Msg[10] = ~((addr+HOST_ID+cmd1+cmd2+data1+data2+data3+data4) & 0xff);
	return SamsungT_Msg;
}

