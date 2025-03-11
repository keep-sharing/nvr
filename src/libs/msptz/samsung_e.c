/*
 * samsung.c
 *
 *  Created on: 2012-11-30
 *      Author: chimmu
 */

#include "samsung_e.h"

#define STX_CODE 		0xa0


//cmd1
#define CMD_ENABLE		0x01
#define SYS_CMD			0x03
#define REQ_MOD_NAME	0xfc

//cmd2
#define FOCUS_FAR		0x01
#define FOCUS_NEAR		0x02
#define ONE_AF			0x04
#define IRIS_OPEN		0x08
#define IRIS_CLOSE		0x10
#define ZOOM_TELE		0x20
#define ZOOM_WIDE		0x40
#define ZOOM_SPEED_ON	0x80

#define PRESET_SET		0x50
#define PRESET_GOTO		0x19
#define PRESET_DELETE	0x51

#define PATTERN_MODE	0x1b
#define AUTOPAN_MODE	0x1a
#define	SCAN_MODE		0x13
#define MENU_MODE		0x17
#define RESET_MODE		0x1e

//cmd3
#define PAN_LEFT		0x01
#define PAN_RIGHT		0x02
#define TILT_UP			0x04
#define TILT_DOWN		0x08
#define ZOOM_SPEED_MIN	0x11
#define ZOOM_SPEED_MAX	0x14

#define AUTOPAN_START	0x01
#define AUTOPAN_STOP	0x00
#define SCAN_START		0x01
#define SCAN_STOP		0x00
#define MENU_ON			0x01
#define MENU_OFF		0x00

//cmd4
#define PAN_SPEED_MIN	0x00
#define PAN_SPEED_MAX	0x40
#define PATTERN_START	0x01
#define PATTERN_STOP	0x00

#define AUTOPAN_NO_MAX	0x04
#define AUTOPAN_NO_MIN	0x01
#define SCAN_NO_MAX		0x04
#define SCAN_NO_MIN		0x01

//cmd5
#define TILT_SPEED_MIN	0x00
#define TILT_SPEED_MAX	0x40

#define PTZ_STOP		0x00
#define PATTERN_NO_MAX	0x03
#define PATTERN_NO_MIN	0x01

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef unsigned char BYTE;
unsigned char SamsungE_Msg[SAMSUNG_E_MSG_LEN];

static unsigned char *SamsungE_Msg_Create(unsigned char sender_addr, unsigned char target_addr, unsigned char cmd1,
									unsigned char cmd2, unsigned char cmd3,
									unsigned char cmd4, unsigned char cmd5);

unsigned char *SamsungE_Left(int sender_addr, int target_addr, int speed)
{
	if (speed < PAN_SPEED_MIN)
		speed = PAN_SPEED_MIN;
	else if (speed > PAN_SPEED_MAX)
		speed = PAN_SPEED_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  PAN_LEFT, (BYTE)speed, 0xff);
}

unsigned char *SamsungE_Right(int sender_addr, int target_addr, int speed)
{
	if (speed < PAN_SPEED_MIN)
		speed = PAN_SPEED_MIN;
	else if (speed > PAN_SPEED_MAX)
		speed = PAN_SPEED_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  PAN_RIGHT, (BYTE)speed, 0xff);
}

unsigned char *SamsungE_Up(int sender_addr, int target_addr, int speed)
{
	if (speed < PAN_SPEED_MIN)
		speed = PAN_SPEED_MIN;
	else if (speed > PAN_SPEED_MAX)
		speed = PAN_SPEED_MAX;
    return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  TILT_UP, 0xff, (BYTE)speed);
}

unsigned char *SamsungE_Down(int sender_addr, int target_addr, int speed)
{
	if (speed < PAN_SPEED_MIN)
		speed = PAN_SPEED_MIN;
	else if (speed > PAN_SPEED_MAX)
		speed = PAN_SPEED_MAX;
    return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  TILT_DOWN, 0xff, (BYTE)speed);
}

unsigned char *SamsungE_TopLeft(int sender_addr, int target_addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN: pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX: pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  PAN_LEFT | TILT_UP, (BYTE)pspeed, (BYTE)tspeed);
}

unsigned char *SamsungE_BottomLeft(int sender_addr, int target_addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN: pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX: pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  PAN_LEFT | TILT_DOWN, (BYTE)pspeed, (BYTE)tspeed);
}

unsigned char *SamsungE_TopRight(int sender_addr, int target_addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN: pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX: pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  PAN_RIGHT | TILT_UP, (BYTE)pspeed,(BYTE)tspeed);
}

unsigned char *SamsungE_BottomRight(int sender_addr, int target_addr, int pspeed, int tspeed)
{
	pspeed = pspeed < PAN_SPEED_MIN ? PAN_SPEED_MIN: pspeed;
	pspeed = pspeed > PAN_SPEED_MAX ? PAN_SPEED_MAX: pspeed;
	tspeed = tspeed < TILT_SPEED_MIN ? TILT_SPEED_MIN : tspeed;
	tspeed = tspeed > TILT_SPEED_MAX ? TILT_SPEED_MAX : tspeed;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  PAN_RIGHT | TILT_DOWN, (BYTE)pspeed, (BYTE)tspeed);
}

unsigned char *SamsungE_AutoScan(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, 0xff, 0xff, 0xff, 0xff, 0xff);
}

unsigned char *SamsungE_Noop(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x00,  0x00, 0xff, 0xff);
}

unsigned char *SamsungE_FocusPlus(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, FOCUS_NEAR,  0x00, 0xff, 0xff);
}

unsigned char *SamsungE_FocusMinus(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, FOCUS_FAR,  0x00, 0xff, 0xff);
}

unsigned char *SamsungE_IrisPlus(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, IRIS_OPEN,  0x00, 0xff, 0xff);
}

unsigned char *SamsungE_IrisMinus(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, IRIS_CLOSE,  0x00, 0xff, 0xff);
}

unsigned char *SamsungE_ZoomPlus(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, ZOOM_TELE,  0x00, 0xff, 0xff);
}

unsigned char *SamsungE_ZoomMinus(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, ZOOM_WIDE,  0x00, 0xff, 0xff);
}

unsigned char *SamsungE_PresetSet(int sender_addr, int target_addr, int preset_no)
{

	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, PRESET_SET, (BYTE)(preset_no - 1), 0xff, 0xff);
}

unsigned char *SamsungE_PresetGoto(int sender_addr, int target_addr, int preset_no)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, PRESET_GOTO, (BYTE)(preset_no - 1), 0xff, 0xff);
}

unsigned char *SamsungE_PresetClear(int sender_addr, int target_addr, int preset_no)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, PRESET_DELETE, (BYTE)(preset_no - 1), 0xff, 0xff);
}


unsigned char *SamsungE_PatternStart(int send_addr, int target_addr, int pattern_no)
{
	if (pattern_no < PATTERN_NO_MIN)
		pattern_no = PATTERN_NO_MIN;
	else if (pattern_no > PATTERN_NO_MAX)
		pattern_no = PATTERN_NO_MAX;
	return SamsungE_Msg_Create((BYTE)send_addr, (BYTE)target_addr, 0xff, 0xff, 0xff, 0xff, 0xff);
}

unsigned char *SamsungE_PatternRun(int sender_addr, int target_addr, int pattern_no)
{
	if (pattern_no < PATTERN_NO_MIN)
		pattern_no = PATTERN_NO_MIN;
	else if (pattern_no > PATTERN_NO_MAX)
		pattern_no = PATTERN_NO_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, PATTERN_MODE, (BYTE)pattern_no, PATTERN_START, 0xff);
}

unsigned char *SamsungE_PatternStop(int sender_addr, int target_addr, int pattern_no)
{
	if (pattern_no < PATTERN_NO_MIN)
		pattern_no = PATTERN_NO_MIN;
	else if (pattern_no > PATTERN_NO_MAX)
		pattern_no = PATTERN_NO_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, PATTERN_MODE, (BYTE)pattern_no, PATTERN_STOP, 0xff);
}

unsigned char *SamsungE_AutoPanStart(int sender_addr, int target_addr, int auto_pan_no)
{
	if (auto_pan_no < AUTOPAN_NO_MIN)
		auto_pan_no = AUTOPAN_NO_MIN;
	else if (auto_pan_no > AUTOPAN_NO_MAX)
		auto_pan_no = AUTOPAN_NO_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, AUTOPAN_MODE, AUTOPAN_START, (BYTE)auto_pan_no, 0xff);
}

unsigned char *SamsungE_AutoPanStop(int sender_addr, int target_addr, int auto_pan_no)
{
	if (auto_pan_no < AUTOPAN_NO_MIN)
		auto_pan_no = AUTOPAN_NO_MIN;
	else if (auto_pan_no > AUTOPAN_NO_MAX)
		auto_pan_no = AUTOPAN_NO_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, AUTOPAN_MODE, AUTOPAN_STOP, (BYTE)auto_pan_no, 0xff);
}

unsigned char *SamsungE_TourRun(int sender_addr, int target_addr, int scan_no)
{
	if (scan_no < SCAN_NO_MIN)
		scan_no = SCAN_NO_MIN;
	else if (scan_no > SCAN_NO_MAX)
		scan_no = SCAN_NO_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, SCAN_MODE, SCAN_START, (BYTE)scan_no, 0xff);
}

unsigned char *SamsungE_TourStop(int sender_addr, int target_addr, int scan_no)
{
	if (scan_no < SCAN_NO_MIN)
		scan_no = SCAN_NO_MIN;
	else if (scan_no > SCAN_NO_MAX)
		scan_no = SCAN_NO_MAX;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, SCAN_MODE, SCAN_STOP, (BYTE)scan_no, 0xff);
}

unsigned char *SamsungE_SetMenuOn(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, MENU_MODE, MENU_ON, 0xff, 0xff);
}

unsigned char *SamsungE_SetMenuOff(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, MENU_MODE, MENU_OFF, 0xff, 0xff);
}

unsigned char *SamsungE_Reset(int sender_addr, int target_addr)
{
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, SYS_CMD, RESET_MODE, 0xff, 0xff, 0xff);
}

unsigned char *SamsungE_SetZoomSpd(int sender_addr, int target_addr, int zm_spd)
{
	zm_spd = zm_spd < 1 ? 1 : zm_spd;
	zm_spd = zm_spd > 4 ? 4 : zm_spd;
	return SamsungE_Msg_Create((BYTE)sender_addr, (BYTE)target_addr, CMD_ENABLE, 0x80, (BYTE)(zm_spd << 3), 0x00, 0x00);
}
static unsigned char *SamsungE_Msg_Create(unsigned char sender_addr, unsigned char target_addr, unsigned char cmd1,
									unsigned char cmd2, unsigned char cmd3,
									unsigned char cmd4, unsigned char cmd5)
{
    unsigned int sum;
	SamsungE_Msg[0] = STX_CODE;
	SamsungE_Msg[1] = sender_addr;
	SamsungE_Msg[2] = target_addr;
	SamsungE_Msg[3] = cmd1;
	SamsungE_Msg[4] = cmd2;
	SamsungE_Msg[5] = cmd3;
	SamsungE_Msg[6] = cmd4;
	SamsungE_Msg[7] = cmd5;

    sum = 0xffff - sender_addr - target_addr - cmd1 - cmd2 - cmd3 - cmd4 -cmd5;
    SamsungE_Msg[8] = (BYTE)sum;
	return SamsungE_Msg;
}

