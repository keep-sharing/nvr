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

#define SENSOR_RST_IO	MS_GPIO(0,0)
#define SENSOR_CLK_IO	MS_GPIO(0,5)
#define SYS_LED_IO		MS_GPIO(6,0)	/* Low on */
#define IRCUT_IO		MS_GPIO(15,0)	/* High change to night */
#define IRLED_IO		MS_GPIO(7,0)	/* High on */
#define ALARM_IN_IO		MS_GPIO(9,4)
#define ALARM_OUT_IO	MS_GPIO(9,6)
#define SYS_RESET_IO	MS_GPIO(9,7)
#define DC_IRIS_IO		MS_GPIO(10,5)	/* DC iris control IO */
#define RS485_EN_IO		MS_GPIO(9,2)

#define ALARM_OUT_IO_0	MS_GPIO(12,5)
#define ALARM_OUT_IO_1	GPIO_INVALID
#define ALARM_OUT_IO_2	GPIO_INVALID
#define ALARM_OUT_IO_3	GPIO_INVALID

#define ALARM_IN_IO_0	MS_GPIO(12,0)
#define ALARM_IN_IO_1	MS_GPIO(12,1)
#define ALARM_IN_IO_2	MS_GPIO(12,2)
#define ALARM_IN_IO_3	MS_GPIO(12,4)
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

#define GREEN_LED_IO    MS_GPIO(5,0)
#define RED_LED_IO      MS_GPIO(8,5)
#define HDD_LED_IO      MS_GPIO(3,5)

#define RS485_DIR_IO    MS_GPIO(7,2)
#define MUTE_CTL_IO     MS_GPIO(9,0)

#define RESET_IO        MS_GPIO(8,4)

#define MCU_P1_0_IO     MS_GPIO(9,7)
#define MCU_P1_1_IO     MS_GPIO(8,6)
#define BUZZ_EN_IO      MS_GPIO(8,0)
#define FAN_EN_IO       MS_GPIO(3,0)

// GIO common defined
typedef GPIO_CMD_E	HI_GPIO_CMD_E;
typedef GPIO_DIR_E	HI_GPIO_DIR_E;
typedef GPIO_DATA_T	HI_GPIO_CTRL_T;


#endif	/* __HI_GPIO_COMMON_H__ */


