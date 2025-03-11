/****
 * 
 * nt_gpio_common.h
 * 
 * Des:
 *   Define common gpio information.
 * 
 * History:
 *  2015/06/16  - [eric@milesight.cn] Create file.
 * 
 * Copyright (C) 2011-2015, Milesight, Inc.
 * 
 * All rights reserved. No Part of this file may be reproduced, stored
 * in a retrieval system, or transmitted, in any form, or by any means,
 * electronic, mechanical, photocopying, recording, or otherwise,
 * without the prior consent of Milesight, Inc.
 * 
 */


#ifndef __GPIO_MAP_H__
#define __GPIO_MAP_H__

#include "gpio_common.h"

// Hisi gpio pin defined
#define MS_GPIO(g,n)	(g*32+n)


#define ALARM_OUT_IO_0	MS_GPIO(0,31)
#define ALARM_OUT_IO_1	GPIO_INVALID
#define ALARM_OUT_IO_2	GPIO_INVALID
#define ALARM_OUT_IO_3	GPIO_INVALID

#define ALARM_IN_IO_0	MS_GPIO(1,0)
#define ALARM_IN_IO_1	MS_GPIO(0,17)
#define ALARM_IN_IO_2	MS_GPIO(0,18)
#define ALARM_IN_IO_3	MS_GPIO(0,19) 
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

#define GREEN_LED_IO    MS_GPIO(1,10)
#define RED_LED_IO      GPIO_INVALID
#define HDD_LED_IO      MS_GPIO(1,9)

#define RS485_DIR_IO    GPIO_INVALID
#define MUTE_CTL_IO     GPIO_INVALID

#define RESET_IO        MS_GPIO(1,8)

#define MCU_P1_0_IO     GPIO_INVALID
#define MCU_P1_1_IO     GPIO_INVALID

#define FAN_EN_IO       MS_GPIO(1,7)

/************BUZZER*************/
#define BUZZ_EN_IO            MS_GPIO(0,30)
#define BUZZ_MUX_REG          0x2C
#define BUZZ_MUX_DATA   (7 << 28)

/************IP1819*************/
#define SWITCH_RESET_IO        	        MS_GPIO(1,17)
#define IP1819_SWITCH_SCL_REG_IO	MS_GPIO(1,30)	
#define IP1819_SWITCH_SDA_REG_IO	MS_GPIO(1,31)	

/************RTL8309*************/
#define MDIO_SCL_REG	MS_GPIO(1,30)
#define MDIO_SDA_REG	MS_GPIO(1,31)

#define MDIO_SCL_MUX_REG          0x54
#define MDIO_SCL_MUX_DATA   (7 << 24)

#define MDIO_SDA_MUX_REG   	  0x54
#define MDIO_SDA_MUX_DATA   (7 << 28)

// GIO common defined
typedef GPIO_CMD_E	NVT_GPIO_CMD_E;
typedef GPIO_DIR_E	NVT_GPIO_DIR_E;
typedef GPIO_DATA_T	NVT_GPIO_CTRL_T;


#endif	/* __NT_GPIO_COMMON_H__ */

