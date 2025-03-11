/*
 * ptz_term_info.c
 *
 *  Created on: 2012-11-22
 *      Author: chimmu
 */
#include "ptz_term_info.h"
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ptz_public.h"

unsigned int ptz_get_baud_rate(unsigned int nBaud)
{
	unsigned int nBaudRate;

	switch(nBaud){
		case B_1200:
//			printf("===set baudrate B1200 ===\n");
			nBaudRate = B1200;
			break;
		case B_1800:
//			printf("===set baudrate B1800 ===\n");
			nBaudRate = B1800;
			break;
		case B_2400:
			nBaudRate = B2400;
//			printf("===set baudrate B2400 ===\n");
			break;
		case B_4800:
//			printf("===set baudrate B4800 ===\n");
			nBaudRate = B4800;
			break;
		case B_9600:
//			printf("===set baudrate B9600 ===\n");
			nBaudRate = B9600;
			break;
		case B_19200:
//			printf("===set baudrate B19200 ===\n");
			nBaudRate = B19200;
			break;
		case B_38400:
//			printf("===set baudrate B38400 ===\n");
			nBaudRate = B38400;
			break;
		case B_57600:
//			printf("===set baudrate B57600 ===\n");
			nBaudRate = B57600;
			break;
		case B_115200:
//			printf("===set baudrate B115200 ===\n");
			nBaudRate = B115200;
			break;
		default:
//			printf("===set baudrate DEFAULT B9600 ===\n");
			nBaudRate = B9600;
			break;
		}

		return nBaudRate;
}

unsigned int ptz_get_data_bit(unsigned int nBit)
{
	unsigned int nDataBit;

	switch(nBit){
		case D5:
//			printf("===set Databit CS5 ===\n");
			nDataBit = CS5;
			break;
		case D6:
//			printf("===set Databit CS6 ===\n");
			nDataBit = CS6;
			break;
		case D7:
//			printf("===set Databit CS7 ===\n");
			nDataBit = CS7;
			break;
		case D8:
//			printf("===set Databit CS8 ===\n");
			nDataBit = CS8;
			break;
		default:
//			printf("===set Databit DEFAULT CS8 ===\n");
			nDataBit = CS8;
			break;
	}

	return nDataBit;
}

int ptz_set_serial_info(int fd, ptz_serialInfo *info)
{
	struct termios	oldtio,newtio;
	//int ptzCtrlSize;

	tcgetattr(fd, &oldtio);

	bzero(&newtio, sizeof(newtio));

	newtio.c_cflag = CLOCAL | CREAD;//Ignore modem control lines.Enable receiver.

	newtio.c_cflag |= ptz_get_baud_rate(info->ptzBaudrate);
  	newtio.c_cflag |= ptz_get_data_bit(info->ptzDatabit);

  	//////////// parity bit /////////////////
  	if(info->ptzParitybit == PARITY_EVEN)
  	{
  		printf("===set paritybit EVEN===\n");
  	  	newtio.c_cflag |= PARENB;//Enable parity generation  on  output  and  parity  checking  for
        						//input.
  	}
  	else if(info->ptzParitybit == PARITY_ODD)
  	{
  		printf("===set paritybit ODD===\n");
  		newtio.c_cflag |= PARENB;
  		newtio.c_cflag |= PARODD;//If  set, then parity for input and output is odd; otherwise even
  								//parity is used
  	}

  	//////////// stop bit //////////////
	if(info->ptzStopbit == 2)
	{
  		printf("===set stopbit 2 ===\n");
 		newtio.c_cflag |= CSTOPB;//Set two stop bits, rather than one.
		newtio.c_cflag |= CSIZE;//Character size mask.  Values are CS5, CS6, CS7, or CS8.
  	}

  	newtio.c_iflag = IGNPAR | ICRNL;//ignore frame errors and parity errors, Translate  carriage  return to newline on input (unless IGNCR is
    								//set).

  	newtio.c_oflag = 0;
  	newtio.c_lflag = ICANON;//enable canonical mode

  	newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
  	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */

  	tcflush(fd, TCIFLUSH);
  	tcsetattr(fd,TCSANOW,&newtio);//the change occurs immediately.


 // 	write(fd, buf, len);

	return 0;
}

int ptz_init_serial_info(ptz_serialInfo *info)
{
	info->ptzBaudrate = 9600;
	info->ptzDatabit = 8;
	info->ptzParitybit = PTZ_PARITY_NONE;
	info->ptzStopbit = 1;
	return 0;
//	switch(proto){
//	case PTZ_PROTOCOL_PELCO_D:
//		info->ptzBaudrate = 9600;
//		info->ptzDatabit = 8;
//		info->ptzParitybit = PTZ_PARITY_NONE;
//		info->ptzStopbit = 1;
//		break;
//	case PTZ_PROTOCOL_PELCO_P:
//        info->ptzBaudrate = 9600;
//		info->ptzDatabit = 8;
//		info->ptzParitybit = PTZ_PARITY_NONE;
//		info->ptzStopbit = 1;
//		break;
//	case PTZ_PROTOCOL_SAMSUNG_E:
//        info->ptzBaudrate = 9600;
//		info->ptzDatabit = 8;
//		info->ptzStopbit = 1;
//		info->ptzParitybit = PTZ_PARITY_NONE;
//		break;
//	case PTZ_PROTOCOL_SAMSUNG_T:
//        info->ptzBaudrate = 9600;
//		info->ptzDatabit = 8;
//		info->ptzParitybit = PTZ_PARITY_NONE;
//		info->ptzStopbit = 1;
//		break;
//	default:
//		printf("Unknown protocol\n");
//		flag = -1;
//		break;
//	}
//	return flag;
}


