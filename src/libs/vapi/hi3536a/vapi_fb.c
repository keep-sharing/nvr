/*
 * ***************************************************************
 * Filename:      	vapi_fb.c
 * Created at:    	2015.10.21
 * Description:   	framebuffer controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "vapi_comm.h"

#define FB_FILE_MAIN "/dev/fb0"
#define FB_FILE_SUB  "/dev/fb1"

#define FB0_MAX_WIDTH  3840
#define FB0_MAX_HEIGHT 2160

#define FB1_MAX_WIDTH  1920
#define FB1_MAX_HEIGHT 1080

#define DEFAULT_FB_WIDTH  1920
#define DEFAULT_FB_HEIGHT 1080

#define PIXFMT TDE2_COLOR_FMT_ARGB8888

static struct fb_bitfield g_a32 = {24, 8, 0};
static struct fb_bitfield g_r32 = {16, 8, 0};
static struct fb_bitfield g_g32 = {8, 8, 0};
static struct fb_bitfield g_b32 = {0, 8, 0};

td_s32 vapi_fb_init(SCREEN_E enScreen)
{
    td_s32                   width, height;
    td_s32                   fd       = -1;
    td_char                  file[12] = "/dev/fb0";
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    td_bool                  bShow;
    td_u8                   *pShowScreen = NULL;

    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));
    memset(&var, 0, sizeof(struct fb_var_screeninfo));

    if (enScreen == SCREEN_MAIN) {
        strcpy(file, FB_FILE_MAIN);
    } else if (enScreen == SCREEN_SUB) {
        strcpy(file, FB_FILE_SUB);
    }

    /* 1. open framebuffer device overlay 0 */
    fd = open(file, O_RDWR, 0);
    if (fd < 0) {
        VAPILOG("open %s failed!\n", file);
        return TD_FAILURE;
    }
    vapi_get_screen_res(enScreen, &width, &height);

    /* 2. set the screen hide */
    bShow = TD_FALSE;
    if (ioctl(fd, FBIOPUT_SHOW_GFBG, &bShow) < 0) {
        VAPILOG("set screen[%d] hide failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }
    /* 3. get the variable screen info */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &var) < 0) {
        VAPILOG("Get variable screen[%d] info failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }
    /* 4. modify the variable screen info*/
    if (enScreen == SCREEN_MAIN) {
        var.xres_virtual = FB0_MAX_WIDTH;      // width;
        var.yres_virtual = FB0_MAX_HEIGHT * 2; // height*2;  //1->QT, 2->TDE scale
    } else if (enScreen == SCREEN_SUB) {
        var.xres_virtual = FB1_MAX_WIDTH;      // width;
        var.yres_virtual = FB1_MAX_HEIGHT * 2; // height*2;
    }

    var.xres           = width;  // DEFAULT_FB_WIDTH;//width;
    var.yres           = height; // DEFAULT_FB_HEIGHT;//height;
    var.transp         = g_a32;
    var.red            = g_r32;
    var.green          = g_g32;
    var.blue           = g_b32;
    var.bits_per_pixel = 32;
    var.activate       = FB_ACTIVATE_NOW;
    var.xoffset        = 0;
    var.yoffset        = 0;

    /* 5. set the variable screeninfo */
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &var) < 0) {
        VAPILOG("Put variable screen[%d] info failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }

    /* 6. get the fix screen info */
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fix) < 0) {
        VAPILOG("Get fix screen[%d] info failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }

    if (var.xres == DEFAULT_FB_WIDTH && var.yres == DEFAULT_FB_HEIGHT) {
        var.yoffset = 0;
    } else if (enScreen == SCREEN_MAIN) {
        var.yoffset = FB0_MAX_HEIGHT; // 2160
        // var.yres_virtual = FB0_MAX_HEIGHT*2;  //1->QT, 2->TDE scale
    } else if (enScreen == SCREEN_SUB) {
        var.yoffset = FB1_MAX_HEIGHT; // 1080
    }

    /*set frame buffer start position*/
    if (ioctl(fd, FBIOPAN_DISPLAY, &var) < 0) {
        VAPILOG("FBIOPAN_DISPLAY failed !\n");
        close(fd);
        return TD_FAILURE;
    }

    /* 7. map the physical video memory for user use */
    pShowScreen = mmap(TD_NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == pShowScreen) {
        VAPILOG("mmap framebuffer[%d] failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }
    /* time to play*/
    bShow = TD_TRUE;
    if (ioctl(fd, FBIOPUT_SHOW_GFBG, &bShow) < 0) {
        VAPILOG("FBIOPUT_SHOW_GFBG[%d] failed!\n", enScreen);
        munmap(pShowScreen, fix.smem_len);
        close(fd);
        return TD_FAILURE;
    }
    close(fd);

    return TD_SUCCESS;
}

td_s32 vapi_switch_display(SCREEN_E enScreen)
{
    td_s32                   width, height;
    td_s32                   fd       = -1;
    td_char                  file[12] = "/dev/fb0";
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    td_bool                  bShow;

    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));
    memset(&var, 0, sizeof(struct fb_var_screeninfo));

    if (enScreen == SCREEN_MAIN) {
        strcpy(file, FB_FILE_MAIN);
    } else if (enScreen == SCREEN_SUB) {
        strcpy(file, FB_FILE_SUB);
    }

    /* 1. open framebuffer device overlay 0 */
    fd = open(file, O_RDWR, 0);
    if (fd < 0) {
        VAPILOG("open %s failed!\n", file);
        return TD_FAILURE;
    }

    vapi_get_screen_res(enScreen, &width, &height);

    /* 2. set the screen hide */
    bShow = TD_FALSE;
    if (ioctl(fd, FBIOPUT_SHOW_GFBG, &bShow) < 0) {
        VAPILOG("set screen[%d] hide failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }
    /* 3. get the variable screen info */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &var) < 0) {
        VAPILOG("Get variable screen[%d] info failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }

    /* 4. modify the variable screen info*/
    // var.xres_virtual = 3840;//width;
    // var.yres_virtual = 2160*2;//height*2;
    var.xres = width;
    var.yres = height;
    // var.transp= g_a32;
    // var.red = g_r32;
    // var.green = g_g32;
    // var.blue = g_b32;
    // var.bits_per_pixel = 32;
    // var.activate = FB_ACTIVATE_NOW;

    if (ioctl(fd, FBIOPUT_VSCREENINFO, &var) < 0) {
        VAPILOG("Put variable screen[%d] info failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }

    if (var.xres == DEFAULT_FB_WIDTH && var.yres == DEFAULT_FB_HEIGHT) {
        var.yoffset = 0;
    } else if (enScreen == SCREEN_MAIN) {
        var.yoffset = FB0_MAX_HEIGHT;
    } else if (enScreen == SCREEN_SUB) {
        var.yoffset = FB1_MAX_HEIGHT;
    }

    if (ioctl(fd, FBIOPAN_DISPLAY, &var) < 0) {
        VAPILOG("FBIOPAN_DISPLAY failed !\n");
        close(fd);
        return TD_FAILURE;
    }

    /* time to play*/
    bShow = TD_TRUE;
    if (ioctl(fd, FBIOPUT_SHOW_GFBG, &bShow) < 0) {
        VAPILOG("FBIOPUT_SHOW_GFBG[%d] failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }

    close(fd);

    return TD_SUCCESS;
}

int vapi_fb_clear_buffer(SCREEN_E enScreen)
{
    td_s32                   fd       = -1;
    td_char                  file[12] = "/dev/fb0";
    struct fb_fix_screeninfo fix;
    td_u8                   *pShowScreen = NULL;

    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));

    if (enScreen == SCREEN_MAIN) {
        strcpy(file, FB_FILE_MAIN);
    } else if (enScreen == SCREEN_SUB) {
        strcpy(file, FB_FILE_SUB);
    }

    fd = open(file, O_RDWR, 0);
    if (fd < 0) {
        VAPILOG("open %s failed!\n", file);
        return TD_FAILURE;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &fix) < 0) {
        VAPILOG("Get fix screen[%d] info failed!\n", enScreen);
        close(fd);
        return TD_FAILURE;
    }

    pShowScreen = mmap(TD_NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == pShowScreen) {
        VAPILOG("mmap framebuffer[%d] failed!\n", enScreen);
        perror("mmap failed!\n");
        close(fd);
        return TD_FAILURE;
    }
    memset(pShowScreen, 0, fix.smem_len);
    munmap(pShowScreen, fix.smem_len);
    close(fd);
    return TD_SUCCESS;
}

td_s32 vapi_fb_uninit()
{
    return TD_SUCCESS;
}
