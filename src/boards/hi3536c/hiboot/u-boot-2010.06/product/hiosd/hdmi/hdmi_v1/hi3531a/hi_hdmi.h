/*
 * Copyright (c) 2015 HiSilicon Technologies Co., Ltd.
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


#ifndef  __HI_HDMI_H__
#define  __HI_HDMI_H__

#include "hi_type.h"
#include "eedefs.h"
#include "hdmitx.h"
#include "infofrm.h"
//#include "optm_m_disp.h"
#include "hi_vo_wrap.h"
#include "de.h"

typedef enum hiHDMI_VIDEO_MODE
{
    HI_HDMI_VIDEO_MODE_RGB444,       /**<RGB444���ģʽ */ 
    HI_HDMI_VIDEO_MODE_YCBCR422,     /**<YCBCR422���ģʽ */ 
    HI_HDMI_VIDEO_MODE_YCBCR444,     /**<YCBCR444���ģʽ */ 
        
    HI_HDMI_VIDEO_MODE_BUTT    
}HI_HDMI_VIDEO_MODE_E;


typedef enum hiHDMI_COLORSPACE_E
{
    HDMI_COLORIMETRY_NO_DATA,
    HDMI_COLORIMETRY_ITU601,
    HDMI_COLORIMETRY_ITU709,
    HDMI_COLORIMETRY_EXTENDED,
    HDMI_COLORIMETRY_XVYCC_601,
    HDMI_COLORIMETRY_XVYCC_709,
}HI_HDMI_COLORSPACE_E;

typedef enum hiHDMI_ASPECT_RATIO_E
{
    HI_HDMI_ASPECT_RATIO_UNKNOWN,  /**< δ֪��߱�*/
    HI_HDMI_ASPECT_RATIO_4TO3,     /**< 4��3*/
    HI_HDMI_ASPECT_RATIO_16TO9,    /**< 16��9*/
    HI_HDMI_ASPECT_RATIO_SQUARE,   /**< ������*/
    HI_HDMI_ASPECT_RATIO_14TO9,    /**< 14��9*/
    HI_HDMI_ASPECT_RATIO_221TO1,   /**< 221��100*/

    HI_HDMI_ASPECT_RATIO_BUTT
}HI_HDMI_ASPECT_RATIO_E;


int hdmi_display(unsigned int dev, unsigned int vosync, unsigned int input, unsigned int output);
void hdmi_stop(void);

HI_U32 HI_DRV_HDMI_UpdateStatus(void);

#endif
