/*
* Copyright (c) 2009-2011 HiSilicon Technologies Co., Ltd.
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


#ifndef SPIFLASH_LOGIFH
#define SPIFLASH_LOGIFH
/******************************************************************************/

#include <spi_flash.h>

/*****************************************************************************/

typedef struct spiflash_logic_t
{
	unsigned long long address;
	unsigned long long length;
	unsigned long long erasesize;
	struct spi_flash *spiflash;
} spiflash_logic_t;

/*****************************************************************************/

spiflash_logic_t *spiflash_logic_open(unsigned long long address, unsigned long long length);

void spiflash_logic_close(spiflash_logic_t *spiflash_logic);

int spiflash_logic_erase
(
 spiflash_logic_t *spiflash_logic,
 unsigned long long offset,   /* should be alignment with spi flash block size */
 unsigned long long length    /* should be alignment with spi flash block size */
 );

int spiflash_logic_write
(
 spiflash_logic_t *spiflash_logic,
 unsigned long long offset,
 unsigned int length,
 unsigned char *buf
 );

int spiflash_logic_read
(
 spiflash_logic_t *spiflash_logic,
 unsigned long long offset,
 unsigned int length,
 unsigned char *buf
 );

/******************************************************************************/
#endif /* SPIFLASH_LOGIFH */

