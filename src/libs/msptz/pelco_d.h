#ifndef __MS_PELCO_D_H__
#define __MS_PELCO_D_H__

#define PELCO_D_MSG_LEN	7

#ifdef __cplusplus
extern "C"
{
#endif




	unsigned char* PelcoD_Up(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoD_Down(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoD_Left(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoD_Right(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoD_TopLeft(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoD_TopRight(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoD_BottomLeft(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoD_BottomRight(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoD_AutoScan(int nDeviceAddress);

	unsigned char* PelcoD_IrisPlus(int nDeviceAddress);
	unsigned char* PelcoD_IrisMinus(int nDeviceAddress);
	unsigned char* PelcoD_FocusPlus(int nDeviceAddress);
	unsigned char* PelcoD_FocusMinus(int nDeviceAddress);
	unsigned char* PelcoD_ZoomPlus(int nDeviceAddress);
	unsigned char* PelcoD_ZoomMinus(int nDeviceAddress);
	unsigned char* PelcoD_LightOn(int nDeviceAddress);
	unsigned char* PelcoD_LightOff(int nDeviceAddress);
	unsigned char* PelcoD_BrushOn(int nDeviceAddress);
	unsigned char* PelcoD_BrushOff(int nDeviceAddress);
	
	unsigned char* PelcoD_Noop(int nDeviceAddress);

	unsigned char *PelcoD_SetFocusSpd(int nDeviceAddress, int spd);
	unsigned char *PelcoD_SetZeroPos(int nDeviceAddress);
	unsigned char *PelcoD_SetZoomSpd(int nDeviceAddress, int spd);
	unsigned char *PelcoD_SetFocusSpd(int nDeviceAddress, int spd);
	unsigned char *PelcoD_SetAutoFocusOn(int nDeviceAddress);
	unsigned char *PelcoD_SetAutoFocusOff(int nDeviceAddress);
	unsigned char *PelcoD_SetAutoIrisOn(int nDeviceAddress);
	unsigned char *PelcoD_SetAutoIrisOff(int nDeviceAddress);
	unsigned char *PelcoD_SetAutoAgcOn(int nDeviceAddress);
	unsigned char *PelcoD_SetAutoAgcOff(int nDeviceAddress);
	unsigned char *PelcoD_SetBackLightOn(int nDeviceAddress);
	unsigned char *PelcoD_SetBackLightOff(int nDeviceAddress);
	unsigned char *PelcoD_SetWhiteBalanceOn(int nDeviceAddress);
	unsigned char *PelcoD_GotoZeroPan(int nDeviceAddress);
	unsigned char *PelcoD_ResetCamera(int nDeviceAddress);
	unsigned char *PelcoD_RemoteReset(int nDeviceAddress);
	unsigned char *PelcoD_SetWhiteBalanceOff(int nDeviceAddress);

	unsigned char* PelcoD_PresetSet(int nDeviceAddress, int nPresetNO);
	unsigned char* PelcoD_PresetClear(int nDeviceAddress, int nPresetNO);
	unsigned char* PelcoD_PresetGoto(int nDeviceAddress, int nPresetNO);
	
	unsigned char* PelcoD_PatternStart(int nDeviceAddress, int nPatid);
	unsigned char* PelcoD_PatternStop(int nDeviceAddress);
	unsigned char* PelcoD_PatternRun(int nDeviceAddress, int nPatid);

    unsigned char* PelcoD_Flip(int nDeviceAddress);
    unsigned char* PelcoD_PresetAutoScanStart(int nDeviceAddress);
    unsigned char* PelcoD_PresetScanStop(int nDeviceAddress);
    unsigned char* PelcoD_PresetScanSetLeftLimit(int nDeviceAddress);
    unsigned char* PelcoD_PresetScanSetRightLimit(int nDeviceAddress);
    unsigned char* PelcoD_PresetScanOn(int nDeviceAddress, int nTourId);
//    unsigned char* PelcoD_PresetScanStart(int nDeviceAddress);
//    unsigned char* PelcoD_PresetScanStop(int nDeviceAddress);
    unsigned char *PelcoD_PresetTourStart(int nDeviceAddress, int tourID);
    unsigned char *PelcoD_PresetTourEnd(int nDeviceAddress);
    unsigned char *PelcoD_PresetTourAdd(int nDeviceAddress, int nPreno, int nSpd, int nTime);
    unsigned char *PelcoD_PresetTourRun(int nDeviceAddress, int tourID);

    unsigned char *PelcoD_AlarmAck(int nDeviceAddress, int ackno);
    unsigned char *PelcoD_MenuOn(int nDeviceAddress);

    //added at 12-24
    unsigned char *PelcoD_ZoneStart(int nDeviceAddress, int nZone);
    unsigned char *PelcoD_ZoneEnd(int nDeviceAddress, int nZone);
    unsigned char *PelcoD_ZoneScanOn(int nDeviceAddress);
    unsigned char *PelcoD_ZoneScanOff(int nDeviceAddress);

    unsigned char *PelcoD_PresetTourOne(int nDeviceAddress);
    unsigned char *PelcoD_PresetTourTwo(int nDeviceAddress);
    unsigned char *PelcoD_PresetTourThree(int nDeviceAddress);

#ifdef __cplusplus
}
#endif


#endif
