/* 
 * ***************************************************************
 * Filename:      	gpio_map.h
 * Created at:    	2015.10.21
 * Description:   	gpio map.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */



#ifndef __GPIO_MAP_H__
#define __GPIO_MAP_H__

#include "gpio_common.h"

// Hisi gpio pin defined
#define MS_GPIO(g,n)	(g*8+n)

#define ALARM_OUT_IO_0	MS_GPIO(1,2) //muxctrl_reg61(0X120F00F4) 0:uart0_rtsn 1:GPIO6_3, uboot set 1 to GPIO
#define ALARM_OUT_IO_1	GPIO_INVALID
#define ALARM_OUT_IO_2	GPIO_INVALID
#define ALARM_OUT_IO_3	GPIO_INVALID

#define ALARM_IN_IO_0	MS_GPIO(10,2)
#define ALARM_IN_IO_1	MS_GPIO(10,3)
#define ALARM_IN_IO_2	MS_GPIO(10,4)
#define ALARM_IN_IO_3	MS_GPIO(10,5) //muxctrl_reg60(0X120F00F0) 0:uart0_ctsn 1:GPIO6_2, uboot set 1 to GPIO
#define ALARM_IN_IO_4	GPIO_INVALID
#define ALARM_IN_IO_5	GPIO_INVALID
#define ALARM_IN_IO_6	GPIO_INVALID
#define ALARM_IN_IO_7	GPIO_INVALID
#define ALARM_IN_IO_8	GPIO_INVALID
#define ALARM_IN_IO_9	GPIO_INVALID
#define ALARM_IN_IO_A	GPIO_INVALID
#define ALARM_IN_IO_B	GPIO_INVALID
#define ALARM_IN_IO_C	GPIO_INVALID
#define ALARM_IN_IO_D	GPIO_INVALID
#define ALARM_IN_IO_E	GPIO_INVALID
#define ALARM_IN_IO_F	GPIO_INVALID

#define GREEN_LED_IO    MS_GPIO(13,7)
#define RED_LED_IO      GPIO_INVALID
#define HDD_LED_IO      MS_GPIO(14,1)

#define RS485_DIR_IO    MS_GPIO(1,1)
#define MUTE_CTL_IO     GPIO_INVALID
#define RESET_IO        MS_GPIO(1,0)

#define MCU_P1_0_IO     GPIO_INVALID
#define MCU_P1_1_IO     GPIO_INVALID
#define BUZZ_EN_IO      MS_GPIO(2,0)

#define FAN_EN_IO       MS_GPIO(1,7)//muxctrl_reg85(0X120F0154) 0:GPIO10_2 1:IR_IN

// GIO common defined
typedef GPIO_CMD_E	HI_GPIO_CMD_E;
typedef GPIO_DIR_E	HI_GPIO_DIR_E;
typedef GPIO_DATA_T	HI_GPIO_CTRL_T;


#endif	/* __HI_GPIO_COMMON_H__ */


