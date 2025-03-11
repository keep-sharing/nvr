/* 
 * ***************************************************************
 * Filename:      	maic.c
 * Created at:    	2019.01.02
 * Description:   	power controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#include "main.h"

typedef enum {
    LAST_POWER_ON = 0x5555,
    LAST_POWER_OFF = 0xaaaa,
}POWER_STATE;

typedef enum{
    STATE_POWEROFF = 0,
    STATE_IDEL,
    STATE_BOOT,
    STATE_RUN,
    STATE_SHUTDOWN,
}STATE_EN;

int main(void)
{
  STATE_EN state = STATE_POWEROFF;
  
  ms_usart_init();
  ms_gpio_init();
  while (1)
  {
    switch(state)
    {
        case STATE_POWEROFF:
            {
                ms_led_off();
                if (ms_get_power_state() == LAST_POWER_OFF)
                    state = STATE_IDEL;
                else
                {
                    ms_power_on();
                    ms_set_power_state(LAST_POWER_ON);
                    state = STATE_BOOT;
                }
            }
            break;
        case STATE_IDEL:
            {
                if (ms_power_key_state(KEY_SHORT) || ms_power_key_state(KEY_LONG))
                {
                    ms_power_on();
                    ms_set_power_state(LAST_POWER_ON);
                    state = STATE_BOOT;
                }
            }
            break;
        case STATE_BOOT:
            {
                if (ms_usart_rcv_cmd(CMD_A))
                {
                    MSLOG("rcv CMD_A \n");
                    state = STATE_RUN;
                }
                if (ms_power_key_state(KEY_SHORT) || ms_power_key_state(KEY_LONG))
                {
                    ms_power_off();
                    ms_set_power_state(LAST_POWER_OFF);
                    state = STATE_IDEL;
                }
            }
            break;
        case STATE_RUN:
            {
                if (ms_power_key_state(KEY_SHORT))
                {
                    ms_usart_send_cmd(CMD_B);
                    MSLOG("send CMD_B \n");
                }
                else if (ms_power_key_state(KEY_LONG))
                {
                    ms_usart_send_cmd(CMD_C);
                    MSLOG("send CMD_C \n");
                    ms_shutdown_ready();
                    state = STATE_SHUTDOWN;
                }

                if (ms_usart_rcv_cmd(CMD_E))
                {
                    MSLOG("rcv CMD_E \n");
                    state = STATE_BOOT;
                }
                else if (ms_usart_rcv_cmd(CMD_F))
                {
                    MSLOG("rcv CMD_F \n");
                    ms_shutdown_ready();
                    state = STATE_SHUTDOWN;
                }

            }
            break;
        case STATE_SHUTDOWN:
            {
                if (ms_usart_rcv_cmd(CMD_D)|| ms_shutdown_is_timeout())
                {
                    MSLOG("rcv CMD_D \n");
                    ms_power_off();
                    ms_set_power_state(LAST_POWER_OFF);
                    state = STATE_IDEL;
                }
            }
            break;
        default:
            break;
    }
  }
}

