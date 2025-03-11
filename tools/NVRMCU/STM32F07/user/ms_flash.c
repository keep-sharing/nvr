/* 
 * ***************************************************************
 * Filename:      	ms_flash.c
 * Created at:    	2019.01.02
 * Description:   	flash W/R api
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include "main.h"

#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR   ((uint32_t)0x08006000)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t)0x08007000)   /* End @ of user Flash area */

#define POWER_STATE_ADDR       ((uint32_t)0)

static void WriteFlash(uint32_t addr, uint32_t data)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);
    FLASH_ErasePage(FLASH_USER_START_ADDR);
    FLASH_ProgramWord(FLASH_USER_START_ADDR + (addr*4), data);
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);
    FLASH_Lock();
}

static uint32_t ReadFlash(uint16_t addr)
{
	uint32_t value;
	value = *(uint32_t*)(FLASH_USER_START_ADDR+(addr*4));
	return value;
}

void ms_set_power_state(uint32_t data)
{
    WriteFlash(POWER_STATE_ADDR, data);
}

uint32_t ms_get_power_state(void)
{
    return ReadFlash(POWER_STATE_ADDR);
}

