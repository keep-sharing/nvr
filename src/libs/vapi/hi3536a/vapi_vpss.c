/*
 * ***************************************************************
 * Filename:      	vapi_vpss.c
 * Created at:    	2015.10.21
 * Description:   	video process sub system controller
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

td_s32 vapi_vpss_create(ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn, FISH_E enFish, td_u32 u32MaxW, td_u32 u32MaxH)
{
    ot_vpss_grp_attr  stGrpAttr   = {0};
    ot_vpss_chn_attr  stChnAttr   = {0};
    ot_vpss_grp_param stVpssParam = {0};
    td_s32            s32Ret;

    /*** Set Vpss Grp Attr ***/
    stGrpAttr.max_width                 = u32MaxW;
    stGrpAttr.max_height                = u32MaxH;
    stGrpAttr.frame_rate.src_frame_rate = -1;
    stGrpAttr.frame_rate.dst_frame_rate = -1;
    stGrpAttr.pixel_format              = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stGrpAttr.ie_en                     = TD_FALSE;
    stGrpAttr.nr_en                     = TD_FALSE;
    stGrpAttr.dci_en                    = TD_FALSE;
    stGrpAttr.dei_mode                  = OT_VPSS_DEI_MODE_OFF;
    stGrpAttr.buf_share_en              = TD_FALSE;

    /*** create vpss group ***/
    s32Ret = ss_mpi_vpss_create_grp(VpssGrp, &stGrpAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_create_grp [%d][%dx%dx] failed with %#x!\n", VpssGrp, u32MaxW, u32MaxH, s32Ret);
        return TD_FAILURE;
    }

    /*** set vpss param ***/
    s32Ret = ss_mpi_vpss_get_grp_param(VpssGrp, &stVpssParam);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_get_grp_param [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    stVpssParam.ie_strength = 0;
    s32Ret                  = ss_mpi_vpss_set_grp_param(VpssGrp, &stVpssParam);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_set_grp_param [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    /*** enable vpss chn, with frame ***/
    stChnAttr.chn_mode                  = OT_VPSS_CHN_MODE_AUTO;
    stChnAttr.compress_mode             = OT_COMPRESS_MODE_NONE;
    stChnAttr.pixel_format              = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stChnAttr.frame_rate.src_frame_rate = -1;
    stChnAttr.frame_rate.dst_frame_rate = -1;
    stChnAttr.depth                     = 1;
    stChnAttr.mirror_en                 = TD_FALSE;
    stChnAttr.flip_en                   = TD_FALSE;
    stChnAttr.border_en                 = TD_FALSE;
    stChnAttr.aspect_ratio.mode         = OT_ASPECT_RATIO_NONE;

    s32Ret = ss_mpi_vpss_set_chn_attr(VpssGrp, VpssChn, &stChnAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_set_chn_attr [%d-%d] failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        return TD_FAILURE;
    }

    if (enFish != FISH_MODE_NONE) {
        ot_vpss_chn_attr stVpssMode;

        s32Ret = ss_mpi_vpss_get_chn_attr(VpssGrp, VpssChn, &stVpssMode);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("ss_mpi_vpss_get_chn_attr [%d] failed with %#x!\n", VpssGrp, s32Ret);
            return s32Ret;
        }
        VAPILOG("w:%u h:%u", u32MaxW, u32MaxH);

        stVpssMode.chn_mode     = OT_VPSS_CHN_MODE_USER;
        stVpssMode.pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVpssMode.depth        = 1;
        if (u32MaxW * u32MaxH > VAPI_FISH_MAX_WIDTH * VAPI_FISH_MAX_HEIGTH) {
            if (u32MaxW > u32MaxH) {
                stVpssMode.width  = VAPI_FISH_MAX_WIDTH;
                stVpssMode.height = ALIGN_BACK(u32MaxH * VAPI_FISH_MAX_WIDTH / u32MaxW, 16);
            } else {
                stVpssMode.width  = ALIGN_BACK(u32MaxW * VAPI_FISH_MAX_HEIGTH / u32MaxH, 16);
                stVpssMode.height = VAPI_FISH_MAX_HEIGTH;
            }
        } else {
            stVpssMode.width  = ALIGN_BACK(u32MaxW, 16);
            stVpssMode.height = ALIGN_BACK(u32MaxH, 16);
        }

        VAPILOG("out w:%u h:%u", stVpssMode.width, stVpssMode.height);
        s32Ret = ss_mpi_vpss_set_chn_attr(VpssGrp, VpssChn, &stVpssMode);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("ss_mpi_vpss_set_chn_attr [%d] failed with %#x!\n", VpssGrp, s32Ret);
            return s32Ret;
        }
    }

    s32Ret = ss_mpi_vpss_enable_chn(VpssGrp, VpssChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_enable_chn [%d-%d] failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        return TD_FAILURE;
    }

    /*** start vpss group ***/
    s32Ret = ss_mpi_vpss_start_grp(VpssGrp);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_start_grp [%d] failed with %#x\n", VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 vapi_vpss_destory(ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn)
{
    td_s32 s32Ret = TD_SUCCESS;

    s32Ret = ss_mpi_vpss_stop_grp(VpssGrp);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_stop_grp [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vpss_disable_chn(VpssGrp, VpssChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_disable_chn [%d-%d] failed with %#x!\n", VpssGrp, VpssChn, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vpss_destroy_grp(VpssGrp);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_destroy_grp [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 vapi_vpss_snap_yuv(ot_vpss_grp VpssGrp, VAPI_YUV_S *pstYUV, ot_video_frame_info *pstFrame)
{
    td_s32 s32Ret = TD_SUCCESS;
    td_u8 *pVirAddr;

    s32Ret = ss_mpi_vpss_get_chn_frame(VpssGrp, 0, pstFrame, 100);
    if (s32Ret != TD_SUCCESS)
        return s32Ret;
    pstYUV->width  = pstFrame->video_frame.stride[0];
    pstYUV->height = pstFrame->video_frame.height;
    pstYUV->size   = pstYUV->width * pstYUV->height * 3 / 2;

    pVirAddr     = (td_u8 *)ss_mpi_sys_mmap(pstFrame->video_frame.phys_addr[0], pstYUV->size);
    pstYUV->data = pVirAddr;
    pstYUV->phy  = pstFrame->video_frame.phys_addr[0];

    return s32Ret;
}

void vapi_vpss_snap_free(VAPI_YUV_S *pstYUV, ot_video_frame_info *pstFrame)
{
    ss_mpi_vpss_release_chn_frame(0, 0, pstFrame);
    ss_mpi_sys_munmap(pstYUV->data, pstYUV->size);
}

td_s32 vapi_vpss_snap_jpeg(ot_vpss_grp VpssGrp, VAPI_JPEG_S *pstJpeg)
{
    td_s32              s32Ret = TD_SUCCESS;
    ot_video_frame_info stFrame;

    s32Ret = ss_mpi_vpss_get_grp_frame(VpssGrp, &stFrame, 500);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_get_grp_frame [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }
    pstJpeg->enType     = ENC_TYPE_MJPEG;
    pstJpeg->u32Qfactor = 50;

    s32Ret = vapi_yuv2jpeg(&stFrame, (td_u8 **)&pstJpeg->pu8InAddr, &pstJpeg->u32InLen, &pstJpeg->u32InWidth, &pstJpeg->u32InHeight);

    // for debug
    if (s32Ret == TD_SUCCESS && 0) {
        FILE *pFile = fopen("/home/jpg.jpg", "wb");
        fwrite(pstJpeg->pu8InAddr, pstJpeg->u32InLen, 1, pFile);
        fclose(pFile);
    }

    ss_mpi_vpss_release_grp_frame(VpssGrp, &stFrame);

    return s32Ret;
}

td_s32 vapi_vpss_set_crop(ot_vpss_grp VpssGrp, ot_rect *pstRect)
{
    td_s32              s32Ret = TD_SUCCESS;
    ot_vpss_crop_info   stCropInfo;
    ot_video_frame_info stVideoFrame;

    s32Ret = ss_mpi_vpss_get_grp_frame(VpssGrp, &stVideoFrame, 500);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_get_grp_frame [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }

    stCropInfo.enable           = TD_TRUE;
    stCropInfo.crop_mode        = OT_COORD_ABS;
    stCropInfo.crop_rect.x      = ALIGN_BACK(pstRect->x, 2);
    stCropInfo.crop_rect.y      = ALIGN_BACK(pstRect->y, 2);
    stCropInfo.crop_rect.width  = ALIGN_BACK(pstRect->width, 4);
    stCropInfo.crop_rect.height = ALIGN_BACK(pstRect->height, 4);
    s32Ret                      = ss_mpi_vpss_set_grp_crop(VpssGrp, &stCropInfo);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_set_grp_crop [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }

    ss_mpi_vpss_send_frame(VpssGrp, &stVideoFrame, 0);
    s32Ret = ss_mpi_vpss_release_grp_frame(VpssGrp, &stVideoFrame);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_release_grp_frame [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }
    return TD_SUCCESS;
}

td_s32 vapi_vpss_chn_get_wh(ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn, td_u32 *pu32Width, td_u32 *pu32Height)
{
    td_s32           s32Ret = TD_SUCCESS;
    ot_vpss_chn_attr stVpssMode;

    s32Ret = ss_mpi_vpss_get_chn_attr(VpssGrp, VpssChn, &stVpssMode);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vpss_get_chn_attr [%d][%d] failed with %#x!\n", VpssGrp, VpssChn, s32Ret);
        return s32Ret;
    }

    *pu32Width  = stVpssMode.width;
    *pu32Height = stVpssMode.height;

    return TD_SUCCESS;
}
