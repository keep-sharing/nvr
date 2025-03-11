/*
 *  spinand driver
 *
 *  Author:	Howard Chang
 *  Created:	Aug 2, 2018
 *  Copyright:	Novatek Inc.
 *
 */
#include "config.h"
#include <common.h>
#include <malloc.h>
#include <nand.h>
#include "../nvt_flash_spi/nvt_flash_spi020_int.h"
#include "../nvt_flash_spi/nvt_flash_spi020_reg.h"

#define NAND_VERSION "1.0.5"

#ifdef CONFIG_CMD_NAND
typedef struct nand_2plane_id {
	char    *name;
	int     mfr_id;
	int     dev_id;
} NAND_2PLANE_ID;

static NAND_2PLANE_ID nand_2plane_list[] = {
	// flash name / Manufacturer ID / Device ID
	{ "MXIC_MX35LF2G14AC",		0xC2,	0x20	},
	{ "MICRON_MT29F2G01AB",		0x2C,	0x24	}
};
#define NUM_OF_2PLANE_ID (sizeof(nand_2plane_list) / sizeof(NAND_2PLANE_ID))

/* error code and state */
enum {
	ERR_NONE	= 0,
	ERR_DMABUSERR	= -1,
	ERR_SENDCMD	= -2,
	ERR_DBERR	= -3,
	ERR_BBERR	= -4,
	ERR_ECC_FAIL	= -5,
	ERR_ECC_UNCLEAN = -6,
};

enum {
	STATE_READY = 0,
	STATE_CMD_HANDLE,
	STATE_DMA_READING,
	STATE_DMA_WRITING,
	STATE_DMA_DONE,
	STATE_PIO_READING,
	STATE_PIO_WRITING,
};

static struct nand_ecclayout hw_smallpage_ecclayout = {
	.eccbytes = 12,
	.eccpos = {0, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	.oobfree = { {1, 3}, {5, 1} }
};

static struct nand_ecclayout spinand_oob_64 = {
	.eccbytes = 32,
	.eccpos = {
		8, 9, 10, 11, 12, 13, 14, 15,
		24, 25, 26, 27, 28, 29, 30, 31,
		40, 41, 42, 43, 44, 45, 46, 47,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobavail = 12,
	.oobfree = {
		{.offset = 16,
			.length = 4},
		{.offset = 32,
			.length = 4},
		{.offset = 48,
			.length = 4},
	}
};

static void drv_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{

}

static void nvt_parse_parameter(struct drv_nand_dev_info *info)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};

	sprintf(path,"/nand@%x",IOADDR_NAND_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "chip-select", NULL);
	if (cell > 0)
		info->flash_info->chip_sel = __be32_to_cpu(cell[0]);
	else
		info->flash_info->chip_sel = 0;
}

static void nvt_parse_frequency(u32 *freq)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};

	sprintf(path,"/nand@%x",IOADDR_NAND_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "clock-frequency", NULL);

	if (cell > 0)
		*freq = __be32_to_cpu(cell[0]);
	else
		*freq = 12000000;
}



static int nvt_scan_nand_type(struct drv_nand_dev_info *info, int32_t* id)
{
	int maf_id, dev_id;
	struct nand_flash_dev *type = nvt_nand_ids;

	maf_id = *id & 0xFF;
	dev_id = (*id >> 8) & 0xFF;

	debug("the maf_id is 0x%x, the dev_id is 0x%x\n", maf_id, dev_id);

	for (; type->name != NULL; type++) {
		if (type->dev_id == dev_id)
			break;
	}

	if (!type->name) {
		return -ENODEV;
	}

	info->flash_info->chip_id = *id;

	info->flash_info->page_size = type->pagesize;
	info->flash_info->block_size = type->erasesize;
	info->flash_info->device_size = type->chipsize;
	info->flash_info->page_per_block = (type->erasesize / type->pagesize);

	debug("page_size %d, block_size %d, device_size %d page_per_block %d\n", \
		info->flash_info->page_size, \
		info->flash_info->block_size, \
		info->flash_info->device_size, \
		info->flash_info->page_per_block);

	if (type->pagesize == 4096) {
		info->flash_info->oob_size = 128;
	} else {
		info->flash_info->oob_size = 64;
	}

	return E_OK;
}

static int drv_nand_readpage(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	return nand_cmd_read_operation(info, (int8_t *)info->data_buff,
			page_addr * info->flash_info->page_size, 1);
}

static int drv_nand_write_page(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	return nand_cmd_write_operation_single(info, (int8_t *)info->data_buff,
				page_addr * info->flash_info->page_size, column);
}

static int drv_nand_dev_ready(struct mtd_info *mtd)
{
/*
	struct nand_chip *this = mtd->priv;
	struct drv_nand_dev_info *info = this->priv;
	return nand_host_check_ready(info);
*/
	return 0;
}


static int drv_nand_read_page_ecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)

{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;
	u8 status = 0, chip_id = info->flash_info->chip_id & 0xFF;
	int ret = 0;

	chip->read_buf(mtd, buf, eccsize * eccsteps);

	if (info->nand_chip.oob_poi)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	if (info->retcode == ERR_ECC_UNCLEAN) {
		mtd->ecc_stats.failed++;
	} else if (info->retcode == ECC_CORRECTED) {
		if (chip_id == NAND_MXIC) {
			ret = nand_cmd_read_flash_ecc_corrected(info);
			mtd->ecc_stats.corrected += ret;
		} else if (chip_id == NAND_TOSHIBA) {
			status = nand_cmd_read_status(info, \
				NAND_SPI_STS_FEATURE_4);

			mtd->ecc_stats.corrected += status;
			ret = status;
		} else {
			mtd->ecc_stats.corrected++;
			ret = 1;
		}
	}

	return ret;

}

static int drv_nand_write_page_ecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required, int page)
{
	/* this function will write page data and oob into nand */
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;

	chip->write_buf(mtd, buf, eccsize * eccsteps);

	return 0;
}

static int drv_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
	struct nand_chip *nand_chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_chip->priv;

	int ret = info->retcode;

	info->retcode = ERR_NONE;

	if (ret < 0)
		return NAND_STATUS_FAIL;
	else
		return E_OK;
}

static uint8_t drv_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	char retval = 0xFF;

	if (info->buf_start < info->buf_count)
		retval = info->data_buff[info->buf_start++];

	return retval;
}

static u16 drv_nand_read_word(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	u16 retval = 0xFFFF;

	if (!(info->buf_start & 0x01) && info->buf_start < info->buf_count) {
		retval = *((u16 *)(info->data_buff+info->buf_start));
		info->buf_start += 2;
	}
	return retval;
}

static void drv_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(buf, info->data_buff + info->buf_start, real_len);

	info->buf_start += real_len;
}

static void drv_nand_write_buf(struct mtd_info *mtd,
	const uint8_t *buf, int len)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(info->data_buff + info->buf_start, buf, real_len);

	info->buf_start += real_len;
}

static void drv_nand_cmdfunc(struct mtd_info *mtd, unsigned command,
		int column, int page_addr)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	int ret;
	uint8_t *ptr;

	info->data_size = 0;
	info->state = STATE_READY;
	info->retcode = ERR_NONE;

	switch (command) {
	case NAND_CMD_READOOB:
		info->buf_count = mtd->writesize + mtd->oobsize;
		info->buf_start = mtd->writesize + column;

		ptr = (uint8_t *)info->data_buff;
		ptr = info->data_buff + info->buf_start;
		if (info->buf_start != info->flash_info->page_size) {
			dev_err(&info->pdev->dev,
			"info->buf_start = %d, != 0\n", info->buf_start);
		}

		nand_cmd_read_page_spare_data(info, (int8_t *)ptr,
				info->flash_info->page_size * page_addr);
		/* We only are OOB, so if the data has error, does not matter */
	break;

	case NAND_CMD_READ0:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			printf("NAND_CMD_READ0 : is not Cache_Line_Size alignment!\n");

		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;

		ret = drv_nand_readpage(info, column, page_addr);
		if (ret == E_CTX)
			info->retcode = ERR_ECC_UNCLEAN;
		else if (ret < 0)
			info->retcode = ERR_SENDCMD;
		else if (ret == ECC_CORRECTED)
			info->retcode = ECC_CORRECTED;
		else
			info->retcode = ERR_NONE;
	break;

	case NAND_CMD_SEQIN:
		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;
		memset(info->data_buff, 0xff, info->buf_count);

		/* save column/page_addr for next CMD_PAGEPROG */
		info->seqin_column = column;
		info->seqin_page_addr = page_addr;
	break;

	case NAND_CMD_PAGEPROG:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			printf("not MIPS_Cache_Line_Size alignment!\n");

		ret = drv_nand_write_page(info, info->seqin_column, info->seqin_page_addr);
		if (ret)
			info->retcode = ERR_SENDCMD;
	break;

	case NAND_CMD_ERASE1:
		ret = nand_cmd_erase_block(info, page_addr);
		if (ret)
			info->retcode = ERR_BBERR;
	break;

	case NAND_CMD_ERASE2:
	break;

	case NAND_CMD_READID:
		info->buf_start = 0;
		info->buf_count = 4;
		if(info->flash_info->chip_id)
			memcpy((uint32_t *)info->data_buff, &info->flash_info->chip_id, 4);
		else {
			ret = nvt_nand_read_id(info, (uint32_t *)info->data_buff);
			if (ret)
				info->retcode = ERR_SENDCMD;
		}
	break;
	case NAND_CMD_STATUS:
		info->buf_start = 0;
		info->buf_count = 1;
		nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);
		if (!(info->data_buff[0] & 0x80))
			info->data_buff[0] = 0x80;
	break;

	case NAND_CMD_RESET:
	break;

	default:
		printf("non-supported command.\n");
	break;
	}
}

static int drv_nand_reset(struct drv_nand_dev_info *info)
{
	uint32_t reg, freq;
	u32 *cell = NULL;
	char path[20] = {0};
	int len;
	u32 flash_pinmux = 0x02;	// assume 4 bit pinmux
	u32 pinmux_val;
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;

	do {
		sprintf(path,"/top@%x/spi",IOADDR_TOP_REG_BASE);

		nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
		if (nodeoffset < 0) {
			printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
			printf("%s: path %s not found\n", __func__, path);
			break;
		}

		cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", &len);
		if (len == 0) {
			printf("%s(%d) len = 0\n",__func__, __LINE__);
			break;
		}
		flash_pinmux = __be32_to_cpu(cell[0]);
		printf("%s: spi flash pinmux 0x%x\r\n", __func__, flash_pinmux);
	} while (0);

	if (flash_pinmux == 0x04) {	// 2 bit
		pinmux_val = 3;
	} else if (flash_pinmux == 0x01) {
		pinmux_val = 1;
	} else {
	}
	switch (flash_pinmux) {
	case 0x01:
		pinmux_val = 1;
		break;
	case 0x02:
		pinmux_val = 2;
		break;
	case 0x04:
		pinmux_val = 3;
		break;
	default:
	case 0x0:			// GPIO
		pinmux_val = 0;
		break;
	}

	/* set pinmux */
	reg = readl(IOADDR_TOP_REG_BASE + 0x20);
	reg &= ~0xFFF00000;
	reg |= (pinmux_val<<28)|(pinmux_val<<24)|(pinmux_val<<20);
	writel(reg, IOADDR_TOP_REG_BASE + 0x20);

	reg = readl(IOADDR_TOP_REG_BASE + 0x24);
	reg &= ~0xFFFF;
	reg |= (pinmux_val<<12)|(pinmux_val<<8)|(pinmux_val<<4)|(pinmux_val<<0);
	writel(reg, IOADDR_TOP_REG_BASE + 0x24);

	/* Enable CG*/
	reg = readl(IOADDR_CG_REG_BASE + 0x54);
	reg |= 0x1 << 11;
	writel(reg, IOADDR_CG_REG_BASE + 0x54);

	reg = readl(IOADDR_CG_REG_BASE + 0x68);
	reg &= ~(0x1 << 9);
	writel(reg, IOADDR_CG_REG_BASE + 0x68);

	nvt_parse_frequency(&freq);

	info->ops_freq = freq;

	info->flash_type = NANDCTRL_SPI_NAND_TYPE;

	return nand_cmd_reset(info);
}


int nvt_nand_read_id(struct drv_nand_dev_info *info, uint32_t *id)
{
	uint8_t  card_id[8];

	if (nand_cmd_read_id(card_id, info) != 0) {
		printf("NAND cmd timeout\r\n");
		return -1;
	} else {
		printf("id =  0x%02x 0x%02x 0x%02x 0x%02x\n",
			card_id[0], card_id[1], card_id[2], card_id[3]);

		*id = card_id[0] | (card_id[1] << 8);
		return 0;
	}

}

int nvt_nand_scan_2plane_id(uint32_t id)
{
	int32_t i, flash_id;

	for (i = 0 ; i < NUM_OF_2PLANE_ID; i++) {
		flash_id = (nand_2plane_list[i].mfr_id + (nand_2plane_list[i].dev_id << 8));
		if (id == flash_id)
			return 1;
    }
	return 0;
}

int nvt_nand_board_nand_init(struct nand_chip *nand)
{
	int32_t id;
	struct drv_nand_dev_info *info;
	u32 status;

#ifdef CONFIG_NVT_SPI_NONE
	return -ENOMEM;
#endif

	info = calloc(1, sizeof(struct drv_nand_dev_info));
	if (!info) {
		printf("alloc drv_nand_dev_info failed!\n");
		return -ENOMEM;
	}

	info->flash_info = calloc(1, sizeof(struct nvt_nand_flash));
	if (!info) {
		printf("alloc nvt_nand_flash failed!\n");
		return -ENOMEM;
	}
	info->flash_info->spi_nand_status.bBlockUnlocked    = FALSE;
	info->flash_info->spi_nand_status.bQuadEnable       = FALSE;
	info->flash_info->spi_nand_status.bQuadProgram      = FALSE;
	info->flash_info->spi_nand_status.uiTimerRecord     = FALSE;

	nand->priv = info;
	info->mmio_base = (void *)(CONFIG_SYS_NAND_BASE);

	drv_nand_reset(info);

	nvt_parse_parameter(info);

	/*Delay 1 ms for spinand characteristic*/
	mdelay(1);

	nvt_nand_read_id(info, (uint32_t *)&id);

#ifndef CONFIG_FLASH_ONLY_DUAL
	if (id != TOSHIBA_TC58CVG)
		info->flash_info->spi_nand_status.bQuadProgram = TRUE;
#endif

	if (nvt_scan_nand_type(info, &id)) {
		printf("flash not support with id 0x%x\n", id);
		return -ENODEV;
	}

	info->data_buff = calloc(info->flash_info->page_size+info->flash_info->oob_size + CACHE_LINE_SIZE, sizeof(unsigned char));

	if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
		info->data_buff = (uint8_t *)((((unsigned long)info->data_buff + CACHE_LINE_SIZE - 1)) & 0xFFFFFFC0);

	if(info->data_buff == NULL) {
		printf("allocate nand buffer fail !\n");
		return 1;
	}

	status = nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);

	status |= SPINAND_CONFIG_ECCEN;

#ifndef CONFIG_FLASH_ONLY_DUAL
	if (((id & 0xFF) == NAND_MXIC) || ((id & 0xFF) == SPI_NAND_GD) || ((id & 0xFF) == SPI_NAND_DOSILICON))
		status |= SPINAND_CONFIG_QUADEN;
#endif

	nand_cmd_write_status(info, NAND_SPI_STS_FEATURE_2, status);

	info->use_dma = 1;
	info->data_size = 0;
	info->state = STATE_READY;

	if (info->flash_info->page_size == 512)
		nand->ecc.layout = &hw_smallpage_ecclayout;
	else
		nand->ecc.layout = &spinand_oob_64;
	nand->ecc.size = 0x200;
	nand->ecc.bytes = 0x8;
	nand->ecc.steps = 0x4;
	nand->ecc.total	= nand->ecc.steps * nand->ecc.bytes;
	nand->ecc.strength = 1;

	nand->cmd_ctrl = drv_nand_hwcontrol;
	nand->dev_ready = drv_nand_dev_ready;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.read_page = drv_nand_read_page_ecc;
	nand->ecc.write_page = drv_nand_write_page_ecc;
	nand->options = NAND_NO_SUBPAGE_WRITE;
	nand->waitfunc = drv_nand_waitfunc;
	nand->read_byte = drv_nand_read_byte;
	nand->read_word = drv_nand_read_word;
	nand->read_buf = drv_nand_read_buf;
	nand->write_buf = drv_nand_write_buf;
	nand->cmdfunc = drv_nand_cmdfunc;
	nand->chip_delay = 0;

	if ((id & 0xFF) == NAND_TOSHIBA)
		nand->ecc.strength = 8;
	else if ((id & 0xFF) == NAND_MXIC)
		nand->ecc.strength = 4;
	
	if (nvt_nand_scan_2plane_id(id) == 1) {
		info->flash_info->plane_flags = 1;
		printf("2 plane en\n");
	} else {
		info->flash_info->plane_flags = 0;
		printf("2 plane dis\n");
	}
#if 0
	if ((MXIC_MX35LF2G14AC == id) || (MICRON_MT29F2G01AB == id))
		info->flash_info->plane_flags = 1;
	else
		info->flash_info->plane_flags = 0;
#endif

	printf("nvt spinand %d-bit mode @ %d Hz\n", \
		info->flash_info->spi_nand_status.bQuadProgram ? 4 : 1, \
		info->ops_freq);

	return 0;
}

#endif

