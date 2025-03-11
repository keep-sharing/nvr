/* 
 * ***************************************************************
 * Filename:      	hi3536_gio_interface.c
 * Created at:    	2015.12.21
 * Description:   	hi3536 gio API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gpio_map.h"

#define GPIO_DEV_NAME	"/dev/hi_gpio"


static int g_gio_fd = -1;

static int hi_gio_open(void)
{
	if(g_gio_fd == -1)
	{		
		g_gio_fd = open(GPIO_DEV_NAME, O_RDWR);
		if(g_gio_fd == -1)  
		{  
			perror("hi_gio_open " GPIO_DEV_NAME);  
			return -1;  
		}
	}
	
	return 0;
}

int hi_gio_close(void)
{
	if(g_gio_fd != -1)
	{
		close(g_gio_fd);
	}
	g_gio_fd = -1;
	
	return 0;
}

int hi_gio_config(Uint32 gio_num, GPIO_DIR_E gio_dir)
{
	int cmd;
	int retval;
	HI_GPIO_CTRL_T gioCtl;	

	if(hi_gio_open() != 0)
	{
		return -1;
	}
	
	// make command
	cmd = GPIO_IOCTL_CMD_MAKE(_IOC_WRITE, 'G', gio_dir, HI_GPIO_CTRL_T);
	gioCtl.gio_id = gio_num;
	gioCtl.data = 0;
	retval = ioctl(g_gio_fd, cmd, &gioCtl);
	if(retval != 0)
	{
		perror("hi_gio_config");  
		return -1;  
	}
	
	return gio_dir;	
}


/**
 * Fun	hi_gio_write
 * args
 *		gio_num: gpio id
 *		gio_level: gpio status
 * Des: this function for write gpio
 * return
 *		success: gio_level, faild: -1
 */
int hi_gio_write(Uint32 gio_num, int gio_level)
{
	int cmd;
	int retval;   
	HI_GPIO_CTRL_T gioCtl;
	
	if((gio_level != 0) && (gio_level != 1))
	{
		printf("gio value error, neither 0 or 1!\n");
		return -1;
	}

	if(hi_gio_open() != 0)
	{
		return -1;
	}

	// make command
	cmd = GPIO_IOCTL_CMD_MAKE(_IOC_WRITE, 'G', GPIO_CMD_WRITE, HI_GPIO_CTRL_T);
	gioCtl.gio_id = gio_num;
	gioCtl.data = gio_level;
	retval = ioctl(g_gio_fd, cmd, &gioCtl);
	if(retval != 0)
	{
		perror("hi_gio_write");  
		return -1;  
	}
	
	return gio_level;
}

/**
 * Fun	hi_gio_read
 * args
 *		gio_num: gpio id
 * Des: this function for write gpio
 * return
 *		success: gio_level, faild: -1
 */
int hi_gio_read(Uint32 gio_num)
{
	int cmd;
	int retval;
	HI_GPIO_CTRL_T gioCtl;

	if(hi_gio_open() != 0)
	{
		return -1;
	}

	// make command
	cmd = GPIO_IOCTL_CMD_MAKE(_IOC_READ, 'G', GPIO_CMD_READ, HI_GPIO_CTRL_T);
	gioCtl.gio_id = gio_num;
	gioCtl.data = -1;
	retval = ioctl(g_gio_fd, cmd, &gioCtl);
	if( retval != 0 )
	{
		perror("hi_gio_read");  
		return -1;  
	}
	
	return gioCtl.data;
}

/**
 * Fun	hi_gio_mux
 * args
 *		mux: value
 * Des: this function for mux gpio
 * return
 *		success: 0, faild: -1
 */
int hi_gio_mux(Uint32 mux, int value)
{
	int cmd;
	int retval;
	HI_GPIO_CTRL_T gioCtl;

	// make command
	cmd = GPIO_IOCTL_CMD_MAKE(_IOC_READ, 'G', GPIO_CMD_MUX, HI_GPIO_CTRL_T);
	gioCtl.gio_id = mux;
	gioCtl.data = value;
	retval = ioctl(g_gio_fd, cmd, &gioCtl);
	if( retval != 0 )
	{
		perror("hi_gio_mux");  
		return -1;  
	}
	
	return 0;
}

