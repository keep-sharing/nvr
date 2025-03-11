/* 
 * ***************************************************************
 * Filename:      	vapi_sys.c
 * Created at:    	2015.10.21
 * Description:   	system resource controller
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

#include "vapi_comm.h"


HI_S32 vapi_sys_init()
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
    stVbConf.u32MaxPoolCnt = 64;

    stVbConf.astCommPool[0].u32BlkSize = 1920*1088*3/2;
    stVbConf.astCommPool[0].u32BlkCnt =  MAX_VENC_NUM;

    stVbConf.astCommPool[1].u32BlkSize = (1280*720)*2;
    stVbConf.astCommPool[1].u32BlkCnt =  5 * MAX_VENC_NUM;

    stVbConf.astCommPool[2].u32BlkSize = VAPI_VENC_MAX_WIDTH*VAPI_VENC_MAX_HEIGTH*3/2; // for jpeg encode
    stVbConf.astCommPool[2].u32BlkCnt =  1;

    stVbConf.astCommPool[3].u32BlkSize = VAPI_FISH_MAX_WIDTH*VAPI_FISH_MAX_HEIGTH*3/2; // for fisheye
    stVbConf.astCommPool[3].u32BlkCnt =  1;

//    stVbConf.astCommPool[4].u32BlkSize = 3840*2160*3/2; // for fisheye
//    stVbConf.astCommPool[4].u32BlkCnt =  4;

        s32Ret = HI_MPI_VB_SetConf(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = 16;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_SYS_SetConf failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID vapi_sys_uninit()
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


