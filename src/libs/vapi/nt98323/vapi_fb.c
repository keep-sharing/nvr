/*
 * ***************************************************************
 * Filename:        vapi_fb.c
 * Created at:      2016.05.03
 * Description:     framebuffer controller
 * Author:          zbing
 * Copyright (C)    milesight
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

#include "vapi_comm.h"

#define FB0_MAX_WIDTH   3840
#define FB0_MAX_HEIGHT  2160

#define FB1_MAX_WIDTH    1920
#define FB1_MAX_HEIGHT   1080

#define DEFAULT_FB_WIDTH  1920
#define DEFAULT_FB_HEIGHT 1080


HI_S32 vapi_fb_init(SCREEN_E enScreen)
{

    return HI_SUCCESS;
}

HI_S32 vapi_switch_display(SCREEN_E enScreen)
{
    return HI_SUCCESS;
}

int vapi_fb_clear_buffer(SCREEN_E enScreen)
{
    return HI_SUCCESS;
}

HI_S32 vapi_fb_uninit()
{
    return HI_SUCCESS;
}

