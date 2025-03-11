/******************************************************************************
*
* Copyright (C) 2015 Hisilicon Technologies Co., Ltd.  All rights reserved. 
*
* This program is confidential and proprietary to Hisilicon  Technologies Co., Ltd. (Hisilicon), 
* and may not be copied, reproduced, modified, disclosed to others, published or used, in
* whole or in part, without the express prior written permission of Hisilicon.
*
******************************************************************************
File Name           : hi_gfx_hal.h
Version             : Initial Draft
Author              : 
Created             : 2015/07/02
Description         : 
                       CNcomment: 操作寄存器 CNend\n
Function List   : 
History         :
Date                        Author                  Modification
2015/07/02                  y00181162               Created file      
******************************************************************************/

#ifndef  __HI_GFX_HAL_H__
#define  __HI_GFX_HAL_H__


/*********************************add include here******************************/

#ifdef CONFIG_GFX_LOGO_KERNEL


#include "hi_go_logo.h"

/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/
typedef struct
{

   HI_S32 csc_in_dc1;
   HI_S32 csc_in_dc0;
   
   HI_S32 csc_out_dc1;
   HI_S32 csc_out_dc0;

   HI_S32 csc_out_dc2;
   HI_S32 csc_in_dc2;

   HI_S32 csc_coef00;
   HI_S32 csc_coef01;
   
   HI_S32 csc_coef10;
   HI_S32 csc_coef02;
   
   HI_S32 csc_coef11;
   HI_S32 csc_coef12;

   HI_S32 csc_coef20;
   HI_S32 csc_coef21;
   
   HI_S32 csc_coef22;
    
}HI_GFX_CSC_COEF_S;

/********************** Global Variable declaration **************************/


/******************************* API declaration *****************************/


/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetVdpReg
* description   : CNcomment: 设置寄存器地址 CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetVdpReg(HI_VOID);

/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetLayerReso
* description   : CNcomment: 设置图层输入分辨率 CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetLayerReso(HI_S32 s32LayerId, HI_GO_DISP_RECT_S stRect);

/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetGpReso
* description   : CNcomment: 设置GP输入输出分辨率 CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetGpReso(HI_S32 s32GpId, HI_GO_DISP_RECT_S stRect);


/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetGfx
* description   : CNcomment: 设置图形层 CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetGfx(HI_S32 s32LayerId,HI_GO_LAYER_IFMT_E eLayerFmt,HI_GO_SURFACE_S *pstLayerInfo);


/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetGp
* description   : CNcomment: 设置Gp CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetGp(HI_S32 s32GpId);


/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetCbm
* description   : CNcomment: 设置叠加寄存器 CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetCbm(HI_VOID);


/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetLayerEnable
* description   : CNcomment: 图层使能 CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetLayerEnable(HI_S32 s32LayerId, HI_BOOL bEnable);

/***************************************************************************************
* func          : HI_GFX_LOGO_HAL_SetCsc
* description   : CNcomment: 设置CSC系数 CNend\n
* param[in]     : HI_VOID
* retval        : NA
* others:       : NA
***************************************************************************************/
HI_VOID HI_GFX_LOGO_HAL_SetCsc(HI_S32 s32GpId,HI_GFX_CSC_COEF_S *pstCscCoef);

#endif

#endif /*__HI_GFX_HAL_H__ */
