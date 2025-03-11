// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include <command.h>
#include <div64.h>
#include <mmc.h>
#include <image-sparse.h>
#include <securec.h>

#define FILLBUF_SIZE               4096

#define EMMC_BLKSIZE_SHIFT         9
#define SZ_1M_SHIFT                20
#define FILLBUF_NUM_MMC_BLKS       FILLBUF_SIZE >> EMMC_BLKSIZE_SHIFT

#ifdef CONFIG_CMD_UFS
#include <ufs.h>
#define UFS_BLKSIZE_SHIFT          12
#define FILLBUF_NUM_UFS_BLKS       FILLBUF_SIZE >> UFS_BLKSIZE_SHIFT
#endif

/* Open it as you need #define DEBUG_EXT4 */

void print_header_info(const sparse_header_t *header)
{
#ifdef DEBUG_EXT4
	printf("sparse header info:\n");
	printf("magic: 0x%x\n", header->magic);
	printf("major_version: 0x%x\n", header->major_version);
	printf("minor_version: 0x%x\n", header->minor_version);
	printf("file_hdr_sz: %d\n", header->file_hdr_sz);
	printf("chunk_hdr_sz: %d\n", header->chunk_hdr_sz);
	printf("blk_sz: %d\n", header->blk_sz);
	printf("total_blks: %d\n", header->total_blks);
	printf("total_chunks: %d\n", header->total_chunks);
	printf("image_checksum: %d\n", header->image_checksum);
#endif
}

void print_chunk_info(const chunk_header_t *chunk)
{
#ifdef DEBUG_EXT4
	printf("\n");
	printf("chunk header info:\n");
	printf("chunk_type: 0x%x\n", chunk->chunk_type);
	printf("chunk_sz: %d\n", chunk->chunk_sz);
	printf("total_sz: %d\n", chunk->total_sz);
#endif
}

int get_unspare_header_info(const u8 *pbuf, sparse_header_t *sparse_header, char* is_sparse)
{
	(void)memcpy_s(sparse_header, sizeof(sparse_header_t), pbuf, sizeof(sparse_header_t));

	if (!is_sparse_image(sparse_header)) {
		*is_sparse = 0;
		return 0;
	}
	*is_sparse = 1;
	print_header_info(sparse_header);
	return 0;
}
static int check_parameter(sparse_header_t *header, u32 cnt, unsigned int type)
{
	u64 img_size;

	if (!is_sparse_image(header)) {
		printf("Invalid sparse format.\n");
		return 1;
	}

	/* check header->blk_sz align to emmc block size */
	if (type == 0) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
		if (header->blk_sz & ((1 << EMMC_BLKSIZE_SHIFT) - 1)) {
			printf("image blk size %d is not aligned to 512Byte.\n", header->blk_sz);
			return 1;
		}
#ifdef CONFIG_CMD_UFS
	if (type == 1) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
		if (header->blk_sz & ((1 << UFS_BLKSIZE_SHIFT) - 1)) {
			printf("image blk size %d is not aligned to 512Byte.\n", header->blk_sz);
			return 1;
		}
#endif

	/* check fs img's real size is larger than partition size */
	img_size = (u64)(header->total_blks * header->blk_sz);
	if (type == 0) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
		if ((img_size >> EMMC_BLKSIZE_SHIFT) > (u64)cnt) {
			printf("partition size %d MB not enough.\n",
			(int)(cnt >> (SZ_1M_SHIFT - EMMC_BLKSIZE_SHIFT)));
			print_header_info(header);
			return 1;
		}
#ifdef CONFIG_CMD_UFS
	if (type == 1) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
		if ((img_size >> UFS_BLKSIZE_SHIFT) > (u64)cnt) {
			printf("partition size %d MB not enough.\n",
			(int)(cnt >> (SZ_1M_SHIFT - UFS_BLKSIZE_SHIFT)));
			print_header_info(header);
			return 1;
		}
#endif
	return 0;
}

static void print_writing_complete(u64 sparse_len, s32 *percent_complete, u32 cnt, u32 blk, unsigned int type)
{
	u64 n;
	int percent;

	if (type == 0) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
		n = (u64)(sparse_len >> EMMC_BLKSIZE_SHIFT) * 100;  /* 100: Percentage for complete */
#ifdef CONFIG_CMD_UFS
	if (type == 1) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
		n = (u64)(sparse_len >> UFS_BLKSIZE_SHIFT) * 100; /* 100: Percentage for complete */
#endif

	do_div(n, cnt);
	percent = (int)n;
	if (percent != *percent_complete) {
		*percent_complete = percent;
		if (type == 0) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
			printf("\rWriting at %d blk# "
				"-- %3d%% complete. \n",
			(int)(blk + (sparse_len >> EMMC_BLKSIZE_SHIFT)), percent);
#ifdef CONFIG_CMD_UFS
		if (type == 1) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
			printf("\rWriting at %d blk# "
				       "-- %3d%% complete.\n",
				       (int)(blk + (sparse_len >> UFS_BLKSIZE_SHIFT)), percent);
#endif
	}
}

static int check_print_results(u64 dense_len, u64 sparse_len, u32 cnt, unsigned int type)
{
	if (type == 0) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
		if (((u64)cnt << EMMC_BLKSIZE_SHIFT) != sparse_len) {
			printf("error: partition size %d MB != ext4 image size %d MB\n",
			(int)(cnt >> (SZ_1M_SHIFT - EMMC_BLKSIZE_SHIFT)),
			(int)(sparse_len >> SZ_1M_SHIFT));
			return 1;
		}
#ifdef CONFIG_CMD_UFS
		if (type == 1) /* 0: ext4_unsparse   1: ufs_ext4_unsparse */
			if (((u64)cnt << UFS_BLKSIZE_SHIFT) != sparse_len) {
				printf("error: partition size %d MB != ext4 image size %d MB\n",
				(int)(cnt >> (SZ_1M_SHIFT - UFS_BLKSIZE_SHIFT)),
				(int)(sparse_len >> SZ_1M_SHIFT));
				return 1;
			}
#endif

	printf("sparse: %d MB / %d MB.\n", (int)(dense_len >> SZ_1M_SHIFT), (int)(sparse_len >> SZ_1M_SHIFT));
	return 0;
}

int ext4_unsparse(struct mmc *mmc, u32 dev, u8 *pbuf, u32 blk, u32 cnt)
{
	u32 i, num;
	u64 dense_len = 0;
	u64 sparse_len = 0;
	u32 chunk_len;
	s32 percent_complete = -1;
	chunk_header_t *chunk = NULL;
	uint32_t fill_buf[FILLBUF_SIZE / sizeof(uint32_t)];
	sparse_header_t *header = (sparse_header_t *)pbuf;

	check_parameter(header, cnt, 0);

	/* skip the sparse header,to visit first chunk */
	pbuf += header->file_hdr_sz;

	/* to visit each chunk */
	for (i = 0; i < header->total_chunks; i++) {
		/* here the chunk_header */
		chunk = (chunk_header_t *)pbuf;

		/* go to next chunk's data */
		pbuf += header->chunk_hdr_sz;

		switch (chunk->chunk_type) {
		case CHUNK_TYPE_RAW:

			/* to calculate the length of each chunk */
			chunk_len = chunk->chunk_sz * header->blk_sz;

			/* verify every chunk to asure it is valid */
			if (chunk->total_sz != (chunk_len + header->chunk_hdr_sz)) {
				printf("No.%d chunk size error.\n", i);
				print_chunk_info(chunk);
				return 1;
			}

			dense_len += chunk_len;
			sparse_len += chunk_len;

			if (sparse_len > ((u64)cnt << EMMC_BLKSIZE_SHIFT)) {
				printf("error: sparse size %d MB is "
				       "larger than partiton size %d MB.\n",
				       (int)(sparse_len >> SZ_1M_SHIFT),
				       (int)(cnt >> (SZ_1M_SHIFT - EMMC_BLKSIZE_SHIFT)));
				print_chunk_info(chunk);
				return 1;
			}

			num = blk_dwrite(mmc_get_blk_desc(mmc), blk,
					 (chunk_len >> EMMC_BLKSIZE_SHIFT), pbuf);
			if (num != (chunk_len >> EMMC_BLKSIZE_SHIFT)) {
				printf("Raw data: No.%d blocks written: %s.\n", num, "ERROR");
				return 1;
			}

			pbuf += chunk_len;
			blk  += (chunk_len >> EMMC_BLKSIZE_SHIFT);
			break;

		case CHUNK_TYPE_DONT_CARE:
			if (chunk->total_sz != header->chunk_hdr_sz) {
				printf("No.%d chunk size error.\n", i);
				print_chunk_info(chunk);
				return 1;
			}

			chunk_len = chunk->chunk_sz * header->blk_sz;
			sparse_len += chunk_len;

			if (sparse_len > ((u64)cnt << EMMC_BLKSIZE_SHIFT)) {
				printf("error: sparse size %d MB is "
				       "larger than partiton size %d MB.\n",
				       (int)(sparse_len >> SZ_1M_SHIFT),
				       (int)(cnt >> (SZ_1M_SHIFT - EMMC_BLKSIZE_SHIFT)));
				print_chunk_info(chunk);
				return 1;
			}

			blk  += (chunk_len >> EMMC_BLKSIZE_SHIFT);
			break;

		case CHUNK_TYPE_FILL: {
			uint32_t fill_val = *(uint32_t *)pbuf;
			u32 blkcnt, blk_writed, j;

			pbuf = (u8 *)pbuf + sizeof(uint32_t);

			if (chunk->total_sz != (header->chunk_hdr_sz + sizeof(uint32_t))) {
				print_chunk_info(chunk);
				printf("Bogus chunk size for chunk type FILL");
				return 1;
			}

			for (j = 0; j < FILLBUF_SIZE / sizeof(uint32_t); j++)
				fill_buf[j] = fill_val;

			chunk_len = chunk->chunk_sz * header->blk_sz;
			blkcnt = chunk_len >> EMMC_BLKSIZE_SHIFT;
			if (blk + blkcnt > blk + cnt) {
				print_chunk_info(chunk);
				printf(
				    "%s: Request would exceed partition size!\n",
				    __func__);
				return 1;
			}

			for (blk_writed = 0; blk_writed < blkcnt;) {
				unsigned int write_blks = FILLBUF_NUM_MMC_BLKS < blkcnt - blk_writed ? blkcnt - blk_writed : FILLBUF_NUM_MMC_BLKS;
				num = blk_dwrite(mmc_get_blk_desc(mmc), blk,
						write_blks, fill_buf);
				if (num != write_blks) {
					print_chunk_info(chunk);
					printf("Raw data: No.%d blocks written: %s.\n", num, "ERROR");
					return 1;
				}
				blk_writed += write_blks;
				blk  += write_blks;
			}
			sparse_len += chunk_len;
			break;
		}
		default:
			printf("sparse: unknow chunk type %04x.\n",
			       chunk->chunk_type);
			return 1;
		}
		print_writing_complete(sparse_len, &percent_complete, cnt, blk, 0);
	}
	puts("\n");

	if (check_print_results(dense_len, sparse_len, cnt, 0))
		return 1;

	return 0;
}

#ifdef CONFIG_CMD_UFS

int ufs_ext4_unsparse(const u8 *pbuf, u32 blk, u32 cnt)
{
	u32 i, num;
	u64 dense_len = 0;
	u64 sparse_len = 0;
	u32 chunk_len;
	s32 percent_complete = -1;
	chunk_header_t *chunk = NULL;
	sparse_header_t *header = (sparse_header_t *)pbuf;
	uint32_t fill_buf[FILLBUF_SIZE / sizeof(uint32_t)];

	check_parameter(header, cnt, 1);
	/* skip the sparse header,to visit first chunk */
	pbuf += header->file_hdr_sz;

	/* to visit each chunk */
	for (i = 0; i < header->total_chunks; i++) {
		/* here the chunk_header */
		chunk = (chunk_header_t *)pbuf;
		/* go to next chunk's data */
		pbuf += header->chunk_hdr_sz;

		switch (chunk->chunk_type) {
		case CHUNK_TYPE_RAW:

			/* to calculate the length of each chunk */
			chunk_len = chunk->chunk_sz * header->blk_sz;

			/* verify every chunk to asure it is valid */
			if (chunk->total_sz
			    != (chunk_len + header->chunk_hdr_sz)) {
				printf("No.%d chunk size error.\n", i);
				print_chunk_info(chunk);
				return 1;
			}

			dense_len += chunk_len;
			sparse_len += chunk_len;

			if (sparse_len > ((u64)cnt << UFS_BLKSIZE_SHIFT)) {
				printf("error: sparse size %d MB is "
				       "larger than partiton size %d MB.\n",
				       (int)(sparse_len >> SZ_1M_SHIFT),
				       (int)(cnt >> (SZ_1M_SHIFT - UFS_BLKSIZE_SHIFT)));
				print_chunk_info(chunk);
				return 1;
			}

			if (ufs_write_storage((uint64_t)pbuf,
					      (blk << UFS_BLKSIZE_SHIFT),
					      chunk_len) == 0)
				num = chunk_len >> UFS_BLKSIZE_SHIFT;
			else
				num = 0;

			if (num != (chunk_len >> UFS_BLKSIZE_SHIFT)) {
				printf("Raw data: No.%d blocks written: %s.\n",
				       num, "ERROR");
				return 1;
			}

			pbuf += chunk_len;
			blk  += (chunk_len >> UFS_BLKSIZE_SHIFT);
			break;

		case CHUNK_TYPE_DONT_CARE:
			if (chunk->total_sz != header->chunk_hdr_sz) {
				printf("No.%d chunk size error.\n", i);
				print_chunk_info(chunk);
				return 1;
			}

			chunk_len = chunk->chunk_sz * header->blk_sz;
			sparse_len += chunk_len;

			if (sparse_len > ((u64)cnt << UFS_BLKSIZE_SHIFT)) {
				printf("error: sparse size %d MB is "
				       "larger than partiton size %d MB.\n",
				       (int)(sparse_len >> SZ_1M_SHIFT),
				       (int)(cnt >> (SZ_1M_SHIFT - UFS_BLKSIZE_SHIFT)));
				print_chunk_info(chunk);
				return 1;
			}

			blk  += (chunk_len >> UFS_BLKSIZE_SHIFT);
			break;
		case CHUNK_TYPE_FILL: {
			uint32_t fill_val = *(uint32_t *)pbuf;
			u32 blkcnt, blk_writed, j;

			pbuf = (u8 *)pbuf + sizeof(uint32_t);

			if (chunk->total_sz != (header->chunk_hdr_sz + sizeof(uint32_t))) {
				print_chunk_info(chunk);
				printf("Bogus chunk size for chunk type FILL");
				return 1;
			}

			for (j = 0; j < FILLBUF_SIZE / sizeof(uint32_t); j++)
				fill_buf[j] = fill_val;

			chunk_len = chunk->chunk_sz * header->blk_sz;
			blkcnt = chunk_len >> EMMC_BLKSIZE_SHIFT;
			if (blk + blkcnt > blk + cnt) {
				print_chunk_info(chunk);
				printf(
				    "%s: Request would exceed partition size!\n",
				    __func__);
				return 1;
			}

			for (blk_writed = 0; blk_writed < blkcnt;) {
				unsigned int write_blks = FILLBUF_NUM_MMC_BLKS < blkcnt - blk_writed ? blkcnt - blk_writed : FILLBUF_NUM_MMC_BLKS;
				num = blk_dwrite(mmc_get_blk_desc(mmc), blk,
						write_blks, fill_buf);
				if (num != write_blks) {
					print_chunk_info(chunk);
					printf("Raw data: No.%d blocks written: %s.\n", num, "ERROR");
					return 1;
				}
				blk_writed += write_blks;
				blk  += write_blks;
			}
			sparse_len += chunk_len;
			break;
		}
		default:
			printf("sparse: unknown chunk type %04x.\n",
			       chunk->chunk_type);
			return 1;
		}

		print_writing_complete(sparse_len, &percent_complete, cnt, blk, 1);
	}
	puts("\n");

	if (check_print_results(dense_len, sparse_len, cnt, 1))
		return 1;

	return 0;
}
#endif
