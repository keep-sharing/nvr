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

/* GPIO pin number translation  */
#define C_GPIO(pin)                  (pin)
#define J_GPIO(pin)                  (pin + 0x20)
#define P_GPIO(pin)                  (pin + 0x40)
#define E_GPIO(pin)                  (pin + 0x80)
#define D_GPIO(pin)                  (pin + 0xA0)
#define S_GPIO(pin)                  (pin + 0xC0)
#define B_GPIO(pin)                  (pin + 0x120)

#define ALARM_OUT_IO_0	P_GPIO(23)
#define ALARM_OUT_IO_1	GPIO_INVALID
#define ALARM_OUT_IO_2	GPIO_INVALID
#define ALARM_OUT_IO_3	GPIO_INVALID

#define ALARM_IN_IO_0	D_GPIO(6)
#define ALARM_IN_IO_1	D_GPIO(7)
#define ALARM_IN_IO_2	D_GPIO(8)
#define ALARM_IN_IO_3	D_GPIO(9) 
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

#define GREEN_LED_IO    P_GPIO(43)
#define RED_LED_IO      GPIO_INVALID
#define HDD_LED_IO      P_GPIO(42)

#define RS485_DIR_IO    GPIO_INVALID
#define MUTE_CTL_IO     GPIO_INVALID

#define RESET_IO        D_GPIO(1)

#define MCU_P1_0_IO     GPIO_INVALID
#define MCU_P1_1_IO     GPIO_INVALID

#define FAN_EN_IO       P_GPIO(31)

/************BUZZER*************/
#define BUZZ_EN_IO            D_GPIO(0)

// GIO common defined
typedef GPIO_CMD_E	NVT_GPIO_CMD_E;
typedef GPIO_DIR_E	NVT_GPIO_DIR_E;
typedef GPIO_DATA_T	NVT_GPIO_CTRL_T;


#endif	/* __NT_GPIO_COMMON_H__ */

