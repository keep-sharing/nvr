// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __UNSPARSE__
#define __UNSPARSE__

int get_unspare_header_info(const u8 *pbuf, sparse_header_t *sparse_header, char *is_sparse);
void print_chunk_info(chunk_header_t *chunk);
int ext4_unsparse(struct mmc *mmc, u32 dev, u8 *pbuf, u32 blk, u32 cnt);
void print_header_info(sparse_header_t *header);
#endif
