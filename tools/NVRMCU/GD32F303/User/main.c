/* 
 * ***************************************************************
 * Filename:      	maic.c
 * Created at:    	2022.08.29
 * Description:   	power controller
 * Author:        	tanggp
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include "gd32f30x.h"
#include "sys_config.h"
#include "ms_command_handler.h"
#include "flash_api.h"
#include "ms_common.h"

__Rcv_Queue ms_uart0;
MS_FREAM_FIELD parse_status;
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

  sys_config();

  while (1)
  {

    ms_receive_data_handler(&ms_uart0);

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
                if (rcv_cmd == CMD_A)
                {
                    //MSLOG("rcv CMD_A \n");
                    alarmInstatus = 0xffff;
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
                    ret_cmd_b();
                    //MSLOG("send CMD_B \n");
                }
                else if (ms_power_key_state(KEY_LONG))
                {
                    ret_cmd_c();
                    //MSLOG("send CMD_C \n");
                    ms_shutdown_ready();
                    state = STATE_SHUTDOWN;
                }

                if (rcv_cmd == CMD_E)
                {
                    //MSLOG("rcv CMD_E \n");
                    state = STATE_BOOT;
                }
                else if (rcv_cmd == CMD_F)
                {
                    //MSLOG("rcv CMD_F \n");
                    ms_shutdown_ready();
                    state = STATE_SHUTDOWN;
                }

            }
            break;
        case STATE_SHUTDOWN:
            {
                if ((rcv_cmd == CMD_D)|| ms_shutdown_is_timeout())
                {
                    //MSLOG("rcv CMD_D \n");
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
