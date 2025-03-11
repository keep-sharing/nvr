/*
 * samsung_t.h
 *
 *  Created on: 2012-11-30
 *      Author: chimmu
 */

#ifndef SAMSUNG_T_H_
#define SAMSUNG_T_H_

#define SAMSUNG_T_MSG_LEN	11

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *SamsungT_Left(int addr, int speed);
unsigned char *SamsungT_Right(int addr, int speed);
unsigned char *SamsungT_Up(int addr, int speed);
unsigned char *SamsungT_Down(int addr, int speed);
unsigned char *SamsungT_TopRight(int addr, int pspeed, int tspeed);
unsigned char *SamsungT_TopLeft(int addr, int pspeed, int tspeed);
unsigned char *SamsungT_BottomLeft(int addr, int pspeed, int tspeed);
unsigned char *SamsungT_BottomRight(int addr, int pspeed, int tspeed);
unsigned char *SamsungT_ZoomPlus(int addr, int zmspd);
unsigned char *SamsungT_ZoomMinus(int addr, int zmspd);
unsigned char *SamsungT_IrisPlus(int addr);
unsigned char *SamsungT_IrisMinus(int addr);
unsigned char *SamsungT_FocusPlus(int addr, int fcspd);
unsigned char *SamsungT_FocusMinus(int addr, int fcspd);
unsigned char *SamsungT_PresetSet(int addr, int pre_no);
unsigned char *SamsungT_PresetGoto(int addr, int pre_no);
unsigned char *SamsungT_PresetClear(int addr, int pre_no);
unsigned char *SamsungT_MenuOn(int addr);
unsigned char *SamsungT_MenuOff(int addr);
unsigned char *SamsungT_MenuEnter(int addr);
unsigned char *SamsungT_MenuCancel(int addr);
unsigned char *SamsungT_PanTiltPosInit(int addr);
unsigned char *SamsungT_ZoomPosInit(int addr);
unsigned char *SamsungT_Noop(int addr);



#ifdef __cplusplus
}
#endif

#endif /* SAMSUNG_T_H_ */
