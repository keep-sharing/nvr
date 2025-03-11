/* 
 * ***************************************************************
 * Filename:      	gpio.c
 * Created at:    	2016.04.30
 * Description:   	GPIO API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include "gpio.h"
#include "hi_unf_gpio.h"
//#include "hi3536_gio_interface.h"


int gpio_open(void)
{
	int ret;

    ret = HI_UNF_GPIO_Init();
	return ret;
}

int gpio_config(int gio_id, GPIO_DIR_E gio_dir )
{
	int ret;

	if(gio_id == GPIO_INVALID)
	{
		return -1;
	}
	
    ret = HI_UNF_GPIO_SetDirBit(gio_id, gio_dir);

	return ret;
}

int gpio_write(int gio_id, int gio_level)
{
	int ret;

	if(gio_id == GPIO_INVALID)
	{
		return -1;
	}
	
    ret = HI_UNF_GPIO_SetDirBit(gio_id, 0);//output
    ret |= HI_UNF_GPIO_WriteBit(gio_id, gio_level);
	return ret;
}

int gpio_read(int gio_id)
{
	int ret;
    int value = 0;
    
	if(gio_id == GPIO_INVALID)
	{
		return -1;
	}
	
    HI_UNF_GPIO_SetDirBit(gio_id, 1);//input
	HI_UNF_GPIO_ReadBit(gio_id, &value);
	return value;
}

int gpio_close(void)
{
	int ret;

    ret = HI_UNF_GPIO_DeInit();
	return ret;
}




