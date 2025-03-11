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

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "vou_coef.h"

const CscCoef_S g_stCSC_RGB2YUV601_tv        
    = {
     257, 504, 98, -148, -291, 439, 439, -368, -71,
     0, 0, 0,
     16, 128, 128, 
    };
const CscCoef_S g_stCSC_RGB2YUV601_pc        
    = {    
     299, 587, 114, -172, -339, 511, 511, -428, -83,
     0, 0, 0,
     0, 128, 128,
    };
const CscCoef_S g_stCSC_RGB2YUV709_tv
    = {
     183, 614, 62, -101, -338, 439, 439, -399, -40,
     0, 0, 0,
     16, 128, 128,
    };
const CscCoef_S g_stCSC_RGB2YUV709_pc
    = {
     213, 715, 72, -117, -394, 511, 511, -464, -47,
     0, 0, 0,
     0, 128, 128,
    };
const CscCoef_S g_stCSC_YUV6012RGB_pc
    = {
     1164, 0, 1596, 1164, -391, -813, 1164, 2018, 0,
     -16, -128, -128,
     0, 0, 0,
    };
const CscCoef_S g_stCSC_YUV7092RGB_pc
    = {
     1164, 0, 1793, 1164, -213, -534, 1164, 2115, 0,
     -16, -128, -128,
     0, 0, 0,
    };
const CscCoef_S g_stCSC_YUV2YUV_601_709
    = {
     1000, -116, 208, 0, 1017, 114, 0, 75, 1025,
     -16, -128, -128,
     16, 128, 128,
    };
const CscCoef_S g_stCSC_Init
    = {
     1000, 0, 0, 0, 1000, 0, 0, 0, 1000,
     -16, -128, -128,
     16, 128, 128,
    };
const int SIN_TABLE[61] = {
  -500,  -485,  -469,  -454,  -438,  -422,  -407,  -391,  -374,  -358,
  -342,  -325,  -309,  -292,  -276,  -259,  -242,  -225,  -208,  -191,
  -174,  -156,  -139,  -122,  -104,   -87,   -70,   -52,   -35,   -17,
     0,    17,    35,    52,    70,    87,   104,   122,   139,   156,
   174,   191,   208,   225,   242,   259,   276,   292,   309,   325,
   342,   358,   374,   391,   407,   422,   438,   454,   469,   485,
   500};

const int COS_TABLE[61] = {
   866,   875,   883,   891,   899,   906,   914,   921,   927,   934,
   940,   946,   951,   956,   961,   966,   970,   974,   978,   982,
   985,   988,   990,   993,   995,   996,   998,   999,   999,  1000,
  1000,  1000,   999,   999,   998,   996,   995,   993,   990,   988,
   985,   982,   978,   974,   970,   966,   961,   956,   951,   946,
   940,   934,   927,   921,   914,   906,   899,   891,   883,   875,
   866};


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

