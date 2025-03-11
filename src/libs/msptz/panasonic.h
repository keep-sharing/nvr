/*
 * panasonic.h
 *
 *  Created on: 2013-1-31
 *      Author: root
 */

#ifndef PANASONIC_H_
#define PANASONIC_H_

#define PANA_MSG_LEN	26
#define PANA_MSG_LEN_SHORT	18

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *pana_up(int addr, int tspd);
unsigned char *pana_down(int addr, int tspd);
unsigned char *pana_left(int addr, int tspd);
unsigned char *pana_right(int addr, int tspd);
unsigned char *pana_left_up(int addr,int pspd, int tspd);
unsigned char *pana_right_up(int addr,int pspd, int tspd);
unsigned char *pana_left_down(int addr,int pspd, int tspd);
unsigned char *pana_right_down(int addr,int pspd, int tspd);
unsigned char *pana_stop(int addr);
unsigned char *pana_zoom_plus(int addr);
unsigned char *pana_zoom_minus(int addr);
unsigned char *pana_iris_plus(int addr);
unsigned char *pana_iris_minus(int addr);
unsigned char *pana_focus_plus(int addr);
unsigned char *pana_focus_minus(int addr);

#ifdef __cplusplus
}
#endif

#endif /* PANASONIC_H_ */
