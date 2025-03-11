/*
 * yaan.h
 *
 *  Created on: 2013-1-28
 *      Author: root
 */

#ifndef YAAN_H_
#define YAAN_H_

#define YAAN_MSG_LEN	6

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *yaan_right(int addr);
unsigned char *yaan_left(int addr);
unsigned char *yaan_up(int addr);
unsigned char *yaan_down(int addr);
unsigned char *yaan_right_up(int addr);
unsigned char *yaan_right_down(int addr);
unsigned char *yaan_left_up(int addr);
unsigned char *yaan_left_down(int addr);
unsigned char *yaan_auto_scan(int addr);
unsigned char *yaan_zoom_plus(int addr);
unsigned char *yaan_zoom_minus(int addr);
unsigned char *yaan_focus_plus(int addr);
unsigned char *yaan_focus_minus(int addr);
unsigned char *yaan_iris_plus(int addr);
unsigned char *yaan_iris_minus(int addr);
unsigned char *yaan_stop(int addr);
unsigned char *yaan_stop_spd(int addr);
unsigned char *yaan_set_spd(int addr, int pan, int tilt);
unsigned char *yaan_preset_set(int addr, int preno);
unsigned char *yaan_preset_goto(int addr, int preno);
unsigned char *yaan_tour_add(int addr, int preno);
unsigned char *yaan_tour_time_set(int addr, int preno);
unsigned char *yaan_tour_clear(int addr);
unsigned char *yaan_tour_auto_scan(int addr);
unsigned char *yaan_line_scan_set_left_lmt(int addr);
unsigned char *yaan_line_scan_set_right_lmt(int addr);
unsigned char *yaan_set_auto_scan_spd(int addr, int spd);

#ifdef __cplusplus
}
#endif

#endif /* YAAN_H_ */
