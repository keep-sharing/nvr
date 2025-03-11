/*
 * sony_d70.h
 *
 *  Created on: 2013-1-29
 *      Author: root
 */

#ifndef SONY_D70_H_
#define SONY_D70_H_

#define SONY_D70_MSG_LEN	9

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char *sonyd70_stop(int addr);
unsigned char *sonyd70_right_down(int addr, int pspd, int tspd);
unsigned char *sonyd70_right_up(int addr, int pspd, int tspd);
unsigned char *sonyd70_left_down(int addr, int pspd, int tspd);
unsigned char *sonyd70_left_up(int addr, int pspd, int tspd);
unsigned char *sonyd70_right(int addr, int pspd);
unsigned char *sonyd70_left(int addr, int pspd);
unsigned char *sonyd70_up(int addr, int tspd);
unsigned char *sonyd70_down(int addr, int tspd);

#ifdef __cplusplus
}
#endif
#endif /* SONY_D70_H_ */
