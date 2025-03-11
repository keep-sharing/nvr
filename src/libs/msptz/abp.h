/*
 * abp.h
 *
 *  Created on: 2013-2-2
 *      Author: root
 */

#ifndef ABP_H_
#define ABP_H_

#define ABP_MSG_LEN	8

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *abp_up(int addr, int tspd);
unsigned char *abp_down(int addr, int tspd);
unsigned char *abp_left(int addr, int pspd);
unsigned char *abp_right(int addr, int pspd);
unsigned char *abp_left_up(int addr, int pspd, int tspd);
unsigned char *abp_left_down(int addr, int pspd, int tspd);
unsigned char *abp_right_up(int addr, int pspd, int tspd);
unsigned char *abp_right_down(int addr, int pspd, int tspd);
unsigned char *abp_zoom_plus(int addr);
unsigned char *abp_zoom_minus(int addr);
unsigned char *abp_focus_plus(int addr);
unsigned char *abp_focus_minus(int addr);
unsigned char *abp_iris_plus(int addr);
unsigned char *abp_iris_minus(int addr);
unsigned char *abp_stop(int addr);
unsigned char *abp_preset_set(int addr, int preno);
unsigned char *abp_preset_goto(int addr, int preno);

#ifdef __cplusplus
}
#endif

#endif /* ABP_H_ */
