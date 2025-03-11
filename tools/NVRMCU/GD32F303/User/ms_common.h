#ifndef MS_COMMON_H
#define MS_COMMON_H

#include "sys_config.h"

#define KEY_TIME_OUT            (3000) //over 3S   indicate long press
#define SHUTDOWN_TIME_MAX       (30000) //over 30S   indicate force to shutdown

typedef struct {
    uint32_t port;
    uint32_t pin;
}ms_gpio_t;

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
      
typedef enum {
    CMD_NONE = 0X0,
    CMD_A = 0x01,
    CMD_B = 0x03,
    CMD_C = 0x05,
    CMD_D = 0x07,
    CMD_E = 0x09,
    CMD_F = 0x0B,
    CMD_ALARM_OUT = 0x0C,
    CMD_ALARM_IN = 0x0D,
    CMD_TEST_ALARM_IN = 0x0E
} UART_CMD_E;

typedef enum{
    KEY_NONE = 0,
    KEY_SHORT,
    KEY_LONG,
}KEY_STATE;

typedef struct{
    volatile unsigned char key_fall_flag;
    volatile KEY_STATE key_state;
    volatile unsigned char key_state_lock;
    volatile unsigned char shutdown_ready;
    volatile unsigned int shutdown_time_ms;
}keyInfo_t;

extern volatile UART_CMD_E rcv_cmd;
extern keyInfo_t keyInfo;
extern ms_gpio_t alarmIn[ALARM_IN_NUM];
extern ms_gpio_t alarmOut[ALARM_OUT_NUM];
extern unsigned int alarmInstatus;

void ms_led_off(void);
void ms_led_on(void);
void ms_led_toggle(void);
unsigned char ms_power_key_state(KEY_STATE state);
void ms_power_key_clear(void);
void ms_power_on(void);
void ms_power_off(void);
void ms_shutdown_ready(void);
unsigned char ms_shutdown_is_timeout(void);
unsigned char alarm_in_trigger(void);
void alarm_in_control(void);
void ret_cmd_b(void);
void ret_cmd_c(void);
void alarm_in_test(void);
#endif
