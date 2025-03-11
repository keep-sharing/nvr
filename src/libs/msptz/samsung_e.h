#ifndef SAMSUNG_E_H
#define SAMSUNG_E_H

#define WEB_TX_ADDR		0xfe
#define PC_PROG_ADDR	0xfd

#define SAMSUNG_E_MSG_LEN	9

#ifdef __cplusplus
extern "C"
{
#endif


unsigned char *SamsungE_ZoomMinus(int sender_addr, int target_addr);
unsigned char *SamsungE_ZoomPlus(int sender_addr, int target_addr);
unsigned char *SamsungE_IrisMinus(int sender_addr, int target_addr);
unsigned char *SamsungE_IrisPlus(int sender_addr, int target_addr);
unsigned char *SamsungE_FocusMinus(int sender_addr, int target_addr);
unsigned char *SamsungE_FocusPlus(int sender_addr, int target_addr);
unsigned char *SamsungE_Noop(int sender_addr, int target_addr);
unsigned char *SamsungE_BottomRight(int sender_addr, int target_addr, int pspeed, int tspeed);
unsigned char *SamsungE_TopRight(int sender_addr, int target_addr, int pspeed, int tspeed);
unsigned char *SamsungE_BottomLeft(int sender_addr, int target_addr, int pspeed, int tspeed);
unsigned char *SamsungE_TopLeft(int sender_addr, int target_addr, int pspeed, int tspeed);
unsigned char *SamsungE_Down(int sender_addr, int target_addr, int speed);
unsigned char *SamsungE_Up(int sender_addr, int target_addr, int speed);
unsigned char *SamsungE_Right(int sender_addr, int target_addr, int speed);
unsigned char *SamsungE_Left(int sender_addr, int target_addr, int speed);
unsigned char *SamsungE_PresetSet(int sender_addr, int target_addr, int preset_no);
unsigned char *SamsungE_PresetGoto(int sender_addr, int target_addr, int preset_no);
unsigned char *SamsungE_PresetClear(int sender_addr, int target_addr, int preset_no);
unsigned char *SamsungE_PatternStart(int sender_addr, int target_addr, int pattern_no);
unsigned char *SamsungE_PatternRun(int sender_addr, int target_addr, int pattern_no);
unsigned char *SamsungE_PatternStop(int sender_addr, int target_addr, int pattern_no);
unsigned char *SamsungE_AutoPanStart(int sender_addr, int target_addr, int auto_pan_no);
unsigned char *SamsungE_AutoPanStop(int sender_addr, int target_addr, int auto_pan_no);
unsigned char *SamsungE_TourStart(int sender_addr, int target_addr, int scan_no);
unsigned char *SamsungE_TourStop(int sender_addr, int target_addr, int scan_no);
unsigned char *SamsungE_SetMenuOn(int sender_addr, int target_addr);
unsigned char *SamsungE_SetMenuOff(int sender_addr, int target_addr);
unsigned char *SamsungE_Reset(int sender_addr, int target_addr);
unsigned char *SamsungE_SetZoomSpd(int sender_addr, int target_addr, int zm_spd);
unsigned char *SamsungE_AutoScan(int sender_addr, int target_addr);
unsigned char *SamsungE_TourRun(int sender_addr, int target_addr, int scan_no);

#ifdef __cplusplus
}
#endif

#endif // SUMSUNG_E_H
