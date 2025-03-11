/* 
 * ***************************************************************
 * Filename:      	gpio_common.h
 * Created at:    	2015.10.21
 * Description:   	gpio common.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */


#ifndef __GPIO_COMMON_H__
#define __GPIO_COMMON_H__

//base defined
#define Int32				int
#define Uint32 				unsigned int
#define GPIO_INVALID    (-1)

#define GPIO_IOCTL_CMD_MAKE(dir,type,cmd,size)    \
	( _IOC((dir),(type),(cmd),_IOC_TYPECHECK(size)))

// GPIO command defined
typedef enum
{
	GPIO_CMD_DIR_OUT = 0, 
	GPIO_CMD_DIR_IN,
	GPIO_CMD_WRITE,
    GPIO_CMD_READ,
    GPIO_CMD_MUX,
	
}GPIO_CMD_E;

// GPIO directory defined
typedef enum
{
	GPIO_DIR_OUT = GPIO_CMD_DIR_OUT,
	GPIO_DIR_IN = GPIO_CMD_DIR_IN,
	
}GPIO_DIR_E;

// GPIO data struct defined
typedef struct
{
	Uint32 	gio_id;
	Int32	data;

}GPIO_DATA_T;


#endif /* __GPIO_COMMON_H__ */

