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


#ifndef	HINFCV300H
#define	HINFCV300H
/******************************************************************************/

#define	HINFC300_REG_BASE_ADDRESS					  (0x60010000)
#define	HINFC300_REG_BASE_ADDRESS_LEN				  (0x100)
#define	HINFC300_BUFFER_BASE_ADDRESS				  (0x24000000)
#define	HINFC300_BUFFER_BASE_ADDRESS_LEN			  (2048	+ 128)

#ifndef	CONFIG_HINFC300_MAX_CHIP
#  define CONFIG_HINFC300_MAX_CHIP					  (1)
#endif /* CONFIG_HINFC300_MAX_CHIP */

#define	HINFC300_CHIP_DELAY							  (25)

#define	HINFC300_ADDR_CYCLE_MASK					  0x4
#define	HINFC300_PERIPHERY_REGBASE					  (0x101f5000)

#define	PERI_CRG30									  (0x00B8)
#define	PERI_CRG30_CLK_EN					 (1U <<	8)	 
#define	PERI_CRG30_CLK_SEL_99M				 (1U <<	16)
/*****************************************************************************/
#define	HINFC300_CON								 0x00
#define	HINFC300_CON_OP_MODE_NORMAL		 (1U <<	0)
#define	HINFC300_CON_PAGEISZE_SHIFT		 (1)
#define	HINFC300_CON_PAGESIZE_MASK		 (0x03)
#define	HINFC300_CON_BUS_WIDTH			 (1U <<	3)
#define	HINFC300_CON_READY_BUSY_SEL		 (1U <<	7)
#define	HINFC300_CON_ECCTYPE_SHIFT		 (8)
#define	HINFC300_CON_ECCTYPE_MASK		 (0x07)

#define	HINFC300_PWIDTH								 0x04
#define	SET_HINFC300_PWIDTH(_w_lcnt, _r_lcnt, _rw_hcnt)	\
	((_w_lcnt) | (((_r_lcnt) & 0x0F) <<	4) | (((_rw_hcnt) &	0x0F) << 8))

#define	HINFC300_CMD								 0x0C
#define	HINFC300_ADDRL								 0x10
#define	HINFC300_ADDRH								 0x14

#define	HINFC300_OP									 0x1C
#define	HINFC300_OP_READ_STATUS_EN		 (1U <<	0)
#define	HINFC300_OP_READ_DATA_EN		 (1U <<	1)
#define	HINFC300_OP_WAIT_READY_EN		 (1U <<	2)
#define	HINFC300_OP_CMD2_EN				 (1U <<	3)
#define	HINFC300_OP_WRITE_DATA_EN		 (1U <<	4)
#define	HINFC300_OP_ADDR_EN				 (1U <<	5)
#define	HINFC300_OP_CMD1_EN				 (1U <<	6)
#define	HINFC300_OP_NF_CS_SHIFT			 (7)
#define	HINFC300_OP_NF_CS_MASK			 (3)
#define	HINFC300_OP_ADDR_CYCLE_SHIFT	 (9)
#define	HINFC300_OP_ADDR_CYCLE_MASK		 (7)

#define	HINFC300_STATUS								  0x20

#define	HINFC300_INTS								  0x28
#define	HINFC300_INTS_UE				 (1U <<	6)
#define	HINFC300_INTCLR								  0x2C
#define	HINFC300_INTCLR_UE				 (1U <<	6)

#define	HINFC300_DMA_CTRL							  0x60
#define	HINFC300_DMA_CTRL_DMA_START		 (1U <<	0)
#define	HINFC300_DMA_CTRL_WE			 (1U <<	1)
#define	HINFC300_DMA_CTRL_BURST4_EN		 (1U <<	4)
#define	HINFC300_DMA_CTRL_BURST8_EN		 (1U <<	5)
#define	HINFC300_DMA_CTRL_BURST16_EN	 (1U <<	6)
#define	HINFC300_DMA_CTRL_ADDR_NUM_SHIFT (7)
#define	HINFC300_DMA_CTRL_ADDR_NUM_MASK	 (1)
#define	HINFC300_DMA_CTRL_CS_SHIFT		 (8)
#define	HINFC300_DMA_CTRL_CS_MASK		 (0x03)

#define	HINFC300_DMA_ADDR_DATA						  0x64
#define	HINFC300_DMA_ADDR_OOB						  0x68

#define	HINFC300_DMA_LEN							  0x6C
#define	HINFC300_DMA_LEN_OOB_SHIFT		 (16)			   
#define	HINFC300_DMA_LEN_OOB_MASK		 (0x1FFF)

#define	HINFC300_DMA_PARA							  0x70
#define	HINFC300_DMA_PARA_DATA_RW_EN	 (1U <<	0)
#define	HINFC300_DMA_PARA_OOB_RW_EN		 (1U <<	1)
#define	HINFC300_DMA_PARA_DATA_EDC_EN	 (1U <<	2)
#define	HINFC300_DMA_PARA_OOB_EDC_EN	 (1U <<	3)
#define	HINFC300_DMA_PARA_DATA_ECC_EN	 (1U <<	4)
#define	HINFC300_DMA_PARA_OOB_ECC_EN	 (1U <<	5)
#define	HINFC300_DMA_PARA_EXT_LEN_SHIFT	 (6)
#define	HINFC300_DMA_PARA_EXT_LEN_MASK	 (0x03)
/*****************************************************************************/

#define	_512B								(512)
#define	_2K									(2048)
#define	_4K									(4096)
#define	_8K									(8192)

/*****************************************************************************/

enum ecc_type
{
	et_ecc_none	   = 0x00,
	et_ecc_1bit	   = 0x01,
	et_ecc_4bytes  = 0x02,
	et_ecc_8bytes  = 0x03,
	et_ecc_24bit1k = 0x04,
};
/*****************************************************************************/
enum page_type
{
	pt_pagesize_512	  =	0x00,
	pt_pagesize_2K	  =	0x01,
	pt_pagesize_4K	  =	0x02,
	pt_pagesize_8K	  =	0x03,
};
/*****************************************************************************/

struct page_page_ecc_info
{
	enum page_type pagetype;
	enum ecc_type  ecctype;
	unsigned int oobsize;
	struct nand_ecclayout *layout;
};
/*****************************************************************************/

struct hinfc_host 
{
	struct nand_chip *chip;
	struct mtd_info	 *mtd;
	void __iomem *iobase;
	void __iomem *sysreg;
	struct device *dev;

	unsigned int offset;
	unsigned int command;

	int	chipselect;

	unsigned int n24bit_ext_len;
	enum ecc_type ecctype;

	unsigned long NFC_CON;
	unsigned long NFC_CON_ECC_NONE;

	unsigned int addr_cycle;
	unsigned int addr_value[2];
	unsigned int column;

	unsigned int dma_oob;
	unsigned int dma_buffer;
	unsigned int pagesize;
	unsigned int oobsize;
	char *buffer;

	unsigned int uc_er;
};
/*****************************************************************************/

#define	hinfc_read(_host, _reg)	\
	readl((char	*)_host->iobase	+ (_reg))

#define	hinfc_write(_host, _value, _reg) \
	writel((_value), (char *)_host->iobase + (_reg))

/*****************************************************************************/

#define	DBG_BUG(fmt, args...) do{\
	printk("%s(%d):	!!!	BUG	" fmt, __FILE__, __LINE__, ##args);	\
	while(1); \
} while	(0)
/*****************************************************************************/

#define	GET_PAGE_INDEX(host) \
	((host->addr_value[0] >> 16) | (host->addr_value[1]	<< 16))

#if	!(defined(CONFIG_HINFC300_DBG_NAND_PROC_FILE) || defined(CONFIG_HINFC300_DBG_NAND_EC_NOTICE))
#  define dbg_nand_ec_notice(_p0)
#  define dbg_nand_ec_init()
#endif /* !(defined(CONFIG_HINFC300_DBG_NAND_PROC_FILE)	|| defined(CONFIG_HINFC300_DBG_NAND_EC_NOTICE))	*/

#if	!defined(CONFIG_HINFC300_DBG_NAND_PROC_FILE)
#  define dbg_nand_proc_save_logs(_p0, _p1)
#  define dbg_nand_proc_init()
#endif /* !defined(CONFIG_HINFC300_DBG_NAND_PROC_FILE) */

#if	!defined(CONFIG_HINFC300_DBG_NAND_STAT_PROC_FILE)
#  define dbg_nand_stat_proc_init(_p0, _p1)
#  define dbg_nand_stat_operation(_p0, _p1)
#endif /* CONFIG_HINFC301_DBG_NAND_STAT_PROC_FILE */
/******************************************************************************/
#endif /* HINFCV300H */
