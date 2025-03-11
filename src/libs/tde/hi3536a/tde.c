/*
 * ***************************************************************
 * Filename:      	vapi_fb.c
 * Created at:    	2015.10.21
 * Description:   	framebuffer controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include "tde.h"
#include "gfbg.h"
#include "msstd.h"
#include "ss_mpi_tde.h"
#include "vapi.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#define FB0_MAX_WIDTH  3840
#define FB0_MAX_HEIGHT 2160

#define FB1_MAX_WIDTH  1920
#define FB1_MAX_HEIGHT 1080

#define DEFAULT_FB_WIDTH  1920
#define DEFAULT_FB_HEIGHT 1080

#define PIXFMT OT_TDE_COLOR_FORMAT_ARGB8888

static MUTEX_OBJECT mutex;

int tde_fb_init(void) {
    /* open TDE module */
    ms_mutex_init(&mutex);
    ss_tde_open();
    return TD_SUCCESS;
}

int tde_fb_uninit(void) {
    /* open TDE module */
    ss_tde_close(); // close TDE
    ms_mutex_uninit(&mutex);
    return TD_SUCCESS;
}

int tde_fb_scale(int enScreen, int srcX, int srcY, int srcWidth, int srcHeight, int *isMinifying) {
    static const td_char *FB_FILE_MAIN = "/dev/fb0";
    static const td_char *FB_FILE_SUB  = "/dev/fb1";

    int                      ret = TD_FAILURE, fd = -1;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    ot_tde_rect              stSrcRect = {0}, stDstRect = {0};

    int            fb_max_height;
    td_s32         s32Handle = OT_ERR_TDE_INVALID_HANDLE;
    ot_tde_surface stScreen[2];
    const td_char *fb_file          = FB_FILE_MAIN;
    int            devicePixelRatio = 1;

    memset(stScreen, 0, sizeof(stScreen));

    ms_mutex_lock(&mutex);

    if (enScreen == SCREEN_MAIN) {
        fb_file       = FB_FILE_MAIN;
        fb_max_height = FB0_MAX_HEIGHT;
    } else if (enScreen == SCREEN_SUB) {
        fb_file       = FB_FILE_SUB;
        fb_max_height = FB1_MAX_HEIGHT;
    } else {
        msprintf("invalid screen!\n");
        goto end;
    }

    /* 1. open framebuffer device overlay 0 */
    fd = open(fb_file, O_RDWR, 0);
    if (fd == -1) {
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
    if (var.xres == DEFAULT_FB_WIDTH && var.yres == DEFAULT_FB_HEIGHT) {
        ret = TD_SUCCESS;
        goto end;
    }

    /*create surface */
    stScreen[0].color_format     = PIXFMT;
    stScreen[0].phys_addr        = fix.smem_start;
    stScreen[0].width            = DEFAULT_FB_WIDTH;
    stScreen[0].height           = DEFAULT_FB_HEIGHT;
    stScreen[0].stride           = fix.line_length;
    stScreen[0].alpha_max_is_255 = TD_TRUE;

    /*create surface */
    stScreen[1].color_format     = PIXFMT;
    stScreen[1].phys_addr        = fix.smem_start + fix.line_length * fb_max_height;
    stScreen[1].width            = var.xres;
    stScreen[1].height           = var.yres;
    stScreen[1].stride           = fix.line_length;
    stScreen[1].alpha_max_is_255 = TD_TRUE;

    devicePixelRatio = stScreen[1].width / stScreen[0].width;
    if (isMinifying)
        *isMinifying = devicePixelRatio < 1. ? 1 : 0;

    if (devicePixelRatio == 2) {
        stSrcRect.pos_x = srcX;
        if (stSrcRect.pos_x < 0)
            stSrcRect.pos_x = 0;
        if (stSrcRect.pos_x > DEFAULT_FB_WIDTH)
            stSrcRect.pos_x = DEFAULT_FB_WIDTH;

        stSrcRect.pos_y = srcY;
        if (stSrcRect.pos_y < 0)
            stSrcRect.pos_y = 0;
        if (stSrcRect.pos_y > DEFAULT_FB_HEIGHT)
            stSrcRect.pos_y = DEFAULT_FB_HEIGHT;

        stSrcRect.width = srcWidth;
        if (stSrcRect.width + stSrcRect.pos_x > DEFAULT_FB_WIDTH)
            stSrcRect.width = DEFAULT_FB_WIDTH - stSrcRect.pos_x;

        stSrcRect.height = srcHeight;
        if (stSrcRect.height + stSrcRect.pos_y > DEFAULT_FB_HEIGHT)
            stSrcRect.height = DEFAULT_FB_HEIGHT - stSrcRect.pos_y;
        stDstRect.pos_x  = stSrcRect.pos_x * devicePixelRatio;
        stDstRect.pos_y  = stSrcRect.pos_y * devicePixelRatio;
        stDstRect.width  = stSrcRect.width * devicePixelRatio;
        stDstRect.height = stSrcRect.height * devicePixelRatio;
    } else {
        stSrcRect.pos_x  = 0;
        stSrcRect.pos_y  = 0;
        stSrcRect.width  = stScreen[0].width;
        stSrcRect.height = stScreen[0].height;
        stDstRect.pos_x  = 0;
        stDstRect.pos_y  = 0;
        stDstRect.width  = stScreen[1].width;
        stDstRect.height = stScreen[1].height;
    }

    if (!stSrcRect.width || !stSrcRect.height)
        goto end;

    /* start TDE job */
    s32Handle = ss_tde_begin_job();
    if (s32Handle == OT_ERR_TDE_INVALID_HANDLE) {
        msprintf("start job failed!\n");
        goto end;
    }

    /* Resize to screen */
    ot_tde_single_src singleSrc = {
        .src_surface = &stScreen[0],
        .dst_surface = &stScreen[1],
        .src_rect    = &stSrcRect,
        .dst_rect    = &stDstRect,
    };

    ret = ss_tde_quick_resize(s32Handle, &singleSrc);
    if (ret != TD_SUCCESS) {
        msprintf("ss_tde_quick_resize failed,ret=0x%x!\n", ret);
        goto end;
    }

    ret = ss_tde_end_job(s32Handle, TD_FALSE, TD_FALSE, 0);
    if (ret != TD_SUCCESS) {
        msprintf("ss_tde_end_job failed,ret=0x%x!\n", ret);
        goto end;
    }
    s32Handle = OT_ERR_TDE_INVALID_HANDLE;
    ret       = TD_SUCCESS;
end:
    if (s32Handle != OT_ERR_TDE_INVALID_HANDLE)
        ss_tde_cancel_job(s32Handle);
    if (fd != -1)
        close(fd);
    ms_mutex_unlock(&mutex);
    return ret;
}
