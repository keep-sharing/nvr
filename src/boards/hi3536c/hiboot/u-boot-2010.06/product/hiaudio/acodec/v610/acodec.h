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

#ifndef _ACODEC_H_
#define _ACODEC_H_

typedef enum hiACODEC_FS_E {
    ACODEC_FS_48000 =   0x1a,
    ACODEC_FS_24000 =   0x19,
    ACODEC_FS_12000 =   0x18,

    ACODEC_FS_44100 =   0x1a,
    ACODEC_FS_22050 =   0x19,
    ACODEC_FS_11025 =   0x18,

    ACODEC_FS_32000 =   0x1a,
    ACODEC_FS_16000 =   0x19,
    ACODEC_FS_8000  =   0x18,

    ACODEC_FS_BUTT = 0x1b,
} ACODEC_FS_E;

#endif /* End of #ifndef _ACODEC_H_ */
