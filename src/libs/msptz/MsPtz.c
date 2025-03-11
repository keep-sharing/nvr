#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MsPtz.h"
#include "pelco_d.h"
#include "pelco_p.h"
#include "samsung_t.h"
#include "samsung_e.h"
#include "yaan.h"
#include "sony_d70.h"
#include "hik.h"
#include "p1602.h"
#include "a01.h"
#include "lilin.h"
#include "panasonic.h"
#include "abd.h"
#include "abp.h"
#include "adv.h"
#include "msstd.h"

#ifndef SRL_SPD_RATIO
#define SRL_SPD_RATIO	8
#endif

static void MsPtzSetProtoLmt(_ms_ptz_info *info, int addr, int tour, int preset,
		int track, int tourspd, int tourtime, int funmode)
{
	info->nAddrLmt = addr;
	info->nTourLmt = tour;
	info->nPreLmt = preset;
	info->nTrackLmt = track;
	info->nTourSpdLmt = tourspd;
	info->nTourTimeLmt = tourtime;
	info->nPtzFunEnb = funmode;
}

//Base Command Set
_ms_ptz_info* MsPtzInfoCreate(MS_PTZ_PROTOCOL nProtocolType, int nDeviceAddress)
{
	_ms_ptz_info* _info = (_ms_ptz_info*)ms_malloc(sizeof(_ms_ptz_info));
	if (!_info)
		return NULL;

	bzero(_info, sizeof(_ms_ptz_info));
	_info->nProtocolType = nProtocolType;
	_info->nDeviceAddress = nDeviceAddress;

    switch(_info->nProtocolType){
    case PTZ_PROTOCOL_PELCO_D:
        _info->nMsgDataLen = PELCO_D_MSG_LEN;
        MsPtzSetProtoLmt(_info, 255, 6, 127,8, 7, 255, PTZ_ALL_FUN_ENABLE);
        break;
    case PTZ_PROTOCOL_PELCO_P:
        _info->nMsgDataLen = PELCO_P_MSG_LEN;
        _info->nDeviceAddress = nDeviceAddress - 1;
        MsPtzSetProtoLmt(_info, 255, 6, 127, 8, 7, 255, PTZ_ALL_FUN_ENABLE);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        _info->nMsgDataLen = SAMSUNG_E_MSG_LEN;
        _info->nSenderAddress = nDeviceAddress;
        MsPtzSetProtoLmt(_info, 255, 4, 128, 3, 0, 0,TRACK_ENABLE | PRESET_ENABLE | PRESET_CLR_ENABLE);
        break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        _info->nMsgDataLen = SAMSUNG_T_MSG_LEN;
        MsPtzSetProtoLmt(_info, 255,0, 128, 0, 0, 0, PRESET_ENABLE | PRESET_CLR_ENABLE);
        break;
    case PTZ_PROTOCOL_YAAN:
    	_info->nMsgDataLen = YAAN_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 255, 1, 128, 0, 255, 0, PRESET_ENABLE | TOUR_SPD_ENABLE);
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	_info->nMsgDataLen = SONY_D70_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 7, 0, 0, 0, 0, 0, 0);
    	break;
    case PTZ_PROTOCOL_HIK:
    	_info->nMsgDataLen = HIK_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 255, 4, 200, 1, 255, 7, PTZ_ALL_FUN_ENABLE);
    	break;
    case PTZ_PROTOCOL_1602:
    	_info->nMsgDataLen = P1602_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 255, 0, 0, 0, 0, 0, 0);
    	break;
    case PTZ_PROTOCOL_LILIN:
    	_info->nMsgDataLen = LILIN_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 64, 0, 0, 0, 0, 0, 0);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_info->nMsgDataLen = PANA_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 99, 0, 64, 1, 0, 0, PRESET_ENABLE | TRACK_ENABLE);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	_info->nMsgDataLen = ABD_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 255, 0, 32, 0, 0, 0, PRESET_ENABLE);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	_info->nMsgDataLen = ABP_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 255, 0, 32, 0, 0, 0, PRESET_ENABLE);
    	break;
    case PTZ_PROTOCOL_ADV:
    	_info->nMsgDataLen = ADV_MSG_LEN;
    	MsPtzSetProtoLmt(_info, 255, 0, 0, 0, 0, 0, 0);
    	break;
    default:
        break;
    }

    _info->nPanSpd = 0x20;
    _info->nTiltSpd = 0x20;

    _info->yaanSpdEnable = 0;
    _info->yaanScanTimeEnable = 0;

	return _info;
}

int MsPtzInfoDestroy(_ms_ptz_info* _ptz_info)
{
	if (!_ptz_info)
		return -1;
	ms_free(_ptz_info);
	_ptz_info = NULL;
	return 0;
}

int MsPtzSetSpeed(_ms_ptz_info* _ptz_info, int nPanSpd, int nTiltSpd, int nZmSpd, int nFcSpd)
{
	if (!_ptz_info)
		return -1;
	_ptz_info->nPanSpd = nPanSpd;
	_ptz_info->nTiltSpd = nTiltSpd;
	_ptz_info->nFcSpd = nFcSpd;
	_ptz_info->nZmSpd = nZmSpd;
	return 0;
}

int MsPtzSetInfo(_ms_ptz_info* _ptz_info, MS_PTZ_PROTOCOL nProtocolType, int nDeviceAddress)
{
	if (!_ptz_info)
		return -1;
	_ptz_info->nProtocolType = nProtocolType;

	if (nDeviceAddress < 0)
		nDeviceAddress = 1;
	_ptz_info->nDeviceAddress = nDeviceAddress;
	switch(_ptz_info->nProtocolType){
	    case PTZ_PROTOCOL_PELCO_D:
	        _ptz_info->nMsgDataLen = PELCO_D_MSG_LEN;
	        MsPtzSetProtoLmt(_ptz_info, 255, 6, 127,8, 7, 255, PTZ_ALL_FUN_ENABLE);
	        break;
	    case PTZ_PROTOCOL_PELCO_P:
	        _ptz_info->nMsgDataLen = PELCO_P_MSG_LEN;
	        _ptz_info->nDeviceAddress = nDeviceAddress - 1;
	        MsPtzSetProtoLmt(_ptz_info, 255, 6, 127, 8, 7, 255, PTZ_ALL_FUN_ENABLE);
	        break;
	    case PTZ_PROTOCOL_SAMSUNG_E:
	        _ptz_info->nMsgDataLen = SAMSUNG_E_MSG_LEN;
	        _ptz_info->nSenderAddress = nDeviceAddress;
	        MsPtzSetProtoLmt(_ptz_info, 255, 4, 128, 3, 0, 0,TRACK_ENABLE | PRESET_ENABLE | PRESET_CLR_ENABLE);
	        break;
	    case PTZ_PROTOCOL_SAMSUNG_T:
	        _ptz_info->nMsgDataLen = SAMSUNG_T_MSG_LEN;
	        MsPtzSetProtoLmt(_ptz_info, 255,0, 128, 0, 0, 0, PRESET_ENABLE | PRESET_CLR_ENABLE);
	        break;
	    case PTZ_PROTOCOL_YAAN:
	    	_ptz_info->nMsgDataLen = YAAN_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 255, 1, 128, 0, 255, 0, PRESET_ENABLE | TOUR_SPD_ENABLE);
	    	break;
	    case PTZ_PROTOCOL_SONY_D70:
	    	_ptz_info->nMsgDataLen = SONY_D70_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 7, 0, 0, 0, 0, 0, 0);
	    	_ptz_info->nDeviceAddress = (nDeviceAddress > 7) ? 7: nDeviceAddress;
	    	break;
	    case PTZ_PROTOCOL_HIK:
	    	_ptz_info->nMsgDataLen = HIK_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 255, 4, 200, 1, 255, 7, PTZ_ALL_FUN_ENABLE);
	    	break;
	    case PTZ_PROTOCOL_1602:
	    	_ptz_info->nMsgDataLen = P1602_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 255, 0, 0, 0, 0, 0, 0);
	    	break;
	    case PTZ_PROTOCOL_LILIN:
	    	_ptz_info->nMsgDataLen = LILIN_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 64, 0, 0, 0, 0, 0, 0);
	    	_ptz_info->nDeviceAddress = (nDeviceAddress > 64) ? 64: nDeviceAddress;
	    	break;
	    case PTZ_PROTOCOL_PANASONIC_CS850:
	    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 99, 0, 64, 1, 0, 0, PRESET_ENABLE | TRACK_ENABLE);
	    	_ptz_info->nDeviceAddress = (nDeviceAddress > 99) ? 99: nDeviceAddress;
	    	break;
	    case PTZ_PROTOCOL_AB_D:
	    	_ptz_info->nMsgDataLen = ABD_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 255, 0, 32, 0, 0, 0, PRESET_ENABLE);
	    	break;
	    case PTZ_PROTOCOL_AB_P:
	    	_ptz_info->nMsgDataLen = ABP_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 255, 0, 32, 0, 0, 0, PRESET_ENABLE);
	    	break;
	    case PTZ_PROTOCOL_ADV:
	    	_ptz_info->nMsgDataLen = ADV_MSG_LEN;
	    	MsPtzSetProtoLmt(_ptz_info, 255, 0, 0, 0, 0, 0, 0);
	    	break;
	    default:
	        _ptz_info->nProtocolType = PTZ_PROTOCOL_PELCO_D;
	        _ptz_info->nMsgDataLen = PELCO_D_MSG_LEN;
	        MsPtzSetProtoLmt(_ptz_info, 255, 6, 127,8, 7, 255, PTZ_ALL_FUN_ENABLE);
	        break;
	    }

	return 0;
}

int MsPtzGetAddrLmt(int proto)
{
	int lmt;
	switch (proto) {
	case PTZ_PROTOCOL_PELCO_D:
	case PTZ_PROTOCOL_PELCO_P:
	case PTZ_PROTOCOL_SAMSUNG_E:
	case PTZ_PROTOCOL_SAMSUNG_T:
	case PTZ_PROTOCOL_YAAN:
	case PTZ_PROTOCOL_HIK:
	case PTZ_PROTOCOL_1602:
	case PTZ_PROTOCOL_AB_D:
	case PTZ_PROTOCOL_AB_P:
	case PTZ_PROTOCOL_ADV:
		lmt = 255;
		break;
	case PTZ_PROTOCOL_LILIN:
		lmt = 64;
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		lmt = 99;
		break;
	case PTZ_PROTOCOL_SONY_D70:
		lmt = 7;
		break;
	default:
		lmt = 0;
		break;
	}
	return lmt;
}

unsigned char* MsPtzUp(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_Up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_Up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
       pMsgData = SamsungT_Up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
       break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_Up(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_YAAN:// yaan protocol need to send two packages
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_up(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, 0x00);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_up(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_LILIN:
    	pMsgData = lilin_up(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_up(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_Up(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzDown(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_Down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_Down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        pMsgData = SamsungT_Down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_Down(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_YAAN:
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_down(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, 0x00, _ptz_info->nTiltSpd);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_down(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_LILIN:
    	pMsgData = lilin_down(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_down(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_Down(_ptz_info->nDeviceAddress, _ptz_info->nTiltSpd);
		break;
	}	
	return pMsgData;
}

unsigned char* MsPtzLeft(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_Left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_Left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        pMsgData = SamsungT_Left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_Left(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
        break;
    case PTZ_PROTOCOL_YAAN:
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_left(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, 0x00);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_left(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_LILIN:
    	pMsgData = lilin_left(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_left(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_Left(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzRight(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
        pMsgData = PelcoD_Right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_Right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        pMsgData = SamsungT_Right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_Right(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
        break;
    case PTZ_PROTOCOL_YAAN:
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_right(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, 0x00);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_right(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_LILIN:
    	pMsgData = lilin_right(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_right(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_Right(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzTopLeft(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_TopLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_TopLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        pMsgData = SamsungT_TopLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_TopLeft(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_YAAN:
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_left_up(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_left_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_left_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_left_up(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_left_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_left_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_left_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_left_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_left_up(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_TopLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzTopRight(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_TopRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_TopRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        pMsgData = SamsungT_TopRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_TopRight(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_YAAN:
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_right_up(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_right_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_right_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_right_up(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_right_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_right_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_right_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_right_up(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_right_up(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_TopRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzBottomLeft(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
        pMsgData = PelcoD_BottomLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_BottomLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        pMsgData = SamsungT_BottomLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_BottomLeft(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_YAAN:
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_left_down(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_left_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_left_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_left_down(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_left_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_left_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_left_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_left_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_left_down(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_BottomLeft(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzBottomRight(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

    switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_BottomRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_BottomRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
    case PTZ_PROTOCOL_SAMSUNG_T:
        pMsgData = SamsungT_BottomRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_SAMSUNG_E:
        pMsgData = SamsungE_BottomRight(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
        break;
    case PTZ_PROTOCOL_YAAN:
    	if (_ptz_info->yaanSpdEnable == 0) {
    		pMsgData = yaan_left_down(_ptz_info->nDeviceAddress);
    		_ptz_info->yaanSpdEnable = 1;
    		return pMsgData;
    	} else {
    		pMsgData = yaan_set_spd(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    		_ptz_info->yaanSpdEnable = 0;
    	}
    	break;
    case PTZ_PROTOCOL_SONY_D70:
    	pMsgData = sonyd70_right_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_HIK:
    	pMsgData = hik_right_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_1602:
    	pMsgData = p1602_right_down(_ptz_info->nDeviceAddress);
    	break;
    case PTZ_PROTOCOL_A01:
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = a01_right_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_PANASONIC_CS850:
    	_ptz_info->nMsgDataLen = PANA_MSG_LEN;
    	_ptz_info->nPanSpd /= SRL_SPD_RATIO;
    	_ptz_info->nTiltSpd /= SRL_SPD_RATIO;
    	pMsgData = pana_right_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_D:
    	pMsgData = abd_right_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_AB_P:
    	pMsgData = abp_right_down(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
    	break;
    case PTZ_PROTOCOL_ADV:
    	pMsgData = adv_right_down(_ptz_info->nDeviceAddress);
    	break;
	default:
	//	pMsgData = PelcoD_BottomRight(_ptz_info->nDeviceAddress, _ptz_info->nPanSpd, _ptz_info->nTiltSpd);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzAutoScan(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_AutoScan(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_AutoScan(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_AutoScan(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_auto_scan(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_auto_scan(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_auto_scan(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_AutoScan(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzIrisPlus(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_IrisPlus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_IrisPlus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_IrisPlus(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_IrisPlus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_1602:
		pMsgData = p1602_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		_ptz_info->nMsgDataLen = PANA_MSG_LEN_SHORT;
		pMsgData = pana_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_iris_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_ADV:
		pMsgData = adv_iris_plus(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_IrisPlus(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzIrisMinus(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_IrisMinus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_IrisMinus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_IrisMinus(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_IrisMinus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_1602:
		pMsgData = p1602_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		_ptz_info->nMsgDataLen = PANA_MSG_LEN_SHORT;
		pMsgData = pana_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_iris_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_ADV:
		pMsgData = adv_iris_minus(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_IrisMinus(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzFocusPlus(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_FocusPlus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_FocusPlus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_FocusPlus(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_FocusPlus(_ptz_info->nDeviceAddress, _ptz_info->nFcSpd);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_1602:
		pMsgData = p1602_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		_ptz_info->nMsgDataLen = PANA_MSG_LEN_SHORT;
		pMsgData = pana_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_focus_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_ADV:
		pMsgData = adv_focus_plus(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_FocusPlus(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzFocusMinus(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_FocusMinus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_FocusMinus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_FocusMinus(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_FocusMinus(_ptz_info->nDeviceAddress, _ptz_info->nFcSpd);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_1602:
		pMsgData = p1602_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		_ptz_info->nMsgDataLen = PANA_MSG_LEN_SHORT;
		pMsgData = pana_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_focus_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_ADV:
		pMsgData = adv_focus_minus(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_FocusMinus(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzZoomPlus(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_ZoomPlus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_ZoomPlus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_ZoomPlus(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_ZoomPlus(_ptz_info->nDeviceAddress, _ptz_info->nZmSpd);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_1602:
		pMsgData = p1602_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		_ptz_info->nMsgDataLen = PANA_MSG_LEN_SHORT;
		pMsgData = pana_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_ADV:
		pMsgData = adv_zoom_plus(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_ZoomPlus(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzZoomMinus(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_ZoomMinus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_ZoomMinus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_ZoomMinus(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_ZoomMinus(_ptz_info->nDeviceAddress, _ptz_info->nZmSpd);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_1602:
		pMsgData = p1602_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		_ptz_info->nMsgDataLen = PANA_MSG_LEN_SHORT;
		pMsgData = pana_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_ADV:
		pMsgData = adv_zoom_minus(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_ZoomMinus(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzLightOn(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_LightOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_LightOn(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_LightOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzLightOff(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_LightOff(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_LightOff(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_LightOff(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzBrushOn(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_BrushOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_BrushOn(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_BrushOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzBrushOff(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_BrushOff(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_BrushOff(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_BrushOff(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}


unsigned char* MsPtzNoop(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_Noop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_Noop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_Noop(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_Noop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SONY_D70:
		pMsgData = sonyd70_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_1602:
		pMsgData = p1602_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PANASONIC_CS850:
		_ptz_info->nMsgDataLen = PANA_MSG_LEN;
		pMsgData = pana_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_stop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_ADV:
		pMsgData = adv_stop(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_Noop(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzPresetSet(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetSet(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetSet(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_PresetSet(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_PresetSet(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_preset_set(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_preset_set(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_A01:
		pMsgData = a01_preset_set(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_preset_set(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_preset_set(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_preset_set(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	default:
	//	pMsgData = PelcoD_PresetSet(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzPresetClear(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetClear(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetClear(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_PresetClear(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_PresetClear(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	default:
	//	pMsgData = PelcoD_PresetClear(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzPresetGoto(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetGoto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetGoto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_PresetGoto(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_PresetGoto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_preset_goto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_preset_goto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_LILIN:
		pMsgData = lilin_preset_goto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_AB_D:
		pMsgData = abd_preset_goto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_AB_P:
		pMsgData = abp_preset_goto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	default:
	//	pMsgData = PelcoD_PresetGoto(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzPatternStart(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PatternStart(_ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PatternStart(_ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_PatternStart(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_pattern_start(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_PatternStart(_ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzPatternStop(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PatternStop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PatternStop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_PatternStop(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_pattern_stop(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_PatternStop(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzPatternRun(_ms_ptz_info* _ptz_info)
{
	unsigned char* pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PatternRun(_ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PatternRun(_ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_PatternRun(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nPreset);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_pattern_run(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_PatternRun(_ptz_info->nDeviceAddress, _ptz_info->nPat);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzFlip(_ms_ptz_info* _ptz_info)
{
    unsigned char* pMsgData = NULL;
    if (!_ptz_info)
        return pMsgData;


    switch (_ptz_info->nProtocolType) {
    case PTZ_PROTOCOL_PELCO_D:
        pMsgData = PelcoD_Flip(_ptz_info->nDeviceAddress);
        break;
    case PTZ_PROTOCOL_PELCO_P:
        pMsgData = PelcoP_Flip(_ptz_info->nDeviceAddress);
        break;
    default:
     //   pMsgData = PelcoD_Flip(_ptz_info->nDeviceAddress);
        break;
    }
    return pMsgData;
}

unsigned char* MsPtzSetWhiteBalanceOn(_ms_ptz_info* _ptz_info)
{
	unsigned char * pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetWhiteBalanceOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
//		pMsgData = PelcoP_SetWhiteBalanceOn(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_SetWhiteBalanceOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char* MsPtzSetWhiteBalanceOff(_ms_ptz_info* _ptz_info)
{
	unsigned char * pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetWhiteBalanceOff(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
//		pMsgData = PelcoP_SetWhiteBalanceOn(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_SetWhiteBalanceOff(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetBackLightOn(_ms_ptz_info *_ptz_info)
{
	unsigned char * pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetBackLightOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
//		pMsgData = PelcoP_SetWhiteBalanceOn(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_SetBackLightOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetAutoFocusOn(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch(_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetAutoFocusOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		break;
	default:
	//	pMsgData = PelcoD_SetAutoFocusOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetAutoFocusOff(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch(_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetAutoFocusOff(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		break;
	default:
	//	pMsgData = PelcoD_SetAutoFocusOff(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetAutoIrisOn(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch(_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetAutoIrisOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		break;
	default:
	//	pMsgData = PelcoD_SetAutoIrisOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetAutoIrisOff(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch(_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetAutoIrisOff(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		break;
	default:
	//	pMsgData = PelcoD_SetAutoIrisOff(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetBackLightOff(_ms_ptz_info *_ptz_info)
{
	unsigned char * pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetBackLightOff(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
//		pMsgData = PelcoP_SetWhiteBalanceOn(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_SetBackLightOff(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetFocusSpd(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetFocusSpd(_ptz_info->nDeviceAddress, _ptz_info->nFcSpd);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		break;
	default:
	//	pMsgData = PelcoD_SetFocusSpd(_ptz_info->nDeviceAddress, _ptz_info->nFcSpd);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzSetZoomSpd(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetZoomSpd(_ptz_info->nDeviceAddress, _ptz_info->nZmSpd);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_SetZoomSpd(_ptz_info->nSenderAddress,_ptz_info->nDeviceAddress, _ptz_info->nZmSpd);
		break;
	default:
	//	pMsgData = PelcoD_SetZoomSpd(_ptz_info->nDeviceAddress, _ptz_info->nZmSpd);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzCameraRst(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_ResetCamera(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_Reset(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_ResetCamera(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzRemoteRst(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_RemoteReset(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_RemoteRst(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_RemoteReset(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzZoneStart(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_ZoneStart(_ptz_info->nDeviceAddress, _ptz_info->nZone);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_ZoneStart(_ptz_info->nDeviceAddress, _ptz_info->nZone);
		break;
	default:
	//	pMsgData = PelcoD_ZoneStart(_ptz_info->nDeviceAddress, _ptz_info->nZone);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzZoneEnd(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_ZoneEnd(_ptz_info->nDeviceAddress, _ptz_info->nZone);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_ZoneEnd(_ptz_info->nDeviceAddress, _ptz_info->nZone);
		break;
	default:
	//	pMsgData = PelcoD_ZoneEnd(_ptz_info->nDeviceAddress, _ptz_info->nZone);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzZoneScanOn(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_ZoneScanOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_ZoneScanOn(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_ZoneScanOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzZoneScanOff(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_ZoneScanOff(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_ZoneScanOff(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_ZoneScanOff(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetAutoScanStart(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetAutoScanStart(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetAutoScanStart(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_PresetAutoScanStart(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetScanStop(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetScanStop(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetScanStop(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_PresetScanStop(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetScanSetLeftLimit(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetScanSetLeftLimit(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetScanSetLeftLimit(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_line_scan_set_left_lmt(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_PresetScanSetLeftLimit(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetScanSetRightLimit(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetScanSetRightLimit(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetScanSetRightLimit(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_line_scan_set_right_lmt(_ptz_info->nDeviceAddress);
		break;
	default:
	//	pMsgData = PelcoD_PresetScanSetRightLimit(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzLineScanOn(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_LineScanOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetScanSetRightLimit(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_PresetScanSetRightLimit(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;

}
unsigned char *MsPtzMenuOn(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_MenuOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_MenuOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_SetMenuOn(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_MenuOn(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_menu_on(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_MenuOn(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}


unsigned char *MsPtzSetZeroPos(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;
	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_SetZeroPos(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_SetZeroPos(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_SAMSUNG_T:
		pMsgData = SamsungT_PanTiltPosInit(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_SetZeroPos(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzGotoZeroPos(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_GotoZeroPan(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_GotoZeroPos(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_GotoZeroPan(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}
/*************************************PRESET TOUR******************************/
unsigned char *MsPtzPresetTourOne(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetTourOne(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetTourOne(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_PresetTourOne(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetTourTwo(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetTourTwo(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetTourTwo(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_PresetTourTwo(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetTourThree(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetTourThree(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetTourThree(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_PresetTourThree(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

/*==============================HIK PELCO==============================*/
unsigned char *MsPtzPresetTourStartStop(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetTourStart(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetTourStart(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_tour_clear(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_PresetTourStart(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetTourEnd(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetTourEnd(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetTourEnd(_ptz_info->nDeviceAddress);
		break;
	default:
//		pMsgData = PelcoD_PresetTourEnd(_ptz_info->nDeviceAddress);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetTourAdd(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;

	switch (_ptz_info->nProtocolType) {
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetTourAdd(_ptz_info->nDeviceAddress, _ptz_info->nPreset, _ptz_info->nTourSpd, _ptz_info->nTimeOut);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetTourAdd(_ptz_info->nDeviceAddress, _ptz_info->nPreset, _ptz_info->nTourSpd, _ptz_info->nTimeOut);
		break;
	case PTZ_PROTOCOL_YAAN:
		if (_ptz_info->yaanScanTimeEnable == 0) {
			pMsgData = yaan_tour_add(_ptz_info->nDeviceAddress, _ptz_info->nPreset);
			_ptz_info->yaanScanTimeEnable = 1;
			return pMsgData;
		} else {
			pMsgData = yaan_tour_time_set(_ptz_info->nDeviceAddress, _ptz_info->nTimeOut);
			_ptz_info->yaanScanTimeEnable = 0;

		}
		break;
	default:
//		pMsgData = PelcoD_PresetTourAdd(_ptz_info->nDeviceAddress, _ptz_info->nPreset, _ptz_info->nTourSpd, _ptz_info->nTimeOut);
		break;
	}
	return pMsgData;
}


unsigned char *MsPtzPresetScanOn(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType){
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetScanOn(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetScanOn(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	default:
//		pMsgData = PelcoD_PresetScanOn(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	}
	return pMsgData;
}

unsigned char *MsPtzPresetTourRun(_ms_ptz_info *_ptz_info)
{
	unsigned char *pMsgData = NULL;

	if (!_ptz_info)
		return pMsgData;
	switch (_ptz_info->nProtocolType){
	case PTZ_PROTOCOL_PELCO_D:
		pMsgData = PelcoD_PresetTourRun(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	case PTZ_PROTOCOL_PELCO_P:
		pMsgData = PelcoP_PresetTourRun(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	case PTZ_PROTOCOL_SAMSUNG_E:
		pMsgData = SamsungE_TourRun(_ptz_info->nSenderAddress, _ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	case PTZ_PROTOCOL_YAAN:
		pMsgData = yaan_tour_auto_scan(_ptz_info->nDeviceAddress);
		break;
	case PTZ_PROTOCOL_HIK:
		pMsgData = hik_tour_run(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	default:
//		pMsgData = PelcoD_PresetTourRun(_ptz_info->nDeviceAddress, _ptz_info->nTourId);
		break;
	}
	return pMsgData;
}
/*==============================HIK PELCO END==========================*/
/*************************************PRESET TOUR END**************************/
