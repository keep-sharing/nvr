/*
 * ***************************************************************
 * Filename:        vapi_vdec.c
 * Created at:      2016.04.29
 * Description:     video decode controller
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

typedef struct vdec_s {
    CHNINFO_S       stChn[MAX_CHN_NUM];
    VDEC_CB_S       stCallback;
    HD_PATH_ID      pathId[MAX_CHN_NUM];
    BOOL            bHasIframe[MAX_CHN_NUM];
} VDEC_S;

static VDEC_S g_stVdec;

HD_RESULT vapi_vdec_chn_create(HI_S32 devId, HI_S32 vdChn, TYPE_E enType, MODE_E enMode,
                               HI_U32 u32Width, HI_U32 u32Height)
{
    HD_RESULT ret = HD_OK;
    HD_PATH_ID pathId = 0;
    HD_VIDEODEC_PATH_CONFIG decConfig;

    if (g_stVdec.stChn[vdChn].enState == STATE_BUSY) {
//        VAPILOG("vdec channel[%d] has existed !\n", vdChn);
        return ret;
    }

    if ((ret = hd_videodec_open(HD_VIDEODEC_IN(0, vdChn), HD_VIDEODEC_OUT(0, vdChn), &pathId)) != HD_OK) {
        VAPILOG("hd_videodec_open vdchn[%d][%dx%d] failed with %d !\n", vdChn, u32Width, u32Height, ret);
        goto VFAIL;
    }

    memset(&decConfig, 0x0, sizeof(HD_VIDEODEC_PATH_CONFIG));
    decConfig.max_mem.dim.w = ALIGN_CEIL(u32Width, 16);
    decConfig.max_mem.dim.h = ALIGN_CEIL(u32Height, 16);
    decConfig.max_mem.frame_rate = 30;
    decConfig.max_mem.bs_counts = 32;
    decConfig.max_mem.codec_type = comm_codec_type(enType);
    decConfig.max_mem.max_ref_num = 4;
    decConfig.max_mem.max_bitrate = 8 * 1024 * 1024;


    /* Set videodec out pool */
    decConfig.data_pool[0].mode = HD_VIDEODEC_POOL_ENABLE;
    decConfig.data_pool[0].ddr_id = 0;
    decConfig.data_pool[0].counts = HD_VIDEODEC_SET_COUNT(6, 0);
    decConfig.data_pool[0].max_counts = HD_VIDEODEC_SET_COUNT(6, 0);

    decConfig.data_pool[1].mode = HD_VIDEODEC_POOL_ENABLE;
    decConfig.data_pool[1].ddr_id = 0;
    decConfig.data_pool[1].counts = HD_VIDEODEC_SET_COUNT(6, 0);
    decConfig.data_pool[1].max_counts = HD_VIDEODEC_SET_COUNT(6, 0);

    decConfig.data_pool[2].mode = HD_VIDEODEC_POOL_DISABLE;
    decConfig.data_pool[3].mode = HD_VIDEODEC_POOL_DISABLE;

    ret = hd_videodec_set(pathId, HD_VIDEODEC_PARAM_PATH_CONFIG, &decConfig);
    if (ret != HD_OK) {
        hd_videodec_close(pathId);
        VAPILOG("hd_videodec_set vdchn[%d][%dx%d] failed with %d !\n", vdChn, u32Width, u32Height, ret);
        goto VFAIL;
    }

    g_stVdec.stChn[vdChn].enState = STATE_BUSY;
    g_stVdec.stChn[vdChn].stFormat.enType = enType;
    g_stVdec.stChn[vdChn].stFormat.width = u32Width;
    g_stVdec.stChn[vdChn].stFormat.height = u32Height;
    g_stVdec.pathId[vdChn] = pathId;
    g_stVdec.bHasIframe[vdChn] = 0;

    if (g_stVdec.stCallback.update_chn_cb != NULL) {
        g_stVdec.stCallback.update_chn_cb(devId, vdChn, STATE_BUSY, u32Width * u32Height);
    }

    return ret;

VFAIL:
    if (g_stVdec.stCallback.update_chn_cb != NULL) {
        g_stVdec.stCallback.update_chn_cb(-1, vdChn, STATE_IDLE, 0);
    }

    return ret;

}

HD_RESULT vapi_vdec_chn_destory(HI_S32 vdChn)
{
    HD_RESULT ret = HD_OK;

    if (g_stVdec.stChn[vdChn].enState != STATE_BUSY) {
        return ret;
    }

    if ((ret = hd_videodec_close(g_stVdec.pathId[vdChn])) != HD_OK) {
        VAPILOG("hd_videodec_close vdchn[%d]failed with %d !\n", vdChn, ret);
        return ret;
    }

    g_stVdec.stChn[vdChn].enState = STATE_IDLE;
    g_stVdec.stChn[vdChn].stFormat.width = 0;
    g_stVdec.stChn[vdChn].stFormat.height = 0;
    g_stVdec.pathId[vdChn] = 0;

    if (g_stVdec.stCallback.update_chn_cb != NULL) {
        g_stVdec.stCallback.update_chn_cb(-1, vdChn, STATE_IDLE, 0);
    }

    return ret;
}

HD_RESULT vapi_vdec_bind_vpss(HI_S32 vdChn, HI_S32 vpssChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videodec_bind(HD_VIDEODEC_OUT(0, vdChn), HD_VIDEOPROC_IN(vpssChn, 0))) != HD_OK) {
        VAPILOG("hd_videodec_bind vdChn[%d]->vpssChn[%d] failed with %d !\n", vdChn, vpssChn, ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vdec_unbind_vpss(HI_S32 vdChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videodec_unbind(HD_VIDEODEC_OUT(0, vdChn))) != HD_OK) {
        VAPILOG("hd_videodec_unbind vdChn[%d] failed with %d !\n", vdChn, ret);
        return ret;
    }

    return ret;
}

HI_VOID vapi_vdec_update_state(HI_S32 vdChn, HI_S32 state)
{
    g_stVdec.stChn[vdChn].enState = state;
}

HI_BOOL vapi_vdec_check_state(HI_S32 vdChn, HI_S32 state)
{
    return (g_stVdec.stChn[vdChn].enState == state);
}

HD_PATH_ID vapi_vdec_get_chn_path(HI_S32 vdChn)
{
    return g_stVdec.pathId[vdChn];
}

HI_VOID vapi_vdec_get_chn_wh(HI_S32 vdChn, UINT32 *W, UINT32 *H)
{
    *W = g_stVdec.stChn[vdChn].stFormat.width;
    *H = g_stVdec.stChn[vdChn].stFormat.height;
}

HI_S32 vapi_vdec_send_frame(HI_S32 vdChn, HI_U8 *pu8Addr, HI_U32 u32Len, HI_U64 u64PTS, int flag)
{
    HD_RESULT ret = HD_OK;
    HD_VIDEODEC_SEND_LIST bs;

    if (!g_stVdec.bHasIframe[vdChn]) {
        if (flag) {
            g_stVdec.bHasIframe[vdChn] = 1;
        } else {
            return 0;
        }
    }
    memset(&bs, 0, sizeof(bs));
    bs.path_id = g_stVdec.pathId[vdChn];
    bs.user_bs.p_bs_buf = (CHAR *)pu8Addr;
    bs.user_bs.bs_buf_size = u32Len;
    bs.user_bs.time_align = HD_VIDEODEC_TIME_ALIGN_DISABLE;
    bs.user_bs.time_diff = 0;
    if ((ret = hd_videodec_send_list(&bs, 1, 1000)) < 0 && ret != -10) {
        VAPILOG("<send bitstream fail(%d)!>\n", ret);
        return ret;
    }

    return (HI_S32)ret;
}

HD_RESULT vapi_vdec_start_stream(HI_S32 vdChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videodec_start(g_stVdec.pathId[vdChn])) != HD_OK) {
        VAPILOG("hd_videodec_start vdchn[%d]failed with %d !\n", vdChn, ret);
        return ret;
    }

    if (g_stVdec.stCallback.start_stream_cb != NULL) {
        g_stVdec.stCallback.start_stream_cb(vdChn);
    }


    return ret;
}

HD_RESULT vapi_vdec_stop_stream(HI_S32 vdChn)
{
    HD_RESULT ret = HD_OK;

    if (g_stVdec.stCallback.stop_stream_cb != NULL) {
        g_stVdec.stCallback.stop_stream_cb(vdChn);
    }

    if ((ret = hd_videodec_stop(g_stVdec.pathId[vdChn])) != HD_OK) {
        VAPILOG("hd_videodec_stop vdchn[%d]failed with %d !\n", vdChn, ret);
        return ret;
    }

    return HD_OK;
}

HI_S32 vapi_vdec_chn_update(HI_S32 devId, HI_S32 vdChn, STATE_E enState)
{
    if (g_stVdec.stCallback.update_chn_cb != NULL) {
        return g_stVdec.stCallback.update_chn_cb(devId, vdChn, enState, 0);
    }
    return 0;
}

HD_RESULT vapi_vdec_init(VDEC_CB_S *pstCb)
{
    HI_S32 i;
    HD_RESULT ret = HD_OK;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        g_stVdec.stChn[i].enState = STATE_IDLE;
        g_stVdec.pathId[i] = 0;
        g_stVdec.bHasIframe[i] = 0;
    }
    g_stVdec.stCallback.start_stream_cb = pstCb->start_stream_cb;
    g_stVdec.stCallback.stop_stream_cb = pstCb->stop_stream_cb;
    g_stVdec.stCallback.update_chn_cb = pstCb->update_chn_cb;

    if ((ret = hd_videodec_init()) != HD_OK) {
        VAPILOG("hd_videodec_init with %#x!\n", ret);
    }

    return ret;

}

HD_RESULT vapi_vdec_uninit()
{
    HI_S32 i;
    HD_RESULT ret = HD_OK;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        vapi_vdec_chn_destory(i);
    }

    g_stVdec.stCallback.start_stream_cb = NULL;
    g_stVdec.stCallback.stop_stream_cb = NULL;
    g_stVdec.stCallback.update_chn_cb = NULL;

    if ((ret = hd_videodec_uninit()) != HD_OK) {
        VAPILOG("hd_videodec_uninit with %#x!\n", ret);
    }

    return ret;
}

