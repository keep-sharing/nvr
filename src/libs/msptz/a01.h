/*
 * a01.h
 *
 *  Created on: 2013-1-31
 *      Author: root
 */

#ifndef A01_H_
#define A01_H_

#define A01_MSG_LEN		5

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *a01_up(int addr, int tspd);
unsigned char *a01_down(int addr, int tspd);
unsigned char *a01_left(int addr, int pspd);
unsigned char *a01_right(int addr, int pspd);
unsigned char *a01_left_up(int addr, int pspd, int tspd);
unsigned char *a01_left_down(int addr, int pspd, int tspd);
unsigned char *a01_right_up(int addr, int pspd, int tspd);
unsigned char *a01_right_down(int addr, int pspd, int tspd);
unsigned char *a01_stop(int addr);
unsigned char *a01_zoom_plus(int addr);
unsigned char *a01_zoom_minus(int addr);
unsigned char *a01_iris_plus(int addr);
unsigned char *a01_iris_minus(int addr);
unsigned char *a01_focus_plus(int addr);
unsigned char *a01_focus_minus(int addr);
unsigned char *a01_preset_set(int addr, int preno);
unsigned char *a01_preset_goto(int addr, int preno);

#ifdef __cplusplus
}
#endif
#endif /* A01_H_ */
