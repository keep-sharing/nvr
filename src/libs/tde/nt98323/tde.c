/* 
 * ***************************************************************
 * Filename:      	vapi_fb.c
 * Created at:    	2015.10.21
 * Description:   	framebuffer controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>     
#include "msstd.h"
#include "vapi.h"
#include "tde.h"

#define FB0_MAX_WIDTH   3840
#define FB0_MAX_HEIGHT  1080

#define FB1_MAX_WIDTH    1920
#define FB1_MAX_HEIGHT   1080

#define DEFAULT_FB_WIDTH  1920
#define DEFAULT_FB_HEIGHT 1080

int tde_fb_init(void)
{
    return 0;
}

int tde_fb_uninit(void)
{
    return 0;
}

int tde_fb_scale(int enScreen, int srcX, int srcY, int srcWidth, int srcHeight, int *isMinifying)
{
    return 0;
}


