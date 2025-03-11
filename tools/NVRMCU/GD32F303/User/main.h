#ifndef MAIN_H
#define MAIN_H

#include "gd32f30x.h"
#include <stdio.h>

#define MSLOG(format, ...) \
    do{\
		printf("\n\r[func:%s file:%s line:%d] " format, \
		 __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)

#if 0
#define MS_LOCK(lock) \
    do{\
        while(lock); \
        lock = 1; \
    }while(0)
#define MS_UNLOCK(lock) \
    do{\
        lock = 0; \
    }while(0)
#else
#define MS_LOCK(lock) 
#define MS_UNLOCK(lock) 
#endif

typedef enum{
    KEY_NONE = 0,
    KEY_SHORT,
    KEY_LONG,
}KEY_STATE;

//指令A:0xAA+0x01+0x55 :CPU启动完成后通知单片机
//指令B:0xAA+0x03+0x55 :短按一次后单片机通知CPU是否要关机
//指令C:0xAA+0x05+0x55 :长按或者连续短按两次后，单片机通知CPU强制关机
//指令D:0xAA+0x07+0x55 :CPU准备好关机后通知单片机需要断电
//指令E:0xAA+0x09+0x55 :QT选择重启后，CPU发送重启命令
//指令F:0xAA+0x0B+0x55 :CPU通知单片机准备关机，LED闪烁等待30S
#define CMD_LEN (3)
typedef enum{
    CMD_NONE = -1,
    CMD_A = 0,
    CMD_B,
    CMD_C,
    CMD_D,
    CMD_E,
    CMD_F,
    CMD_NUM,
}CMD_EN;

void ms_delay(uint32_t ms);

void ms_usart_init(void);
void ms_usart_send_cmd(CMD_EN cmdNo);
uint8_t ms_usart_rcv_cmd(CMD_EN cmdNo);

void ms_gpio_init(void);

void ms_set_power_state(uint32_t data);
uint32_t ms_get_power_state(void);

void  ms_led_off(void);
void  ms_led_on(void);
void ms_led_toggle(void);

uint8_t ms_power_key_state(KEY_STATE state);
void ms_power_key_clear(void);

void ms_power_on(void);
void ms_power_off(void);

void ms_shutdown_ready(void);
uint8_t ms_shutdown_is_timeout(void);

#endif /* MAIN_H */
