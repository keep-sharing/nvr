/*
 * ***************************************************************
 * Filename:      	vapi_sys.c
 * Created at:    	2015.10.21
 * Description:   	system resource controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vapi_comm.h"

td_s32 vapi_sys_init()
{
    ot_mpp_sys_cfg stSysConf = {0};
    ot_vb_cfg      stVbConf;
    td_s32         s32Ret = TD_FAILURE;

    ss_mpi_sys_exit();

    ot_vb_uid vb_uid = 0;
    for (; vb_uid < OT_VB_MAX_USER; ++vb_uid) {
        ss_mpi_vb_exit_mod_common_pool(vb_uid);
    }
    ot_vb_pool vb_pool = 0;
    for (; vb_pool < OT_VB_MAX_POOLS; ++vb_pool) {
        ss_mpi_vb_destroy_pool(vb_pool);
    }
    ss_mpi_vb_exit();

    memset(&stVbConf, 0, sizeof(stVbConf));
#if 1
    stVbConf.max_pool_cnt = 64;

    stVbConf.common_pool[0].blk_size = 1920 * 1080 * 3 / 2;
    stVbConf.common_pool[0].blk_cnt  = MAX_VENC_NUM;

    stVbConf.common_pool[1].blk_size = (1280 * 720) * 2;
    stVbConf.common_pool[1].blk_cnt  = 5 * MAX_VENC_NUM;

    stVbConf.common_pool[2].blk_size = VAPI_VENC_MAX_WIDTH * VAPI_VENC_MAX_HEIGTH * 3 / 2; // for jpeg encode
    stVbConf.common_pool[2].blk_cnt  = 1;

    stVbConf.common_pool[3].blk_size = VAPI_FISH_MAX_WIDTH * VAPI_FISH_MAX_HEIGTH * 3 / 2; // for fisheye
    stVbConf.common_pool[3].blk_cnt  = 1;

    stVbConf.common_pool[4].blk_size = 3840 * 2160 * 3 / 2; // for fisheye
    stVbConf.common_pool[4].blk_cnt  = 4;
#else
    ot_pic_buf_attr buf_attr = {0};
    buf_attr.align           = 0;
    buf_attr.bit_width       = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode   = OT_COMPRESS_MODE_NONE;
    buf_attr.width           = 1920;
    buf_attr.height          = 1080;
    buf_attr.pixel_format    = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    stVbConf.max_pool_cnt            = 1;
    stVbConf.common_pool[0].blk_size = ot_common_get_pic_buf_size(&buf_attr);
    stVbConf.common_pool[0].blk_cnt  = 10;
#endif

    // VAPILOG("ss_mpi_vb_set_cfg, blk_size:%llu, blk_cnt:%u", stVbConf.common_pool[0].blk_size, stVbConf.common_pool[0].blk_cnt)
    s32Ret = ss_mpi_vb_set_cfg(&stVbConf);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vb_set_cfg failed with %#x!\n", s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vb_init();
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vb_init failed with %#x!\n", s32Ret);
        return TD_FAILURE;
    }

    stSysConf.align = 16;
    s32Ret          = ss_mpi_sys_set_cfg(&stSysConf);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_sys_set_cfg failed with %#x!\n", s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_sys_init();
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_sys_init failed with %#x!\n", s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_void vapi_sys_uninit()
{

    td_s32 i;

    ss_mpi_sys_exit();
    for (i = 0; i < OT_VB_MAX_USER; i++) {
        ss_mpi_vb_exit_mod_common_pool(i);
    }

    for (i = 0; i < OT_VB_MAX_POOLS; i++) {
        ss_mpi_vb_destroy_pool(i);
    }

    ss_mpi_vb_exit();

    return;
}
