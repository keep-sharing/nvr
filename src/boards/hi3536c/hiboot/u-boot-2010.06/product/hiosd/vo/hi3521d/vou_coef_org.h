/*
* Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef __VOU_COEF_ORG_H__
#define __VOU_COEF_ORG_H__

#include "vou_coef.h"

/**/
#define VOU_ZOOM_COEF_SIZE  (VOU_ZOOM_COEF_ITEM*VOU_ZOOM_COEF_MAXTAP * 2)  

/* 8tap*/
extern const int coefficient_lanczos3[18][8];//normalized ok
extern const int coefficient8_cubic[18][8];//normalized ok
extern const int coefficient8_lanczos2_8tap[18][8];//normalized ok
extern const int coefficient8_3M_a19[18][8];//normalized ok
extern const int coefficient8_2M_a05[18][8];
extern const HI_S32 coefficient8_1_5M_a05[18][8]; 

/* 6tap*/
extern const int coefficient6_6M_a25[18][6];//normalized ok
extern const int coefficient6_cubic[18][6];//normalized ok
extern const int coefficient6_5M_a25[18][6];//normalized ok
extern const int coefficient6_4M_a20[18][6];//normalized ok
extern const int coefficient6_3M_a15[18][6];//normalized ok
extern const HI_S32 coefficient6_2M_a05[18][6];
extern const HI_S32 coefficient6_1_5M_a05[18][6]; 

/* 4tap*/
extern const int coefficient4_5M_a15[18][4];//normalized ok
extern const int coefficient4_cubic[18][4];//normalized ok
extern const int coefficient4_4M_a15[18][4];//normalized ok
extern const HI_S32 coefficient4_2M_a05[18][4];
extern const HI_S32 coefficient4_1_5M_a05[18][4];

/**/
extern const HI_S32 *g_pOrgZoomCoef[VOU_ZOOM_COEF_BUTT][VOU_ZOOM_TAP_BUTT];
#if 0
extern const HI_U8 g_aVoGammaCoef[VOU_GAMM_COEF_ROW][VOU_GAMM_COEF_COL];
extern const HI_U16 g_aVoAccCoef[VOU_ACC_COEF_NUM];
#endif
/* RGB->YUV601  */
extern const CscCoef_S g_stCSC_RGB2YUV601_tv;
/* RGB->YUV601 */
extern const CscCoef_S g_stCSC_RGB2YUV601_pc;
/* RGB->YUV709  */
extern const CscCoef_S g_stCSC_RGB2YUV709_tv;
/* RGB->YUV709 */
extern const CscCoef_S g_stCSC_RGB2YUV709_pc;
/* YUV601->RGB  */
extern const CscCoef_S g_stCSC_YUV6012RGB_pc;
/* YUV709->RGB  */
extern const CscCoef_S g_stCSC_YUV7092RGB_pc;
/* YUV601->YUV709  */
extern const CscCoef_S g_stCSC_YUV2YUV_601_709;
/* YUV709->YUV601  */
extern const CscCoef_S g_stCSC_YUV2YUV_709_601;
/* YUV601->YUV709  */
extern const CscCoef_S g_stCSC_Init;


extern const int SIN_TABLE[61];
extern const int COS_TABLE[61];


#endif
