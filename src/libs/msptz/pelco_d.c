#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pelco_d.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef unsigned char BYTE;

#define PELCO_D_SYNC  0xFF


#define PAN_MIN_SPEED	0x00
#define PAN_MAX_SPEED	0xFF
#define TILT_MIN_SPEED	0x00
#define TILT_MAX_SPEED	0x3F


// Pan and Tilt Commands
// Command1
#define FOCUS_NEAR	0x01
#define IRIS_OPEN		0x02
#define IRIS_CLOSE	0x04
#define CAMERA_ON_OFF	0x08
#define MANUAL_SCAN	0x10
#define AUTO_SCAN		0x90

#define PRESET_SET_CMD1 0x02


//Command2
#define PAN_RIGHT		0x02
#define PAN_LEFT		0x04
#define TILT_UP		0x08
#define TILT_DOWN		0x10
#define ZOOM_TELE		0x20
#define ZOOM_WIDE		0x40
#define FOCUS_FAR		0x80
#define AUX_ACTION_SET		0x09
#define AUX_ACTION_CLEAR		0x0B
#define PRESET_SET			0x03
#define PRESET_CLEAR		0x05
#define PRESET_GOTO		0x07
#define PATTERN_START	0x1F
#define PATTERN_STOP		0x21
#define PATTERN_RUN		0x23
#define SET_ZOOM_SPD		0x25
#define SET_FOCUS_SPD	0x27
#define SET_AUTO_FOCUS	0x2B
#define SET_AUTO_IRIS	0x2D
#define SET_AUTO_AGC		0x2F
#define SET_BKLT_COMP	0x31
#define SET_WHITE_BALANCE	0x33
#define GOTO_ZERO_PAN	0x07
#define REMOTE_RESET		0x0f
#define RESET_CAREMA		0x29
#define ALARM_ACK			0x19
#define SET_ZERO_POS		0x49
#define SET_FOCUS_SPD	0x27
#define ZONE_START		0x11
#define ZONE_END			0x13
#define ZONE_SCAN_ON		0x1b
#define ZONE_SCAN_OFF	0x1d

#define PRESET_SET_CMD2 0xff

//#define FLIP_Byte2      0x07
#define FLIP      0x21

#define PRESET_SCAN_START 0x63
#define PRESET_SCAN_STOP  0x60
#define MENU_ON	0x5f

#define LINE_SCAN_ON	0x33
#define LINE_SCAN_OFF	0x34
#define LEFT_LMT		0x33
#define RIGHT_LMT		0x34

#define TOUR_START		0x4d
#define TOUR_END		0x4b
#define TOUR_ADD		0x49
#define TOUR_RUN		0x4f
#define TOUR_SCAN		0x47

 unsigned char PelcoD_Message[PELCO_D_MSG_LEN] = {0};

 static unsigned char* PelcoD_CreatePackage(unsigned char Address, unsigned char Command1,
									unsigned char Command2, unsigned char Data1,
									unsigned char Data2);

unsigned char* PelcoD_Up(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < TILT_MIN_SPEED)
		nSpeed = TILT_MIN_SPEED;
	else if (nSpeed > TILT_MAX_SPEED)
		nSpeed = TILT_MAX_SPEED;

	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_UP, 0x00, (BYTE)nSpeed);
}

unsigned char* PelcoD_Down(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < TILT_MIN_SPEED)
		nSpeed = TILT_MIN_SPEED;
	else if (nSpeed > TILT_MAX_SPEED)
		nSpeed = TILT_MAX_SPEED;
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, TILT_DOWN, 0x00, (BYTE)nSpeed);
}

unsigned char* PelcoD_Left(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < PAN_MIN_SPEED)
		nSpeed = PAN_MIN_SPEED;
	else if (nSpeed > PAN_MAX_SPEED)
		nSpeed = PAN_MAX_SPEED;
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_LEFT, (BYTE)nSpeed, 0x00);
}

unsigned char* PelcoD_Right(int nDeviceAddress, int nSpeed)
{
	if (nSpeed < PAN_MIN_SPEED)
		nSpeed = PAN_MIN_SPEED;
	else if (nSpeed > PAN_MAX_SPEED)
		nSpeed = PAN_MAX_SPEED;
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_RIGHT, (BYTE)nSpeed, 0x00);
}

unsigned char* PelcoD_TopLeft(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed < min(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed > max(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_LEFT|TILT_UP, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoD_TopRight(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed < min(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed > max(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_RIGHT|TILT_UP, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoD_BottomLeft(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed > max(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed < min(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_LEFT|TILT_DOWN, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoD_BottomRight(int nDeviceAddress, int nSpeed, int nTiltSpd)
{
	if (nSpeed > max(PAN_MIN_SPEED, TILT_MAX_SPEED))
		nSpeed = max(PAN_MIN_SPEED, TILT_MAX_SPEED);
	else if (nSpeed < min(PAN_MAX_SPEED, TILT_MAX_SPEED))
		nSpeed = min(PAN_MAX_SPEED, TILT_MAX_SPEED);
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PAN_RIGHT|TILT_DOWN, (BYTE)nSpeed, (BYTE)nTiltSpd);
}

unsigned char* PelcoD_AutoScan(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress,AUTO_SCAN,0x00,0x00,0x00);
}

unsigned char* PelcoD_IrisPlus(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, IRIS_OPEN, 0x00, 0x00, 0x00);
}

unsigned char* PelcoD_IrisMinus(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, IRIS_CLOSE, 0x00, 0x00, 0x00);
}

unsigned char* PelcoD_FocusPlus(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, FOCUS_NEAR, 0x00, 0x00, 0x00);
}

unsigned char* PelcoD_FocusMinus(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, FOCUS_FAR, 0x00, 0x00);
}

unsigned char* PelcoD_ZoomPlus(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, ZOOM_WIDE, 0x00, 0x00);
}

unsigned char* PelcoD_ZoomMinus(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, ZOOM_TELE, 0x00, 0x00);
}

//spd range from 0 - 3
unsigned char *PelcoD_SetZoomSpd(int nDeviceAddress, int spd)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, SET_ZOOM_SPD, 0x00, spd);
}

//spd range from 0 - 3
unsigned char *PelcoD_SetFocusSpd(int nDeviceAddress, int spd)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, SET_FOCUS_SPD, 0x00, spd);
}

unsigned char *PelcoD_SetAutoFocusOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, SET_AUTO_FOCUS, 0x00, 0x02);
}

unsigned char *PelcoD_SetAutoFocusOff(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_AUTO_FOCUS, 0x00, 0x01);
}

unsigned char *PelcoD_SetAutoIrisOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_AUTO_IRIS, 0x00, 0x02);
}

unsigned char *PelcoD_SetAutoIrisOff(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_AUTO_IRIS, 0x00, 0x01);
}

unsigned char *PelcoD_SetAutoAgcOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_AUTO_AGC, 0x00, 0x02);
}

unsigned char *PelcoD_SetAutoAgcOff(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_AUTO_AGC, 0x00, 0x01);
}

unsigned char *PelcoD_SetBackLightOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_BKLT_COMP, 0x00, 0x02);
}

unsigned char *PelcoD_SetBackLightOff(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_BKLT_COMP, 0x00, 0x01);
}

unsigned char *PelcoD_SetWhiteBalanceOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_WHITE_BALANCE, 0x00, 0x02);
}

unsigned char *PelcoD_SetWhiteBalanceOff(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, SET_WHITE_BALANCE, 0x00, 0x01);
}

unsigned char *PelcoD_GotoZeroPan(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, GOTO_ZERO_PAN, 0x00, 0x22);
}

unsigned char *PelcoD_RemoteReset(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, REMOTE_RESET, 0x00, 0x00);
}

unsigned char *PelcoD_ResetCamera(int nDeviceAddress)
{
	return PelcoD_CreatePackage(nDeviceAddress, 0x00, RESET_CAREMA, 0x00, 0x00);
}

unsigned char* PelcoD_LightOn(int nDeviceAddress)
{
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x03);//auxiliaryID light 0x03  brush 0x04
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x02);
}

unsigned char* PelcoD_LightOff(int nDeviceAddress)
{
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x03);//auxiliaryID light 0x03  brush 0x04
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x02);
}

unsigned char* PelcoD_BrushOn(int nDeviceAddress)
{
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x04);//auxiliaryID light 0x03  brush 0x04
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_SET, 0x00, 0x01);
} 

unsigned char* PelcoD_BrushOff(int nDeviceAddress)
{
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x04);//auxiliaryID light 0x03  brush 0x04
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, AUX_ACTION_CLEAR, 0x00, 0x01);
}


unsigned char* PelcoD_Noop(int nDeviceAddress)
{
    return PelcoD_CreatePackage((BYTE)nDeviceAddress,0x00,0x00,0x00,0x00);
}
//changed at 11-26
unsigned char* PelcoD_PresetSet(int nDeviceAddress, int nPresetNO)
{
    unsigned char *buf;
//    if (nPresetNO <= 32)
        buf = PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_SET, 0x00, (BYTE)nPresetNO);
//    else
//        buf = PelcoD_CreatePackage((BYTE)nDeviceAddress, PRESET_SET_CMD1, PRESET_SET_CMD2, (BYTE)nPresetNO, 0x01);
    return buf;
}

unsigned char* PelcoD_PresetClear(int nDeviceAddress, int nPresetNO)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_CLEAR, 0x00, (BYTE)nPresetNO);
}

//changed at 11-26
unsigned char* PelcoD_PresetGoto(int nDeviceAddress, int nPresetNO)
{
    unsigned char* buf;
//    if (nPresetNO <= 32)
        buf = PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, (BYTE)nPresetNO);
//    else
//        buf = PelcoD_CreatePackage((BYTE)nDeviceAddress, PRESET_SET_CMD1, PRESET_SET_CMD2, (BYTE)nPresetNO, 0x00);
    return buf;
}

unsigned char* PelcoD_PatternStart(int nDeviceAddress, int nPatid)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PATTERN_START, 0x00, nPatid);
}

unsigned char* PelcoD_PatternStop(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PATTERN_STOP, 0x00, 0x00);
}

unsigned char* PelcoD_PatternRun(int nDeviceAddress, int nPatid)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PATTERN_RUN, 0x00, nPatid);
}

unsigned char* PelcoD_Flip(int nDeviceAddress)
{
    return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, FLIP);
}

//added at 11-26
unsigned char* PelcoD_PresetScanSetLeftLimit(int nDeviceAddress)
{
    //return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x5c);
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_SET, 0x00, LEFT_LMT);
}

//added at 11-26
unsigned char* PelcoD_PresetScanSetRightLimit(int nDeviceAddress)
{
    //return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x5d);
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_SET, 0x00, RIGHT_LMT);
}

//added at 11-26


//added at 11-26
unsigned char* PelcoD_PresetAutoScanStart(int nDeviceAddress)
{
    return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRESET_SCAN_START);
}
//added at 11-26
unsigned char* PelcoD_PresetScanStop(int nDeviceAddress)
{
    return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, PRESET_SCAN_STOP);
}

unsigned char *PelcoD_AlarmAck(int nDeviceAddress, int ackno)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, ALARM_ACK, 0x00, ackno);
}

unsigned char *PelcoD_SetZeroPos(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, SET_ZERO_POS, 0x00, 0x00);
}

// added at 12-24
unsigned char *PelcoD_ZoneStart(int nDeviceAddress, int nZone)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_START, 0x00, nZone);
}

unsigned char *PelcoD_ZoneEnd(int nDeviceAddress, int nZone)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_END, 0x00, nZone);
}

unsigned char *PelcoD_ZoneScanOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_SCAN_ON, 0x00, 0x00);
}

unsigned char *PelcoD_ZoneScanOff(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, ZONE_SCAN_OFF, 0x00, 0x00);
}

unsigned char *PelcoD_MenuOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, MENU_ON);
}

unsigned char *PelcoD_LineScanOn(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, LINE_SCAN_ON);
}

unsigned char *PelcoD_LineScanOff(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00,LINE_SCAN_OFF);
}

/***************************PRESET SCAN************************************************/

unsigned char *PelcoD_PresetTourOne(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x20);
}

unsigned char *PelcoD_PresetTourTwo(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x35);
}

unsigned char *PelcoD_PresetTourThree(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x31);
}

/*=====================================HIK PELCO=====================================*/

unsigned char *PelcoD_PresetTourStart(int nDeviceAddress, int tourID) //start or empty the tour
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, TOUR_START, 0x00, tourID);
}

unsigned char *PelcoD_PresetTourEnd(int nDeviceAddress)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, TOUR_END, 0x00, 0x00);
}

unsigned char *PelcoD_PresetTourAdd(int nDeviceAddress, int nPreno, int nSpd, int nTime)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, nPreno, TOUR_ADD, nSpd, nTime);
}

unsigned char* PelcoD_PresetScanOn(int nDeviceAddress, int nTourId)
{
    return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x47, 0x00, 0x00, nTourId);
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x20, 0x00, 0x00, 0x00);
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x4c);//调用76号预置位
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, 0x96, 0x00, 0x20);
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x20);
	//return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, PRESET_GOTO, 0x00, 0x33);
}

unsigned char *PelcoD_PresetTourRun(int nDeviceAddress, int tourID)
{
	return PelcoD_CreatePackage((BYTE)nDeviceAddress, 0x00, TOUR_RUN, 0x01, tourID);
}
/*=====================================HIK PELCO END=================================*/

/****************************PRESET SCAN END******************************************/
static unsigned char* PelcoD_CreatePackage(unsigned char Address, unsigned char Command1, unsigned char Command2, unsigned char Data1, unsigned char Data2)
{
	PelcoD_Message[0] = PELCO_D_SYNC;
	PelcoD_Message[1] = Address;
	PelcoD_Message[2] = Command1;
	PelcoD_Message[3] = Command2;
	PelcoD_Message[4] = Data1;
	PelcoD_Message[5] = Data2;
	PelcoD_Message[6] = (unsigned char)(Address + Command1 + Command2 + Data1 + Data2);
	return PelcoD_Message;
}
