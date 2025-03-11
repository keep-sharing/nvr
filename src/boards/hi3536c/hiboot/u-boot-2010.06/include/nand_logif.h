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


#ifndef NAND_LOGIFH
#define NAND_LOGIFH
/******************************************************************************/

#include <nand.h>

/*****************************************************************************/

typedef struct nand_logic_t
{
	unsigned long long address;
	unsigned long long length;
	unsigned long long erasesize; /* fixed for 64bit  */
	nand_info_t *nand;

} nand_logic_t;

/*****************************************************************************/

nand_logic_t *nand_logic_open(unsigned long long address, unsigned long long length);

void nand_logic_close(nand_logic_t *nand_logic);

int nand_logic_erase
(
 nand_logic_t *nand_logic,
 unsigned long long offset,   /* should be alignment with nand block size */
 unsigned long long length    /* should be alignment with nand block size */
 );

int nand_logic_write
(
 nand_logic_t *nand_logic,
 unsigned long long offset,  /* should be alignment with nand page size */
 unsigned int length,        /* should be alignment with nand page size */
 unsigned char *buf,
 int withoob
 );

int nand_logic_read
(
 nand_logic_t *nand_logic,
 unsigned long long offset,  /* should be alignment with nand page size */
 unsigned int length,        /* should be alignment with nand page size */
 unsigned char *buf,
 int withoob
 );

/******************************************************************************/
#endif /* NAND_LOGIFH */

