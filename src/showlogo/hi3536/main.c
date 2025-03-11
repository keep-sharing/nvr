#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "msstd.h"
#include "msdefs.h"
#include "showlogo.h"

static int g_exit = 0;

static void signal_handler(int signal)
{
    msprintf("showlogo singnal = %d", signal);
    g_exit = 1;
}

static HI_S32 logo_sys_init()
{
    MPP_SYS_CONF_S stSysConf = {0};
    VB_CONF_S stVbConf;
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 i;

    HI_MPI_SYS_Exit();
    
    for(i=0;i<VB_MAX_USER;i++)
    {
         HI_MPI_VB_ExitModCommPool(i);
    }
    for(i=0; i<VB_MAX_POOLS; i++)
    {
         HI_MPI_VB_DestroyPool(i);
    }
    HI_MPI_VB_Exit();
    
    memset(&stVbConf, 0, sizeof(VB_CONF_S)); 
    s32Ret = HI_MPI_VB_SetConf(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = 16;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_SYS_SetConf failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_VOID logo_sys_uninit()
{

    HI_S32 i;

    HI_MPI_SYS_Exit();
    for(i=0;i<VB_MAX_USER;i++)
    {
         HI_MPI_VB_ExitModCommPool(i);
    }
 
    for(i=0; i<VB_MAX_POOLS; i++)
    {
         HI_MPI_VB_DestroyPool(i);
    }   

    HI_MPI_VB_Exit();
    
    return;
}



int main(int argc, char **argv)
{
    //HI_U32 u32Len = 0;
    //HI_U32 buffsize = 1920*1080*3/2;
    //FILE *pFd = HI_NULL;
    HI_S32 s32Ret = HI_FAILURE;
    
    if (argc != 2)
    {
        msprintf("\n usage : showlogo <filename> \n");
        return -1;
    }
    
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGUSR1, signal_handler);
	signal(SIGPIPE, signal_handler);
	signal(SIGSEGV, signal_handler);

    logo_sys_init();
    // 1 .初始化输出设备
    s32Ret = logo_vo_dev_open(LOGO_VO_DEV_DHD0, SCREEN_1920X1080_60);
    if (s32Ret != HI_SUCCESS)
    {
        logo_sys_uninit();
        return HI_FAILURE;
    }
#if 0
//    logo_vo_dev_open(LOGO_VO_DEV_DHD1, SCREEN_1920X1080_60);

    //2.创建jpg 解码通道
    logo_vdec_chn_create(0, ENC_TYPE_MJPEG, 1920, 1080);

    //3.连接通道
    logo_vpss_create(0, 0, 1920);
//    logo_vpss_create(1, 0, 1920);    
    logo_vdec_bind_vpss(0, 0, 0);
//    logo_vdec_bind_vpss(0, 1, 0);
    logo_vo_bind_vpss(LOGO_VO_LAYER_VHD0, DIS_CHN_ID, 0, 0);
//    logo_vo_bind_vpss(LOGO_VO_LAYER_VHD1, DIS_CHN_ID, 1, 0);
    msprintf("\n===========start show logo============\n");
#if 0 //version:7.0.5-beta1 delete by bruce.milesight
    //4.读取jpg文件
    HI_U8 * pu8Addr;
    pFd = fopen(argv[1], "rb");
    if (pFd == HI_NULL)
    {
        msprintf("open jpgfile %s failed !", argv[1]);
        return -1;
    }    
    pu8Addr = (HI_U8 * )ms_malloc(buffsize);
    u32Len = fread(pu8Addr, 1, buffsize, pFd);
    if (u32Len == 0)
    {
        msprintf("read jpgfile %s failed !", argv[1]);        
        ms_free(pu8Addr);
        fclose(pFd);
        return -1;
    }
    
    fclose(pFd);

    //5.开始解码显示
    logo_vdec_send_frame(0, pu8Addr, u32Len, 0);

    while(!g_exit)
    {
        usleep(100000);
    }
    ms_free(pu8Addr);
#endif
    msprintf("\n===========stop show logo============\n");
    logo_vo_unbind_vpss(LOGO_VO_LAYER_VHD0, DIS_CHN_ID, 0, 0);
//    logo_vo_unbind_vpss(LOGO_VO_LAYER_VHD1, DIS_CHN_ID, 1, 0);
    logo_vdec_unbind_vpss(0, 0, 0);
//    logo_vdec_unbind_vpss(0, 1, 0);    
    logo_vpss_destory(0, 0);
//    logo_vpss_destory(1, 0);    
    logo_vdec_chn_destory(0);
    logo_vo_dev_close(LOGO_VO_DEV_DHD0);
//    logo_vo_dev_close(LOGO_VO_DEV_DHD1);
#endif
    logo_sys_uninit();
    
	return 0;
}

