/*
 * ***************************************************************
 * Filename:        vapi_venc.c
 * Created at:      2018.10.16
 * Description:     video encode controller
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
#include "vapi_comm.h"

HD_RESULT vapi_venc_init()
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoenc_init()) != HD_OK) {
        VAPILOG("hd_videoenc_init fail with %d \n", ret);
    }

    return ret;
}

HD_RESULT vapi_venc_uninit()
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoenc_uninit()) != HD_OK) {
        VAPILOG("hd_videoenc_uninit fail with %d \n", ret);
    }

    return ret;
}


