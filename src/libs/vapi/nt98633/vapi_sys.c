/*
 * ***************************************************************
 * Filename:        vapi_sys.c
 * Created at:      2016.4.28
 * Description:     system resource controller
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

HD_RESULT vapi_sys_init()
{
    HD_RESULT ret = HD_OK;
    /* init hdal and allocate memory */
    ret = hd_common_init(1);
    if (ret != HD_OK) {
        VAPILOG("common init fail with %d \n", ret);
    }

    return ret;
}

HD_RESULT vapi_sys_uninit()
{
    HD_RESULT ret = HD_OK;

    ret = hd_common_uninit();
    if (ret != HD_OK) {
        VAPILOG("common uninit fail with %d \n", ret);
    }

    return ret;
}

