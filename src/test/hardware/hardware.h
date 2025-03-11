/* 
 * ***************************************************************
 * Filename:      	hardware.h
 * Created at:    	2016.01.05
 * Description:   	hardware test api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "msstd.h"
#include "hd_module.h"

#define RED 			"\033[0;31m"
#define GREEN 			"\033[0;32m"
#define CLRCOLOR		"\033[0;39m "

#define hdprintf(color, format, ...) \
    do{\
		printf(color format CLRCOLOR, ##__VA_ARGS__);\
    }while(0);
    
#define hdinfo(format, ...) \
    do{\
		printf(GREEN format CLRCOLOR, ##__VA_ARGS__);\
    }while(0);

#define hderr(format, ...) \
    do{\
		printf(RED format CLRCOLOR, ##__VA_ARGS__);\
    }while(0);

#define SHOW_YES     \
    hdinfo("\n");\
    hdinfo("---------------------\n");\
    hdinfo("|       YES         |\n");\
    hdinfo("---------------------\n");\
    hdinfo("\n");
#if 0
    hdinfo("|*     * *******  ******\n");\
    hdinfo(" *   *  *       *\n");\
    hdinfo("  ***   *       *\n");\
    hdinfo("   *    *******  ******\n");\
    hdinfo("   *    *              *\n");\
    hdinfo("   *    *              *\n");\
    hdinfo("   *    ******* *******\n"); 
#endif

#define SHOW_NO     \
    hdinfo("\n");\
    hderr("---------------------\n");\
    hderr("|       NO          |\n");\
    hderr("---------------------\n");\
    hdinfo("\n");
#if 0
    hderr("**      *    *********\n");\
    hderr("* *     *    *       *\n");\
    hderr("*  *    *    *       *\n");\
    hderr("*   *   *    *       *\n");\
    hderr("*    *  *    *       *\n");\
    hderr("*     * *    *       *\n");\
    hderr("*      **    *********\n");
#endif

extern int g_isAuto;
void gpio_test_start(void);
void gpio_test_stop(void);

int get_hardware_version(unsigned char *device_info);

#ifdef __cplusplus
}
#endif

#endif

