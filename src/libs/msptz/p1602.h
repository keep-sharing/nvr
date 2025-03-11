/*
 * p1602.h
 *
 *  Created on: 2013-1-31
 *      Author: root
 */

#ifndef P1602_H_
#define P1602_H_

#define P1602_MSG_LEN	11

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *p1602_up(int addr);
unsigned char *p1602_down(int addr);
unsigned char *p1602_left(int addr);
unsigned char *p1602_right(int addr);
unsigned char *p1602_left_up(int addr);
unsigned char *p1602_left_down(int addr);
unsigned char *p1602_right_up(int addr);
unsigned char *p1602_right_down(int addr);
unsigned char *p1602_zoom_plus(int addr);
unsigned char *p1602_zoom_minus(int addr);
unsigned char *p1602_iris_plus(int addr);
unsigned char *p1602_iris_minus(int addr);
unsigned char *p1602_focus_plus(int addr);
unsigned char *p1602_focus_minus(int addr);
unsigned char *p1602_stop(int addr);

#ifdef __cplusplus
}
#endif

#endif /* P1602_H_ */
