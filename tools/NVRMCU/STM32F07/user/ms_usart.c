/* 
 * ***************************************************************
 * Filename:      	ms_usart.c
 * Created at:    	2019.01.02
 * Description:   	usart api
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include "main.h"

//指令A:0xAA+0x01+0x55 :CPU启动完成后通知单片机
//指令B:0xAA+0x03+0x55 :短按一次后单片机通知CPU是否要关机
//指令C:0xAA+0x05+0x55 :长按或者连续短按两次后，单片机通知CPU强制关机
//指令D:0xAA+0x07+0x55 :CPU准备好关机后通知单片机需要断电
//指令E:0xAA+0x09+0x55 :QT选择重启后，CPU发送重启命令
//指令F:0xAA+0x0B+0x55 :CPU通知单片机准备关机，LED闪烁等待30S
uint16_t gCmd[CMD_NUM][CMD_LEN] ={
            {0xAA, 0x01, 0x55},
            {0xAA, 0x03, 0x55},
            {0xAA, 0x05, 0x55},
            {0xAA, 0x07, 0x55},
            {0xAA, 0x09, 0x55},
            {0xAA, 0x0B, 0x55},};
            
volatile CMD_EN rcv_cmd = CMD_NONE;

int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint16_t) ch);
  /* Loop until transmit data register is empty */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
  {}

  return ch;
}

static CMD_EN cmd_parse(uint16_t *pCmd)
{
    uint8_t x, y;

    for (x=0; x<CMD_NUM; x++)
    {
        for(y=0; y<CMD_LEN; y++)
        {
            if (pCmd[y] != gCmd[x][y])
                break;
        }
        if (y == CMD_LEN)
            return (CMD_EN)x;
        else
            continue;
    }

    return CMD_NONE;
}

void USART1_IRQHandler ()
{
    uint16_t ch;
    static uint8_t cnt = 0;
    static uint16_t rcvCmdBuff[CMD_LEN];
    
    if (USART_GetFlagStatus(USART1, USART_FLAG_PE) != RESET)
        USART_ClearFlag(USART1, USART_FLAG_PE);

    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
        USART_ClearFlag(USART1, USART_FLAG_ORE);

    if (USART_GetFlagStatus(USART1, USART_FLAG_FE) != RESET)
        USART_ClearFlag(USART1, USART_FLAG_FE);
        
    if(USART_GetITStatus(USART1, USART_IT_RXNE)== SET)
    {
        ch = USART_ReceiveData(USART1);
        if (ch != 0xAA && cnt == 0)
            return;
        rcvCmdBuff[cnt] = ch;
        cnt++;
        if (cnt == CMD_LEN)
        {
            rcv_cmd = cmd_parse(rcvCmdBuff);
            MSLOG("usart1 rcv_cmd = %d\n", rcv_cmd);
            cnt = 0;
        }
    }
}

void USART2_IRQHandler ()
{
    uint16_t ch;
    static uint8_t cnt = 0;
    static uint16_t rcvCmdBuff[CMD_LEN];
    
    if (USART_GetFlagStatus(USART2, USART_FLAG_PE) != RESET)
        USART_ClearFlag(USART2, USART_FLAG_PE);

    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
        USART_ClearFlag(USART2, USART_FLAG_ORE);

    if (USART_GetFlagStatus(USART2, USART_FLAG_FE) != RESET)
        USART_ClearFlag(USART2, USART_FLAG_FE);
        
    if(USART_GetITStatus(USART2, USART_IT_RXNE)== SET)
    {
        ch = USART_ReceiveData(USART2);
        if (ch != 0xAA && cnt == 0)
            return;
        rcvCmdBuff[cnt] = ch;
        cnt++;
        if (cnt == CMD_LEN)
        {
            rcv_cmd = cmd_parse(rcvCmdBuff);
            MSLOG("usart2 rcv_cmd = %d\n", rcv_cmd);
            cnt = 0;
        }
    }
}

void ms_usart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable GPIO clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    /* Enable USART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 

    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART configuration */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Init(USART2, &USART_InitStructure);
    
    /* Enable USART */
    USART_Cmd(USART1, ENABLE);
    USART_Cmd(USART2, ENABLE);

    //打开USART的串口接收中断：
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_Init(&NVIC_InitStructure);
}

void ms_usart_send_cmd(CMD_EN cmdNo)
{
    uint8_t i;

    for (i=0; i<CMD_LEN; i++)
    {
        USART_SendData(USART2, gCmd[cmdNo][i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
        {}
    }
}

uint8_t ms_usart_rcv_cmd(CMD_EN cmdNo)
{   
    if (cmdNo == rcv_cmd)
    {
        rcv_cmd = CMD_NONE;
        return 1;
    }
    return 0;
}

