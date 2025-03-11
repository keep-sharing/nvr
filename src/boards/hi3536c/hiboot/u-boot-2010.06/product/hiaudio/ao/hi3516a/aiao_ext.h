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


#ifndef __AIAO_EXT_H__
#define __AIAO_EXT_H__

typedef struct hiAIO_DRV_DEV_CTX_S
{
    struct
    {
        HI_U32          u32BufSize;         /*Ñ­»·bufµÄ´óÐ¡*/
        HI_U32          u32PhyAddr;         /*Ñ­»·bufÎïÀíÆðÊ¼µØÖ·*/
        HI_U8*          pu8VirAddr;         /*Ñ­»·bufÐéÄâÆðÊ¼µØÖ·*/

        HI_U32          u32RptrOffSet;         /*Ñ­»·buf¶ÁÖ¸Õë£¬¼ÇÂ¼¶ÁÖ¸ÕëÏà¶ÔÓÚÆðÊ¼µØÖ·µÄÆ«ÒÆÁ¿*/
        HI_U32          u32WptrOffSet;         /*Ñ­»·bufÐ´Ö¸Õë£¬¼ÇÂ¼Ð´Ö¸ÕëÏà¶ÔÓÚÆðÊ¼µØÖ·µÄÆ«ÒÆÁ¿*/

    } stCirBuf;

    AIO_ATTR_S          stAioAttr;
    HI_BOOL             bEnable;

    HI_U64              u64LastPts;         /*ÉÏÖ¡Ê±¼ä´Á*/
    HI_U32              u32FrmTime;         /*Ö¡Ê±¼ä¼ä¸ô*/
    HI_U32              u32MaxFrmTime;      /*Ö¡Ö®¼ä×î´óÊ±¼ä¼ä¸ô*/
    HI_U32              u32IsrTime;         /*ÖÐ¶Ï´¦ÀíÊ±¼ä*/
    HI_U32              u32MaxIsrTime;      /*ÖÐ¶Ï´¦ÀíÀúÊ·×î³¤Ê±¼ä*/
    HI_U32              u32AioFifoLen;      /*AIO FIFO ¿í¶È,Ò»´ÎDMA´«ÊäµÄÒôÆµÊý¾Ý£¬u32AioFifoLen¸ö×Ö½Ú£¬×îÐ¡Îª8£¬×î´óÎª32*/
    HI_U32              u32FifoLenBase;     /*AIO FIFO ¿í¶ÈµÄ2µÄÃÝ¼¶£¬ÀýÈç8ÊÇ2µÄÈý´ÎÃÝ(ÓÃÓÚÍ¨µÀ·ÖÀëºÏ²¢Ëã·¨µÄÒÆÎ»²Ù×÷)*/
    HI_U32              u32FifoShift;       /*AIO FIFOÖÐÊý¾ÝÒÆÎ»(±ê×¼PCMÐèÒªÒÆÎ»1Î»)*/
    HI_U32              u32TransLen;        /*×ÜµÄdma´«Êä³¤¶È,byteÎªµ¥Î»(²ÉÑùµã¸öÊý*u32AioFifoLen) */

    HI_S32              as32ChnIndex[AIO_MAX_CHN_NUM];

    HI_U32              u32IntCnt;
    HI_U32              u32fifoIntCnt;          /*ÖÐ¶ÏÊýÄ¿£¬µ÷ÊÔÓÃ*/
    HI_U32              u32buffIntCnt;          /*ÖÐ¶ÏÊýÄ¿£¬µ÷ÊÔÓÃ*/
    AUDIO_TRACK_MODE_E  enTrackMode;
    HI_BOOL             bMute;
    AUDIO_FADE_S        stFade;
    HI_S32              s32Volume;
	HI_BOOL             bMicInl;
	HI_BOOL             bMicInr;
}AIO_DRV_DEV_CTX_S;

#endif
