#ifndef __MS_PELCO_P_H__
#define __MS_PELCO_P_H__

#define PELCO_P_MSG_LEN	8

#ifdef __cplusplus
extern "C"
{
#endif



	unsigned char* PelcoP_Up(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoP_Down(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoP_Left(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoP_Right(int nDeviceAddress, int nSpeed);
	unsigned char* PelcoP_TopLeft(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoP_TopRight(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoP_BottomLeft(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoP_BottomRight(int nDeviceAddress, int nSpeed, int nTiltSpd);
	unsigned char* PelcoP_AutoScan(int nDeviceAddress);

	unsigned char* PelcoP_IrisPlus(int nDeviceAddress);
	unsigned char* PelcoP_IrisMinus(int nDeviceAddress);
	unsigned char* PelcoP_FocusPlus(int nDeviceAddress);
	unsigned char* PelcoP_FocusMinus(int nDeviceAddress);
	unsigned char* PelcoP_ZoomPlus(int nDeviceAddress);
	unsigned char* PelcoP_ZoomMinus(int nDeviceAddress);
	unsigned char* PelcoP_LightOn(int nDeviceAddress);
	unsigned char* PelcoP_LightOff(int nDeviceAddress);
	unsigned char* PelcoP_BrushOn(int nDeviceAddress);
	unsigned char* PelcoP_BrushOff(int nDeviceAddress);
	
	unsigned char* PelcoP_Noop(int nDeviceAddress);

	unsigned char* PelcoP_PresetSet(int nDeviceAddress, int nPresetNO);
	unsigned char* PelcoP_PresetClear(int nDeviceAddress, int nPresetNO);
	unsigned char* PelcoP_PresetGoto(int nDeviceAddress, int nPresetNO);
	unsigned char *PelcoP_PresetAutoScanStart(int nDeviceAddress);
	unsigned char *PelcoP_PresetScanStop(int nDeviceAddress);
    unsigned char *PelcoP_PresetTourStart(int nDeviceAddress, int tourID);
    unsigned char *PelcoP_PresetTourEnd(int nDeviceAddress);
    unsigned char *PelcoP_PresetTourAdd(int nDeviceAddress, int nPreno, int nSpd, int nTime);
    unsigned char *PelcoP_PresetTourRun(int nDeviceAddress, int tourID);
	unsigned char *PelcoP_PresetScanOn(int nDeviceAddress, int tourID);//to be continued
	unsigned char *PelcoP_PresetRandomScan(int nDeviceAddress);
	unsigned char *PelcoP_PresetScanSetLeftLimit(int nDeviceAddress);
	unsigned char *PelcoP_PresetScanSetRightLimit(int nDeviceAddress);
	unsigned char *PelcoP_MenuOn(int nDeviceAddress);
	
	unsigned char* PelcoP_PatternStart(int nDeviceAddress, int nPatid);
	unsigned char* PelcoP_PatternStop(int nDeviceAddress);
	unsigned char* PelcoP_PatternRun(int nDeviceAddress, int nPatid);

    unsigned char* PelcoP_Flip(int nDeviceAddress);
    // added at 12-24
    unsigned char *PelcoP_RemoteRst(int nDeviceAddress);
    unsigned char *PelcoP_ZoneStart(int nDeviceAddress, int nZone);
    unsigned char *PelcoP_ZoneEnd(int nDeviceAddress, int nZone);
    unsigned char *PelcoP_ZoneScanOn(int nDeviceAddress);
    unsigned char *PelcoP_ZoneScanOff(int nDeviceAddress);
    unsigned char *PelcoP_GotoZeroPos(int nDeviceAddress);
    unsigned char *PelcoP_SetZeroPos(int nDeviceAddress);

    unsigned char *PelcoD_LineScanOn(int nDeviceAddress);
    unsigned char *PelcoD_LineScanOff(int nDeviceAddress);

    unsigned char *PelcoP_PresetTourOne(int nDeviceAddress);
    unsigned char *PelcoP_PresetTourTwo(int nDeviceAddress);
    unsigned char *PelcoP_PresetTourThree(int nDeviceAddress);


#ifdef __cplusplus
}
#endif


#endif
