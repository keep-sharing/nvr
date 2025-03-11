
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

/*********************************add include here******************************/
#include "jpeg_hdec_rwreg.h"
#include "hi_drv_jpeg_reg.h"

HI_VOID JPEG_HDEC_WriteReg(volatile HI_CHAR *pJpegRegVirAddr, HI_S32 s32PhyOff, HI_S32 s32Val)
{

		volatile HI_S32  *ps32Addr = NULL;

		/**
		** the phycial offset address can not larger than register length
		** CNcomment:  CNend\n
		**/
		if(s32PhyOff < JPGD_REG_LENGTH)
		{
		    ps32Addr  = (volatile HI_S32*)(pJpegRegVirAddr + s32PhyOff); /*lint !e826 ignore by y00181162, because this is needed */  
            *ps32Addr = s32Val;
		}
	  
}

HI_S32 JPEG_HDEC_ReadReg(const volatile HI_CHAR *pJpegRegVirAddr, HI_S32 s32PhyOff)
{

		return s32PhyOff < JPGD_REG_LENGTH ?  *(volatile int *)(pJpegRegVirAddr + s32PhyOff) : HI_FAILURE; /*lint !e826 ignore by y00181162, because this is needed */  

}

HI_VOID JPEG_HDEC_CpyData2Reg(volatile HI_CHAR *pJpegRegVirAddr,const HI_VOID *pInMem,HI_S32 s32PhyOff,HI_U32 u32Bytes)
{

      
		HI_U32 u32Cnt = 0;

		for(u32Cnt = 0; u32Cnt < u32Bytes; u32Cnt += 4)
		{
			*(volatile int *)(pJpegRegVirAddr + s32PhyOff + u32Cnt)=*(int *)((char*)pInMem + u32Cnt);/*lint !e826 ignore by y00181162, because this is needed */  
		}

}

HI_S32 JPEG_HDEC_CpyData2Buf(const volatile HI_CHAR *pJpegRegVirAddr,const HI_S32 s32PhyOff,const HI_U32 u32Bytes,HI_VOID *pOutMem)
{

		HI_U32 u32Cnt = 0;

		for(u32Cnt = 0; u32Cnt < u32Bytes; u32Cnt += 4)
		{ 
			*(volatile int *)((char*)pOutMem + u32Cnt)=*(int *)(pJpegRegVirAddr + s32PhyOff + u32Cnt);/*lint !e826 ignore by y00181162, because this is needed */  
		}
		return HI_SUCCESS;
}
