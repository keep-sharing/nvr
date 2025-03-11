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


extern const CscCoef_S g_stCSC_RGB2YUV601_pc;

extern const CscCoef_S g_stCSC_RGB2YUV709_pc;

extern const CscCoef_S g_stCSC_YUV6012RGB_pc;

extern const CscCoef_S g_stCSC_YUV7092RGB_pc;

extern const CscCoef_S g_stCSC_YUV2YUV_601_709;
extern const CscCoef_S g_stCSC_YUV2YUV_709_601;

extern const CscCoef_S g_stCSC_Init;


extern const int SIN_TABLE[61];
extern const int COS_TABLE[61];


#endif
