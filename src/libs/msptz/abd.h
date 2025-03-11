/*
 * abd.h
 *
 *  Created on: 2013-2-1
 *      Author: root
 */

#ifndef ABD_H_
#define ABD_H_

#define ABD_MSG_LEN	7

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *abd_up(int addr, int tspd);
unsigned char *abd_down(int addr, int tspd);
unsigned char *abd_left(int addr, int pspd);
unsigned char *abd_right(int addr, int pspd);
unsigned char *abd_left_up(int addr, int pspd, int tspd);
unsigned char *abd_left_down(int addr, int pspd, int tspd);
unsigned char *abd_right_up(int addr, int pspd, int tspd);
unsigned char *abd_right_down(int addr, int pspd, int tspd);
unsigned char *abd_stop(int addr);
unsigned char *abd_zoom_plus(int addr);
unsigned char *abd_zoom_minus(int addr);
unsigned char *abd_focus_plus(int addr);
unsigned char *abd_focus_minus(int addr);
unsigned char *abd_iris_plus(int addr);
unsigned char *abd_iris_minus(int addr);
unsigned char *abd_preset_set(int addr, int preno);
unsigned char *abd_preset_goto(int addr, int preno);

#ifdef __cplusplus
}
#endif

#endif /* ABD_H_ */
