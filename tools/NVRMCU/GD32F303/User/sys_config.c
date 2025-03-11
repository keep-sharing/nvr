/* 
 * ***************************************************************
 * Filename:      	sys_config.c
 * Created at:    	2022.08.29
 * Description:
 * Author:        	tanggp
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include "gd32f30x.h"
#include "sys_config.h"
#include "flash_api.h"
#include "ms_command_handler.h"
#include "ms_common.h"

static void rcc_init(void)
{
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_TIMER3);
    rcu_periph_clock_enable(RCU_TIMER4);
}

static void all_gpio_init(void)
{
    int i = 0;
    //led
    gpio_init(LED_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);

    //power_en
    gpio_init(POWER_EN_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, POWER_EN_PIN);

    //power_key
    gpio_init(POWER_KEY_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, POWER_KEY_PIN);
    
    //uart0
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    //alarm out
    for (i = 0; i < ALARM_IN_NUM; i++) {
        if (i < ALARM_OUT_NUM) {
            gpio_init(alarmOut[i].port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, alarmOut[i].pin);
        }
        gpio_init(alarmIn[i].port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, alarmIn[i].pin);
    }
}

static void exit_config(void)
{
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_2);
    exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_interrupt_flag_clear(EXTI_2);
    nvic_irq_enable(EXTI2_IRQn, 0, 0);
}

static void timer_config(uint32_t timer, uint16_t prescaler, uint16_t period)
{
    timer_parameter_struct timer_initpara;

    timer_deinit(timer);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER1 configuration */
    timer_initpara.prescaler         = prescaler;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = period;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(timer, &timer_initpara);
    
    /* enable the TIMER interrupt */
    timer_interrupt_flag_clear(timer, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(timer, TIMER_INT_UP);

    timer_enable(timer);
}

static void uart0_init(uint32_t baudrate)
{
    usart_deinit(USART0);
    usart_baudrate_set(USART0, baudrate);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
    nvic_irq_enable(USART0_IRQn, 0, 1);
    usart_interrupt_enable(USART0, USART_INT_RBNE);
}

void uart0_sendstring(unsigned char arr[], unsigned int length)
{
    int i;
    arr[length - 1] = 0;

    for (i = 2; i < length - 1; i++) {
        arr[length - 1] += arr[i];
    }

    for (unsigned int i = 0; i < length; i++) {
        usart_data_transmit(USART0, arr[i]);
        while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET);
    }
}

extern MS_FREAM_FIELD parse_status;
void sys_config(void)
{
    parse_status = MS_CMD_NONE;
    rcc_init();
    all_gpio_init();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    uart0_init(115200);
    exit_config();
    timer_config(TIMER3, 119, 1000);//1ms
    timer_config(TIMER4, 119, 10000);//10ms
    nvic_irq_enable(TIMER3_IRQn, 1, 0);
    nvic_irq_enable(TIMER4_IRQn, 1, 1);
    init_flash();
}
