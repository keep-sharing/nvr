// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <spi_flash.h>
#include <linux/mtd/mtd.h>
#include <fat.h>
#include <console.h>
#include <mmc.h>
#include <sparse_format.h>
#include <linux/kernel.h>
#include <nand.h>
#include "image-sparse.h"
#include <securec.h>

#if (CONFIG_AUTO_UPDATE == 1)  /* cover the whole file */

#if (CONFIG_AUTO_SD_UPDATE == 1)
#ifndef CONFIG_MMC
#error "should have defined CONFIG_MMC"
#endif
#include <mmc.h>
#include "mmc_init.c"
#endif

#if defined CONFIG_AUTO_USB_UPDATE
#ifndef CONFIG_VENDOR_MC
#if !defined CONFIG_USB_OHCI && !defined CONFIG_USB_XHCI_HCD
#error "should have defined CONFIG_USB_OHCI or CONFIG_USB_XHCI"
#endif
#endif
#ifndef CONFIG_USB_STORAGE
#error "should have defined CONFIG_USB_STORAGE"
#endif
#include <usb.h>
#include "usb_init.c"
#endif
#include "../../fs/ext4/unsparse.h"

#undef AU_DEBUG
#undef debug_print
#ifdef  AU_DEBUG
#define debug_print(fmt, args...) printf(fmt, ##args)
#else
#define debug_print(fmt, args...)
#endif  /* AU_DEBUG */

/* possible names of files on the medium. */
#define AU_CONFIG   "config"
/* config file's size < 1K */
#define CONFIG_MAX_SIZE  	2048
#define BLOCK_SIZE           	512
#define STR_LEN			80
#define LINE			16
#define EMMC_BLOCK_SHIFT	9
#define NAME_LEN		20
#define SIZE_M			(1024 * 1024)
#define SIZE_K			1024
#define DECI_VALUE		10
#define STR_STEPS		2
#define PERCENT_VALUE		100
#define STOR_DEV_OFFSET		16
#define STOR_DEV_MASK		0x03
#define STOR_PART		18
#define STOR_PART_MASK		0x1f

#ifdef CONFIG_VENDOR_UPGRADE_BY_SEGMENT
#define SECTION_SIZE         0x1000000ULL   /* 16M  */
#define BLOCK_SIZE           512
#define DEFAULT_BLOCK_SIZE   4096
#define CHUNK_HEAD_SIZE      12
#define BOUNDARY             (DEFAULT_BLOCK_SIZE + CHUNK_HEAD_SIZE)
#define HEX_DUMP_LEN         128
unsigned long long map_size;
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
char is_sparse = 0;
#endif
#endif

#define AU_FIRMWARE "u-boot.bin"
#define AU_KERNEL   "kernel"

int boot_medium_type = 0xff;

#include "nand.h"
struct flash_layout {
	long start;
	long end;
};

struct seg_update_parm {
	int extra;
	int nsect;
	int uboot_updated;
	int updatefile_found;
	unsigned int section_size;
	loff_t sz;
	loff_t actread;
	loff_t pos;
	loff_t remain;
};
struct update_medium_interface {
	char name[NAME_LEN];
	int (*init)(void);
	int (*erase)(unsigned long offset, unsigned long len);
	int (*write)(unsigned long offset, unsigned long len,
			 unsigned char *buf);
	int (*write_yaffs)(unsigned long offset, unsigned long len,
			unsigned char *buf);
	int (*write_ext4)(unsigned long offset, unsigned long len,
			unsigned char *buf, char is_sparse);
};

#if defined(CONFIG_CMD_SF) || defined(CONFIG_CMD_NAND)
static void schedule_notify(unsigned long offset, unsigned long len,
				unsigned long off_start)
{
	int percent_complete = -1;
	unsigned long long n;

	do {
		n = (unsigned long long)(offset - off_start) * PERCENT_VALUE;
		int percent;

		do_div(n, len);
		percent = (int)n;

		/* output progress message only at whole percent
		 * steps to reduce the number of messages
		 * printed on (slow) serial consoles
		 */
		if (percent != percent_complete)
			/* do not to add '\n' to this message. */
			printf("\rOperation at 0x%lx -- %3d%% complete",
			       offset, percent);
	} while (0);
}
#endif

#ifdef CONFIG_CMD_SF
static struct spi_flash *spinor_flash;
static int spinor_flash_init(void)
{
	spinor_flash = spi_flash_probe(0, 0, 0, 0);
	return 0;
}

static int spi_flash_erase_op(struct spi_flash *flash, unsigned long offset,
					unsigned long len)
{
	int ret;
	struct mtd_info_ex *spiflash_info = get_spiflash_info();
	unsigned long erase_start, erase_len, erase_step;

	erase_start = offset;
	erase_len   = len;
	erase_step  = spiflash_info->erasesize;

	while (len > 0) {
		if (len < erase_step)
			erase_step = len;

		ret = spi_flash_erase(flash, (u32)offset, erase_step);
		if (ret)
			return 1;

		len -= erase_step;
		offset += erase_step;
		/* notify real time schedule */
		schedule_notify(offset, erase_len, erase_start);
	}

	return ret;
}

static int spinor_flash_erase(unsigned long offset, unsigned long len)
{
	return spi_flash_erase_op(spinor_flash, offset, len);
}

static int spi_flash_write_op(struct spi_flash *flash, unsigned long offset,
				unsigned long len, unsigned char *buf)
{
	int ret;
	unsigned long write_start, write_len, write_step;
	unsigned char *pbuf = buf;
	struct mtd_info_ex *spiflash_info = get_spiflash_info();

	write_start = offset;
	write_len   = len;
	write_step  = spiflash_info->erasesize;

	while (len > 0) {
		if (len < write_step)
			write_step = len;

		ret = flash->write(flash, offset, write_step, pbuf);
		if (ret)
			break;

		offset += write_step;
		pbuf   += write_step;
		len    -= write_step;
		/* notify real time schedule */
		schedule_notify(offset, write_len, write_start);
	}

	return ret;
}

static int spinor_flash_write(unsigned long offset, unsigned long len, unsigned char *buf)
{
	return spi_flash_write_op(spinor_flash, offset, len, (unsigned char *)buf);
}
#endif

#ifdef CONFIG_CMD_NAND
struct mtd_info *nand_flash;

static int nand_flash_init(void)
{
	nand_flash = nand_info[0];
	return 0;
}

static int nand_flash_erase(unsigned long offset, unsigned long len)
{
	int ret;
	unsigned long erase_len;
	unsigned long erase_step;
	unsigned long length;
	nand_erase_options_t opts;

	(void)memset_s(&opts, sizeof(opts), 0, sizeof(opts));

	length = len;
	erase_step = nand_flash->erasesize;
	erase_len = length;
	opts.length  = erase_step;
	opts.offset = offset;
	opts.quiet = 1;

	while (length > 0) {
		if (length < erase_step)
			erase_step = length;

		ret = nand_erase_opts(nand_flash, &opts);
		if (ret)
			return 1;

		length -= erase_step;
		opts.offset += erase_step;
		/* notify real time schedule */
		schedule_notify(opts.offset, erase_len, offset);
	}

	return ret;
}

static int nand_flash_write(unsigned long offset, unsigned long len,
						unsigned char *buf)
{
	int ret;
	unsigned long offset_notify;
	unsigned long write_start;
	unsigned long write_len;
	unsigned long write_step;
	size_t length;
	unsigned char *pbuf = buf;

	if (offset == 0) {
		/* Make sure the length is block size algin */
		length = len & (nand_flash->erasesize - 1) ? (size_t)(len +
				 (nand_flash->erasesize - len %
				  nand_flash->erasesize)) : len;
		write_step  = nand_flash->erasesize;
	} else {
		/* Make sure the length is writesize algin */
		length = len & (nand_flash->erasesize - 1) ? (size_t)(len +
				 (nand_flash->writesize - len %
				  nand_flash->writesize)) : len;
		write_step  = nand_flash->writesize;
	}

	write_start = offset;
	offset_notify = offset;
	write_len   = length;

	while (length > 0) {
		size_t block_offset = offset & (nand_flash->erasesize - 1);
		size_t *rw_size;

		if (nand_flash->_block_isbad(nand_flash, offset & ~
					(nand_flash->erasesize - 1))) {
			printf("Skip bad block 0x%08llx\n",
				   offset & ~(loff_t)(nand_flash->erasesize - 1));
			offset += nand_flash->erasesize - block_offset;
			continue;
		}

		rw_size = (size_t *)&write_step;

		struct mtd_info *mtd;
		mtd = get_nand_dev_by_index(0);
		ret = nand_write(mtd, offset, rw_size, pbuf);
		if (ret) {
			printf("NAND write to offset %lx failed %d\n",
				   offset, ret);
			break;
		}
		offset += write_step;
		pbuf   += write_step;
		length -= write_step;
		offset_notify += write_step;
		/* notify real time schedule */
		schedule_notify(offset_notify, write_len, write_start);
	}

	return ret;
}

static int nand_flash_yaffs_write(unsigned long offset, unsigned long len,
							unsigned char *buf)
{
	int ret;
	size_t rw_size = len;

	ret = nand_write_yaffs_skip_bad(nand_flash, offset, &rw_size,
							(u_char *)buf);
	if (ret)
		printk("Write yaffs fail !!\n");
	printk("Write yaffs done\n");

	return ret;
}

/* get count of area's bad block for nand flash */
int get_bad_block_count(unsigned long offset, unsigned long len)
{
	int count = 0;
	unsigned long block_offset = 0;
	if (offset & (nand_flash->erasesize - 1))
		block_offset = offset & (nand_flash->erasesize - 1);

	if (len & (nand_flash->erasesize - 1))
		len = ALIGN(len, nand_flash->erasesize);

	for (int i = 0; i < len / nand_flash->erasesize; i++) {
		if (nand_block_isbad(nand_flash, offset))
			count++;
		offset += nand_flash->erasesize - block_offset;
	}
	return count;
}
#endif

void hex_dump(const unsigned char *buf, int len, int addr)
{
	int i;
	int j;
	int k;
	char binstr[STR_LEN];
	if (buf == NULL)
		return;
	if (len > strlen((char *)buf))
		return;

	for (i = 0; i < len; i++) {
		if ((i % LINE) == 0) {
			(void)sprintf_s(binstr, STR_LEN, "%08x -", i + addr);
			(void)sprintf_s(binstr, STR_LEN, "%s %02x", binstr, (unsigned char)buf[i]);
		} else if ((i % LINE) == (LINE - 1)) {
			(void)sprintf_s(binstr, STR_LEN, "%s %02x", binstr,
					(unsigned char)buf[i]);
			(void)sprintf_s(binstr, STR_LEN, "%s  ", binstr);
			for (j = i - (LINE - 1); j <= i; j++)
				(void)sprintf_s(binstr, STR_LEN, "%s%c", binstr,
						(buf[j] > '!' &&
						 buf[j] <= '~') ? buf[j] : '.');
			printf("%s\n", binstr);
		} else {
			(void)sprintf_s(binstr, STR_LEN, "%s %02x", binstr,
					(unsigned char)buf[i]);
		}
	}

	if ((i % LINE) != 0) {
		k = LINE - (i % LINE);
		for (j = 0; j < k; j++)
			(void)sprintf_s(binstr, STR_LEN, "%s   ", binstr);
		(void)sprintf_s(binstr, STR_LEN, "%s  ", binstr);
		k = LINE - k;
		for (j = i - k; j < i; j++)
			(void)sprintf_s(binstr, STR_LEN, "%s%c", binstr,
					(buf[j] > '!' && buf[j] <= '~') ? buf[j] : '.');
		printf("%s\n", binstr);
	}
}

#ifdef CONFIG_SUPPORT_EMMC_BOOT
static int mmc_save_init(void)
{
	struct mmc *mmc = find_mmc_device(0);

	if (!mmc) {
		printf("%s:find mmc device failed\n", __func__);
		return -1;
	}

	(void)mmc_init(mmc);

	return 0;
}

static int mmc_save_write(unsigned long offset, unsigned long len,
		 unsigned char *buf)
{
	struct mmc *mmc = find_mmc_device(0);

	if (!mmc) {
		printf("%s:find mmc device failed\n", __func__);
		return -1;
	}
	if (len % MMC_MAX_BLOCK_LEN)
		blk_dwrite(mmc_get_blk_desc(mmc), (offset >> EMMC_BLOCK_SHIFT),
				   (len >> EMMC_BLOCK_SHIFT) + 1, buf);
	else
		blk_dwrite(mmc_get_blk_desc(mmc), (offset >> EMMC_BLOCK_SHIFT),
				   (len >> EMMC_BLOCK_SHIFT), buf);

	return 0;
}

static int mmc_save_write_ext4(unsigned long offset, unsigned long len,
		unsigned  char *buf, char is_sparse)
{
	struct mmc *mmc = find_mmc_device(0);
	int retlen;

	if (!mmc) {
		printf("%s:find mmc device failed\n", __func__);
		return -1;
	}
	if (is_sparse) {
		if (len % MMC_MAX_BLOCK_LEN)
			retlen = ext4_unsparse(mmc, 0, (unsigned  char *)buf,
					(offset >> EMMC_BLOCK_SHIFT),
					(len >> EMMC_BLOCK_SHIFT) + 1);
		else
			retlen = ext4_unsparse(mmc, 0, (unsigned  char *)buf,
					(offset >> EMMC_BLOCK_SHIFT),
					(len >> EMMC_BLOCK_SHIFT));
	} else {
		retlen = mmc_save_write(offset, (unsigned long)len, buf);
	}
	return retlen;
}
#endif

#define UPDATE_MEDIUM_SPINOR 0
#define UPDATE_MEDIUM_NAND   1
#define UPDATE_MEDIUM_EMMC   2

static struct update_medium_interface update_intf[3] = {
#ifdef CONFIG_CMD_SF
	{"spinor", spinor_flash_init, spinor_flash_erase, spinor_flash_write,
		NULL, NULL},
#else
	{"none", NULL, NULL, NULL, NULL, NULL},
#endif

#ifdef CONFIG_CMD_NAND
	{"nand", nand_flash_init, nand_flash_erase, nand_flash_write,
		nand_flash_yaffs_write, NULL},
#else
	{"none",  NULL,  NULL, NULL, NULL, NULL},
#endif
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	{"emmc", mmc_save_init, NULL, mmc_save_write, NULL, mmc_save_write_ext4}
#endif
};

static struct update_medium_interface *update_intf_p;

struct medium_interface {
	char name[NAME_LEN];
	int (*init)(void);
	void (*exit)(void);
};

#define MAX_UPDATE_INTF 2
static struct medium_interface s_intf[MAX_UPDATE_INTF] = {
#if (CONFIG_AUTO_SD_UPDATE == 1)
	{ .name = "mmc", .init = mmc_stor_init, .exit = mmc_stor_exit, },
#endif
#if (CONFIG_AUTO_USB_UPDATE == 1)
	{ .name = "usb", .init = usb_stor_init, .exit = usb_stor_exit, },
#endif
};

static int au_stor_curr_dev; /* current device */

/* index of each file in the following arrays */
#define IDX_FIRMWARE    0
#define IDX_KERNEL  1
#define IDX_ROOTFS  2

/* max. number of files which could interest us */
#define AU_MAXFILES 32

/* pointers to file names */
char *aufile[AU_MAXFILES] = {
	0
};

#define NAME_MAX_LEN 0x20
char aufile_table[AU_MAXFILES][NAME_MAX_LEN] = {{0, }, };
unsigned long aufile_size[AU_MAXFILES] = {0};

/* sizes of flash areas for each file */
long ausize[AU_MAXFILES] = {0};

/* array of flash areas start and end addresses */
struct flash_layout aufl_layout[AU_MAXFILES] = {
	{ 0,    0, }
};

#ifdef CONFIG_VENDOR_UPGRADE_BY_SEGMENT

#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
sparse_header_t sparse_header;  /* Sparse file header information */
chunk_header_t chunk_header;
int is_chunk_header(void *pbuf)
{
	chunk_header_t *s_chunk_header = NULL;

	if (pbuf == NULL)
		return 0;
	s_chunk_header = pbuf;
	if (s_chunk_header->chunk_type == CHUNK_TYPE_RAW 	   || \
		s_chunk_header->chunk_type == CHUNK_TYPE_DONT_CARE || \
		s_chunk_header->chunk_type == CHUNK_TYPE_FILL)
		return 1;
	return 0;
}

typedef struct buf_info {
	int one_block;
	unsigned int chunk_head_size;
	bool had_sparse_header;
	bool set_chunk_header;
	bool had_chunk_header;
	bool large_chunk_header;
	void *buf;
	unsigned long off;
}buf_info_t;

void assign_data(buf_info_t *info, void *pbuf)
{
	if (info == NULL || pbuf == NULL)
		return;
	/* clear info data struct to 0 */
	info->one_block = 0;
	info->buf = pbuf;
	info->chunk_head_size = 0;
	info->had_sparse_header = false;
	info->set_chunk_header = false;
	info->had_chunk_header = false;
	info->off = 0;

	sparse_header.total_blks = 0;
	sparse_header.total_chunks = 0;
	sparse_header.image_checksum = 0;
}

chunk_header_t *check_buf_for_head(buf_info_t *info, void *pbuf)
{
	chunk_header_t *chunk = NULL;

	if (info == NULL || info->buf == NULL)
		return NULL;

	if (is_sparse_image(info->buf)) {
		info->off += sparse_header.file_hdr_sz;
		info->buf += sparse_header.file_hdr_sz;
		info->had_sparse_header = true;
	}

	if (is_chunk_header(info->buf)) {
		info->had_chunk_header = true;
		chunk = (chunk_header_t *)info->buf;
		if (chunk->total_sz < SECTION_SIZE)
			info->off += chunk->total_sz;
		else
			info->large_chunk_header = true;
	}

	if (!is_chunk_header(info->buf) && is_chunk_header(&chunk_header)) {
		/* Process the remaining data in large chunks */
		(void)memcpy_s(info->buf - sparse_header.chunk_hdr_sz, sizeof(chunk_header_t),
				&chunk_header, sizeof(chunk_header_t));
		info->set_chunk_header = true;
		info->buf -= sparse_header.chunk_hdr_sz;
		chunk = (chunk_header_t *)info->buf;
		if (chunk->total_sz < SECTION_SIZE)
			info->off += chunk->total_sz - sparse_header.chunk_hdr_sz;
	}
	return chunk;
}

void *process_large_chunks(buf_info_t *info, chunk_header_t *chunk,
		unsigned long *sparse_len, loff_t *pos, void *pbuf)
{
	unsigned char skip_chunk_head = 0;

	if (info == NULL || chunk == NULL || sparse_len == NULL || pos == NULL)
		return NULL;
	/* large chunk: head + len > SECTION_SIZE, so it has 1 chunk */
	sparse_header.total_chunks = 1;
	if (info->large_chunk_header) {
		/* head takes up 1 block */
		info->one_block = 1;
		info->chunk_head_size = sparse_header.chunk_hdr_sz;
	} else {
		info->chunk_head_size = 0;
		info->one_block = 0;
	}

	if (info->set_chunk_header)
		skip_chunk_head = sparse_header.chunk_hdr_sz;
	sparse_header.total_blks += SECTION_SIZE / sparse_header.blk_sz -
		info->one_block;
	/* (16M - 12byte) / 4096byte  ---> 16M/4096byte - 1 */
	*sparse_len += (SECTION_SIZE / sparse_header.blk_sz - info->one_block) *
		sparse_header.blk_sz;
	/* here can not set next chunk header,so i save it to chunk_header  */
	chunk_header.chunk_type = chunk->chunk_type;
	chunk_header.total_sz = chunk->total_sz - *sparse_len;
	chunk_header.chunk_sz = chunk->chunk_sz - (SECTION_SIZE /
			sparse_header.blk_sz - info->one_block);
	/* update cur header */
	chunk->chunk_sz = SECTION_SIZE / sparse_header.blk_sz - info->one_block;
	chunk->total_sz = *sparse_len + sparse_header.chunk_hdr_sz;
	info->off = *sparse_len + info->chunk_head_size;
	/* set cur pbuf sparse header */
	(void)memcpy_s(pbuf - sparse_header.file_hdr_sz - skip_chunk_head, sizeof(sparse_header_t),
			&sparse_header, sizeof(sparse_header_t));
	*pos += info->off;
	return pbuf - sparse_header.file_hdr_sz - skip_chunk_head;
}

void *process_small_chunks(buf_info_t *info, chunk_header_t *chunk, unsigned long *sparse_len,
				loff_t *pos, loff_t file_size, void *pbuf)
{
	unsigned char skip_chunk_head;

	if (info == NULL || chunk == NULL || sparse_len == NULL || pos == NULL)
		return NULL;
	*sparse_len += chunk->chunk_sz * sparse_header.blk_sz;
	/* skip first chunk */
	info->buf += chunk->total_sz;
	sparse_header.total_chunks++;
	sparse_header.total_blks += chunk->chunk_sz;
	debug_print("total_blks:%u, total_chunks:%u,off:%lu, line:%d\n", sparse_header.total_blks,
			sparse_header.total_chunks, info->off, __LINE__);
	do {
		if (!is_chunk_header(info->buf)) {
			printf("this chunk header is not correct,please check off\n");
			return NULL;
		}
		chunk = (chunk_header_t *)info->buf;
		print_chunk_info(chunk);
		sparse_header.total_blks += chunk->chunk_sz;
		sparse_header.total_chunks++;
		info->buf += chunk->total_sz;
		info->off += chunk->total_sz;
		*sparse_len += chunk->chunk_sz * sparse_header.blk_sz;
		debug_print("buf:%p,off:%lu,total_blks:%u\n", info->buf, info->off,
				sparse_header.total_blks);
	} while (info->off < SECTION_SIZE && (*pos + info->off) != file_size);

	if ((*pos + info->off) == file_size) {
		skip_chunk_head = info->set_chunk_header ? sparse_header.chunk_hdr_sz : 0;
		(void)memcpy_s(pbuf - sparse_header.file_hdr_sz - skip_chunk_head, sizeof(sparse_header_t),
				&sparse_header, sizeof(sparse_header_t));
		pbuf -= (sparse_header.file_hdr_sz + skip_chunk_head);
		*pos += info->off;
		return pbuf;
	} else if (info->off > SECTION_SIZE) {
		/* discarding incomplete chunk */
		sparse_header.total_blks -= chunk->chunk_sz;
		sparse_header.total_chunks--;
		info->off -= chunk->total_sz;
		info->buf -= chunk->total_sz;
		*sparse_len -= chunk->chunk_sz * sparse_header.blk_sz;
		debug_print("buf:%p, off:%lu\n", info->buf, info->off);
	}
	/* updating the sparse header */
	if (info->had_sparse_header)
		(void)memcpy_s(pbuf, sizeof(sparse_header_t), &sparse_header, sizeof(sparse_header_t));
	if (!info->had_sparse_header) {
		(void)memcpy_s(pbuf - sparse_header.file_hdr_sz, sizeof(sparse_header_t),
							&sparse_header, sizeof(sparse_header_t));
		pbuf -= sparse_header.file_hdr_sz;
	}
	debug_print("total_blks:%u, total_chunks:%u,off:%lu\n", sparse_header.total_blks,
			sparse_header.total_chunks, info->off);
	*pos += info->off;
	return pbuf;
}

char* get_buffer_chunk_layout(char *pbuf, loff_t *pos,
		unsigned long *sparse_len, loff_t file_size)
{
	chunk_header_t *chunk = NULL;
	buf_info_t info;

	assign_data(&info, pbuf);
	chunk = check_buf_for_head(&info, pbuf);
	if (chunk == NULL) {
		printf("there is no chunk header or sparse header in buff\n");
		return NULL;
	}

	if (chunk->total_sz >= SECTION_SIZE)
		return process_large_chunks(&info, chunk, sparse_len, pos, pbuf);
	else
		return process_small_chunks(&info, chunk, sparse_len, pos,
				file_size, pbuf);
}
#endif
#endif
/* where to load files into memory */
#define LOAD_ADDR (unsigned char *)CONFIG_SYS_LOAD_ADDR
/* the app is the largest image */
#define MAX_LOADSZ ausize[IDX_ROOTFS]

#ifdef CONFIG_VENDOR_UPGRADE_BY_SEGMENT
int write_yaffs2_fs(int idx, unsigned int sz, unsigned char *pbuf)
{
#ifdef CONFIG_CMD_NAND
	int bad_block_count;
	unsigned long pages_len;
	int ret;
	unsigned long tmp_start_addr = aufl_layout[idx].start;
	unsigned int sec_yaffs = nand_flash->writesize + nand_flash->oobsize;

	if (update_intf_p->write_yaffs) {
		pages_len = (sz / sec_yaffs) * nand_flash->writesize;
		bad_block_count = get_bad_block_count(tmp_start_addr, pages_len);
		debug_print("write offset:%ld, bad block count:%d\n",
				aufl_layout[idx].start, bad_block_count);

		tmp_start_addr += pages_len + bad_block_count * nand_flash->erasesize;

		if (tmp_start_addr > aufl_layout[idx].end) {
			printf("The actual length of the partition is greater \
				than the specified value due to the \
				address offset caused by bad blocks.\n");
			return 1;
		}
		ret = update_intf_p->write_yaffs(aufl_layout[idx].start, sz,
				pbuf);
		if (ret) {
			printf("write yaffs failed\n");
			return ret;
		}
		aufl_layout[idx].start += pages_len + bad_block_count *
			nand_flash->erasesize;
	}
#endif
	return 0;
}
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
int write_ext4_fs(int idx, unsigned char *pbuf, unsigned long long sz)
{
	int ret;

	ret = update_intf_p->write_ext4(
			aufl_layout[idx].start, sz, pbuf, is_sparse);
	if (ret) {
		printf("write ext4 fs error\n");
		return 1;
	}
	aufl_layout[idx].start += sz;
	return ret;
}
#endif
int write_ubi_fs(int idx, unsigned int sz, unsigned char *pbuf)
{
#ifdef CONFIG_CMD_NAND
	int bad_block_count;
	unsigned long tmp_start_addr;
	int ret;
#endif
	if (boot_medium_type == BOOT_FROM_NAND) {
#ifdef CONFIG_CMD_NAND
		tmp_start_addr = aufl_layout[idx].start;
		while (sz & (nand_flash->writesize - 1))
			sz = ALIGN(sz, nand_flash->writesize);
		bad_block_count = get_bad_block_count(tmp_start_addr, sz);
		debug_print("write offset: start:%ld, bad block count:%d\n",
				tmp_start_addr, bad_block_count);
		tmp_start_addr += sz + bad_block_count * nand_flash->erasesize;
		if (tmp_start_addr > aufl_layout[idx].end) {
			printf("The actual length of the partition is greater\
				than the specified value due to the address\
				offset caused by bad blocks.\n");
			return 1;
		}
		ret = update_intf_p->write((aufl_layout[idx].start), sz, pbuf);
		if (ret) {
			printf("spi nand write ubifs error.\n");
			return 1;
		}
		aufl_layout[idx].start += sz + bad_block_count *
			nand_flash->erasesize;
#endif
	} else {
		/* ubifs for spi nor */
		update_intf_p->write((aufl_layout[idx].start), sz, pbuf);
		aufl_layout[idx].start += sz;
	}

	return 0;
}

int write_other_files(int idx, unsigned int sz, unsigned char *pbuf)
{
	int ret = 0;
#ifdef CONFIG_CMD_NAND
	int bad_block_count;
	unsigned long tmp_start_addr;
#endif
	if (boot_medium_type == BOOT_FROM_NAND) {
#ifdef CONFIG_CMD_NAND
		tmp_start_addr = aufl_layout[idx].start;
		while (sz & (nand_flash->writesize - 1))
			sz = ALIGN(sz, nand_flash->writesize);
		debug_print("write start:%ld, len:%u,pagesize:%u\n",
			aufl_layout[idx].start, sz, nand_flash->writesize);
		bad_block_count = get_bad_block_count(tmp_start_addr, sz);
		tmp_start_addr += sz + bad_block_count * nand_flash->erasesize;
		/* u-boot */
		if (strcmp((char *)AU_FIRMWARE, aufile[idx]) == 0) {
			if (tmp_start_addr > aufl_layout[idx].end) {
				printf("The address offset caused by bad\
				blocks causes the u-boot.bin file to overwrite the environment variable.\n");
				return 1;
			}
		} else if (strcmp((char *)AU_KERNEL, aufile[idx]) == 0) {
			if (tmp_start_addr > aufl_layout[idx].end) {
				printf("The address offset caused by bad blocks causes the kernel file to\
						overwrite the File system.\n");
				return 1;
			}
		}
		ret = update_intf_p->write((aufl_layout[idx].start), sz, pbuf);
		aufl_layout[idx].start += sz + bad_block_count *
			nand_flash->erasesize;

#endif
	} else if (boot_medium_type == BOOT_FROM_EMMC) {
		if (sz & (BLOCK_SIZE - 1))
			sz = ALIGN(sz, BLOCK_SIZE);
		debug_print("write start:%ld, len:%#x\n", aufl_layout[idx].start, sz);
		ret = update_intf_p->write((aufl_layout[idx].start), sz, pbuf);
		aufl_layout[idx].start += sz;
	} else if (boot_medium_type == BOOT_FROM_SPI) {
		debug_print("write start:%ld, len:%#x\n", aufl_layout[idx].start, sz);
		ret = update_intf_p->write((aufl_layout[idx].start), sz, pbuf);
		aufl_layout[idx].start += sz;
	}

	if (ret) {
		printf("write file %s failed\n", aufile[idx]);
		return ret;
	}
	return 0;
}
#endif
#ifndef CONFIG_VENDOR_UPGRADE_BY_SEGMENT
static int au_do_update(int idx, long sz)
{
	unsigned long start;
	unsigned long len;
	unsigned long write_len;
	int rc = 0;
	char *buf;
	void *pbuf;
	char sparse_flag = 0;

	start = aufl_layout[idx].start;
	len = aufl_layout[idx].end - aufl_layout[idx].start + 1;

	/* erase the address range. */
	if (boot_medium_type == BOOT_FROM_SPI || boot_medium_type == BOOT_FROM_NAND) {
		printf("%s erase...\n", update_intf_p->name);
		rc = update_intf_p->erase(start, len);
		if (rc) {
			printf("sector erase failed\n");
			return 1;
		}
	}

	buf = map_physmem((unsigned long)LOAD_ADDR, len, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	/* write the whole file to flash;uboot and rootfs image have head
	 * kernel has head,it's head also be writed to flash
	 */
	pbuf = buf;
	write_len = aufile_size[idx];

	/* copy the data from RAM to FLASH */
	printf("\n%s write...\n", update_intf_p->name);

	if (strstr(aufile[idx], "yaffs")) {
		if (update_intf_p->write_yaffs)
			rc = update_intf_p->write_yaffs(start, write_len, pbuf);
	} else if (strstr(aufile[idx], "ext4")) {
		if (update_intf_p->write_ext4) {
#ifdef CONFIG_EXT4_SPARSE
			sparse_flag = is_sparse_image((sparse_header_t *)pbuf);
#endif
			rc = update_intf_p->write_ext4(start, len, pbuf, sparse_flag);
		}
	} else {
		rc = update_intf_p->write(start, write_len, pbuf);
	}

	if (rc) {
		printf("write failed, return %d\n", rc);
		return 1;
	}

	unmap_physmem(buf, len);
	return 0;
}
#else
static int au_do_update(int idx, unsigned int sz, int segment,
			void *new_pos, unsigned long sparse_len)
{
	int rc;
	void *buf;
	unsigned char *pbuf;
	unsigned long erase_len;
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
	unsigned long op_sz;
#endif
	if (segment > 0)
		goto write_op;

	if (boot_medium_type == BOOT_FROM_SPI || boot_medium_type == BOOT_FROM_NAND) {
		printf("%s erase...\n", update_intf_p->name);
		erase_len = aufl_layout[idx].end - aufl_layout[idx].start + 1;
		rc = update_intf_p->erase(aufl_layout[idx].start, erase_len);
		debug_print("erase start:%lx erase end:%lx erase len:%lx\n",
			aufl_layout[idx].start, aufl_layout[idx].end, erase_len);
		if (rc) {
			printf("sector erase failed\n");
			return 1;
		}
	}

write_op:
	buf = map_physmem((unsigned long)LOAD_ADDR, map_size, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	/*
	 * write the whole file to flash;uboot and rootfs image have head
	 * kernel has head,it's head also be writed to flash
	 */
	pbuf = buf;

	/* copy the data from RAM to FLASH */
	printf("%s write...\n", update_intf_p->name);

	if (strstr(aufile[idx], "yaffs")) {
		rc = write_yaffs2_fs(idx, sz, pbuf);
		if (rc) {
			printf("write yaffs fs failed\n");
			return rc;
		}
	} else if (strstr(aufile[idx], "ext4")) {
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
		pbuf = is_sparse ? new_pos : (pbuf + BLOCK_SIZE);
		op_sz = is_sparse ? sparse_len : sz;
		rc = write_ext4_fs(idx, pbuf, op_sz);
		if (rc) {
			printf("write ext4 fs failed\n");
			return rc;
		}
#endif
	} else if (strstr(aufile[idx], "ubifs")) {
		rc = write_ubi_fs(idx, sz, pbuf);
		if (rc) {
			printf("write ubi fs failed\n");
			return rc;
		}
	} else {
		rc = write_other_files(idx, sz, pbuf);
		if (rc) {
			printf("write file operation failed\n");
			return rc;
		}
	}
	unmap_physmem(buf, map_size);
	return 0;
}
#endif

/*
 * If none of the update file(u-boot, kernel or rootfs) was found
 * in the medium, return -1;
 * If u-boot has been updated, return 1;
 * Others, return 0;
 */
#ifndef CONFIG_VENDOR_UPGRADE_BY_SEGMENT
static int update_to_flash(void)
{
	int i;
	loff_t sz;
	int res;
	int cnt;
	int uboot_updated = 0;

	/* just loop thru all the possible files */
	for (i = 0; i < AU_MAXFILES && aufile[i] != NULL; i++) {
		printf("\n");

		if (!fat_exists(aufile[i])) {
			printf("%s not found!\n", aufile[i]);
			continue;
		}

		/* get file's real size */
		if (!fat_size(aufile[i], &sz)) {
			aufile_size[i] = ALIGN((unsigned long)sz, CONFIG_SYS_CACHELINE_SIZE);
		} else {
			printf("get size of %s failed!\n", aufile[i]);
			continue;
		}
		printf("aligned size=0x%08lx\n", aufile_size[i]);

		(void)memset_s(LOAD_ADDR, aufile_size[i], 0xff, aufile_size[i]);
		sz = file_fat_read(aufile[i], LOAD_ADDR, (unsigned long)sz);
		debug_print("read %s sz %ld hdr %lu\n",
			  aufile[i], (unsigned long)sz, (unsigned long)sizeof(image_header_t));
		if (sz <= 0) {
			printf("read %s failed!\n", aufile[i]);
			continue;
		}
		/* get file's real size */
		aufile_size[i] = (unsigned long)sz;
		printf("size=0x%08lx\n", aufile_size[i]);

		/* If u-boot had been updated, we need to
		 * save current env to flash */
		if (strcmp((char *)AU_FIRMWARE, aufile[i]) == 0)
			uboot_updated = 1;

		/* this is really not a good idea, but it's what the */
		/* customer wants. */
		cnt = 0;
		do {
			res = au_do_update(i, (unsigned long)sz);
			/* let the user break out of the loop */
			if (ctrlc() || had_ctrlc()) {
				clear_ctrlc();

				break;
			}
			cnt++;
		} while (res < 0);
	}

	if (uboot_updated == 1)
		return 1;
	else
		return -1;
}
#else
void get_sparse_header(int file_idx, struct seg_update_parm *update_parm)
{
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
	int ret;
	if (strstr(aufile[file_idx], "ext4") || strstr(aufile[file_idx], "ufs")) {
		ret = file_fat_read_at(aufile[file_idx],
			update_parm->pos, LOAD_ADDR, BLOCK_SIZE, &update_parm->actread);
		if (ret)
			printf("read %s failed, actread:%lld\n",
					aufile[file_idx], update_parm->actread);
		(void)memset_s(&sparse_header, sizeof(sparse_header_t), 0, sizeof(sparse_header_t));
		get_unspare_header_info(LOAD_ADDR, &sparse_header, &is_sparse);
	}
#endif

	return;
}

void get_section_size(int file_idx, struct seg_update_parm *update_parm)
{
#ifdef CONFIG_CMD_NAND
	if (strstr(aufile[file_idx], "yaffs2")) {
		if (update_parm->sz > SECTION_SIZE)
			update_parm->section_size = (SECTION_SIZE / (nand_flash->writesize +
		nand_flash->oobsize)) * (nand_flash->writesize + nand_flash->oobsize);
		else
			update_parm->section_size = update_parm->sz;
	debug_print("pagesize:%u, oobsize:%u, section_size:%u\n", nand_flash->writesize,
		nand_flash->oobsize, update_parm->section_size);
	} else {
		update_parm->section_size = SECTION_SIZE;
	}
#endif

#ifndef CONFIG_CMD_NAND
	update_parm->section_size = SECTION_SIZE;
#endif

	return;
}

void loop_au_do_update(int file_idx, unsigned int opsz, int segment,
		void *buf, unsigned long sparse_len)
{
	int cnt;
	int res;

	do {
		res = au_do_update(file_idx, opsz, segment, buf, sparse_len);
		/* let the user break out of the loop */
		if (ctrlc() || had_ctrlc()) {
			clear_ctrlc();

			break;
		}
		cnt++;
	} while (res < 0);

	return;
}

void *handle_sparse_file(int file_idx, loff_t *pos,
		unsigned long *sparse_len, void *buff)
{
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
	void *new_buf = NULL;
	if (buff == NULL)
		return NULL;
	if (strstr(aufile[file_idx], "ext4") || strstr(aufile[file_idx], "ufs")) {
		if (!is_sparse)
			return buff;

		if (aufile_size[file_idx] <= SECTION_SIZE)
			return buff;

		*sparse_len = 0;
		if (aufile_size[file_idx] > SECTION_SIZE) {
			new_buf = get_buffer_chunk_layout(buff, pos, sparse_len,
					aufile_size[file_idx]);
			if (new_buf == NULL) {
				printf("Failed to parse the buffer.\n");
				return NULL;
			}
			return new_buf;
		}
	}
#endif
	return buff;
}
void segment_data_save(int file_idx, struct seg_update_parm *update_parm)
{
	unsigned int opsz;
	int ret;
	void *new_pos = NULL;
	void *buff = NULL;
	unsigned long sparse_len = 0;
	int segment;

	if (strstr(aufile[file_idx], "ext4") || strstr(aufile[file_idx], "ufs"))
		buff = LOAD_ADDR + BLOCK_SIZE;
	else
		buff = LOAD_ADDR;

	for (segment = 0; update_parm->pos != aufile_size[file_idx]; segment++) {
		if ((segment == update_parm->nsect) && update_parm->remain)
			opsz = update_parm->remain;
		else
			opsz = update_parm->section_size;

		printf("\nnsect:%d, remain:%lld, section_size:%u, pos:%llu, opsz:%u, map_size:%llu\n",
				update_parm->nsect, update_parm->remain,
				update_parm->section_size, update_parm->pos, opsz, map_size);
		(void)memset_s(LOAD_ADDR, map_size, 0xff, map_size);
		ret = file_fat_read_at(aufile[file_idx], update_parm->pos, buff, opsz,
				&update_parm->actread);
		if (ret)
			printf("read %s failed,opsz:%d,actread:%lld,pos:%lld\n",
				aufile[file_idx], opsz, update_parm->actread, update_parm->pos);
		new_pos = handle_sparse_file(file_idx, &update_parm->pos, &sparse_len, buff);
		if (new_pos == NULL)
			return;
		printf("reading %d part of %s ,size:%u, start:%lld\n", segment,
				aufile[file_idx], opsz, update_parm->pos);
		update_parm->updatefile_found = 1;

		/* If u-boot had been updated, we need to
		 * save current env to flash */
		if (strcmp((char *)AU_FIRMWARE, aufile[file_idx]) == 0)
			update_parm->uboot_updated = 1;

		loop_au_do_update(file_idx, opsz, segment, new_pos, sparse_len);
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
		if (strstr(aufile[file_idx], "ext4") || strstr(aufile[file_idx], "ufs")) {
			if (!is_sparse)
				update_parm->pos += opsz;
		} else {
			update_parm->pos += opsz;
		}
#else
		update_parm->pos += update_parm->section_size;
#endif
	}
	return;
}

static int update_to_flash(void)
{
	int i;
	struct seg_update_parm update_parm = {0};
#if (defined CONFIG_EMMC) || (defined CONFIG_CMD_UFS)
	(void)memset_s(&chunk_header, sizeof(sparse_header_t), 0, sizeof(sparse_header_t));
#endif
	/* just loop thru all the possible files */
	for (i = 0; i < AU_MAXFILES && aufile[i] != NULL; i++) {
		printf("\n\n");

		update_parm.pos = 0;
		update_parm.extra = 0;
		if (!fat_exists(aufile[i])) {
			printf("%s not found!\n", aufile[i]);
			continue;
		}

		/* get file's real size */
		if (!fat_size(aufile[i], (loff_t *)&update_parm.sz)) {
			aufile_size[i] = update_parm.sz;
		} else {
			printf("get size of %s failed!\n", aufile[i]);
			continue;
		}
		printf("filesize size= %ld\n", aufile_size[i]);
		if (strstr(aufile[i], "ext4") || strstr(aufile[i], "ufs"))
			map_size = SECTION_SIZE + BLOCK_SIZE;
		else
			map_size = SECTION_SIZE;
		(void)memset_s(LOAD_ADDR, map_size, 0xff, map_size);

		get_sparse_header(i, &update_parm);
		get_section_size(i, &update_parm);

		update_parm.nsect = aufile_size[i] / update_parm.section_size;
		update_parm.remain = aufile_size[i] % update_parm.section_size;

		if (update_parm.remain)
			update_parm.extra = 1;
		segment_data_save(i, &update_parm);
	}

	if (update_parm.uboot_updated == 1)
		return 1;
	else if (update_parm.updatefile_found == 1)
		return 0;
	else
		return -1;
}
#endif

#define ENV_MAX_LEN    (CONFIG_MAX_SIZE / 2)

/*
 * pick up env form config file and set env
 * fail:return -1;
 * ok:  return 0;
 */
static int env_pick_up(const char *envname, const char *buffer)
{
	char *str, *str_s, *str_e;
	char env[ENV_MAX_LEN];

	str = strstr(buffer, envname);
	if (!str)
		goto env_err;

	str_s = strchr(str, '\'');
	if (!str_s)
		goto env_err;

	str_e = strchr(++str_s, '\'');
	if (!str_e)
		goto env_err;

	if ((unsigned long)(str_e - str_s) > ENV_MAX_LEN) {
		printf("ERROR:%s too long!\n", envname);
		goto err;
	}

	if (strncpy_s(env, ENV_MAX_LEN, str_s, (size_t)(str_e - str_s))) {
		printf("strncpy_s error!\n");
		goto err;
	}

	env[(size_t)(str_e - str_s)] = '\0';
	env_set((char *)envname, env);

	return 0;

env_err:
	printf("ERROR:%s not found!\n", envname);
err:
	return -1;
}

/*
 * pick up bootargs and bootcmd from config file and save env
 * fail:return -1;
 * ok:  return 0;
 */
static int get_env_from_config(void)
{
	char *aufile = AU_CONFIG;
	int ret;
	long sz = file_fat_read(aufile, (void *)LOAD_ADDR, CONFIG_MAX_SIZE);
	if (sz <= 0) {
		printf("ERROR:%s not found!\n", aufile);
		goto err;
	}

	ret = env_pick_up("bootargs", (char *)LOAD_ADDR);
	if (ret < 0)
		goto err;

	ret = env_pick_up("bootcmd", (char *)LOAD_ADDR);
	if (ret < 0)
		goto err;

	return 0;
err:
	return -1;
}

struct mtd_part_s {
	char *str;
	int unit;
	struct mtd_part_s *next;
};

/*
 * insert node to list
 */
struct mtd_part_s *list_insert(struct mtd_part_s *head, struct mtd_part_s *node)
{
	struct mtd_part_s *p = head;

	if (!head)
		head = node;

	while (p) {
		if (p->next == NULL) {
			p->next = node;
			break;
		}
		p = p->next;
	}

	return head;
}

/*
 * sort list by str
 */
struct mtd_part_s *mtd_list_sort(struct mtd_part_s *head)
{
	int flag;
	struct mtd_part_s *p, *pt1, *pt2;

	if (!head)
		return NULL;

	do {
		flag = 0;
		if (head->next == NULL)
			return head;

		if ((uintptr_t)(head->str) >= (uintptr_t)
				(head->next->str)) {
			pt1 = head->next->next;
			pt2 = head->next;
			pt2->next = head;
			head->next = pt1;
			head = pt2;
		}

		for (p = head; p->next->next != NULL; p = p->next) {
			if ((uintptr_t)(p->next->str) >= (uintptr_t)
					(p->next->next->str)) {
				pt1 = p->next->next->next;
				pt2 = p->next->next;
				pt2->next = p->next;
				p->next->next = pt1;
				p->next = pt2;
				flag = 1;
			}
		}
	} while (flag);

	return head;
}

/*
 * get mtd parttion info list from env string
 */
struct mtd_part_s *get_mtd_parts(char *env)
{
	char *str = NULL;
	struct mtd_part_s *part_p = NULL;
	struct mtd_part_s *head = NULL;

	if (env == NULL) {
		printf("env is null!\n");
		return NULL;
	}

	str = env;
	while ((str = strstr(str, "M("))) {
		part_p = malloc(sizeof(struct mtd_part_s));
		if (part_p == NULL)
			return NULL;

		part_p->str = str;
		part_p->unit = SIZE_M;
		part_p->next = NULL;
		head = list_insert(head, part_p);
		str++;
	}

	str = env;
	while ((str = strstr(str, "K("))) {
		part_p = malloc(sizeof(struct mtd_part_s));
		if (part_p == NULL)
			return NULL;

		part_p->str = str;
		part_p->unit = SIZE_K;
		part_p->next = NULL;
		head = list_insert(head, part_p);
		str++;
	}

	head = mtd_list_sort(head);
	return head;
}

unsigned long get_mtd_part_size(const char *str_num)
{
	unsigned long size = 0;
	int k;
	int j = 0;
	int num;
	if (str_num == NULL)
		return -1;

	while (*str_num >= '0' && *str_num <= '9') {
		k = j;
		num = *str_num - '0';
		while (k) {
			num *= DECI_VALUE;
			k--;
		}
		size += num;
		j++;
		str_num--;
	}

	return size;
}

int bootargs_analyze(void)
{
	char *str;
	char *str_0;
	int i;
	long start = 0;
	struct mtd_part_s *part_p = NULL;
	char env[ENV_MAX_LEN];
	char *envp = env_get("bootargs");
	if (envp == NULL) {
		printf("ERROR:bootargs is NULL!\n");
		return -1;
	}

	if (strlen(envp) > ENV_MAX_LEN - 1) {
		printf("ERROR:bootargs is too long!\n");
		return -1;
	}
	if (strcpy_s(env, ENV_MAX_LEN, envp)) {
		printf("%s %d ERR:strcpy_s fail !\n", __func__, __LINE__);
		return -1;
	}
	str = env;
	str_0 = env;
	i = 0;
	part_p = get_mtd_parts(env);

	while (part_p) {
		if (i >= AU_MAXFILES) {
			printf("ERROR:Num of partition is more than %0d!\n",
					AU_MAXFILES);
			break;
		}

		if (part_p->str == NULL)
			return -1;
		str = part_p->str;
		if ((get_mtd_part_size(str - 1) * part_p->unit) <= ULONG_MAX)
			ausize[i] = get_mtd_part_size(str - 1) * part_p->unit;
		else
			return -1;
		aufl_layout[i].start = start;
		aufl_layout[i].end = start + ausize[i] - 1;
		start += ausize[i];

		str += STR_STEPS;
		str_0 = strstr(str, ")");
		if ((unsigned long)(str_0 - str) > NAME_MAX_LEN) {
			printf("file name len is too long\n");
			return -1;
		}
		if (strncpy_s(aufile_table[i], NAME_MAX_LEN, str, (unsigned long)(str_0 - str))) {
			printf("strncpy_s error!\n");
			return -1;
		}
		aufile_table[i][str_0 - str] = '\0';
		aufile[i] = &(aufile_table[i][0]);
		printf("\n[%0d]=%-16s start=0x%08lx end=0x%08lx size=0x%08lx", i,
				aufile_table[i], (unsigned long)(aufl_layout[i].start),
			   (unsigned long)(aufl_layout[i].end), (unsigned long)ausize[i]);
		i++;
		part_p = part_p->next;
	}

	if (i == 0) {
		printf("ERROR:Can't find valid partition info!\n");
		return -1;
	}
	return 0;
}

#ifdef CONFIG_EMMC
unsigned int dos_start_lba = 0;
/* 0:emmc 1:sd */
int target_dev = 1;
/* the user's upgrade image stores the partition number in the eMMC medium */
int target_paratition;

void *get_target_dev_value(void)
{
	return &target_dev;
}

void *get_target_paratition_dev(void)
{
	return &target_paratition;
}

void *get_dos_start_lba_value(void)
{
	return &dos_start_lba;
}

/*
 * get mtd parttion info list from env string in order
 */
struct mtd_part_s *get_mtd_parts_inorder(char *env, int *total_paratition)
{
	char *str = env;
	struct mtd_part_s *part_p;
	struct mtd_part_s *head = NULL;
	int num = 0;

	str = env;

	if (str == NULL)
		return NULL;

	str = strstr(str, "(");
	while (str != NULL) {
		printf("get_mtd_parts_inorder str=%s\n", str);
		part_p = malloc(sizeof(struct mtd_part_s));
		if (part_p == NULL)
			return NULL;
		if ((*(str - 1) == 'M') || (*(str - 1) == 'm'))
			part_p->unit = SIZE_M;
		else if ((*(str - 1) == 'K') || (*(str - 1) == 'k'))
			part_p->unit = SIZE_K;
		else
			part_p->unit = 1;

		part_p->str = str;
		part_p->next = NULL;
		head = list_insert(head, part_p);

		str++;
		str = strstr(str, "(");
		num++;
	}

	head = mtd_list_sort(head);

	if (total_paratition != NULL)
		*total_paratition = num;

	return head;
}

int bootargs_getend(void)
{
	char *str;
	int i;
	unsigned long start = 0;
	char filename[NAME_MAX_LEN] = {0};
	int cur_target_paratition;
	struct mtd_part_s *part_p = NULL;
	char env[ENV_MAX_LEN];
	char *envp = env_get("bootargs");

	if (envp == NULL) {
		printf("ERROR:bootargs is NULL!\n");
		return -1;
	}

	if (strlen(envp) > ENV_MAX_LEN - 1) {
		printf("ERROR:bootargs is too long!\n");
		return -1;
	}

	if (strcpy_s(env, ENV_MAX_LEN, envp)) {
		printf("%s %d ERR:strcpy_s fail !\n", __func__, __LINE__);
		return -1;
	}
	str = env;
	i = 0;
	part_p = get_mtd_parts_inorder(env, NULL);
	cur_target_paratition = target_paratition;

	while (part_p) {
		if (i >= AU_MAXFILES) {
			printf("ERROR:Num of partition is more than %0d!\n", AU_MAXFILES);
			break;
		}
		str = part_p->str;
		start += get_mtd_part_size(str - 1) * part_p->unit;
		str += 1;
		if (strstr(str, ")") == NULL || (unsigned long)(strstr(str, ")") - str) >
				(NAME_MAX_LEN - 1)) {
			printf("file name is more than NAME_MAX_LEN\n");
			return -1;
		}
		if (strncpy_s(filename, NAME_MAX_LEN, str, (unsigned long)(strstr(str, ")") - str))) {
			printf("strncpy_s error!\n");
			return -1;
		}
		filename[strstr(str, ")") - str] = '\0';

		printf("\n[%0d]=%-16s start=0x%08lx", i, filename, start);
		i++;
		part_p = part_p->next;
		if (i >= cur_target_paratition)
			break;
	}

	if (i == 0) {
		printf("ERROR:Can't find valid partition info!\n");
		dos_start_lba = 0;
		return -1;
	}

	if (target_dev == 0)
		/* emmc device */
		dos_start_lba = start >> EMMC_BLOCK_SHIFT;
	else
		/* sd device */
		dos_start_lba = 0;

	printf("\n dos_start_lba = %d\n", dos_start_lba);
	return 0;
}
#endif

void get_boot_info(void)
{
	unsigned int reg;
	reg = readl((void *)(SYS_CTRL_REG_BASE + REG_SYSSTAT));
	if (get_sys_boot_mode(reg) == BOOT_FROM_SPI_NAND) {
		boot_medium_type = BOOT_FROM_NAND;
		update_intf_p = &update_intf[UPDATE_MEDIUM_NAND];
	} else if (get_sys_boot_mode(reg) == BOOT_FROM_SPI) {
		boot_medium_type = BOOT_FROM_SPI;
		update_intf_p = &update_intf[UPDATE_MEDIUM_SPINOR];
	} else if (get_sys_boot_mode(reg) == BOOT_FROM_NAND) {
		boot_medium_type = BOOT_FROM_NAND;
		update_intf_p = &update_intf[UPDATE_MEDIUM_NAND];
	} else if (get_sys_boot_mode(reg) == BOOT_FROM_EMMC) {
		boot_medium_type = BOOT_FROM_EMMC;
		update_intf_p = &update_intf[UPDATE_MEDIUM_EMMC];
	}
}

struct blk_desc *detect_external_storage(int dev_index)
{
	struct blk_desc *stor_dev;
	int dev = 0;

	if (s_intf[dev_index].init == NULL)
		return NULL;

	au_stor_curr_dev = -1;
	au_stor_curr_dev = s_intf[dev_index].init();
	if (au_stor_curr_dev == -1) {
		printf("%s storage device not found!\n", s_intf[dev_index].name);
		return NULL;
	}
	printf("%s storage device found!\n", s_intf[dev_index].name);
	if (strncmp("mmc", s_intf[dev_index].name, sizeof("mmc")) == 0) {
#ifdef CONFIG_EMMC
		dev = target_dev;
#else
		dev = 0;
#endif
	}
	stor_dev = blk_get_dev(s_intf[dev_index].name, dev);
	if (stor_dev == NULL) {
		printf("Unknow device type!\n");
		return NULL;
	}

	return stor_dev;
}
/*
 * This is called by board_init() after the hardware has been set up
 * and is usable.
 * return -1, otherwise it will return 0;
 */
int do_auto_update(void)
{
	struct blk_desc *stor_dev = NULL;
	int old_ctrlc;
	int j;
	int state;

	for (j = 0; j < MAX_UPDATE_INTF; j++) {
		stor_dev = detect_external_storage(j);
		if (stor_dev == NULL)
			continue;

#ifdef CONFIG_EMMC
		if (target_dev == 0) {
			/* emmc upgrade */
			bootargs_getend();
			/* re-do the entry->test(dev_desc) to set the
			 * dev_desc->part_type to PART_TYPE_DOS
			 */
			part_init(stor_dev);
		}
#endif

		if (fat_register_device(stor_dev, 1) != 0) {
			debug_print("Unable to use %s %d:%d for fatls\n", s_intf[j].name,
					au_stor_curr_dev, 1);
			continue;
		}

		if (file_fat_detectfs() != 0) {
			debug_print("file_fat_detectfs failed\n");
			continue;
		}
		get_boot_info();
		update_intf_p->init();

		if (get_env_from_config()) {
			auto_update_flag = 0;
			printf("Warning:no config! Try to use old env!\n");
		};

		if (bootargs_analyze()) {
			printf("ERROR:bootargs analyze fail!\n");
			continue;
		}

		/* make sure that we see CTRL-C and save the old state */
		old_ctrlc = disable_ctrlc(0);
		state = update_to_flash();
		/* restore the old state */
		disable_ctrlc(old_ctrlc);

		s_intf[j].exit();
		if (state == -1)
			continue;
		/* update files have been found on current medium, so just break here */
		break;
	}

	/* no update file found */
	if (state == -1)
		return -1;
	env_save();
	return 0;
}

#endif /* CONFIG_AUTO_UPDATE */
