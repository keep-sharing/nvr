/* 
 * ***************************************************************
 * Filename:      	vapi_sys.c
 * Created at:    	2016.4.28
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
    HI_UNF_MCE_STOPPARM_S   stStop;   
    HI_S32 s32Ret = HI_FAILURE;
    
    s32Ret = HI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_SYS_Init failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_UNF_MCE_Init(HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_MCE_Init failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_UNF_MCE_ClearLogo();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_MCE_ClearLogo failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    stStop.enStopMode = HI_UNF_AVPLAY_STOP_MODE_STILL;
    stStop.enCtrlMode = HI_UNF_MCE_PLAYCTRL_BY_TIME;
    stStop.u32PlayTimeMs = 0;
    s32Ret = HI_UNF_MCE_Stop(&stStop);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_MCE_Stop failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_UNF_MCE_Exit(HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_MCE_Exit failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    HI_UNF_MCE_DeInit();
    
    return HI_SUCCESS;
}

HI_VOID vapi_sys_uninit()
{
    HI_SYS_DeInit();
}

