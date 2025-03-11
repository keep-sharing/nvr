#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pelco_p.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef unsigned char BYTE;


#define PELCO_P_STX	0xA0
#define PELCO_P_ETX	0xAF

#define PAN_MIN_SPEED	0x00
#define PAN_MAX_SPEED	0x40
#define TILT_MIN_SPEED	0x00
#define TILT_MAX_SPEED	0x3F

// Pan and Tilt Commands
// Data1
#define FOCUS_FAR	0x01
#define FOCUS_NEAR	0x02
#define IRIS_OPEN	0x04
#define IRIS_CLOSE	0x08
#define CAMERA_ONOFF	0x10
#define AUTO_SCAN	0x63
#define CAMERA_ON	0x40

// Data2
#define PAN_RIGHT	0x02
#define PAN_LEFT	0x04
#define TILT_UP		0x08
#define TILT_DOWN	0x10
#define ZOOM_TELE	0x20
#define ZOOM_WIDE	0x40
#define AUX_ACTION_SET		0x09
#define AUX_ACTION_CLEAR	0x0B

#define PRESET_SET		0x03
#define PRESET_CLEAR	0x05
#define PRESET_GOTO		0x07
#define PATTERN_START	0x1F
#define PATTERN_STOP	0x21
#define PATTERN_RUN		0x23
#define REMOTE_RESET	0x0f
#define ZONE_START		0x11
#define ZONE_END		0x13
#define ZONE_SCAN_ON	0x1b
#define ZONE_SCAN_OFF	0x1d

#define FLIP_Byte2      0x07
#define FLIP_Byte6      0x21
#define GOTO_ZERO_POS	0x22
#define SET_ZERO_POS	0x49

#define PRE_RANDOM_SCAN	0x61
#define PRE_AUTO_SCAN	0x63
#define PRE_SCAN_STOP	0x60
#define PRE_LEFT_LMT	0x33
#define PRE_RIGHT_LMT	0x34
#define PRE_MENU_ON		0x5f

#define TOUR_START		0x4d
#define TOUR_END		0x4b
#define TOUR_ADD		0x49
#define TOUR_RUN		0x4f
#define TOUR_SCAN		0x47


static unsigned char PelcoP_Message[PELCO_P_MSG_LEN] = {0};

static unsigned char* PelcoP_CreatePackage(unsigned char Address, unsigned char Data1,
									unsigned char Data2, unsigned char Data3,
									unsigned char Data4);

unsigned char* PelcoP_Up(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < TILT_MIN_SPEED)
		nSpeed = TILT_MIN_SPEED;
	else if (nSpeed > TILT_MAX_SPEED)
		nSpeed = TILT_MAX_SPEED;
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_UP, 0x00, (BYTE)nSpeed);
}

unsigned char* PelcoP_Down(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < TILT_MIN_SPEED)
		nSpeed = TILT_MIN_SPEED;
	else if (nSpeed > TILT_MAX_SPEED)
		nSpeed = TILT_MAX_SPEED;
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_DOWN, 0x00,  (BYTE)nSpeed);
}

unsigned char* PelcoP_Left(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < PAN_MIN_SPEED)
		nSpeed = PAN_MIN_SPEED;
	else if (nSpeed > PAN_MAX_SPEED)
		nSpeed = PAN_MAX_SPEED;
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_LEFT, (BYTE)nSpeed, 0x00);
}

unsigned char* PelcoP_Right(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < PAN_MIN_SPEED)
		nSpeed = PAN_MIN_SPEED;
	else if (nSpeed > PAN_MAX_SPEED)
		nSpeed = PAN_MAX_SPEED;
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_RIGHT, (BYTE)nSpeed, 0x00);
}

unsigned char* PelcoP_TopLeft(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed < max(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed > min(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_UP|PAN_LEFT, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoP_TopRight(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed < max(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed > min(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_UP|PAN_RIGHT, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoP_BottomLeft(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed < max(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed > min(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_DOWN|PAN_LEFT, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoP_BottomRight(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed < max(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed > min(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_DOWN|PAN_RIGHT, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoP_AutoScan(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, AUTO_SCAN);
}


unsigned char* PelcoP_IrisPlus(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, IRIS_OPEN, 0x00, 0x00, 0x00);
}

unsigned char* PelcoP_IrisMinus(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, IRIS_CLOSE, 0x00, 0x00, 0x00);
}

unsigned char* PelcoP_FocusPlus(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, FOCUS_NEAR, 0x00, 0x00, 0x00);
}

unsigned char* PelcoP_FocusMinus(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, FOCUS_FAR, 0x00, 0x00, 0x00);
}

unsigned char* PelcoP_ZoomPlus(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, ZOOM_WIDE, 0x00, 0x00);
}

unsigned char* PelcoP_ZoomMinus(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, ZOOM_TELE, 0x00, 0x00);
}

unsigned char* PelcoP_LightOn(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x03);//auxiliaryID light 0x03  brush 0x04
	//return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x02);
}

unsigned char* PelcoP_LightOff(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x03);//auxiliaryID light 0x03  brush 0x04
	//return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x02);
}

unsigned char* PelcoP_BrushOn(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x04);//auxiliaryID light 0x03  brush 0x04
	//return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x03);
}

unsigned char* PelcoP_BrushOff(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x04);//auxiliaryID light 0x03  brush 0x04
	//return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x03);
}

unsigned char* PelcoP_Noop(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, 0x00, 0x00, 0x00);//auxiliaryID light 0x03  brush 0x04
}

unsigned char* PelcoP_PresetSet(int nDeviceAddress, int nPresetNO)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_SET, 0x00, (BYTE)nPresetNO);
}

unsigned char* PelcoP_PresetClear(int nDeviceAddress, int nPresetNO)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_CLEAR, 0x00, (BYTE)nPresetNO);
}

unsigned char* PelcoP_PresetGoto(int nDeviceAddress, int nPresetNO)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, (BYTE)nPresetNO);
}


unsigned char* PelcoP_PatternStart(int nDeviceAddress, int nPatid)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PATTERN_START, 0x00, nPatid);
}

unsigned char* PelcoP_PatternStop(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PATTERN_STOP, 0x00, 0x00);
}

unsigned char* PelcoP_PatternRun(int nDeviceAddress, int nPatid)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PATTERN_RUN, 0x00, nPatid);
}

unsigned char* PelcoP_Flip(int nDeviceAddress)
{
    return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, FLIP_Byte2, 0x00, FLIP_Byte6);
}

unsigned char *PelcoP_RemoteRst(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, REMOTE_RESET, 0x00, 0x00);
}

unsigned char *PelcoP_ZoneStart(int nDeviceAddress, int nZone)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_START, 0x00, nZone);
}

unsigned char *PelcoP_ZoneEnd(int nDeviceAddress, int nZone)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_END, 0x00, nZone);
}

unsigned char *PelcoP_ZoneScanOn(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_SCAN_ON, 0x00, 0x00);
}

unsigned char *PelcoP_ZoneScanOff(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_SCAN_OFF, 0x00, 0x00);
}

unsigned char *PelcoP_PresetAutoScanStart(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRE_AUTO_SCAN);
}

unsigned char *PelcoP_PresetRandomScan(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRE_RANDOM_SCAN);
}

unsigned char *PelcoP_PresetScanStop(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRE_SCAN_STOP);
}

unsigned char *PelcoP_PresetScanSetLeftLimit(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRE_LEFT_LMT);
}

unsigned char *PelcoP_PresetScanSetRightLimit(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRE_RIGHT_LMT);
}

unsigned char *PelcoP_MenuOn(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRE_MENU_ON);
}

unsigned char *PelcoP_GotoZeroPos(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, GOTO_ZERO_POS);
}

unsigned char *PelcoP_SetZeroPos(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, SET_ZERO_POS, 0x00, 0x00);
}

/***************************************PRESET TOUR******************************************/
unsigned char *PelcoP_PresetTourOne(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x20);
}

unsigned char *PelcoP_PresetTourTwo(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x35);
}

unsigned char *PelcoP_PresetTourThree(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x31);
}

/***************************************PRESET TOUR END**************************************/

/**************************************HIK PRESET TOUR**************************************/

unsigned char *PelcoP_PresetTourStart(int nDeviceAddress, int tourID)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TOUR_START, 0x00, tourID);
}

unsigned char *PelcoP_PresetTourEnd(int nDeviceAddress)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TOUR_END, 0x00, 0x00);
}

unsigned char *PelcoP_PresetTourAdd(int nDeviceAddress, int nPreno, int nSpd, int nTime)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, nPreno,TOUR_ADD, nSpd, nTime);
}

unsigned char *PelcoP_PresetScanOn(int nDeviceAddress, int tourID)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, 0x47, 0x00, tourID);
}

unsigned char *PelcoP_PresetTourRun(int nDeviceAddress, int tourID)
{
	return PelcoP_CreatePackage((BYTE)nDeviceAddress, 0x00, TOUR_RUN, 0x01, tourID);
}

/**************************************HIK PRESET TOUR END**********************************/

static unsigned char* PelcoP_CreatePackage(unsigned char Address, unsigned char Data1, unsigned char Data2, unsigned char Data3, unsigned char Data4)
{
	PelcoP_Message[0] = PELCO_P_STX;
	PelcoP_Message[1] = Address;
	PelcoP_Message[2] = Data1;
	PelcoP_Message[3] = Data2;
	PelcoP_Message[4] = Data3;
	PelcoP_Message[5] = Data4;
	PelcoP_Message[6] = PELCO_P_ETX;
    PelcoP_Message[7] = (BYTE)(PELCO_P_STX ^Address ^ Data1 ^ Data2 ^ Data3 ^ Data4^PELCO_P_ETX);
//	PelcoP_Message[7] = (BYTE)(Address ^ Data1 ^ Data2 ^ Data3 ^ Data4);
	return PelcoP_Message;
}
