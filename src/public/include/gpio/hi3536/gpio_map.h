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

#define ALARM_OUT_IO_0	MS_GPIO(0,0)
#define ALARM_OUT_IO_1	MS_GPIO(0,1)
#define ALARM_OUT_IO_2	MS_GPIO(0,2)
#define ALARM_OUT_IO_3	MS_GPIO(0,3)

#define ALARM_IN_IO_0	MS_GPIO(0,7)
#define ALARM_IN_IO_1	MS_GPIO(14,2)
#define ALARM_IN_IO_2	MS_GPIO(4,0)
#define ALARM_IN_IO_3	MS_GPIO(4,2)
#define ALARM_IN_IO_4	MS_GPIO(14,1)
#define ALARM_IN_IO_5	MS_GPIO(14,4)
#define ALARM_IN_IO_6	MS_GPIO(0,5)
#define ALARM_IN_IO_7	MS_GPIO(15,4)
#define ALARM_IN_IO_8	MS_GPIO(0,6)
#define ALARM_IN_IO_9	MS_GPIO(0,4)
#define ALARM_IN_IO_A	MS_GPIO(14,0)
#define ALARM_IN_IO_B	MS_GPIO(4,1)
#define ALARM_IN_IO_C	MS_GPIO(4,3)
#define ALARM_IN_IO_D	MS_GPIO(14,3)
#define ALARM_IN_IO_E	MS_GPIO(11,4)
#define ALARM_IN_IO_F	MS_GPIO(4,5)

#define GREEN_LED_IO    MS_GPIO(8,4)
#define RED_LED_IO      MS_GPIO(8,5)
#define HDD_LED_IO      MS_GPIO(8,6)

#define RS485_DIR_IO    MS_GPIO(7,2)
#define MUTE_CTL_IO     MS_GPIO(2,6)
#define MCU_P1_0_IO     MS_GPIO(2,5)
#define MCU_P1_1_IO     MS_GPIO(2,4)
#define BUZZ_EN_IO      MS_GPIO(4,7)
#define FAN_EN_IO       MS_GPIO(7,3)
#define RESET_IO        MS_GPIO(15,3)

// GIO common defined
typedef GPIO_CMD_E	HI_GPIO_CMD_E;
typedef GPIO_DIR_E	HI_GPIO_DIR_E;
typedef GPIO_DATA_T	HI_GPIO_CTRL_T;


#endif	/* __HI_GPIO_COMMON_H__ */


