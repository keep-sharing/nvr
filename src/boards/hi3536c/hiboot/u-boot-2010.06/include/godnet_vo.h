/*
 * Copyright (c) 2010 HiSilicon Technologies Co., Ltd.
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

#ifndef __HI35XX_VO_H__
#define __HI35XX_VO_H__

#define CFG_MAXARGS 10

#define HI_353X_VO  1
#define PIC_MAX_WIDTH   1920
#define PIC_MAX_HEIGHT  1080
#define PIC_MIN_LENTH   16

#define VO_INTF_CVBS    (0x01L<<0)
#define VO_INTF_YPBPR   (0x01L<<1)
#define VO_INTF_VGA     (0x01L<<2)
#define VO_INTF_BT656   (0x01L<<3)
#define VO_INTF_BT1120  (0x01L<<4)
#define VO_INTF_HDMI    (0x01L<<5)

typedef enum hiVO_INTF_SYNC_E
{
    VO_OUTPUT_PAL = 0,
    VO_OUTPUT_NTSC,
    
    VO_OUTPUT_1080P24,
    VO_OUTPUT_1080P25,
    VO_OUTPUT_1080P30,
    
    VO_OUTPUT_720P50, 
    VO_OUTPUT_720P60,   
    VO_OUTPUT_1080I50,
    VO_OUTPUT_1080I60,    
    VO_OUTPUT_1080P50,
    VO_OUTPUT_1080P60,            

    VO_OUTPUT_576P50,
    VO_OUTPUT_480P60,

    VO_OUTPUT_800x600_60,            /* VESA 800 x 600 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1024x768_60,           /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1280x1024_60,          /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1366x768_60,           /* VESA 1366 x 768 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1440x900_60,           /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
    VO_OUTPUT_1280x800_60,           /* 1280*800@60Hz VGA@60Hz*/
    
    VO_OUTPUT_USER,
    VO_OUTPUT_BUTT

} VO_INTF_SYNC_E;

typedef enum hiVO_DEV_E
{
    VOU_DEV_DHD0  = 0,                 /* high definition device */
    VOU_DEV_DHD1  = 1,                 /* assistant device */
    VOU_DEV_DSD0  = 2,                 /* spot device */
    VOU_DEV_DSD1  = 3,
    VOU_DEV_DSD2  = 4,
    VOU_DEV_DSD3  = 5,
    VOU_DEV_DSD4  = 6,
    VOU_DEV_DSD5  = 7,
    VOU_DEV_BUTT
} VO_DEV_E;

typedef enum hiVO_GRAPHIC_E
{
    VO_GRAPHC_G0    = 0,
    VO_GRAPHC_G1    = 1,
    VO_GRAPHC_G2    = 2,
    VO_GRAPHC_G3    = 3,
    //VO_GRAPHC_G4    = 4,
    VO_GRAPHC_BUTT
} VO_GRAPHIC_E;

typedef enum
{
    VOU_LAYER_VHD0  = 0,
    VOU_LAYER_VHD1  = 1,
    VOU_LAYER_VHD2  = 2,
    VOU_LAYER_VSD0  = 3,
    VOU_LAYER_VSD1  = 4,
    VOU_LAYER_VSD2  = 5,
    VOU_LAYER_VSD3  = 6,
    VOU_LAYER_VSD4  = 7,
    VOU_LAYER_G0    = 8,
    VOU_LAYER_G1    = 9,
    VOU_LAYER_G2    = 10,
    VOU_LAYER_G3    = 11,
    VOU_LAYER_G4    = 12,
    VOU_LAYER_HC0   = 13,
    VOU_LAYER_HC1   = 14,
    VOU_LAYER_WBC2  = 15,
    VOU_LAYER_BUTT
}VOU_LAYER_E;

#endif

