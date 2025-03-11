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
#include "hi_tde_api.h"
#include "hi_tde_type.h"
#include "hi_tde_errcode.h"
#include "hifb.h"
#include "tde.h"

#define FB0_MAX_WIDTH   3840
#define FB0_MAX_HEIGHT  2160

#define FB1_MAX_WIDTH    1920
#define FB1_MAX_HEIGHT   1080

#define DEFAULT_FB_WIDTH  1920
#define DEFAULT_FB_HEIGHT 1080


#define PIXFMT  TDE2_COLOR_FMT_ARGB8888

static MUTEX_OBJECT mutex;

int tde_fb_init(void)
{
    /* open TDE module */
    ms_mutex_init(&mutex);
    HI_TDE2_Open();
    HI_TDE2_EnableRegionDeflicker(HI_TRUE);
    return HI_SUCCESS;
}

int tde_fb_uninit(void)
{
    /* open TDE module */
    HI_TDE2_Close(); //close TDE
    ms_mutex_uninit(&mutex);
    return HI_SUCCESS;
}

int tde_fb_scale(int enScreen, int srcX, int srcY, int srcWidth, int srcHeight, int *isMinifying)
{
    static const HI_CHAR *FB_FILE_MAIN = "/dev/fb0";
    static const HI_CHAR *FB_FILE_SUB  = "/dev/fb1";

    int ret = HI_FAILURE, fd = -1;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    TDE2_RECT_S stSrcRect = { 0 }, stDstRect = { 0 };

    int fb_max_height;
    TDE_HANDLE s32Handle = HI_ERR_TDE_INVALID_HANDLE;
    TDE2_SURFACE_S stScreen[2];
    const HI_CHAR *fb_file = FB_FILE_MAIN;
    int devicePixelRatio = 1;

    memset(stScreen, 0, sizeof(stScreen));
    
    ms_mutex_lock(&mutex);

    if (enScreen == SCREEN_MAIN) {
        fb_file = FB_FILE_MAIN;
        fb_max_height = FB0_MAX_HEIGHT;
    } else if (enScreen == SCREEN_SUB) {
        fb_file = FB_FILE_SUB;
        fb_max_height = FB1_MAX_HEIGHT;
    } else {
        msprintf("invalid screen!\n");
        goto end;
    }
    
    /* 1. open framebuffer device overlay 0 */
    fd = open(fb_file, O_RDWR, 0);
    if(fd == -1) {
        msprintf("open %s failed!\n", fb_file);
        goto end;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &fix) < 0) {
        msprintf("FBIOGET_FSCREENINFO error\n");
        goto end;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &var) < 0) {
        msprintf("FBIOGET_VSCREENINFO error\n");
        goto end;
    }
    
    /* output res = 1080p, no need tde. */
    if(var.xres == DEFAULT_FB_WIDTH && var.yres == DEFAULT_FB_HEIGHT) {
        ret = HI_SUCCESS;
        goto end;
    }

    /*create surface */
    stScreen[0].enColorFmt = PIXFMT;
    stScreen[0].u32PhyAddr = fix.smem_start;
    stScreen[0].u32Width = DEFAULT_FB_WIDTH;
    stScreen[0].u32Height = DEFAULT_FB_HEIGHT;
    stScreen[0].u32Stride = fix.line_length;
    stScreen[0].bAlphaMax255 = HI_TRUE;

    /*create surface */
    stScreen[1].enColorFmt = PIXFMT;
    stScreen[1].u32PhyAddr = fix.smem_start + fix.line_length*fb_max_height;
    stScreen[1].u32Width = var.xres;
    stScreen[1].u32Height = var.yres;
    stScreen[1].u32Stride = fix.line_length;
    stScreen[1].bAlphaMax255 = HI_TRUE;

    devicePixelRatio = stScreen[1].u32Width / stScreen[0].u32Width;
    if (isMinifying)
        *isMinifying = devicePixelRatio < 1. ? 1 : 0;

    if (devicePixelRatio == 2) {
        stSrcRect.s32Xpos = srcX;
        if (stSrcRect.s32Xpos < 0)
            stSrcRect.s32Xpos = 0;
        if (stSrcRect.s32Xpos > DEFAULT_FB_WIDTH)
            stSrcRect.s32Xpos = DEFAULT_FB_WIDTH;

        stSrcRect.s32Ypos = srcY;
        if (stSrcRect.s32Ypos < 0)
            stSrcRect.s32Ypos = 0;
        if (stSrcRect.s32Ypos > DEFAULT_FB_HEIGHT)
            stSrcRect.s32Ypos = DEFAULT_FB_HEIGHT;
    
        stSrcRect.u32Width = srcWidth;
        if (stSrcRect.u32Width + stSrcRect.s32Xpos > DEFAULT_FB_WIDTH)
            stSrcRect.u32Width = DEFAULT_FB_WIDTH - stSrcRect.s32Xpos;

        stSrcRect.u32Height = srcHeight;
        if (stSrcRect.u32Height + stSrcRect.s32Ypos > DEFAULT_FB_HEIGHT)
            stSrcRect.u32Height = DEFAULT_FB_HEIGHT - stSrcRect.s32Ypos;
        stDstRect.s32Xpos = stSrcRect.s32Xpos * devicePixelRatio;
        stDstRect.s32Ypos = stSrcRect.s32Ypos * devicePixelRatio;
        stDstRect.u32Width = stSrcRect.u32Width * devicePixelRatio;
        stDstRect.u32Height = stSrcRect.u32Height * devicePixelRatio;
    } else {
        stSrcRect.s32Xpos = 0;
        stSrcRect.s32Ypos = 0;
        stSrcRect.u32Width = stScreen[0].u32Width;
        stSrcRect.u32Height = stScreen[0].u32Height;
        stDstRect.s32Xpos = 0;
        stDstRect.s32Ypos = 0;
        stDstRect.u32Width = stScreen[1].u32Width;
        stDstRect.u32Height = stScreen[1].u32Height;
    }

    if (!stSrcRect.u32Width || !stSrcRect.u32Height)
        goto end;

    /* start TDE job */
    s32Handle = HI_TDE2_BeginJob();
    if(s32Handle == HI_ERR_TDE_INVALID_HANDLE) {
        msprintf("start job failed!\n");
        goto end;
    }

    /* Resize to screen */
    if (HI_TDE2_QuickDeflicker(s32Handle, &stScreen[0], &stSrcRect, &stScreen[1], &stDstRect) < 0) {
        msprintf("HI_TDE2_QuickResize failed,ret=0x%x!\n", ret);
        goto end;
    }

    if (HI_TDE2_EndJob(s32Handle, HI_FALSE, HI_FALSE, 0) < 0) {
        msprintf("HI_TDE2_EndJob failed,ret=0x%x!\n", ret);
        goto end;
    }

    s32Handle = HI_ERR_TDE_INVALID_HANDLE;
    ret = HI_SUCCESS;
end:
    if (s32Handle != HI_ERR_TDE_INVALID_HANDLE)
        HI_TDE2_CancelJob(s32Handle);
    if (fd != -1)
        close(fd);
    ms_mutex_unlock(&mutex);    
    return ret;
}


