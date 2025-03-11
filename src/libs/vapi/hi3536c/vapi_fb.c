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
     
#include "vapi_comm.h"

//#include "hi_tde_api.h"
//#include "hi_tde_type.h"
//#include "hi_tde_errcode.h"

#define FB_FILE_MAIN   "/dev/fb0" 
#define FB_FILE_SUB    "/dev/fb1" 

#define FB0_MAX_WIDTH   3840
#define FB0_MAX_HEIGHT  1080

#define FB1_MAX_WIDTH    1920
#define FB1_MAX_HEIGHT   1080

#define DEFAULT_FB_WIDTH  1920
#define DEFAULT_FB_HEIGHT 1080

#define PIXFMT  TDE2_COLOR_FMT_ARGB8888

static struct fb_bitfield g_a32 = {24,8,0};
static struct fb_bitfield g_r32 = {16,8,0};
static struct fb_bitfield g_g32 = {8,8,0};
static struct fb_bitfield g_b32 = {0,8,0};


HI_S32 vapi_fb_init(SCREEN_E enScreen)
{
    HI_S32 width, height;
    HI_S32 fd = -1;
	HI_CHAR file[12] = "/dev/fb0";	
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    HI_BOOL bShow; 
    HI_U8 *pShowScreen = NULL;

    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));
    memset(&var, 0, sizeof(struct fb_var_screeninfo));
    
    if (enScreen == SCREEN_MAIN)
    {
		strcpy(file, FB_FILE_MAIN);
    }
    else if (enScreen == SCREEN_SUB)
    {        
		strcpy(file, FB_FILE_SUB);
    }
    
    /* 1. open framebuffer device overlay 0 */
	fd = open(file, O_RDWR, 0);
	if(fd < 0)
	{
		VAPILOG("open %s failed!\n", file);
		return HI_FAILURE;
	}
    vapi_get_screen_res(enScreen, &width, &height);

    /* 2. set the screen hide */
    bShow = HI_FALSE;    
    if (ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
    {
        VAPILOG("set screen[%d] hide failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }
    /* 3. get the variable screen info */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &var) < 0)
    {
        VAPILOG("Get variable screen[%d] info failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }    
    /* 4. modify the variable screen info*/

    if (enScreen == SCREEN_MAIN)
    {
        var.xres_virtual = FB0_MAX_WIDTH;//width;
        var.yres_virtual = FB0_MAX_HEIGHT+2160;//height*2;  //1->QT, 2->TDE scale
    }
    else if (enScreen == SCREEN_SUB)
    {        
        var.xres_virtual = FB1_MAX_WIDTH;//width;
        var.yres_virtual = FB1_MAX_HEIGHT*2;//height*2;
    }
    
	var.xres = width;//DEFAULT_FB_WIDTH;//width;
    var.yres = height;//DEFAULT_FB_HEIGHT;//height;
    var.transp= g_a32;
    var.red = g_r32;
    var.green = g_g32;
    var.blue = g_b32;
    var.bits_per_pixel = 32;
    var.activate = FB_ACTIVATE_NOW;
    var.xoffset = 0;
    var.yoffset = 0;
    /* 5. set the variable screeninfo */
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &var) < 0)
    {
        VAPILOG("Put variable screen[%d] info failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }
    
    /* 6. get the fix screen info */
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fix) < 0)
    {
        VAPILOG("Get fix screen[%d] info failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }

	if(var.xres == DEFAULT_FB_WIDTH && var.yres == DEFAULT_FB_HEIGHT)
	{
		var.yoffset = 0;
	}
    else if (enScreen == SCREEN_MAIN)
    {
        var.yoffset = FB0_MAX_HEIGHT; //1080
        //var.yres_virtual = FB0_MAX_HEIGHT*2;  //1->QT, 2->TDE scale
    }
    else if (enScreen == SCREEN_SUB)
    {        
        var.yoffset = FB1_MAX_HEIGHT; //1080
    }

	/*set frame buffer start position*/
	if (ioctl(fd, FBIOPAN_DISPLAY, &var) < 0)
	{
		VAPILOG("FBIOPAN_DISPLAY failed !\n");
		close(fd);
        return HI_FAILURE;
	}

    /* 7. map the physical video memory for user use */
    pShowScreen = mmap(HI_NULL, fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(MAP_FAILED == pShowScreen)
    {
        VAPILOG("mmap framebuffer[%d] failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }
    /* time to play*/
    bShow = HI_TRUE;
    if (ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
    {
        VAPILOG("FBIOPUT_SHOW_HIFB[%d] failed!\n", enScreen);
        munmap(pShowScreen, fix.smem_len);
        close(fd);
        return HI_FAILURE;
    }
	close(fd);

    return HI_SUCCESS;
}

HI_S32 vapi_switch_display(SCREEN_E enScreen)
{
    HI_S32 width, height;
    HI_S32 fd = -1;
	HI_CHAR file[12] = "/dev/fb0";	
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    HI_BOOL bShow; 

    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));
    memset(&var, 0, sizeof(struct fb_var_screeninfo));
    
    if (enScreen == SCREEN_MAIN)
    {
		strcpy(file, FB_FILE_MAIN);
    }
    else if (enScreen == SCREEN_SUB)
    {        
		strcpy(file, FB_FILE_SUB);
    }
    
    /* 1. open framebuffer device overlay 0 */
	fd = open(file, O_RDWR, 0);
	if(fd < 0)
	{
		VAPILOG("open %s failed!\n", file);
		return HI_FAILURE;
	}

    vapi_get_screen_res(enScreen, &width, &height);
    
    /* 2. set the screen hide */
    bShow = HI_FALSE;    
    if (ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
    {
        VAPILOG("set screen[%d] hide failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }
    /* 3. get the variable screen info */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &var) < 0)
    {
        VAPILOG("Get variable screen[%d] info failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }    

    /* 4. modify the variable screen info*/
    //var.xres_virtual = 3840;//width;
    //var.yres_virtual = 2160*2;//height*2;
    var.xres = width;
    var.yres = height;
    //var.transp= g_a32;
    //var.red = g_r32;
    //var.green = g_g32;
    //var.blue = g_b32;
    //var.bits_per_pixel = 32;
    //var.activate = FB_ACTIVATE_NOW;
    
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &var) < 0)
    {
        VAPILOG("Put variable screen[%d] info failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }

	if(var.xres == DEFAULT_FB_WIDTH && var.yres == DEFAULT_FB_HEIGHT)
	{
		var.yoffset = 0;
	}
    else if(enScreen == SCREEN_MAIN)
	{
		var.yoffset = FB0_MAX_HEIGHT;
	}
	else if(enScreen == SCREEN_SUB)
	{
		var.yoffset = FB1_MAX_HEIGHT;
	}

	if (ioctl(fd, FBIOPAN_DISPLAY, &var) < 0)
	{
		VAPILOG("FBIOPAN_DISPLAY failed !\n");
		close(fd);
        return HI_FAILURE;
	}

    /* time to play*/
    bShow = HI_TRUE;
    if (ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
    {
        VAPILOG("FBIOPUT_SHOW_HIFB[%d] failed!\n", enScreen);
        close(fd);
        return HI_FAILURE;
    }

	close(fd);
    
    return HI_SUCCESS;
}

int vapi_fb_clear_buffer(SCREEN_E enScreen)
{
    HI_S32 fd = -1;
	HI_CHAR file[12] = "/dev/fb0";	
    struct fb_fix_screeninfo fix;
    HI_U8 *pShowScreen = NULL;

    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));
    
    if (enScreen == SCREEN_MAIN)
    {
		strcpy(file, FB_FILE_MAIN);
    }
    else if (enScreen == SCREEN_SUB)
    {        
		strcpy(file, FB_FILE_SUB);
    }

	fd = open(file, O_RDWR, 0);
	if(fd < 0)
	{
		VAPILOG("open %s failed!\n", file);
		return HI_FAILURE;
	}
	
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fix) < 0)
    {
        VAPILOG("Get fix screen[%d] info failed!\n", enScreen);
		close(fd);
        return HI_FAILURE;
    }
    
	pShowScreen = mmap(HI_NULL, fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(MAP_FAILED == pShowScreen)
    {
        VAPILOG("mmap framebuffer[%d] failed!\n", enScreen);
        perror("mmap failed!\n");
		close(fd);
        return HI_FAILURE;
    }
    memset(pShowScreen, 0, fix.smem_len);
    munmap(pShowScreen, fix.smem_len);
    close(fd);
    return HI_SUCCESS;
}

HI_S32 vapi_fb_uninit()
{
    return HI_SUCCESS;
}



