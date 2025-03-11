#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_
#include <stdint.h>
#include <stdio.h>

#define LED_PORT                GPIOA
#define LED_PIN                 GPIO_PIN_1

#define POWER_EN_PORT           GPIOA
#define POWER_EN_PIN            GPIO_PIN_12

#define POWER_KEY_PORT          GPIOA
#define POWER_KEY_PIN           GPIO_PIN_2

//alarm out
#define ALARM_OUT_NUM           4
#define ALARM_OUT1_PORT         GPIOB
#define ALARM_OUT1_PIN          GPIO_PIN_9
#define ALARM_OUT2_PORT         GPIOB
#define ALARM_OUT2_PIN          GPIO_PIN_8
#define ALARM_OUT3_PORT         GPIOB
#define ALARM_OUT3_PIN          GPIO_PIN_7
#define ALARM_OUT4_PORT         GPIOB
#define ALARM_OUT4_PIN          GPIO_PIN_6

//alarm in
#define ALARM_IN_NUM            16
#define ALARM_IN1_PORT          GPIOB
#define ALARM_IN1_PIN           GPIO_PIN_5
#define ALARM_IN2_PORT          GPIOB
#define ALARM_IN2_PIN           GPIO_PIN_4
#define ALARM_IN3_PORT          GPIOB
#define ALARM_IN3_PIN           GPIO_PIN_3
#define ALARM_IN4_PORT          GPIOA
#define ALARM_IN4_PIN           GPIO_PIN_15
#define ALARM_IN5_PORT          GPIOA
#define ALARM_IN5_PIN           GPIO_PIN_11
#define ALARM_IN6_PORT          GPIOA
#define ALARM_IN6_PIN           GPIO_PIN_8
#define ALARM_IN7_PORT          GPIOB
#define ALARM_IN7_PIN           GPIO_PIN_15
#define ALARM_IN8_PORT          GPIOB
#define ALARM_IN8_PIN           GPIO_PIN_14
#define ALARM_IN9_PORT          GPIOB
#define ALARM_IN9_PIN           GPIO_PIN_13
#define ALARM_IN10_PORT         GPIOB
#define ALARM_IN10_PIN          GPIO_PIN_12
#define ALARM_IN11_PORT         GPIOB
#define ALARM_IN11_PIN          GPIO_PIN_11
#define ALARM_IN12_PORT         GPIOB
#define ALARM_IN12_PIN          GPIO_PIN_10
#define ALARM_IN13_PORT         GPIOB
#define ALARM_IN13_PIN          GPIO_PIN_1
#define ALARM_IN14_PORT         GPIOB
#define ALARM_IN14_PIN          GPIO_PIN_0
#define ALARM_IN15_PORT         GPIOA
#define ALARM_IN15_PIN          GPIO_PIN_7
#define ALARM_IN16_PORT         GPIOA
#define ALARM_IN16_PIN          GPIO_PIN_6



void uart0_sendstring(unsigned char arr[], unsigned int length);
void sys_config(void);
#endif
