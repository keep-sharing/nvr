/*
 * ptz_term_info.h
 *
 *  Created on: 2012-11-22
 *      Author: chimmu
 */

#ifndef COMM_H_
#define COMM_H_

#define PTZ_PARITYBIT_MAX                            3
#define PTZ_PARITY_NONE                              0
#define PTZ_PARITY_ODD                               1
#define PTZ_PARITY_EVEN                              2


typedef struct
{
    int ptzDatabit;
    int ptzParitybit;
    int ptzStopbit;
    int ptzBaudrate;
}ptz_serialInfo;


#ifdef __cplusplus
extern "C"
{
#endif



void ptz_set_baud_rate(int baudRate);
unsigned int ptz_get_baud_rate(unsigned int nBaud);
unsigned int ptz_get_data_bit(unsigned int nBit);
int ptz_init_serial_info(ptz_serialInfo *info);
int ptz_set_serial_info(int fd, ptz_serialInfo *info);

#ifdef __cplusplus
}
#endif

#endif /* COMM_H_ */
