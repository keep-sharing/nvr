/* 
 * ***************************************************************
 * Filename:      	gpio.h
 * Created at:    	2015.10.21
 * Description:   	common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */


#ifndef __GPIO_H__
#define __GPIO_H__


//gio define
#include "gpio_map.h"

#define GIO_ALARM_OUT_0         (ALARM_OUT_IO_0)
#define GIO_ALARM_OUT_1         (ALARM_OUT_IO_1)
#define GIO_ALARM_OUT_2         (ALARM_OUT_IO_2)
#define GIO_ALARM_OUT_3         (ALARM_OUT_IO_3)

#define GIO_ALARM_IN_0          (ALARM_IN_IO_0)
#define GIO_ALARM_IN_1          (ALARM_IN_IO_1)
#define GIO_ALARM_IN_2          (ALARM_IN_IO_2)
#define GIO_ALARM_IN_3          (ALARM_IN_IO_3)
#define GIO_ALARM_IN_4          (ALARM_IN_IO_4)
#define GIO_ALARM_IN_5          (ALARM_IN_IO_5)
#define GIO_ALARM_IN_6          (ALARM_IN_IO_6)
#define GIO_ALARM_IN_7          (ALARM_IN_IO_7)
#define GIO_ALARM_IN_8          (ALARM_IN_IO_8)
#define GIO_ALARM_IN_9          (ALARM_IN_IO_9)
#define GIO_ALARM_IN_10         (ALARM_IN_IO_A)
#define GIO_ALARM_IN_11         (ALARM_IN_IO_B)
#define GIO_ALARM_IN_12         (ALARM_IN_IO_C)
#define GIO_ALARM_IN_13         (ALARM_IN_IO_D)
#define GIO_ALARM_IN_14         (ALARM_IN_IO_E)
#define GIO_ALARM_IN_15         (ALARM_IN_IO_F)
#define GIO_ALARM_UNDEFINED     0XFF

#define STATE_LED_GREEN         (GREEN_LED_IO)
#define STATE_LED_RED           (RED_LED_IO)
#define STATE_LED_HDD           (HDD_LED_IO)
#if !defined(_HI3536A_)
#define LED_ON            (0)
#define LED_OFF           (1)  
#else
#define LED_ON            (1)
#define LED_OFF           (0)
#endif
#define GIO_RS485_CTL_EN 		(RS485_DIR_IO)
#define RS485_DIR_RX 			(0)
#define RS485_DIR_TX 			(1)

#define GIO_MUTE_CTL_EN         (MUTE_CTL_IO)
#define MUTE_ON 			    (0)
#define MUTE_OFF 			    (1)

#define GIO_MCU_P1_0            (MCU_P1_0_IO)
#define GIO_MCU_P1_1            (MCU_P1_1_IO)

#define GIO_BUZZ_EN             (BUZZ_EN_IO)
#define GIO_FAN_EN              (FAN_EN_IO)
#define GIO_RESET_FACTORY       (RESET_IO)


typedef enum
{
	GPIO_LOW		= 0,	// gio low level
	GPIO_HIGH,				// gio high level
	
}GPIO_STATUS_E;

int gpio_config(int gio_id, GPIO_DIR_E gio_dir );
int gpio_write(int gio_id, int gio_level);
int gpio_read(int gio_id);
int gpio_open(void);
int gpio_close(void);


#endif /* __GPIO_INTERFACE_H__ */


