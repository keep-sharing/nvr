/*
 * ***************************************************************
 * Filename:        vapi_jpeg.c
 * Created at:      2019.08.21
 * Description:     video jpeg encoder
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
#include <sys/mman.h>

#include "vapi_comm.h"

struct map_info {
    void *va;
    unsigned int mapSize;
};

typedef struct buffer_s {
    //for real use
    union {
        HD_VIDEO_FRAME frame;
        HD_VIDEOENC_BS bs;
    };

    //memory info
    UINT64 pool;
    int size;
    HD_COMMON_MEM_DDR_ID ddr_id;

    //address
    HD_COMMON_MEM_VB_BLK blk;
    void *va;

} BUFFER_T;

typedef struct jpath_s {
    HD_PATH_ID       vdec;
    HD_PATH_ID       vpe;
    HD_PATH_ID       venc;
} JPATH_S;

typedef struct jpeg_s {
    pthread_mutex_t mutex;
    JPATH_S pathid;
    int memFd;
} JPEG_T;

static JPEG_T g_stJpeg;

static void *mmap_pa_to_va(unsigned int pa, unsigned int size, struct map_info *va_map_info)
{
    int page_size, mapSize;
    unsigned int aligned_pa;
    void *mmap_va;
    if (pa == 0 || size == 0) {
        VAPILOG("Invalid param\n");
        return NULL;
    }
    page_size = getpagesize();
    aligned_pa = (pa / page_size) * page_size;
    mapSize = (size + pa - aligned_pa);
    mmap_va = mmap(0, mapSize, (PROT_READ | PROT_WRITE), MAP_SHARED, g_stJpeg.memFd, (int)aligned_pa);
    if (mmap_va == MAP_FAILED) {
        VAPILOG("mmap pa to va fail>\n");
        return NULL;
    }
    if (va_map_info) {
        va_map_info->mapSize = mapSize;
        va_map_info->va = mmap_va;
    }
    return mmap_va + pa - aligned_pa;
}

static INT allocate_buffer(UINT64 pool, int size, HD_COMMON_MEM_DDR_ID ddr_id, BUFFER_T *pBuffer)
{
    INT ret = 0;
    HD_COMMON_MEM_VB_BLK blk;
    UINT32 pa, sign;
    VOID *va;

    pBuffer->pool = pool;
    pBuffer->size = size;
    pBuffer->ddr_id = ddr_id;

    blk = hd_common_mem_get_block(pool, size, ddr_id);
    if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
        VAPILOG("hd_common_mem_get_block fail\r\n");
        ret =  -1;
        goto exit;
    }
    pa = hd_common_mem_blk2pa(blk);
    if (pa == 0) {
        VAPILOG("hd_common_mem_blk2pa fail, blk = %#lx\r\n", blk);
        hd_common_mem_release_block(blk);
        ret =  -1;
        goto exit;
    }
    va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, pa, size);

    pBuffer->blk = blk;
    pBuffer->va = va;

    sign = pBuffer->frame.sign;
    if (sign == MAKEFOURCC('V', 'F', 'R', 'M')) {
        pBuffer->frame.phy_addr[0] = pa;
    } else {
        pBuffer->bs.video_pack[0].phy_addr = pa;
    }

exit:
    return ret;
}

static INT free_buffer(BUFFER_T *pBuffer)
{
    INT ret = 0;
    HD_RESULT hd_ret;
    hd_ret = hd_common_mem_munmap(pBuffer->va, pBuffer->size);
    if (hd_ret != HD_OK) {
        VAPILOG("hd_common_mem_munmap fail\r\n");
        ret =  -1;
        goto exit;
    }
    hd_ret = hd_common_mem_release_block((HD_COMMON_MEM_VB_BLK)pBuffer->blk);
    if (hd_ret != HD_OK) {
        VAPILOG("hd_common_mem_munmap fail\r\n");
        ret =  -1;
        goto exit;
    }
exit:
    return ret;
}

static INT encode_jpg(HD_PATH_ID pathId, UINT32 yuvPhyAddr, FRAME_T *poutFrame)
{
    HD_RESULT  ret = HD_OK;
    INT bsMaxSize;
    HD_VIDEOENC_BS videoBitstream;
    BUFFER_T bsOutBuffer;
    HD_VIDEO_FRAME frame;
    HI_U8 *data = NULL;
    HI_U32 size = 0;

    frame.sign = MAKEFOURCC('V', 'F', 'R', 'M');
    frame.ddr_id = DDR_ID0;
    frame.dim.w = poutFrame->width;
    frame.dim.h = poutFrame->height;
    frame.pxlfmt = HD_VIDEO_PXLFMT_YUV420_MB;
    frame.phy_addr[0] = yuvPhyAddr;

    bsMaxSize = poutFrame->width * poutFrame->height / 2;
    bsOutBuffer.bs.sign = MAKEFOURCC('V', 'S', 'T', 'M');
    bsOutBuffer.bs.video_pack[0].size = bsMaxSize;
    bsOutBuffer.bs.ddr_id = DDR_ID0;
    ret = allocate_buffer(HD_COMMON_MEM_USER_BLK, bsMaxSize, bsOutBuffer.bs.ddr_id, &bsOutBuffer);
    if (ret != 0) {
        VAPILOG("allocate_buffer fail\n");
        return  ret;
    }

    bsOutBuffer.bs.video_pack[0].size = bsMaxSize;
    ret = hd_videoenc_push_in_buf(pathId, &frame, &(bsOutBuffer.bs), 500);
    if (ret != HD_OK) {
        VAPILOG("hd_videoenc_push_in_buf fail\n");
        goto exit;
    }
    ret = hd_videoenc_pull_out_buf(pathId, &videoBitstream, 500);
    if (ret != HD_OK) {
        VAPILOG("hd_videoenc_pull_out_buf fail\n");
    } else {
        size = videoBitstream.video_pack[0].size;
        data = ms_malloc(size);
        if (data == NULL) {
            VAPILOG("malloc fail\n");
            goto exit;
        }
        memcpy(data, bsOutBuffer.va, size);
    }

exit:
    ret = free_buffer(&bsOutBuffer);
    if (ret != HD_OK) {
        VAPILOG("free_buffer fail\n");
    }

    poutFrame->vriAddr = data;
    poutFrame->size = size;
    poutFrame->enType = ENC_TYPE_MJPEG;

    return ret;
}

static HD_RESULT set_encoder(HD_PATH_ID vencPath, UINT32 quality, UINT32 width, UINT32 height)
{
    HD_RESULT ret = HD_OK;

    HD_VIDEOENC_IN videoInParam;
    HD_VIDEOENC_OUT videoOutParam;
    HD_H26XENC_RATE_CONTROL rcParam;

    /* set main stream param */
    videoInParam.dim.w = width;
    videoInParam.dim.h = height;
    videoInParam.pxl_fmt = HD_VIDEO_PXLFMT_YUV420_MB;
    videoInParam.dir = HD_VIDEO_DIR_NONE;
    ret = hd_videoenc_set(vencPath, HD_VIDEOENC_PARAM_IN, &videoInParam);
    if (ret != HD_OK) {
        return ret;
    }

    ret = hd_videoenc_get(vencPath, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &videoOutParam);
    if (ret != HD_OK) {
        return ret;
    }
    videoOutParam.codec_type = HD_CODEC_TYPE_JPEG;
    videoOutParam.jpeg.retstart_interval = 0;
    videoOutParam.jpeg.image_quality = quality;
    /* videoOutParam.h26x.gop_num = 60; */
    ret = hd_videoenc_set(vencPath, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &videoOutParam);
    if (ret != HD_OK) {
        return ret;
    }

    ret = hd_videoenc_get(vencPath, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rcParam);
    if (ret != HD_OK) {
        return ret;
    }
    rcParam.rc_mode = HD_RC_MODE_CBR;
    rcParam.cbr.frame_rate_base = 1;   /* fps = 30/1 = 30 */
    rcParam.cbr.frame_rate_incr = 1;
    rcParam.cbr.bitrate = 2048 * 1024;
    ret = hd_videoenc_set(vencPath, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rcParam);
    if (ret != HD_OK) {
        return ret;
    }

    return ret;
}

static HD_RESULT start_encoding(HD_PATH_ID vdecPath, HD_PATH_ID vpePath, HD_PATH_ID vencPath,
                                FRAME_T *poutFrame, HI_BOOL isDecRelease)
{
    HD_RESULT ret;
    HD_VIDEO_FRAME outBuffer;
    void *outBufferVa = NULL;
    INT bufSize;
    UINT32 flag = 0;
    UINT32 vedioW, vedioH;
    struct map_info voutVaMapInfo = {0};
    UINT64 pool = HD_COMMON_MEM_USER_BLK;
    UINT32 scaleOutBlkSize = 0;
    UINT32 scaleInBlkSize = 0;
    HD_COMMON_MEM_VB_BLK scaleOutBlk = 0;
    HD_COMMON_MEM_VB_BLK scaleInBlk = 0;
    UINT32 scaleOutPa = 0;
    UINT32 scaleOutVa = 0;
    UINT32 scaleInPa = 0;
    UINT32 scaleInVa = 0;
    HD_COMMON_MEM_DDR_ID ddr = DDR_ID0;
    HD_VIDEO_FRAME scaleInBuffer = {0}, scaleOutBuffer = {0};
    HD_VIDEOPROC_CROP crop;
    HD_VIDEOPROC_OUT out;

    /* Pull dec out buffer */
    memset(&outBuffer, 0x0, sizeof(HD_VIDEO_FRAME));
    ret = hd_videodec_pull_out_buf(vdecPath, &outBuffer, 500);
    if (ret != HD_OK) {
        VAPILOG("hd_videodec_pull_out_buf fail\r\n");
        isDecRelease = HI_FALSE;
        goto exit_enc;
    }
    vedioW = outBuffer.dim.w;
    vedioH = outBuffer.dim.h;

    /*
        H264 size range: min(64x64) ~ max(4096x4096)
        H264 dim alignment: first(16x16), extra(16x16)
        H264 output format: first(YUV420), extra(YUV420)
        H265 size range: min(64x64) ~ max(4096x4096)
        H265 dim alignment: first(64x64), extra(16x16)
        H265 output format: first(YUV420_MB2), extra(YUV420)
    */
    if (outBuffer.pxlfmt == HD_VIDEO_PXLFMT_YUV420) {
        outBuffer.dim.w = ALIGN_CEIL(outBuffer.dim.w, 16); //h264
        outBuffer.dim.h = ALIGN_CEIL(outBuffer.dim.h, 16);
    } else {
        outBuffer.dim.w = ALIGN_CEIL(outBuffer.dim.w, 64); //h265
        outBuffer.dim.h = ALIGN_CEIL(outBuffer.dim.h, 64);
    }
    
    bufSize = hd_common_mem_calc_buf_size((void *)&outBuffer);
    outBufferVa = mmap_pa_to_va((unsigned int)outBuffer.phy_addr[0], bufSize, &voutVaMapInfo);
    if (!outBufferVa) {
        VAPILOG("mmap_pa_to_va fail\n");
        ret = HD_ERR_NG;
        goto exit_enc;
    }

    /* prepare proc input buf */
    scaleInBlkSize = outBuffer.dim.w * outBuffer.dim.h * 2;
    scaleInBlk = hd_common_mem_get_block(pool, scaleInBlkSize, ddr);
    if (HD_COMMON_MEM_VB_INVALID_BLK == scaleInBlk) {
        VAPILOG("hd_common_mem_get_block fail\r\n");
        ret = HD_ERR_NG;
        goto exit_enc;
    }
    scaleInPa = hd_common_mem_blk2pa(scaleInBlk);
    if (scaleInPa == 0) {
        VAPILOG("hd_common_mem_blk2pa fail, blk = %#lx\r\n", scaleInBlk);
        ret = HD_ERR_NG;
        goto exit_enc;
    }
    scaleInVa = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, scaleInPa, scaleInBlkSize);
    if (!scaleInVa) {
        VAPILOG("hd_common_mem_mmap fail\n");
        ret = HD_ERR_NG;
        goto exit_enc;
    }
    memcpy((void *)scaleInVa, outBufferVa, bufSize);
    /* Release dec out buffer.ASAP */
    if (isDecRelease) {
        isDecRelease = HI_FALSE;
        ret = hd_videodec_release_out_buf(vdecPath, &outBuffer);
        if (ret != HD_OK) {
            VAPILOG("hd_videodec_pull_out_buf fail\n");
            goto exit_enc;
        }
    }

    /* Set parameters */
    crop.win.rect.x = 0;
    crop.win.rect.y = 0;
    crop.win.rect.w = vedioW;
    crop.win.rect.h = vedioH;
    crop.win.coord.w = outBuffer.dim.w;
    crop.win.coord.h = outBuffer.dim.h;

    ret = hd_videoproc_set(vpePath, HD_VIDEOPROC_PARAM_IN_CROP, (void *)&crop);
    if (ret != HD_OK) {
        VAPILOG("hd_videoproc_set in crop fail\n");
        goto exit_enc;
    }
    out.rect.x = 0;
    out.rect.y = 0;
    out.rect.w = poutFrame->width;
    out.rect.h = poutFrame->height;
    out.bg.w = poutFrame->width;
    out.bg.h = poutFrame->height;
    out.pxlfmt = HD_VIDEO_PXLFMT_YUV420_MB;
    out.dir = HD_VIDEO_DIR_NONE;
    ret = hd_videoproc_set(vpePath, HD_VIDEOPROC_PARAM_OUT, (void *)&out);
    if (ret != HD_OK) {
        VAPILOG("hd_videoproc_set out param fail\n");
        goto exit_enc;
    }

    flag = VENDOR_VIDEOPROC_FLAG_UV_SWAP;
    ret = vendor_videoproc_set(vpePath, VENDOR_VIDEOPROC_USER_FLAG, &flag);
    if (ret != HD_OK) {
        VAPILOG("vendor_videoproc_set user_flag fail(%d)\n", ret);
        goto exit_enc;
    }

    scaleOutBlkSize = poutFrame->width * poutFrame->height * 2;
    scaleOutBlk = hd_common_mem_get_block(pool, scaleOutBlkSize, ddr);
    if (HD_COMMON_MEM_VB_INVALID_BLK == scaleOutBlk) {
        VAPILOG("hd_common_mem_get_block fail\r\n");
        ret = HD_ERR_NG;
        goto exit_enc;
    }
    scaleOutPa = hd_common_mem_blk2pa(scaleOutBlk);
    if (scaleOutPa == 0) {
        VAPILOG("hd_common_mem_blk2pa fail, blk = %#lx\r\n", scaleOutBlk);
        ret = HD_ERR_NG;
        goto exit_enc;
    }
    scaleOutVa = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, scaleOutPa, scaleOutBlkSize);

    scaleInBuffer.ddr_id = DDR_ID0;
    scaleInBuffer.dim.w = outBuffer.dim.w;
    scaleInBuffer.dim.h = outBuffer.dim.h;
    scaleInBuffer.pxlfmt = outBuffer.pxlfmt;
    scaleInBuffer.phy_addr[0] = scaleInPa;
    scaleOutBuffer.phy_addr[0] = scaleOutPa;
    scaleOutBuffer.ddr_id = DDR_ID0;
    scaleOutBuffer.dim.w = poutFrame->width;
    scaleOutBuffer.dim.h = poutFrame->height;
    scaleOutBuffer.pxlfmt = HD_VIDEO_PXLFMT_YUV420_MB;

    ret = hd_videoproc_push_in_buf(vpePath, &scaleInBuffer, &scaleOutBuffer, 500);
    if (ret != HD_OK) {
        VAPILOG("hd_videoproc_push_in_buf fail\n");
        goto exit_enc;
    }

    /* Pull out buffer */
    ret = hd_videoproc_pull_out_buf(vpePath, &scaleOutBuffer, 500);
    if (ret != HD_OK) {
        VAPILOG("hd_videoproc_pull_out_buf fail\n");
        goto exit_enc;
    }

    ret = encode_jpg(vencPath, scaleOutPa, poutFrame);

exit_enc:
    if (isDecRelease) {
        hd_videodec_release_out_buf(vdecPath, &outBuffer);
    }
    if (outBufferVa) {
        munmap(voutVaMapInfo.va, voutVaMapInfo.mapSize);
    }
    if (scaleOutVa) {
        hd_common_mem_munmap((void *)scaleOutVa, scaleOutBlkSize);
    }
    if (scaleInVa) {
        hd_common_mem_munmap((void *)scaleInVa, scaleInBlkSize);
    }
    if (scaleOutBlk) {
        hd_common_mem_release_block(scaleOutBlk);
    }
    if (scaleInBlk) {
        hd_common_mem_release_block(scaleInBlk);
    }

    return ret;
}

static HD_RESULT vapi_yuv_to_jpeg(HD_PATH_ID vdecPath, FRAME_T *pinFrame, FRAME_T *poutFrame, HI_BOOL isDecRelease)
{
    HD_RESULT ret = HD_OK;
    HD_PATH_ID vpePath = g_stJpeg.pathid.vpe;
    HD_PATH_ID vencPath = g_stJpeg.pathid.venc;

    poutFrame->width = VAPI_MIN(pinFrame->width, poutFrame->width);
    poutFrame->height = VAPI_MIN(pinFrame->height, poutFrame->height);
    poutFrame->width  = ALIGN_UP(VAPI_MIN(poutFrame->width, VAPI_VENC_MAX_WIDTH), 16);
    poutFrame->height = ALIGN_UP(VAPI_MIN(poutFrame->height, VAPI_VENC_MAX_HEIGTH), 16);

    ret = set_encoder(vencPath, poutFrame->quality, poutFrame->width, poutFrame->height);
    if (ret != HD_OK) {
        return ret;
    }

    return start_encoding(vdecPath, vpePath, vencPath, poutFrame, isDecRelease);
}

static HD_RESULT vapi_h26x_to_jpeg(HD_PATH_ID vdecPath, FRAME_T *pinFrame, FRAME_T *poutFrame)
{
    INT32 frameSize = pinFrame->size;
    void *frameData = pinFrame->vriAddr;
    UINT64 pool = HD_COMMON_MEM_USER_BLK;
    HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
    HD_COMMON_MEM_VB_BLK decInBlk = 0;
    HD_COMMON_MEM_VB_BLK decOutBlk = 0;
    void *decInBlkVa = 0;
    UINT32 decInBlkPa = 0;
    HD_RESULT ret = HD_OK;
    HD_VIDEODEC_PATH_CONFIG decConfig;
    HD_VIDEODEC_BS bsInBuffer = {0};
    HD_VIDEO_FRAME decOutBuffer = {0};
    unsigned int frameBufSize = 0;

    memset(&decConfig, 0x0, sizeof(HD_VIDEODEC_PATH_CONFIG));
    decConfig.max_mem.dim.w = ALIGN_CEIL(pinFrame->width, 16);
    decConfig.max_mem.dim.h = ALIGN_CEIL(pinFrame->height, 16);
    decConfig.max_mem.frame_rate = 1;
    decConfig.max_mem.bs_counts = 1;
    decConfig.max_mem.codec_type = comm_codec_type(pinFrame->enType);
    decConfig.max_mem.max_ref_num = 1;
    decConfig.max_mem.max_bitrate = 2 * 1024 * 1024;

    ret = hd_videodec_set(vdecPath, HD_VIDEODEC_PARAM_PATH_CONFIG, &decConfig);
    if (ret != HD_OK) {
        VAPILOG("hd_videodec_set [%dx%d] failed with %d !\n", pinFrame->width, pinFrame->height, ret);
        goto exit_dec;
    }

    decInBlk = hd_common_mem_get_block(pool, frameSize, ddr_id);
    if (HD_COMMON_MEM_VB_INVALID_BLK == decInBlk) {
        VAPILOG("hd_common_mem_get_block fail\r\n");
        ret = HD_ERR_NG;
        goto exit_dec;
    }

    decInBlkPa = hd_common_mem_blk2pa(decInBlk);
    if (decInBlkPa == 0) {
        VAPILOG("hd_common_mem_blk2pa fail, in_blk = %#lx\r\n", decInBlk);
        ret = HD_ERR_NG;
        goto exit_dec;
    }

    decInBlkVa = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, decInBlkPa, frameSize);
    if (!decInBlkVa) {
        VAPILOG("hd_common_mem_mmap fail\n");
        ret = HD_ERR_NG;
        goto exit_dec;
    }
    memcpy(decInBlkVa, frameData, frameSize);

    ret = vendor_common_mem_cache_sync(decInBlkVa, frameSize, VENDOR_COMMON_MEM_DMA_TO_DEVICE);
    if (ret != HD_OK) {
        VAPILOG("hd_common_mem_cache_sync fail\n");
        goto exit_dec;
    }
    bsInBuffer.ddr_id = ddr_id;
    bsInBuffer.size = frameSize;
    bsInBuffer.phy_addr = decInBlkPa;
    if (pinFrame->enType == ENC_TYPE_H264) {
        decOutBuffer.sign = MAKEFOURCC('2', '6', '4', 'D');
    } else {
        decOutBuffer.sign = MAKEFOURCC('2', '6', '5', 'D');
    }
    decOutBuffer.dim.w  = ALIGN_CEIL_16(pinFrame->width);
    decOutBuffer.dim.h  = ALIGN_CEIL_16(pinFrame->height);
    decOutBuffer.ddr_id = ddr_id;
    frameBufSize = hd_common_mem_calc_buf_size(&decOutBuffer);

    decOutBlk = hd_common_mem_get_block(pool, frameBufSize, ddr_id);
    if (HD_COMMON_MEM_VB_INVALID_BLK == decOutBlk) {
        VAPILOG("hd_common_mem_get_block fail\n");
        ret = HD_ERR_NG;
        goto exit_dec;
    }
    decOutBuffer.phy_addr[0] = hd_common_mem_blk2pa(decOutBlk);
    if (decOutBuffer.phy_addr[0] == 0) {
        VAPILOG("hd_common_mem_blk2pa fail, blk = %#lx\r\n", decOutBlk);
        ret = HD_ERR_NG;
        goto exit_dec;
    }
    ret = hd_videodec_push_in_buf(vdecPath, &bsInBuffer, &decOutBuffer, 1000);
    if (ret != HD_OK) {
        VAPILOG("hd_videodec_push_in_buf fail\n");
        goto exit_dec;
    }

    ret = vapi_yuv_to_jpeg(vdecPath, pinFrame, poutFrame, HI_FALSE);
    if (ret != HD_OK) {
        VAPILOG("vapi_yuv_to_jpeg fail\n");
    }

exit_dec:

    if (decInBlkVa) {
        hd_common_mem_munmap(decInBlkVa, frameSize);
    } 
    if (decInBlk) {
        hd_common_mem_release_block(decInBlk);
    }
    if (decOutBlk) {
        hd_common_mem_release_block(decOutBlk);
    }

    return ret;
}

HI_S32 vapi_jpeg_hw(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight,
                    HI_U8 *pu8InAddr, HI_U32 u32InLen,
                    HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight,
                    HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen, HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight,
                    pthread_mutex_t *mutex)
{
    HI_S32 s32Ret = HI_FAILURE;
    FRAME_T stInFrame;
    FRAME_T stOutFrame;

    stInFrame.enType = enType;
    stInFrame.width = u32InWidth;
    stInFrame.height = u32InHeight;
    stInFrame.size = u32InLen;
    stInFrame.vriAddr = pu8InAddr;

    stOutFrame.enType = ENC_TYPE_MJPEG;
    stOutFrame.quality = u32Qfactor;
    stOutFrame.width = u32OutWidth;
    stOutFrame.height = u32OutHeight;

    comm_mutex_lock(mutex);
    s32Ret = vapi_h26x_to_jpeg(g_stJpeg.pathid.vdec, &stInFrame, &stOutFrame);
    comm_mutex_unlock(mutex);
    if (s32Ret == HI_SUCCESS) {
        *ppu8OutAddr = stOutFrame.vriAddr;
        *pu32OutLen = stOutFrame.size;
        *pu32OutWidth = stOutFrame.width;
        *pu32OutHeight = stOutFrame.height;
    }

    return s32Ret;
}

HI_S32 vapi_yuv2jpeg(HI_S32 vdChn, VAPI_JPEG_S *pstJpeg)
{
    HD_PATH_ID vdecPath = vapi_vdec_get_chn_path(vdChn);
    FRAME_T stInFrame;
    FRAME_T stOutFrame;
    HI_S32 s32Ret = HI_FAILURE;

    vapi_vdec_get_chn_wh(vdChn, &stInFrame.width, &stInFrame.height);
    stOutFrame.enType   = ENC_TYPE_MJPEG;
    stOutFrame.quality  = 50;
    stOutFrame.width    = stInFrame.width ;
    stOutFrame.height   = stInFrame.height;

    s32Ret =  vapi_yuv_to_jpeg(vdecPath, &stInFrame, &stOutFrame, HI_TRUE);
    if (s32Ret == HI_SUCCESS) {
        pstJpeg->enType = ENC_TYPE_MJPEG;
        pstJpeg->u32Qfactor  = stOutFrame.quality;
        pstJpeg->u32InWidth  = stOutFrame.width;
        pstJpeg->u32InHeight = stOutFrame.height;
        pstJpeg->pu8InAddr   = stOutFrame.vriAddr;
        pstJpeg->u32InLen    = stOutFrame.size;
    }

    return s32Ret;
}

HD_RESULT vapi_jpeg_init()
{
    HD_RESULT ret = HD_OK;

    memset(&g_stJpeg, 0, sizeof(JPEG_T));
    comm_mutex_init(&g_stJpeg.mutex);

    if ((g_stJpeg.memFd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        VAPILOG("open /dev/mem failed.\n");
        return -1;
    }

    ret = hd_videodec_open(HD_VIDEODEC_IN(0, VAPI_JPEG_VDEC_ID), HD_VIDEODEC_OUT(0, VAPI_JPEG_VDEC_ID),
                           &g_stJpeg.pathid.vdec);
    if (ret != HD_OK) {
        VAPILOG("hd_videodec_open failed with %d", ret);
        return ret;
    }

    ret = hd_videoproc_open(HD_VIDEOPROC_IN(VAPI_JPEG_VPE_ID, 0), HD_VIDEOPROC_OUT(VAPI_JPEG_VPE_ID, 0),
                            &g_stJpeg.pathid.vpe);
    if (ret != HD_OK) {
        VAPILOG("hd_videoproc_open failed with %d", ret);
        return ret;
    }

    ret = hd_videoenc_open(HD_VIDEOENC_IN(0, VAPI_JPEG_VENC_ID), HD_VIDEOENC_OUT(0, VAPI_JPEG_VENC_ID),
                           &g_stJpeg.pathid.venc);
    if (ret != HD_OK) {
        VAPILOG("hd_videoenc_open failed with %d", ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_jpeg_uninit()
{
    HD_RESULT ret = HD_OK;
    ret = hd_videodec_close(g_stJpeg.pathid.vdec);
    if (ret != HD_OK) {
        VAPILOG("hd_videodec_close failed with %d", ret);
        return ret;
    }

    ret = hd_videoproc_close(g_stJpeg.pathid.vpe);
    if (ret != HD_OK) {
        VAPILOG("hd_videoproc_close failed with %d", ret);
        return ret;
    }

    ret = hd_videoenc_close(g_stJpeg.pathid.venc);
    if (ret != HD_OK) {
        VAPILOG("hd_videoenc_close failed with %d", ret);
        return ret;
    }

    comm_mutex_uninit(&g_stJpeg.mutex);
    if (g_stJpeg.memFd != -1) {
        close(g_stJpeg.memFd);
    }

    return ret;
}


