/*
 * lilin.h
 *
 *  Created on: 2013-1-31
 *      Author: root
 */

#ifndef LILIN_H_
#define LILIN_H_

#define LILIN_MSG_LEN	3

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *lilin_right(int addr);
unsigned char *lilin_left(int addr);
unsigned char *lilin_up(int addr);
unsigned char *lilin_down(int addr);
unsigned char *lilin_stop(int addr);
unsigned char *lilin_auto_scan(int addr);
unsigned char *lilin_zoom_plus(int addr);
unsigned char *lilin_zoom_minus(int addr);
unsigned char *lilin_focus_plus(int addr);
unsigned char *lilin_focus_minus(int addr);
unsigned char *lilin_iris_plus(int addr);
unsigned char *lilin_iris_minus(int addr);
unsigned char *lilin_preset_set(int addr, int preno);
unsigned char *lilin_preset_goto(int addr, int preno);

#ifdef __cplusplus
}
#endif
#endif /* LILIN_H_ */
