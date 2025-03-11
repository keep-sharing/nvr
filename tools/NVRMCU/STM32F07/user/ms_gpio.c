/* 
 * ***************************************************************
 * Filename:      	ms_gpio.c
 * Created at:    	2019.01.02
 * Description:   	gpio led/key api
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include "main.h"

#define LED_PIN         GPIO_Pin_4
#define POWER_EN        GPIO_Pin_1
#define POWER_KEY       GPIO_Pin_0
#define KEY_TIME_OUT    (3000) //over 3S   indicate long press
#define SHUTDOWN_TIME_MAX    (30000) //over 30S   indicate force to shutdown

volatile uint8_t led_state = 0;

volatile uint8_t key_fall_flag = 0;
volatile KEY_STATE key_state = KEY_NONE;
volatile uint8_t key_state_lock = 0;

volatile uint8_t shutdown_ready = 0;
volatile uint32_t shutdown_time_ms = 0;

void EXTI0_1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        key_fall_flag = 1;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void TIM3_IRQHandler(void)
{
    static uint16_t key_holdon_ms = 0;
    
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        if(key_fall_flag)
        {
            if(GPIO_ReadInputDataBit(GPIOA, POWER_KEY) == 0)
            {
                if(key_holdon_ms < KEY_TIME_OUT)
                {
                    key_holdon_ms++;
                }
                else if(key_holdon_ms >= KEY_TIME_OUT)
                {
                    key_holdon_ms = 0;
                    
                    MS_LOCK(key_state_lock);
                    key_state = KEY_LONG;
                    MS_UNLOCK(key_state_lock);
                    
                    key_fall_flag = 0;
                    MSLOG("power key long press!!\n");
                }
            }
            else
            {
                if(key_holdon_ms > 50)
                {
                    key_holdon_ms = 0;
                    MS_LOCK(key_state_lock);
                    key_state = KEY_SHORT;
                    MS_UNLOCK(key_state_lock);
                    MSLOG("power key short press!!\n");
                }
                else
                {
                    key_holdon_ms = 0;
                    MS_LOCK(key_state_lock);
                    key_state = KEY_NONE;
                    MS_UNLOCK(key_state_lock);
                    MSLOG("power key release!!\n");
                }
                key_fall_flag = 0;
            }
        }

        if (shutdown_ready)
        {
            shutdown_time_ms++;
            if (shutdown_time_ms%500 == 0)
                ms_led_toggle();
        }
    }
}

static void led_init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin        = LED_PIN ;
    GPIO_InitStructure.GPIO_Mode       = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType      = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed      = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_PuPd       = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void key_init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    
    // PA1: power_en -->output
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin        = POWER_EN ;
    GPIO_InitStructure.GPIO_Mode       = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType      = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed      = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_PuPd       = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    
    // PA1: power_key -->input
    GPIO_InitStructure.GPIO_Pin        = POWER_KEY ;            
    GPIO_InitStructure.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd       = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure); 

      /* Enable GPIOA clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    /* Configure PA0 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = POWER_KEY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    /* Connect EXTI0 Line to PA0 pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    /* Configure EXTI0 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void timer_init(uint16_t ms)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = ms * 1000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler =(8-1); 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
 
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_Cmd(TIM3, ENABLE);
							 
}

void ms_delay(uint32_t ms)
{
    int x, y;
    for (x=0; x<ms; x++)
        for(y=0; y<1000; y++);
}

void  ms_led_off(void)
{
    GPIO_SetBits(GPIOA, LED_PIN);
    led_state = 0;
}

void  ms_led_on(void)
{
    GPIO_ResetBits(GPIOA, LED_PIN);
    led_state = 1;
}

void ms_led_toggle(void)
{
    if (led_state)
        ms_led_off();
    else
        ms_led_on();
}

uint8_t ms_power_key_state(KEY_STATE state)
{
    if (key_state == state) 
    {
        key_state = KEY_NONE;
        return 1;
    }
    return 0;
}

void ms_power_key_clear(void)
{
    key_state = KEY_NONE;
}

void ms_power_on(void)
{
    GPIO_SetBits(GPIOA, POWER_EN);
    ms_led_on();
}

void ms_power_off(void)
{
    GPIO_ResetBits(GPIOA, POWER_EN);
    ms_led_off();
    key_state = KEY_NONE;
    shutdown_ready = 0;
}

void ms_shutdown_ready(void)
{
    if (shutdown_ready)
        return;
    shutdown_ready = 1;
    shutdown_time_ms = 0;
}

uint8_t ms_shutdown_is_timeout(void)
{    
    if (shutdown_ready == 0)
        return 0;
    return (shutdown_time_ms >= SHUTDOWN_TIME_MAX);
}

void ms_gpio_init(void)
{
    led_init();
    key_init();
    timer_init(1);
}

