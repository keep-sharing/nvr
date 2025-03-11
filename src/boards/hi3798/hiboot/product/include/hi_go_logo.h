/******************************************************************************
*
* Copyright (C) 2015 Hisilicon Technologies Co., Ltd.  All rights reserved. 
*
* This program is confidential and proprietary to Hisilicon  Technologies Co., Ltd. (Hisilicon), 
* and may not be copied, reproduced, modified, disclosed to others, published or used, in
* whole or in part, without the express prior written permission of Hisilicon.
*
******************************************************************************
File Name           : hi_go_logo.h
Version             : Initial Draft
Author              : 
Created             : 2015/07/02
Description         : 
                       CNcomment: ����logoͼ���������ͷ�ļ� CNend\n
Function List   : 
History         :
Date                        Author                  Modification
2015/07/02                  y00181162               Created file      
******************************************************************************/

#ifndef  __HI_GO_LOGO_H__
#define  __HI_GO_LOGO_H__


/*********************************add include here******************************/

#include "hi_go_common.h"

/***************************** Macro Definition ******************************/



/*************************** Structure Definition ****************************/


/**===========================================================
 **layer
 **==========================================================**/
/**
 **����ͼ���������ظ�ʽ
 **/
typedef enum tagHIGoLayerIFmtE
{
    HI_GO_LAYER_IFMT_RGB_565     = 0x42,  /** RGB565   **/
    HI_GO_LAYER_IFMT_ARGB_1555   = 0x49,  /** ARGB1555 **/
    HI_GO_LAYER_IFMT_RGB_888     = 0x50,  /** RGB888   **/
    HI_GO_LAYER_IFMT_ARGB_8888   = 0x68,  /** ARGB8888 **/
    HI_GO_LAYER_IFMT_RGBA_5551   = 0xc9,
    HI_GO_LAYER_IFMT_RGBA_8888   = 0xe8,
    HI_GO_LAYER_IFMT_ABGR_8888   = 0xef,
    HI_GO_LAYER_IFMT_BUTT
}HI_GO_LAYER_IFMT_E;

typedef struct hiGoDispRectS
{
    HI_U32 u32SX;   /** source horizontal start position   **/
    HI_U32 u32SY;   /** source vertical start position     **/
    HI_U32 u32DXS;  /** dispaly horizontal start position  **/
    HI_U32 u32DYS;  /** display vertical start position    **/
    HI_U32 u32DXL;  /** dispaly horizontal end position    **/
    HI_U32 u32DYL;  /** display vertical end position      **/
    HI_U32 u32VX;   /** video horizontal start position    **/
    HI_U32 u32VY;   /** video vertical start position      **/
    HI_U32 u32VXL;  /** video horizontal start position    **/
    HI_U32 u32VYL;  /** video vertical start position      **/
    HI_U32 u32IWth; /** input width                        **/
    HI_U32 u32IHgt; /** input height                       **/
    HI_U32 u32OWth; /** output width                       **/
    HI_U32 u32OHgt; /** output height                      **/
}HI_GO_DISP_RECT_S;

/**===========================================================
 **decode
 **==========================================================**/
typedef enum hiGO_DEC_IMGTYPE_E
{
    HI_GO_DEC_IMGTYPE_JPEG = 0,
    HI_GO_DEC_IMGTYPE_BMP,
    HI_GO_DEC_IMGTPYE_BUTT
}HI_GO_DEC_IMGTYPE_E;

typedef struct hiGO_DEC_INSTANCE_S
{
	HI_U32  u64DecHandle;  /** �ڲ����������   **/
    HI_S32 (*CreateDecode)  (HI_U64 *pu64DecHandle,HI_CHAR* pSrcBuf,HI_U32 u32SrcLen);
    HI_S32 (*DestroyDecode) (HI_U64 u64DecHandle);
    HI_S32 (*GetImageInfo)  (HI_U64 u64DecHandle, HI_HANDLE hSurface);
    HI_S32 (*StartDecode)   (HI_U64 u64DecHandle, HI_HANDLE hSurface);
}HI_GO_DEC_INSTANCE_S;


/********************** Global Variable declaration **************************/


/******************************* API declaration *****************************/

/***************************************************************************************
* func          : HI_GO_SetCsc
* description   : CNcomment: ����CSCϵ����ͼ�����ΪYUV444��������ҪCSCת�� CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GO_SetCsc(HI_S32 s32GpId);


/*****************************************************************************
* func            : HI_GO_BLIT_Init
* description     : tde��ʼ��
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_BLIT_Init(HI_VOID);


/*****************************************************************************
* func            : HI_GO_Blit
* description     : 2d���Ӳ���
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_Blit(HI_HANDLE u32SrcSurface, const HI_GO_RECT_S* pSrcRect,HI_HANDLE u32DstSurface, const HI_GO_RECT_S* pDstRect,const HIGO_BLTOPT_S* pBlitOpt);


/*****************************************************************************
* func            : HI_GO_FillRect
* description     : ������
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_FillRect(HI_HANDLE hSurface, const HI_GO_RECT_S* pstRect, HI_U32 u32Color,HIGO_COMPOPT_E eComPopt);



/*****************************************************************************
* func            : HI_GO_Init
* description     : logo��ʼ��
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_Init(HI_VOID);

/*****************************************************************************
* func            : HI_GO_CreateLayer
* description     : ����ͼ��
* in              : ͼ�����
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_CreateLayer(HI_GO_LAYER_ID_E enLayerID, HIGO_PF_E enPixelFormat,HI_HANDLE *pSurface);

/*****************************************************************************
* func            : HI_GO_RefreshLayer
* description     : ˢ��ͼ��
* in              : ͼ�����
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_RefreshLayer(HI_HANDLE hSurface);

/*****************************************************************************
* func            : HI_GO_CreateSurface
* description     : ����surface
* in              : surface��Ϣ
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_CreateSurface(HI_U32 u32Width, HI_U32 u32Height, HIGO_PF_E enPixelFormat,HI_HANDLE *pSurface);

/*****************************************************************************
* func            : HI_GO_CreateDecSurface
* description     : ��������surface
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_CreateDecSurface(HI_HANDLE hSurface);

/*****************************************************************************
* func            : HI_GO_GetSurfaceSize
* description     : ��ȡsurface��С
* in              : surface��Ϣ
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_GetSurfaceSize(HI_HANDLE hSurface, HI_U32* pu32Width, HI_U32* pu32Height);

/*****************************************************************************
* func            : HI_GO_FreeSurface
* description     : �ͷ�surface
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_FreeSurface(HI_HANDLE hSurface);

/*****************************************************************************
* func            : HI_GO_FreeDecSurface
* description     : �ͷ�surface
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_FreeDecSurface(HI_HANDLE hSurface);


/*****************************************************************************
* func            : HI_GO_LockSurface
* description     : ��ȡsurface��Ϣ
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_LockSurface(HI_HANDLE hSurface, HI_PIXELDATA pData, HI_BOOL bSync);


/*****************************************************************************
* func            : HI_GO_UnlockSurface
* description     : ���surface��Ϣ
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_UnlockSurface(HI_HANDLE hSurface);


/*****************************************************************************
* func            : HI_GO_DestroyLayer
* description     : ����ͼ��
* in              : ͼ�����
* retval          : HI_SUCCESS
* retval          : HI_FAILURE
*****************************************************************************/
HI_S32 HI_GO_DestroyLayer(HI_HANDLE hSurface);


/*****************************************************************************
* func			:  HI_GO_CreateDecode
* description	:  ����������
* retval		:  NA
* others:		:  NA
*****************************************************************************/
HI_S32 HI_GO_CreateDecode(HI_HANDLE *pDecHandle,HI_CHAR* pSrcAddr,HI_U32 u32SrcLen);

/*****************************************************************************
* func			:  HI_GO_StartDecode
* description	:  ��ʼ����
* retval		:  NA
* others:		:  NA
*****************************************************************************/
HI_S32 HI_GO_StartDecode(HI_HANDLE DecHandle,HI_HANDLE *pSurface);

/*****************************************************************************
* func			:  HI_GO_DestroyDecode
* description	:  ���ٽ�����
* retval		:  NA
* others:		:  NA
*****************************************************************************/
HI_S32 HI_GO_DestroyDecode(HI_HANDLE DecHandle);


#endif /*__HI_GO_LOGO_H__ */
