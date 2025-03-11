/* 
 * ***************************************************************
 * Filename:      	gpio.c
 * Created at:    	2015.12.21
 * Description:   	GPIO API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include "gpio.h"
#include "hi3536c_gio_interface.h"

int gpio_config(int gio_id, GPIO_DIR_E gio_dir )
{
	int ret;

	if(gio_id == GPIO_INVALID)
	{
		return -1;
	}
	
	ret = hi_gio_config(gio_id, gio_dir);

	return ret;
}

int gpio_write(int gio_id, int gio_level)
{
	int ret;

	if(gio_id == GPIO_INVALID)
	{
		return -1;
	}	

	ret = hi_gio_write(gio_id, gio_level);

	return ret;
}

int gpio_read(int gio_id)
{
	int ret;

	if(gio_id == GPIO_INVALID)
	{
		return -1;
	}
	
	ret = hi_gio_read(gio_id);
	return ret;
}

int gpio_open(void)
{
	return 0;
}

int gpio_close(void)
{
	int ret;

	ret = hi_gio_close();

	return ret;
}



