/*
* Copyright (c) 2009-2010 HiSilicon Technologies Co., Ltd.
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


#include <common.h>
#include <linux/mtd/mtd.h>
#include "../spi_ids.h"

/*****************************************************************************/

#define SET_READ_STD(_dummy_, _size_, _clk_) \
	static struct spi_operation read_std_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_STD, SPI_CMD_READ, _dummy_, _size_, _clk_ }

#define READ_STD(_dummy_, _size_, _clk_) read_std_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_READ_FAST(_dummy_, _size_, _clk_) \
	static struct spi_operation read_fast_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_FAST, SPI_CMD_FAST_READ, _dummy_, _size_, _clk_ }

#define READ_FAST(_dummy_, _size_, _clk_) read_fast_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_READ_DUAL(_dummy_, _size_, _clk_) \
	static struct spi_operation read_dual_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_DUAL, SPI_CMD_READ_DUAL, _dummy_, _size_, _clk_ }

#define READ_DUAL(_dummy_, _size_, _clk_) read_dual_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_READ_QUAD(_dummy_, _size_, _clk_) \
	static struct spi_operation read_quad_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD, SPI_CMD_READ_QUAD, _dummy_, _size_, _clk_ }

#define READ_QUAD(_dummy_, _size_, _clk_) read_quad_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_READ_DUAL_ADDR(_dummy_, _size_, _clk_) \
	static struct spi_operation \
		read_dual_addr_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_DUAL_ADDR, SPI_CMD_READ_DUAL_ADDR, _dummy_, _size_, _clk_ }

#define READ_DUAL_ADDR(_dummy_, _size_, _clk_) \
	read_dual_addr_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_READ_QUAD_ADDR(_dummy_, _size_, _clk_) \
	static struct \
		spi_operation read_quad_addr_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD_ADDR, SPI_CMD_READ_QUAD_ADDR, _dummy_, _size_, _clk_ }

#define READ_QUAD_ADDR(_dummy_, _size_, _clk_) \
	read_quad_addr_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_WRITE_STD(_dummy_, _size_, _clk_) \
	static struct spi_operation write_std_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_STD, SPI_CMD_PP, _dummy_, _size_, _clk_ }

#define WRITE_STD(_dummy_, _size_, _clk_) write_std_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_WRITE_DUAL(_dummy_, _size_, _clk_) \
	static struct spi_operation write_dual_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_DUAL, SPI_CMD_WRITE_DUAL, _dummy_, _size_, _clk_ }

#define WRITE_DUAL(_dummy_, _size_, _clk_) write_dual_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_WRITE_QUAD(_dummy_, _size_, _clk_) \
	static struct spi_operation write_quad_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_QUAD, SPI_CMD_WRITE_QUAD, _dummy_, _size_, _clk_ }

#define WRITE_QUAD(_dummy_, _size_, _clk_) \
	write_quad_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_WRITE_DUAL_ADDR(_dummy_, _size_, _clk_) \
	static struct spi_operation \
	write_dual_addr_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_DUAL_ADDR, \
	SPI_CMD_WRITE_DUAL_ADDR, _dummy_, _size_, _clk_ }

#define WRITE_DUAL_ADDR(_dummy_, _size_, _clk_) \
	write_dual_addr_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_WRITE_QUAD_ADDR(_dummy_, _size_, _clk_) \
	static struct spi_operation \
	write_quad_addr_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_QUAD_ADDR, \
	SPI_CMD_WRITE_QUAD_ADDR, _dummy_, _size_, _clk_ }

#define WRITE_QUAD_ADDR(_dummy_, _size_, _clk_) \
	write_quad_addr_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_ERASE_SECTOR_4K(_dummy_, _size_, _clk_) \
	static struct spi_operation \
		erase_sector_4k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_4K, SPI_CMD_SE_4K, _dummy_, _size_, _clk_ }

#define ERASE_SECTOR_4K(_dummy_, _size_, _clk_) \
	erase_sector_4k_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_ERASE_SECTOR_32K(_dummy_, _size_, _clk_) \
	static struct spi_operation \
		erase_sector_32k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_32K, SPI_CMD_SE_32K, _dummy_, _size_, _clk_ }

#define ERASE_SECTOR_32K(_dummy_, _size_, _clk_) \
	erase_sector_32k_##_dummy_##_size_##_clk_
/*****************************************************************************/

#define SET_ERASE_SECTOR_64K(_dummy_, _size_, _clk_) \
	static struct spi_operation \
		erase_sector_64k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_64K, SPI_CMD_SE_64K, _dummy_, _size_, _clk_ }

#define ERASE_SECTOR_64K(_dummy_, _size_, _clk_) \
	erase_sector_64k_##_dummy_##_size_##_clk_

/*****************************************************************************/

#define SET_ERASE_SECTOR_256K(_dummy_, _size_, _clk_) \
	static struct spi_operation \
		erase_sector_256k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_256K, SPI_CMD_SE_256K, _dummy_, _size_, _clk_ }

#define ERASE_SECTOR_256K(_dummy_, _size_, _clk_) \
	erase_sector_256k_##_dummy_##_size_##_clk_
/*****************************************************************************/
SET_READ_STD(0, INFINITE, 0);
SET_READ_STD(0, INFINITE, 20);
SET_READ_STD(0, INFINITE, 32);
SET_READ_STD(0, INFINITE, 33);
SET_READ_STD(0, INFINITE, 40);
SET_READ_STD(0, INFINITE, 50);
SET_READ_STD(0, INFINITE, 54);
SET_READ_STD(0, INFINITE, 66);

SET_READ_FAST(1, INFINITE, 50);
SET_READ_FAST(1, INFINITE, 64);
SET_READ_FAST(1, INFINITE, 66);
SET_READ_FAST(1, INFINITE, 75);
SET_READ_FAST(1, INFINITE, 80);
SET_READ_FAST(1, INFINITE, 86);
SET_READ_FAST(1, INFINITE, 104);
SET_READ_FAST(1, INFINITE, 108);

SET_READ_DUAL(1, INFINITE, 64);
SET_READ_DUAL(1, INFINITE, 75);
SET_READ_DUAL(1, INFINITE, 80);
SET_READ_DUAL(1, INFINITE, 104);
SET_READ_DUAL(1, INFINITE, 108);

SET_READ_QUAD(1, INFINITE, 64);
SET_READ_QUAD(1, INFINITE, 80);
SET_READ_QUAD(1, INFINITE, 108);

SET_READ_DUAL_ADDR(1, INFINITE, 56);
SET_READ_DUAL_ADDR(2, INFINITE, 64);
SET_READ_DUAL_ADDR(0, INFINITE, 80);
SET_READ_DUAL_ADDR(1, INFINITE, 80);
SET_READ_DUAL_ADDR(2, INFINITE, 108);

SET_READ_QUAD_ADDR(3, INFINITE, 64);
SET_READ_QUAD_ADDR(3, INFINITE, 80);
SET_READ_QUAD_ADDR(5, INFINITE, 108);
/*****************************************************************************/
SET_WRITE_STD(0, 256, 0);
SET_WRITE_STD(0, 256, 50);
SET_WRITE_STD(0, 256, 64);
SET_WRITE_STD(0, 256, 66);
SET_WRITE_STD(0, 256, 75);
SET_WRITE_STD(0, 256, 80);
SET_WRITE_STD(0, 256, 86);
SET_WRITE_STD(0, 256, 100);
SET_WRITE_STD(0, 256, 104);
SET_WRITE_STD(0, 256, 108);

SET_WRITE_DUAL(0, 256, 64);
SET_WRITE_DUAL(0, 256, 75);
SET_WRITE_DUAL(0, 256, 108);

SET_WRITE_QUAD(0, 256, 80);
SET_WRITE_QUAD(0, 256, 108);

SET_WRITE_DUAL_ADDR(0, 256, 64);
SET_WRITE_DUAL_ADDR(0, 256, 108);

SET_WRITE_QUAD_ADDR(0, 256, 108);
/*****************************************************************************/

SET_ERASE_SECTOR_32K(0, _32K, 0);

SET_ERASE_SECTOR_64K(0, _64K, 0);
SET_ERASE_SECTOR_64K(0, _64K, 50);
SET_ERASE_SECTOR_64K(0, _64K, 64);
SET_ERASE_SECTOR_64K(0, _64K, 66);
SET_ERASE_SECTOR_64K(0, _64K, 75);
SET_ERASE_SECTOR_64K(0, _64K, 80);
SET_ERASE_SECTOR_64K(0, _64K, 86);
SET_ERASE_SECTOR_64K(0, _64K, 100);
SET_ERASE_SECTOR_64K(0, _64K, 104);
SET_ERASE_SECTOR_64K(0, _64K, 108);

SET_ERASE_SECTOR_256K(0, _256K, 50);
SET_ERASE_SECTOR_256K(0, _256K, 104);
/*****************************************************************************/

#include "hisfc300new_spi_general.c"
static struct spi_driver  spi_driver_general = {
	.wait_ready   = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr  = spi_general_entry_4addr,
	.bus_prepare  = spi_general_bus_prepare,
	.qe_enable = spi_general_qe_enable,
};

#include "hisfc300new_spi_s25fl256s.c"
static struct spi_driver  spi_driver_s25fl256s = {
	.wait_ready   = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr  = spi_s25fl256s_entry_4addr,
	.bus_prepare  = spi_general_bus_prepare,
	.qe_enable = spi_general_qe_enable,
};

#include "hisfc300new_spi_n25q256a.c"
static struct spi_driver  spi_driver_n25q256a = {
	.wait_ready   = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr  = spi_n25q256a_entry_4addr,
	.bus_prepare  = spi_general_bus_prepare,
	.qe_enable = spi_n25q256a_qe_enable,
};

#include "hisfc300new_spi_gd25qxxx.c"
static struct spi_driver  spi_driver_gd25qxxx = {
	.wait_ready   = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr  = spi_general_entry_4addr,
	.bus_prepare  = spi_general_bus_prepare,
	.qe_enable = spi_gd25qxxx_qe_enable,
};
/*****************************************************************************/
struct spi_info hisfc300new_spi_info_table[] = {
	/* name        id                id_len chipsize(Bytes) erasesize */
	{
		"at25fs010",  {0x1f, 0x66, 0x01}, 3,  _128K,  _32K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_32K(0, _32K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"at25fs040",  {0x1f, 0x66, 0x04}, 3,  _512K,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"at25df041a", {0x1f, 0x44, 0x01}, 3,  _512K,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"at25df641",  {0x1f, 0x48, 0x00}, 3,  _8M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"at26f004",   {0x1f, 0x04, 0x00}, 3,  _512K,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"at26df081a", {0x1f, 0x45, 0x01}, 3,  _1M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"at26df161a", {0x1f, 0x46, 0x01}, 3,  _2M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"at26df321",  {0x1f, 0x47, 0x01}, 3,  _4M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	/* Macronix */
	{
		"mx25l4005a",  {0xc2, 0x20, 0x13}, 3, _512K,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"MX25L8006E",  {0xc2, 0x20, 0x14}, 3, _1M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 86),
			&READ_DUAL(1, INFINITE, 80),
			0
		},
		{
			&WRITE_STD(0, 256, 86),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 86),
			0
		},
		&spi_driver_general,
	},

	{
		"MX25L1606E",  {0xc2, 0x20, 0x15}, 3, _2M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 86),
			&READ_DUAL(1, INFINITE, 80),
			0
		},
		{
			&WRITE_STD(0, 256, 86),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 86),
			0
		},
		&spi_driver_general,
	},

	{
		"mx25l3205d",  {0xc2, 0x20, 0x16}, 3, _4M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"MX25L6406E",  {0xc2, 0x20, 0x17}, 3, _8M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 86),
			&READ_DUAL(1, INFINITE, 80),
			0
		},

		{
			&WRITE_STD(0, 256, 86),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 86),
			0
		},
		&spi_driver_general,
	},

	{
		"MX25L128XX", {0xc2, 0x20, 0x18}, 3, _16M, _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 108),
			0
		},

		{
			&WRITE_STD(0, 256, 108),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 108),
			0
		},
		&spi_driver_general,
	},
	/*
	 The follow chips have the same chipid, but command have some difference

	{"MX25L12836E", {0xc2, 0x20, 0x18}, 3, _16M,   _64K, 3,
		{&READ_STD(0, INFINITE, 50), &READ_FAST(1, INFINITE, 108), 0},
		{&WRITE_STD(0, 256, 108), 0},
		{&ERASE_SECTOR_64K(0, _64K, 108), 0}},

	{"MX25L12845E", {0xc2, 0x20, 0x18}, 3, _16M,   _64K, 3,
		{&READ_STD(0, INFINITE, 50), &READ_FAST(1, INFINITE, 108), 0},
		{&WRITE_STD(0, 256, 108), 0},
		{&ERASE_SECTOR_64K(0, _64K, 108), 0}},
	*/

	{
		"MX25L25635E/735E",
		{0xc2, 0x20, 0x19}, 3, _32M, _64K, 4,
		{
			&READ_STD(0, INFINITE, 40/*50*/),
			&READ_FAST(1, INFINITE, 64/*80*/),
			&READ_DUAL_ADDR(1, INFINITE, 56/*70*/),
			/*&READ_QUAD_ADDR(3, INFINITE, 75),*/
			0
		},

		{
			&WRITE_STD(0, 256,/* 64*/75/*80*/),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, /*64*/80),
			0
		},
		&spi_driver_general,
	},

	{
		"mx25l1655d",  {0xc2, 0x26, 0x15}, 3, _2M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"mx25l12855e", {0xc2, 0x26, 0x18}, 3, _16M,   _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"s25sl004a", {0x01, 0x02, 0x12}, 3, (_64K * 8),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"s25sl008a", {0x01, 0x02, 0x13}, 3, (_64K * 16),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"s25sl016a", {0x01, 0x02, 0x14}, 3, (_64K * 32),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"S25FL064P", {0x01, 0x02, 0x16, 0x4d}, 4, (_64K * 128), _64K, 3,
		{
			&READ_STD(0, INFINITE, 40),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 80),
			/*&READ_QUAD(1, INFINITE, 80),*/
			0
		},

		{
			&WRITE_STD(0, 256, 104),
			/*&WRITE_QUAD(0, 256, 80),*/
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		},
		&spi_driver_general,
	},

	{
		"s25sl064a", {0x01, 0x02, 0x16}, 3, (_64K * 128), _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},
	/* Spansion */

	{
		"S25FL032P", {0x01, 0x02, 0x15, 0x4d}, 4, (_64K * 64),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 40),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			/*&READ_QUAD(1, INFINITE, 80),*/
			/*&READ_QUAD_ADDR(2, INFINITE, 64),*/
			0
		},

		{
			&WRITE_STD(0, 256, 104),
			/*&WRITE_QUAD(0, 256, 80),*/
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		},
		&spi_driver_general,
	},

	{
		"S25FL032A", {0x01, 0x02, 0x15}, 3, (_64K * 64),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 50),
			0
		},

		{
			&WRITE_STD(0, 256, 50),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 50),
			0
		},
		&spi_driver_general,
	},

	{
		"S25FL128P-0",
		{0x01, 0x20, 0x18, 0x03, 0x00}, 5, (_256K * 64),  _256K, 3,
		{
			&READ_STD(0, INFINITE, 40),
			&READ_FAST(1, INFINITE, 104),
			0
		},

		{
			&WRITE_STD(0, 256, 104),
			0
		},

		{
			&ERASE_SECTOR_256K(0, _256K, 104),
			0
		},
		&spi_driver_general,
	},

	{
		"S25FL128P-1",
		{0x01, 0x20, 0x18, 0x03, 0x01}, 5, (_64K * 256),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 40),
			&READ_FAST(1, INFINITE, 104),
			0
		},

		{
			&WRITE_STD(0, 256, 104), 0},

		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		},
		&spi_driver_general,
	},

	{
		"S25FL129P0",
		{0x01, 0x20, 0x18, 0x4d, 0x00}, 5, (_256K * 64),  _256K, 3,
		{
			&READ_STD(0, INFINITE, 40),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(0, INFINITE, 80),
			/*&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(2, INFINITE, 80),*/
			0
		},

		{
			&WRITE_STD(0, 256, 104),
			/*&WRITE_QUAD(0, 256, 80),*/
			0
		},

		{
			&ERASE_SECTOR_256K(0, _256K, 104),
			0
		},
		&spi_driver_general,
	},

	{
		"S25FL129P1",
		{0x01, 0x20, 0x18, 0x4d, 0x01}, 5, (_64K * 256),  _64K,  3,
		{
			&READ_STD(0, INFINITE, 40),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 64),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
			0
		},

		{
			&WRITE_STD(0, 256, 104),
			&WRITE_QUAD(0, 256, 80),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		},
		&spi_driver_general,
	},

	{
		"S25FL256S", {0x01, 0x02, 0x19, 0x4d, 0x01}, 5, _32M,  _64K,  4,
		{
			&READ_STD(0, INFINITE, 40),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 64),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 64),
			&READ_QUAD_ADDR(3, INFINITE, 64),
			0
		},

		{
			&WRITE_STD(0, 256, 104),
			&WRITE_QUAD(0, 256, 80),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		},
		&spi_driver_s25fl256s,
	},

	/*
	The chip and chip W25Q16B have the same chipid,
	but clock frequency have some difference

	{"S25FL016K", {0xef, 0x40, 0x15}, 3, (_64K * 32),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 104),
			&READ_QUAD(1, INFINITE, 104),
			0
		},
		{
			&WRITE_STD(0, 256, 104),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		}
	},
	*/

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{
		"sst25vf040b", {0xbf, 0x25, 0x8d}, 3, (_64K * 8),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"sst25vf080b", {0xbf, 0x25, 0x8e}, 3, (_64K * 16), _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"sst25vf016b", {0xbf, 0x25, 0x41}, 3, (_64K * 32), _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"sst25vf032b", {0xbf, 0x25, 0x4a}, 3, (_64K * 64), _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"sst25wf512",  {0xbf, 0x25, 0x01}, 3, (_64K * 1),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"sst25wf010",  {0xbf, 0x25, 0x02}, 3, (_64K * 2),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"sst25wf020",  {0xbf, 0x25, 0x03}, 3, (_64K * 4),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"sst25wf040",  {0xbf, 0x25, 0x04}, 3, (_64K * 8),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	/* ST Microelectronics -- newer production may have feature updates */
	{
		"m25p05",  {0x20, 0x20, 0x10}, 3, (_32K * 2), _32K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_32K(0, _32K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m25p10",  {0x20, 0x20, 0x11}, 3, (_32K * 4), _32K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_32K(0, _32K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m25p20",  {0x20, 0x20, 0x12}, 3, (_64K * 4),   _64K,  3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m25p40",  {0x20, 0x20, 0x13}, 3, (_64K * 8),   _64K,  3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m25p80",  {0x20, 0x20, 0x14}, 3, (_64K * 16),  _64K,  3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m25p16",  {0x20, 0x20, 0x15}, 3, (_64K * 32),  _64K,  3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"M25P32",  {0x20, 0x20, 0x16, 0x10}, 4, _4M, _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 75),
			0
		},

		{
			&WRITE_STD(0, 256, 75),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 75),
			0
		},
		&spi_driver_general,
	},

	{
		"m25p64",  {0x20, 0x20, 0x17}, 3, (_64K * 128), _64K,  3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"M25P128", {0x20, 0x20, 0x18}, 3, _16M, _256K, 3,
		{
			&READ_STD(0, INFINITE, 20),
			&READ_FAST(1, INFINITE, 50),
			0
		},

		{
			&WRITE_STD(0, 256, 50),
			0
		},

		{
			&ERASE_SECTOR_256K(0, _256K, 50),
			0
		},
		&spi_driver_general,
	},

	{
		"m45pe10", {0x20, 0x40, 0x11}, 3, (_64K * 2),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m45pe80", {0x20, 0x40, 0x14}, 3, (_64K * 16),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m45pe16", {0x20, 0x40, 0x15}, 3, (_64K * 32),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m25pe80", {0x20, 0x80, 0x14}, 3, (_64K * 16), _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"m25pe16", {0x20, 0x80, 0x15}, 3, (_64K * 32), _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"N25Q032", {0x20, 0xba, 0x16}, 3, (_64K * 64), _64K, 3,
		{
			&READ_STD(0, INFINITE, 32/*54*/),
			&READ_FAST(1, INFINITE, 64/*108*/),
			&READ_DUAL(1, INFINITE, 64/*108*/),
			&READ_DUAL_ADDR(2, INFINITE, 64/*108*/),
			/*&READ_QUAD(1, INFINITE, 64),
			&READ_QUAD_ADDR(5, INFINITE, 64),*/
			0
		},

		{
			&WRITE_STD(0, 256, 64/*108*/),
			&WRITE_DUAL(0, 256, 64/*108*/),
			&WRITE_DUAL_ADDR(0, 256, 64/*108*/),
			/*&WRITE_QUAD(0, 256, 64),
			&WRITE_QUAD_ADDR(0, 256, 64),*/
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 64/*108*/),
			0
		},
		&spi_driver_general,
	},

	{
		"N25Q128A",   {0x20, 0xba, 0x18}, 3, (_64K * 256), _64K, 3,
		{
			&READ_STD(0, INFINITE, 54),
			&READ_FAST(1, INFINITE, 108),
			&READ_DUAL(1, INFINITE, 108),
			&READ_QUAD(1, INFINITE, 108),
			&READ_DUAL_ADDR(2, INFINITE, 108),
			&READ_QUAD_ADDR(5, INFINITE, 108),
			0
		},

		{
			&WRITE_STD(0, 256, 108),
			&WRITE_DUAL(0, 256, 108),
			&WRITE_QUAD(0, 256, 108),
			&WRITE_DUAL_ADDR(0, 256, 108),
			&WRITE_QUAD_ADDR(0, 256, 108),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 108),
			0
		},
		&spi_driver_general,
	},

	{
		"N25Q256A",   {0x20, 0xba, 0x19, 0x10}, 3, (_32M), _64K, 4,
		{
			&READ_STD(0, INFINITE, 54),
			&READ_FAST(1, INFINITE, 108),
			&READ_DUAL(1, INFINITE, 108),
			&READ_QUAD(1, INFINITE, 108),
			&READ_DUAL_ADDR(2, INFINITE, 108),
			&READ_QUAD_ADDR(5, INFINITE, 108),
			0
		},

		{
			&WRITE_STD(0, 256, 108),
			&WRITE_DUAL(0, 256, 108),
			&WRITE_QUAD(0, 256, 108),
			&WRITE_DUAL_ADDR(0, 256, 108),
			&WRITE_QUAD_ADDR(0, 256, 108),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 64/*108*/),
			0
		},
		&spi_driver_n25q256a,
	},

	{
		"M25PX16",  {0x20, 0x71, 0x15}, 3, (_64K * 32),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			0
		},

		{
			&WRITE_STD(0, 256, 75),
			&WRITE_DUAL(0, 256, 75),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 75),
			0
		},
		&spi_driver_general,
	},

	{
		"M25PX32", {0x20, 0x71, 0x16}, 3, (_64K * 64),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			0
		},

		{
			&WRITE_STD(0, 256, 75),
			&WRITE_DUAL(0, 256, 75),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 75),
			0
		},
		&spi_driver_general,
	},

	{
		"m25px64",  {0x20, 0x71, 0x17}, 3, (_64K * 128), _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{
		"w25x10",  {0xef, 0x30, 0x11}, 3, (_64K * 2),    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"w25x20",  {0xef, 0x30, 0x12}, 3, (_64K * 4),    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

			{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"w25x40",  {0xef, 0x30, 0x13}, 3, (_64K * 8),    _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"w25x80",  {0xef, 0x30, 0x14}, 3, (_64K * 16),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

			{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"w25x16",  {0xef, 0x30, 0x15}, 3, (_64K * 32),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},

		{
			&WRITE_STD(0, 256, 0),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"w25x32",  {0xef, 0x30, 0x16}, 3, (_64K * 64),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 0), 0},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"w25x64",  {0xef, 0x30, 0x17}, 3, (_64K * 128),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 0),
			0
		},
		{
			&WRITE_STD(0, 256, 0),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 0),
			0
		},
		&spi_driver_general,
	},

	{
		"W25Q80BV",  {0xef, 0x40, 0x14}, 3, (_64K * 16),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			/*&READ_QUAD(1, INFINITE, 80),*/
			0
		},

		{
			&WRITE_STD(0, 256, 80),
			/*&WRITE_QUAD(0, 256, 80),*/
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_general,
	},

	{
		"W25Q16(B/C)V/S25FL016K",
		{0xef, 0x40, 0x15}, 3, (_64K * 32), _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			/*&READ_QUAD(1, INFINITE, 80),*/
			0
		},

		{
			&WRITE_STD(0, 256, 80),
			/*&WRITE_QUAD(0, 256, 80),*/
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_general,
	},
	/*
	 The follow chips have the same chipid, but command have some difference
	{
		"W25Q16BV",  {0xef, 0x40, 0x15}, 3, (_64K * 32),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 80),
			0
		},
		{
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 80),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		}
	},

	{
		"W25Q16CV",  {0xef, 0x40, 0x15}, 3, (_64K * 32),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 80),
			0
		},
		{
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 80),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		}
	},

	*/
	{
		"W25Q32(B/F)V",  {0xef, 0x40, 0x16}, 3, (_64K * 64),   _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			/*&READ_QUAD(1, INFINITE, 80),*/
			0
		},

		{
			&WRITE_STD(0, 256, 80),
			/*&WRITE_QUAD(0, 256, 80),*/
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_general,
	},

		{
		"W25Q64FV",  {0xef, 0x40, 0x17}, 3, _8M,   _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 80),
			0
		},

		{
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 80),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_general,
	},

	{
		"W25Q128B", {0xEF, 0x40, 0x18}, 3, _16M, _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 104),
			&READ_QUAD(1, INFINITE, /*70*/80),
			0
		},

		{
			&WRITE_STD(0, 256, 104),
			&WRITE_QUAD(0, 256, /*70*/80),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		},
		&spi_driver_general,
	},

	/* Eon -- fit clock frequency of RDSR instruction*/
	{
		"EN25F80", {0x1c, 0x31, 0x14}, 3, (_64K * 16),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 66),
			&READ_FAST(1, INFINITE, 66/*100*/),
			0
		},

		{
			&WRITE_STD(0, 256, 66/*100*/),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 66/*100*/),
			0
		},
		&spi_driver_general,
	},

	{
		"EN25F16", {0x1c, 0x31, 0x15}, 3, _2M,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 66),
			&READ_FAST(1, INFINITE, 66/*100*/),
			0
		},

		{
			&WRITE_STD(0, 256, 66/*100*/),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 66/*100*/),
			0
		},
		&spi_driver_general,
	},

	{
		"EN25Q32B", {0x1c, 0x30, 0x16}, 3, (_64K * 64),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80/*104*/),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			/*&READ_QUAD(3, INFINITE, 80), */
			0
		},

		{
			&WRITE_STD(0, 256, 80/*104*/),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 80/*104*/),
			0
		},
		&spi_driver_general,
	},

/*need to add dual rw*/
	{
		"EN25Q64", {0x1c, 0x30, 0x17}, 3, (_64K * 128),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80/*104*/),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			/*&READ_QUAD(3, INFINITE, 80), */
			0
		},

		{
			&WRITE_STD(0, 256, 80/*104*/),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 80/*104*/),
			0
		},
		&spi_driver_general,
	},

	{
		"EN25Q128", {0x1c, 0x30, 0x18}, 3, (_64K * 256),  _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80/*104*/),
			&READ_DUAL(1, INFINITE, 64/*80*/),
			&READ_DUAL_ADDR(1, INFINITE,/* 64*/80),
			/*&READ_QUAD(3, INFINITE, 80), */
			0
		},

		{
			&WRITE_STD(0, 256, 80/*104*/),
			0
		},

		{
			&ERASE_SECTOR_64K(0, _64K, 80/*104*/),
			0
		},
		&spi_driver_general,
	},

	{
		"GD25Q128", {0xC8, 0x40, 0x18}, 3, _16M,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 66),
			&READ_QUAD(1, INFINITE, 80),
			0
		},
		{
			&WRITE_STD(0, 256, 100),
			&WRITE_QUAD(0, 256, 80),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 100),
			0
		},
		&spi_driver_gd25qxxx,
	},
	{0, {0}, 0, 0, 0, 0, {0}, {0}, {0}, NULL},
};
