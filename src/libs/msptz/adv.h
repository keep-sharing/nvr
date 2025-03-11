/*
 * adv.h
 *
 *  Created on: 2013-2-2
 *      Author: root
 */

#ifndef ADV_H_
#define ADV_H_

#define ADV_MSG_LEN	8

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *adv_up(int addr);
unsigned char *adv_down(int addr);
unsigned char *adv_left(int addr);
unsigned char *adv_right(int addr);
unsigned char *adv_left_up(int addr);
unsigned char *adv_left_down(int addr);
unsigned char *adv_right_up(int addr);
unsigned char *adv_right_down(int addr);
unsigned char *adv_stop(int addr);
unsigned char *adv_zoom_plus(int addr);
unsigned char *adv_zoom_minus(int addr);
unsigned char *adv_focus_plus(int addr);
unsigned char *adv_focus_minus(int addr);
unsigned char *adv_iris_plus(int addr);
unsigned char *adv_iris_minus(int addr);

#ifdef __cplusplus
}
#endif
#endif /* ADV_H_ */
