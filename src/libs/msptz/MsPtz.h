#ifndef __MS_PTZ_H__
#define __MS_PTZ_H__


#include "ptz_public.h"
#include "ptz_term_info.h"

typedef struct ms_ptz_info {
	MS_PTZ_PROTOCOL nProtocolType;
	int nSenderAddress;
	int nDeviceAddress;
	int nPreset; // preset number
	int nPat; //pattern id
	int nPanSpd; //pan
	int nTiltSpd;
	int nZmSpd; // zoom speed
	int nFcSpd; // focus speed
	int nZone; // zone number
	int nMsgDataLen; //Message length, PELCO-D:7 PELCO-P:8

	int nTourId;
	int nTourSpd;
	int nTimeOut;

	int yaanSpdEnable; // for yaan protocol only
	int yaanScanTimeEnable; // whether tour scan time has been set

	int nPtzFunEnb; // ptz function enable flag
	int nPreLmt; // max presets limit
	int nTourLmt; //max tours limit
	int nTrackLmt; // max tracks limit
	int nAddrLmt; // max addrs limit
	int nTourTimeLmt; // max tour scan time
	int nTourSpdLmt; // max tour speed

} _ms_ptz_info;


#ifdef __cplusplus
extern "C"
{
#endif
	
	//Calloc Destroy
	_ms_ptz_info* MsPtzInfoCreate(MS_PTZ_PROTOCOL nProtocolType, int nDeviceAddress);
	int MsPtzInfoDestroy(_ms_ptz_info* _ptz_info);

	int MsPtzSetInfo(_ms_ptz_info* _ptz_info, MS_PTZ_PROTOCOL nProtocolType, int nDeviceAddress);
	int MsPtzSetSpeed(_ms_ptz_info* _ptz_info, int nPanSpd, int nTiltSpd, int nZmSpd, int nFcSpd);
	int MsPtzGetAddrLmt(int proto);

	//Base Command Set
	unsigned char* MsPtzUp(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzDown(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzLeft(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzRight(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzTopLeft(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzTopRight(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzBottomLeft(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzBottomRight(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzAutoScan(_ms_ptz_info* _ptz_info);

	unsigned char* MsPtzIrisPlus(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzIrisMinus(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzFocusPlus(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzFocusMinus(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzZoomPlus(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzZoomMinus(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzLightOn(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzLightOff(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzBrushOn(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzBrushOff(_ms_ptz_info* _ptz_info);

	unsigned char* MsPtzNoop(_ms_ptz_info* _ptz_info);

	unsigned char* MsPtzPresetSet(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzPresetClear(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzPresetGoto(_ms_ptz_info* _ptz_info);

	unsigned char* MsPtzPatternStart(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzPatternStop(_ms_ptz_info* _ptz_info);
	unsigned char* MsPtzPatternRun(_ms_ptz_info* _ptz_info);

    unsigned char* MsPtzFlip(_ms_ptz_info* _ptz_info);
    unsigned char* MsPtzSetWhiteBalanceOn(_ms_ptz_info* _ptz_info);
    unsigned char* MsPtzSetWhiteBalanceOff(_ms_ptz_info* _ptz_info);
    unsigned char* MsPtzSetBackLightOn(_ms_ptz_info* _ptz_info);
    unsigned char* MsPtzSetBackLightOff(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzSetAutoFocusOn(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzSetAutoFocusOff(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzSetAutoIrisOn(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzSetAutoIrisOff(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzSetZeroPos(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzGotoZeroPos(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzSetZoomSpd(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzSetFocusSpd(_ms_ptz_info *_ptz_info);
    // added at 12-24
    unsigned char *MsPtzCameraRst(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzRemoteRst(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzZoneStart(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzZoneEnd(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzZoneScanOn(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzZoneScanOff(_ms_ptz_info *_ptz_info);

    unsigned char *MsPtzPresetAutoScanStart(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetScanStop(_ms_ptz_info *_ptz_info);

    unsigned char *MsPtzPresetScanSetLeftLimit(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetScanSetRightLimit(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzMenuOn(_ms_ptz_info *_ptz_info);

    unsigned char *MsPtzPresetTourOne(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetTourTwo(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetTourTwo(_ms_ptz_info *_ptz_info);

    unsigned char *MsPtzPresetTourStartStop(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetTourEnd(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetTourAdd(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetScanOn(_ms_ptz_info *_ptz_info);
    unsigned char *MsPtzPresetTourRun(_ms_ptz_info *_ptz_info);

#ifdef __cplusplus
}
#endif

#endif
