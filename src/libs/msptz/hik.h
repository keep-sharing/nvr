/*
 * hik.h
 *
 *  Created on: 2013-1-30
 *      Author: root
 */

#ifndef HIK_H_
#define HIK_H_

#define HIK_MSG_LEN	8

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *hik_up(int addr, int tspd);
unsigned char *hik_down(int addr, int tspd);
unsigned char *hik_left(int addr, int pspd);
unsigned char *hik_right(int addr, int pspd);
unsigned char *hik_left_up(int addr, int pspd, int tspd);
unsigned char *hik_right_up(int addr, int pspd, int tspd);
unsigned char *hik_left_down(int addr, int pspd, int tspd);
unsigned char *hik_right_down(int addr, int pspd, int tspd);
unsigned char *hik_stop(int addr);
unsigned char *hik_auto_scan(int addr);
unsigned char *hik_preset_set(int addr, int preno);
unsigned char *hik_preset_goto(int addr, int preno);
unsigned char *hik_pattern_start(int addr);
unsigned char *hik_pattern_stop(int addr);
unsigned char *hik_pattern_run(int addr);
unsigned char *hik_zoom_plus(int addr);
unsigned char *hik_zoom_minus(int addr);
unsigned char *hik_focus_plus(int addr);
unsigned char *hik_focus_minus(int addr);
unsigned char *hik_iris_plus(int addr);
unsigned char *hik_iris_minus(int addr);
unsigned char *hik_tour_clear(int addr, int preno);
unsigned char *hik_tour_run(int addr, int tourid);
unsigned char *hik_menu_on(int addr);

#ifdef __cplusplus
}
#endif
#endif /* HIK_H_ */
