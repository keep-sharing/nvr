/*!
    \file    gd32f30x_it.c
    \brief   interrupt service routines

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f30x_it.h"
#include "systick.h"
#include "sys_config.h"
#include "ms_command_handler.h"
#include "ms_common.h"

extern __Rcv_Queue ms_uart0;
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    delay_decrement();
}

void TIMER3_IRQHandler(void)
{
    static uint16_t key_holdon_ms = 0;

    if (SET == timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP)) {
        /* clear update interrupt bit */
        timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
        if(keyInfo.key_fall_flag)
        {
            if(gpio_input_bit_get(POWER_KEY_PORT, POWER_KEY_PIN) == 0)
            {
                if(key_holdon_ms < KEY_TIME_OUT)
                {
                    key_holdon_ms++;
                }
                else if(key_holdon_ms >= KEY_TIME_OUT)
                {
                    key_holdon_ms = 0;
                    
                    MS_LOCK(keyInfo.key_state_lock);
                    keyInfo.key_state = KEY_LONG;
                    MS_UNLOCK(keyInfo.key_state_lock);
                    
                    keyInfo.key_fall_flag = 0;
                    //MSLOG("power key long press!!\n");
                }
            }
            else
            {
                if(key_holdon_ms > 50)
                {
                    key_holdon_ms = 0;
                    MS_LOCK(keyInfo.key_state_lock);
                    keyInfo.key_state = KEY_SHORT;
                    MS_UNLOCK(keyInfo.key_state_lock);
                    //MSLOG("power key short press!!\n");
                }
                else
                {
                    key_holdon_ms = 0;
                    MS_LOCK(keyInfo.key_state_lock);
                    keyInfo.key_state = KEY_NONE;
                    MS_UNLOCK(keyInfo.key_state_lock);
                    //MSLOG("power key release!!\n");
                }
                keyInfo.key_fall_flag = 0;
            }
        }

        if (keyInfo.shutdown_ready)
        {
            keyInfo.shutdown_time_ms++;
            if (keyInfo.shutdown_time_ms%500 == 0)
                ms_led_toggle();
        }
    }
}

void TIMER4_IRQHandler(void)
{
    static int cnt = 0;

    if (SET == timer_interrupt_flag_get(TIMER4, TIMER_INT_FLAG_UP)) {
        /* clear update interrupt bit */
        timer_interrupt_flag_clear(TIMER4, TIMER_INT_FLAG_UP);
        ++cnt;
        if (cnt >= 4) {//40ms
            cnt = 0;
            alarm_in_control();
        }
    }
}

void EXTI2_IRQHandler(void)
{
    if (RESET != exti_interrupt_flag_get(EXTI_2)) {
        exti_interrupt_flag_clear(EXTI_2);
        keyInfo.key_fall_flag = 1;
    }
}

void USART0_IRQHandler(void)
{
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) {
        /* receive data */
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
        ms_data_enter_queue(&ms_uart0, (uint8_t)usart_data_receive(USART0));
    }
}
