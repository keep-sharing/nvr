/* 
 * ***************************************************************
 * Filename:      	vapi_vpss.c
 * Created at:    	2015.10.21
 * Description:   	video process sub system controller
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

HI_S32 vapi_vpss_create(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, FISH_E enFish, HI_U32 u32MaxW, HI_U32 u32MaxH)
{
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
    HI_S32 s32Ret;

    /*** Set Vpss Grp Attr ***/
    stGrpAttr.u32MaxW = u32MaxW;
    stGrpAttr.u32MaxH = u32MaxH;
    stGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;    
    stGrpAttr.bIeEn = HI_FALSE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bDciEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_FALSE;
    stGrpAttr.bEsEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;

    /*** create vpss group ***/
    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_CreateGrp [%d][%dx%dx] failed with %#x!\n",VpssGrp, u32MaxW, u32MaxH, s32Ret);
        return HI_FAILURE;
    }

    /*** set vpss param ***/
    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_GetGrpParam [%d] failed with %#x!\n",VpssGrp, s32Ret);
        return HI_FAILURE;
    }
    
    stVpssParam.u32IeStrength = 0;
    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_SetGrpParam [%d] failed with %#x!\n",VpssGrp, s32Ret);
        return HI_FAILURE;
    }

    /*** enable vpss chn, with frame ***/
    /* Set Vpss Chn attr */
    stChnAttr.bSpEn = HI_FALSE;
    stChnAttr.bUVInvert = HI_FALSE;
    
    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_SetChnAttr [%d-%d] failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        return HI_FAILURE;
    }

    if (enFish != FISH_MODE_NONE)
    {
        VPSS_CHN_MODE_S stVpssMode;
        
        s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, VpssChn, &stVpssMode);
        if(s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_VPSS_GetChnMode [%d] failed with %#x!\n", VpssGrp, s32Ret);
            return s32Ret; 
        }
        VAPILOG("w:%u h:%u", u32MaxW, u32MaxH);

        stVpssMode.enChnMode = VPSS_CHN_MODE_USER;
        stVpssMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        if (u32MaxW*u32MaxH > VAPI_FISH_MAX_WIDTH*VAPI_FISH_MAX_HEIGTH)
        {
            if (u32MaxW > u32MaxH)
            {
                stVpssMode.u32Width = VAPI_FISH_MAX_WIDTH;
                stVpssMode.u32Height = ALIGN_BACK(u32MaxH*VAPI_FISH_MAX_WIDTH/u32MaxW, 16);
            }
            else
            {
                stVpssMode.u32Width = ALIGN_BACK(u32MaxW*VAPI_FISH_MAX_HEIGTH/u32MaxH, 16);
                stVpssMode.u32Height = VAPI_FISH_MAX_HEIGTH;
            }
        }
        else
        {
            stVpssMode.u32Width = ALIGN_BACK(u32MaxW, 16);
            stVpssMode.u32Height = ALIGN_BACK(u32MaxH, 16);
        }

        VAPILOG("out w:%u h:%u", stVpssMode.u32Width, stVpssMode.u32Height);
        s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stVpssMode);
        if(s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_VPSS_GetChnMode [%d] failed with %#x!\n",VpssGrp, s32Ret);
            return s32Ret;
        }
        VAPILOG("grp:%d chn:%d", VpssGrp, VpssChn);
        s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, VpssChn, 1);
        VAPILOG("set queue depth stat %d", s32Ret);
    }

    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_EnableChn [%d-%d] failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        return HI_FAILURE;
    }
    
    /*** start vpss group ***/
    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_StartGrp [%d] failed with %#x\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_S32 vapi_vpss_destory(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_StopGrp [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_DisableChn [%d-%d] failed with %#x!\n", VpssGrp, VpssChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        VAPIERR("HI_MPI_VPSS_DestroyGrp [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

HI_S32 vapi_vpss_snap_yuv(VPSS_GRP VpssGrp, VAPI_YUV_S *pstYUV, VIDEO_FRAME_INFO_S *pstFrame)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8   *pVirAddr;

    s32Ret = HI_MPI_VPSS_GetChnFrame(VpssGrp, 0, pstFrame, 100);
    if (s32Ret != HI_SUCCESS)
        return s32Ret;
    pstYUV->width = pstFrame->stVFrame.u32Stride[0];
    pstYUV->height = pstFrame->stVFrame.u32Height;
    pstYUV->size = pstYUV->width*pstYUV->height*3/2;

	pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap(pstFrame->stVFrame.u32PhyAddr[0], pstYUV->size);
    pstYUV->data = pVirAddr;
    pstYUV->phy = pstFrame->stVFrame.u32PhyAddr[0];

    return s32Ret;
}

void vapi_vpss_snap_free(VAPI_YUV_S *pstYUV, VIDEO_FRAME_INFO_S *pstFrame) 
{
    HI_MPI_VPSS_ReleaseChnFrame(0, 0, pstFrame);
    HI_MPI_SYS_Munmap(pstYUV->data, pstYUV->size);
}

HI_S32 vapi_vpss_snap_jpeg(VPSS_GRP VpssGrp, VAPI_JPEG_S *pstJpeg)
{

    HI_S32 s32Ret = HI_SUCCESS;
    VIDEO_FRAME_INFO_S stFrame;

    s32Ret = HI_MPI_VPSS_GetGrpFrame(VpssGrp, &stFrame, 0);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_GetGrpFrame [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }
    pstJpeg->enType = ENC_TYPE_MJPEG;
    pstJpeg->u32Qfactor = 50;

    s32Ret = vapi_yuv2jpeg(&stFrame, (HI_U8 **)&pstJpeg->pu8InAddr, &pstJpeg->u32InLen,
                          &pstJpeg->u32InWidth, &pstJpeg->u32InHeight);

    if (s32Ret == HI_SUCCESS && 0) //debug
    {
        FILE *pFile = fopen("/home/jpg.jpg", "wb");
        fwrite(pstJpeg->pu8InAddr, pstJpeg->u32InLen, 1, pFile);
        fclose(pFile);
    }
    
    HI_MPI_VPSS_ReleaseGrpFrame(VpssGrp, &stFrame);

    return s32Ret;
}

HI_S32 vapi_vpss_set_crop(VPSS_GRP VpssGrp, RECT_S *pstRect)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CROP_INFO_S stCropInfo;
    VIDEO_FRAME_INFO_S stVideoFrame;
    
    s32Ret = HI_MPI_VPSS_GetGrpFrame(VpssGrp, &stVideoFrame, 0);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_GetGrpFrame [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }
    
    stCropInfo.bEnable = HI_TRUE;
    stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
    stCropInfo.stCropRect.s32X = ALIGN_BACK(pstRect->s32X, 2);
    stCropInfo.stCropRect.s32Y = ALIGN_BACK(pstRect->s32Y, 2);
    stCropInfo.stCropRect.u32Width = ALIGN_BACK(pstRect->u32Width, 4);
    stCropInfo.stCropRect.u32Height = ALIGN_BACK(pstRect->u32Height, 4);
    s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp, &stCropInfo);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_SetGrpCrop [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }
    
    HI_MPI_VPSS_SendFrame(VpssGrp, &stVideoFrame, 0);
    s32Ret = HI_MPI_VPSS_ReleaseGrpFrame(VpssGrp, &stVideoFrame);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_ReleaseGrpFrame [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 vapi_vpss_chn_get_wh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 *pu32Width, HI_U32 *pu32Height)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHN_MODE_S stVpssMode;

    s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, VpssChn, &stVpssMode);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VPSS_GetChnMode [%d][%d] failed with %#x!\n", VpssGrp, VpssChn, s32Ret);
        return s32Ret;
    }

    *pu32Width = stVpssMode.u32Width;
    *pu32Height = stVpssMode.u32Height;
    
    return HI_SUCCESS;
}

