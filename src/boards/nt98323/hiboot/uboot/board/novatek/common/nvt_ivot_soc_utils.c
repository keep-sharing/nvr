/**
    NVT utilities for command customization

    @file       na51055_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <common.h>
#include <fs.h>
#include <asm/io.h>
#include <nand.h>
#include <mmc.h>
#include <spi_flash.h>
#include <lzma/LzmaTools.h>
#include <asm/nvt-common/modelext/emb_partition_info.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <asm/arch/crypto.h>
#include "nvt_ivot_soc_utils.h"
#include "nvt_ivot_pack.h"
#include <u-boot/zlib.h>
#include <stdlib.h>


#define FW_PART1_SIZE_OFFSET (RTOS_CODEINFO_OFFSET + 0x1C) //ref to CodeInfo.S on rtos (addr of _section_01_size)

#define UINT32_SWAP(data)           (((((UINT32)(data)) & 0x000000FF) << 24) | \
									 ((((UINT32)(data)) & 0x0000FF00) << 8) | \
									 ((((UINT32)(data)) & 0x00FF0000) >> 8) | \
									 ((((UINT32)(data)) & 0xFF000000) >> 24))   ///< Swap [31:24] with [7:0] and [23:16] with [15:8].

extern HEADINFO gHeadInfo;

int atoi(const char *str)
{
	return (int)simple_strtoul(str, '\0', 10);
}



#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT) || defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) || defined(CONFIG_NVT_LINUX_EMMC_BOOT)
int nvt_part_config(char *p_cmdline, EMB_PARTITION *partition_ptr)
{
	EMB_PARTITION *p_partition = NULL;
	uint32_t i, part_num, pst_part_num, ro_attr;
	int exist_rtos = 0;
	int ret = 0;
	char cmd[CONFIG_CMDLINE_SIZE];
	char buf[40];

#if defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) || defined(CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT)
#if defined(CONFIG_NVT_SPI_NAND) || defined(CONFIG_NVT_SPI_NOR_NAND)
	struct mtd_info *nand = nand_info[nand_curr_device];
#elif defined(CONFIG_NVT_IVOT_EMMC)
	struct mmc *mmc = find_mmc_device(CONFIG_NVT_IVOT_EMMC);
#endif /* CONFIG_NVT_SPI_NAND */
#endif /* CONFIG_NVT_LINUX_RAMDISK_BOOT || CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	/* MMC device doesn't need to init mtdparts parameter */
	if (p_cmdline == NULL) {
		return 0;
	}
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */

	if (partition_ptr == NULL) {
		p_partition = emb_partition_info_data_curr;
		ret = nvt_getfdt_emb((ulong)nvt_fdt_buffer, p_partition);
		if (ret < 0) {
			printf("failed to get partition resource.\r\n");
			return -1;
		}
	} else {
		/* Receive partition table from argu */
		p_partition = partition_ptr;
	}

#if defined(CONFIG_NVT_SPI_NAND)
	sprintf(cmd, "nand0=spi_nand.0");
	ret = setenv("mtdids", cmd);
	if (ret) {
		printf("%s: error set\n", __func__);
		return ret;
	}

	sprintf(cmd, "mtdparts=spi_nand.0:");
#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(cmd, "nor0=spi_nor.0");
	ret = setenv("mtdids", cmd);
	if (ret) {
		printf("%s: error set\n", __func__);
		return ret;
	}

	sprintf(cmd, "mtdparts=spi_nor.0:");
	if (p_cmdline != NULL) {
		/* To find if mtdparts string existed. If yes, it needs to expand mtdparts environment */
		char *mtdparts_off = NULL;
		mtdparts_off = strstr((char *)p_cmdline, "mtdparts=");
		if (mtdparts_off) {
			p_cmdline = mtdparts_off;
			*p_cmdline = '\0';
		}
		strcat(p_cmdline, "mtdparts=spi_nor.0:");
	}
#elif defined(CONFIG_NVT_SPI_NOR_NAND)
	sprintf(cmd, "nor0=spi_nor.0,nand0=spi_nand.0");
	ret = setenv("mtdids", cmd);
	if (ret) {
		printf("%s: error set\n", __func__);
		return ret;
	}

	sprintf(cmd, "mtdparts=spi_nand.0:");
#elif defined(CONFIG_NVT_IVOT_EMMC)
	if (p_cmdline != NULL) {
		/* To find if mtdparts string existed. If yes, it needs to expand mtdparts environment */
		char *nvtemmcpart_off = NULL;
		nvtemmcpart_off = strstr((char *)p_cmdline, "nvtemmcpart=");
		if (nvtemmcpart_off) {
			p_cmdline = nvtemmcpart_off;
			*p_cmdline = '\0';
		}
		strcat(p_cmdline, "nvtemmcpart=");
	}
#else
	return 0;
#endif
	part_num = 0;
	pst_part_num = 0;
	ro_attr = 0;
	/* To parse mtdparts for rootfs partition table */
	for (i = 0 ; i < EMB_PARTITION_INFO_COUNT ; i++) {
		const EMB_PARTITION *p = p_partition + i;
		unsigned int PartitionSize = 0, PartitionOffset = 0;

		PartitionSize = p->PartitionSize;
		PartitionOffset = p->PartitionOffset;


		if (p->EmbType != EMBTYPE_UNKNOWN && PartitionSize == 0) {
			printf("%s skip mtdpart config of partition[%d], because its size is 0\n%s", ANSI_COLOR_YELLOW, i, ANSI_COLOR_RESET);
			continue;
		}
		switch (p->EmbType) {
		case EMBTYPE_UITRON:
			sprintf(buf, "0x%x@0x%x(uitron),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				sprintf(buf, "0x%x@0x%x(uitron)ro,", PartitionSize, PartitionOffset);
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_FDT:
			sprintf(buf, "0x%x@0x%x(fdt),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				sprintf(buf, "0x%x@0x%x(fdt)ro,", PartitionSize, PartitionOffset);
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_UBOOT:
			sprintf(buf, "0x%x@0x%x(uboot),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				sprintf(buf, "0x%x@0x%x(uboot)ro,", PartitionSize, PartitionOffset);
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_UENV:
			sprintf(buf, "0x%x@0x%x(uenv),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_LINUX:
			sprintf(buf, "0x%x@0x%x(linux),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				sprintf(buf, "0x%x@0x%x(linux)ro,", PartitionSize, PartitionOffset);
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_RAMFS:
			sprintf(buf, "0x%x@0x%x(ramfs),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_ROOTFS:
			sprintf(buf, "0x%x@0x%x(rootfs%d),", PartitionSize, PartitionOffset, p->OrderIdx);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				strcat(p_cmdline, buf);
				/* Multiple rootfs partitions if OrderIdx > 0 */
				ro_attr = p->OrderIdx;
#endif
			}
			break;
		case EMBTYPE_ROOTFSL:
			sprintf(buf, "0x%x@0x%x(rootfsl%d),", PartitionSize, PartitionOffset, p->OrderIdx);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				strcat(p_cmdline, buf);
				/* Multiple rootfs partitions if OrderIdx > 0 */
				ro_attr = p->OrderIdx;
#endif
			}
			break;
		case EMBTYPE_RTOS:
			exist_rtos = 1;
			sprintf(buf, "0x%x@0x%x(rtos),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				sprintf(buf, "0x%x@0x%x(rtos)ro,", PartitionSize, PartitionOffset);
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_APP:
			sprintf(buf, "0x%x@0x%x(app),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				sprintf(buf, "0x%x@0x%x(app),", PartitionSize, PartitionOffset);
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_TEEOS:
			sprintf(buf, "0x%x@0x%x(teeos),", PartitionSize, PartitionOffset);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
			#if defined(CONFIG_NVT_IVOT_EMMC)
			sprintf(buf, "0x%x@0x%x(teeos),", PartitionSize, PartitionOffset);
			strcat(p_cmdline, buf);
			#endif
			}
			break;
		case EMBTYPE_USER:
			sprintf(buf, "0x%x@0x%x(user%u_%d),", PartitionSize, PartitionOffset, GET_USER_PART_NUM(p->EmbType), p->OrderIdx);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				strcat(p_cmdline, buf);
#endif
			}
			break;
		case EMBTYPE_USERRAW:
			sprintf(buf, "0x%x@0x%x(userraw%u_%d),", PartitionSize, PartitionOffset, GET_USER_PART_NUM(p->EmbType), p->OrderIdx);
			strcat(cmd, buf);
			if (p_cmdline != NULL) {
#if defined(CONFIG_NVT_IVOT_EMMC)
				strcat(p_cmdline, buf);
#endif
			}
			break;
		default:
			break;
		}
	}

	if (p_cmdline != NULL && !exist_rtos) {
		/* To add entire nand mtd device */
#if defined(CONFIG_NVT_IVOT_EMMC)
		sprintf(buf, "0x%llx@0(total),", mmc->capacity / MMC_MAX_BLOCK_LEN);
		strcat(p_cmdline, buf);
#endif
		/*
		 * Add bootarg rootfs extension parameter
		 */
		p_cmdline[strlen(p_cmdline) - 1] = ' ';
		ret = nvt_getfdt_rootfs_mtd_num((ulong)nvt_fdt_buffer, &part_num, &ro_attr);
		if (ret < 0) {
			printf("%s We can't find your root device, \
					please check if your dts is without partition_rootfs name %s\n",
				   ANSI_COLOR_RED, ANSI_COLOR_RESET);
			return -1;
		}
#if (defined(CONFIG_NVT_UBIFS_SUPPORT) || (defined(CONFIG_NVT_SPI_NAND) && defined(CONFIG_NVT_SQUASH_SUPPORT)))
#if (defined(CONFIG_NVT_SPI_NAND) && defined(CONFIG_NVT_SQUASH_SUPPORT))
		ro_attr = 1;
#endif
		if (ro_attr > 0) {
			sprintf(buf, " ubi.mtd=%d ro ", part_num);
		} else {
			sprintf(buf, " ubi.mtd=%d rw ", part_num);
		}
		strcat(p_cmdline, buf);
#elif defined(CONFIG_NVT_EXT4_SUPPORT) || (defined(CONFIG_NVT_LINUX_EMMC_BOOT) && defined(CONFIG_NVT_SQUASH_SUPPORT))
		sprintf(buf, " root=/dev/mmcblk0p%d nvt_pst=/dev/mmcblk0p%d ", part_num, pst_part_num);
		strcat(p_cmdline, buf);
#elif defined(CONFIG_NVT_LINUX_RAMDISK_BOOT)
		printf("RAMDISK boot\n");
#else
		/*
		 * To handle non-ubifs rootfs type (squashfs/jffs2)
		 */
		if (ro_attr > 0) {
			sprintf(buf, " root=/dev/mtdblock%d ro ", part_num);
		} else {
			sprintf(buf, " root=/dev/mtdblock%d ", part_num);
		}
		strcat(p_cmdline, buf);
#endif /* _NVT_ROOTFS_TYPE_ */
	}

	/* To handle uboot mtd env config */
	cmd[strlen(cmd) - 1] = '\0';
	ret = setenv("mtdparts", cmd);

	sprintf(cmd, "printenv mtdparts");
	run_command(cmd, 0);

	return ret;
}
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT || CONFIG_NVT_LINUX_AUTODETECT || CONFIG_NVT_LINUX_SPINOR_BOOT || defined(CONFIG_NVT_LINUX_EMMC_BOOT) */

#ifdef  CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
static int nvt_getfdt_rtos_addr_size(ulong addr, u32 *fdt_rtos_addr, u32 *fdt_rtos_len);
static int nvt_runfw_bin_chk_valid(ulong addr)
{
	unsigned int size = gHeadInfo.Resv1[0];
	NVTPACK_VERIFY_OUTPUT np_verify = {0};
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_MEM mem_in = {(void *)addr, size};

	memset(&np_get_input, 0, sizeof(np_get_input));

	if (nvtpack_verify(&mem_in, &np_verify) != NVTPACK_ER_SUCCESS) {
		printf("verify failed.\r\n");
		return -1;
	}
	if (np_verify.ver != NVTPACK_VER_16072017) {
		printf("wrong all-in-one bin version\r\n");
		return -1;
	}

	return 0;
}

static int nvt_on_partition_enum_copy_to_dest(unsigned int id, NVTPACK_MEM *p_mem, void *p_user_data)
{
	EMB_PARTITION *pEmb = (EMB_PARTITION *)p_user_data;
	u32 src_addr = 0, dst_addr = 0, dst_size = 0;
	//NVTPACK_BFC_HDR *pBFC = (NVTPACK_BFC_HDR *)p_mem->p_data;

	/* Copy to dest address */
	switch (pEmb[id].EmbType) {
	case EMBTYPE_LOADER:
		printf("RUNFW: Ignore T.bin loader\n");
		break;
	case EMBTYPE_FDT:
		dst_addr = (u32)nvt_fdt_buffer;
		dst_size = p_mem->len;
		src_addr = (unsigned int)p_mem->p_data;
		printf("RUNFW: Copy FDT from 0x%08x to 0x%08x with length %d bytes\r\n",
			   (unsigned int)p_mem->p_data, (unsigned int)dst_addr, dst_size);
		break;
	case EMBTYPE_UITRON:
	case EMBTYPE_ECOS:
	case EMBTYPE_UBOOT:
	case EMBTYPE_DSP:
	case EMBTYPE_RTOS:
		break;
	case EMBTYPE_LINUX:
		dst_addr = CONFIG_LINUX_SDRAM_START;
		dst_size = p_mem->len;
		src_addr = (unsigned int)p_mem->p_data;
		printf("RUNFW: Copy Linux from 0x%08x to 0x%08x with length %d bytes\r\n",
			   (unsigned int)p_mem->p_data, (unsigned int)dst_addr, dst_size);
		break;
	case EMBTYPE_ROOTFS:
#ifdef CONFIG_NVT_LINUX_RAMDISK_BOOT
		nvt_ramdisk_size = ALIGN_CEIL(p_mem->len, 4096);
		nvt_ramdisk_addr = _BOARD_LINUX_ADDR_ + _BOARD_LINUX_SIZE_ - nvt_ramdisk_size - 0x10000;
		dst_addr = nvt_ramdisk_addr;
		dst_size = nvt_ramdisk_size;
		src_addr = (unsigned int)p_mem->p_data;
#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
		// Skip nvt head info
		src_addr += 64;
		dst_size -= 64;
#endif
		printf("copy sd to ram: ramdisk addr:%x size:%d\n", dst_addr, dst_size);
		if ((dst_size) > CONFIG_RAMDISK_SDRAM_SIZE) {
			printf("%sRUNFW: The ramdisk image size %d bytes is larger than %d bytes, can't boot. %s\r\n",
				   ANSI_COLOR_RED, dst_size, CONFIG_RAMDISK_SDRAM_SIZE, ANSI_COLOR_RESET);
			printf("%sRUNFW: Please check your modelext dram partition.%s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			while (1);
		} else {
			printf("RUNFW: Copy RAMDISK from 0x%08x to 0x%08x with length %d bytes\r\n",
				   (unsigned int)src_addr, (unsigned int)dst_addr, dst_size);
		}
#endif /* CONFIG_NVT_LINUX_RAMDISK_BOOT */
		break;
	default:
		break;
	}

	if (dst_size != 0) {
		memcpy((void *)dst_addr, (void *)src_addr, dst_size);
		flush_dcache_range((u32)dst_addr, (u32)(dst_addr + dst_size));
	}
	return 0;
}

#define GZ_BUF_SIZE 0x4000 //16KB are enough
unsigned char gz_buf[0x4000] = {0};
unsigned char *mp_gz_buf_curr = gz_buf;
static void *gz_alloc(void *x, unsigned items, unsigned size)
{
	void *p = NULL;
	if (mp_gz_buf_curr + ALIGN_CEIL_4(size) > gz_buf + GZ_BUF_SIZE) {
		printf("gzip temp memory not enough, need more: %d bytes\r\n", ALIGN_CEIL_4(size));
		return p;
	}

	p = (void *)(mp_gz_buf_curr);
	mp_gz_buf_curr += ALIGN_CEIL_4(size);
	return p;
}

static void gz_free(void *x, void *addr, unsigned nb)
{
	mp_gz_buf_curr = gz_buf;
}

static int gz_uncompress(unsigned char *in, unsigned char *out,
						 unsigned int insize, unsigned int outsize)
{
	int err;
	z_stream stream = {0};
	stream.next_in = (Bytef *)in;
	stream.avail_in = insize;
	stream.next_out = (Bytef *)out;
	stream.avail_out = outsize;
	stream.zalloc = (alloc_func)gz_alloc;
	stream.zfree = (free_func)gz_free;
	stream.opaque = (voidpf)0;
	err = inflateInit(&stream);
	if (err != Z_OK) {
		printf("Failed to inflateInit, err = %d\r\n", err);
		return -1;
	}

	err = inflate(&stream, Z_NO_FLUSH);

	inflateEnd(&stream);

	if (err == Z_STREAM_END) {
		return stream.total_out;
	}

	return 0;
}

static int nvt_on_rtos_partition_enum_copy_to_dest(unsigned int id, NVTPACK_MEM *p_mem, void *p_user_data)
{
	int ret = 0;
	u32 rtos_addr, rtos_size;
	EMB_PARTITION *pEmb = (EMB_PARTITION *)p_user_data;
	NVTPACK_BFC_HDR *pBFC = (NVTPACK_BFC_HDR *)p_mem->p_data;

	nvt_getfdt_rtos_addr_size((ulong)nvt_fdt_buffer, &rtos_addr, &rtos_size);

	/* Copy to dest address */
	switch (pEmb[id].EmbType) {
	case EMBTYPE_RTOS:
		/* Boot compressed uitron or non-compressed uitron */
		if (pBFC->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			printf("RUNFW: Boot compressed rtos at 0x%08x with length %d bytes\r\n",
				   (unsigned int)p_mem->p_data, p_mem->len);
			if ((cpu_to_be32(pBFC->uiAlgorithm) & 0xFF) == 11) {
				/* lzma compressed image*/
				printf("%sDecompressed lzma rtos%s \r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
				size_t Compsz = cpu_to_be32(pBFC->uiSizeComp);
				size_t Uncompsz = cpu_to_be32(pBFC->uiSizeUnComp);
				ret = lzmaBuffToBuffDecompress((unsigned char *)rtos_addr, &Uncompsz, (unsigned char *)(p_mem->p_data + sizeof(NVTPACK_BFC_HDR)), Compsz);
				if (ret != 0) {
					printf("Decompressed lzma fail\n");
					return ret;
				}  else {
					flush_dcache_range((u32)rtos_addr, (u32)(rtos_addr + Uncompsz));
				}
			} else {
				/* lz77 compressed image*/
				lz_uncompress((unsigned char *)(p_mem->p_data + sizeof(NVTPACK_BFC_HDR)), (unsigned char *)rtos_addr, cpu_to_be32(pBFC->uiSizeComp));
				flush_dcache_range((u32)rtos_addr,
								   (u32)(rtos_addr + rtos_size));
			}
		} else {
			printf("RUNFW: Copy rtos from 0x%08x to 0x%08x with length %d bytes\r\n",
				   (unsigned int)p_mem->p_data, (unsigned int)rtos_addr, p_mem->len);
			memcpy((void *)rtos_addr, (void *)p_mem->p_data, p_mem->len);
			flush_dcache_range((u32)rtos_addr, (u32)(rtos_addr + p_mem->len));

			//check partial compress load
			BININFO *pbininfo = (BININFO *)(rtos_addr + BIN_INFO_OFFSET_RTOS);
			SHMINFO *p_shminfo = (SHMINFO *)CONFIG_SMEM_SDRAM_BASE;
			if (pbininfo->head.Resv1[HEADINFO_RESV_IDX_BOOT_FLAG] & BOOT_FLAG_PARTLOAD_EN) {
				u32 part1_size = *(u32 *)(rtos_addr + FW_PART1_SIZE_OFFSET);
				//align to block size
				part1_size = ALIGN_CEIL(part1_size, _EMBMEM_BLK_SIZE_);
				printf("part1_size_aligned=%08X\r\n", part1_size);
				pBFC = (NVTPACK_BFC_HDR *)((UINT32)p_mem->p_data + part1_size);
				if (pBFC->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
					printf("RUNFW: decompress gzip.\r\n");
					UINT32 uiLength = UINT32_SWAP(pBFC->uiSizeComp);
					UINT32 uiSizeComp = UINT32_SWAP(pBFC->uiSizeComp);
					UINT32 uiSizeUnComp = UINT32_SWAP(pBFC->uiSizeUnComp);
					UINT32 uiAddrCompress = ((UINT32)pBFC) + sizeof(NVTPACK_BFC_HDR);
					int decoded_size = gz_uncompress((unsigned char *)(uiAddrCompress)
													 , (unsigned char *)(rtos_addr + part1_size)
													 , uiLength
													 , uiSizeUnComp);
					if (decoded_size == 0) {
						printf("Decompressed gzip fail\n");
						return -1;
					}
					flush_dcache_range((u32)rtos_addr, (u32)(rtos_addr + pbininfo->head.BinLength));
				}
			}
			//update LdLoadSize
			p_shminfo->boot.LdLoadSize = pbininfo->head.BinLength;
			flush_dcache_range((u32)p_shminfo, (u32)p_shminfo + sizeof(SHMINFO));
		}
		break;
	default:
		break;
	}

	return 0;
}


static int nvt_runfw_bin_unpack_to_dest(ulong addr)
{
	u32 i;
	EMB_PARTITION *pEmb = emb_partition_info_data_curr;
	int ret = 0;
	unsigned int size = gHeadInfo.Resv1[0];
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_ENUM_PARTITION_INPUT np_enum_input;
	NVTPACK_MEM mem_in = {(void *)addr, size};
	NVTPACK_MEM mem_out = {0};

	memset(&np_get_input, 0, sizeof(np_get_input));
	memset(&np_enum_input, 0, sizeof(np_enum_input));

	ret = nvt_getfdt_emb((ulong)nvt_fdt_buffer, pEmb);
	if (ret < 0) {
		printf("failed to get current partition resource.\r\n");
		return -1;
	}

	/* Check for all partition existed */
	for (i = 1; i < EMB_PARTITION_INFO_COUNT; i++) {
		if (pEmb[i].PartitionSize != 0) {
			switch (pEmb[i].EmbType) {
			case EMBTYPE_FDT:
#ifndef _NVT_LINUX_SMP_ON_
			case EMBTYPE_UITRON:
#endif
			case EMBTYPE_UBOOT:
			case EMBTYPE_LINUX:
#ifdef CONFIG_NVT_LINUX_RAMDISK_BOOT
			case EMBTYPE_ROOTFS:
#endif
				//check this type exist in all-in-one
				np_get_input.id = i;
				np_get_input.mem = mem_in;
				if (nvtpack_check_partition(pEmb[i], &np_get_input, &mem_out) != NVTPACK_ER_SUCCESS) {
					printf("RUNFW boot: need partition[%d]\n", i);
					return -1;
				}
				break;
			}
		}
	}

	/* Enum all partition to do necessary handling */
	np_enum_input.mem = mem_in;
	np_enum_input.p_user_data = pEmb;
	/* Loading images */
	np_enum_input.fp_enum = nvt_on_partition_enum_copy_to_dest;

	if (nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
		printf("failed sanity.\r\n");
		return -1;
	}

	/* Make sure images had been already copied */
	flush_dcache_all();

	return 0;
}

static int nvt_runfw_bin_unpack_to_dest_rtos(ulong addr)
{
	u32 i;
	EMB_PARTITION *pEmb = emb_partition_info_data_curr;
	int ret = 0;
	unsigned int size = gHeadInfo.Resv1[0];
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_ENUM_PARTITION_INPUT np_enum_input;
	NVTPACK_MEM mem_in = {(void *)addr, size};
	NVTPACK_MEM mem_out = {0};

	memset(&np_get_input, 0, sizeof(np_get_input));
	memset(&np_enum_input, 0, sizeof(np_enum_input));

	ret = nvt_getfdt_emb((ulong)nvt_fdt_buffer, pEmb);
	if (ret < 0) {
		printf("failed to get current partition resource.\r\n");
		return -1;
	}

	/* Check for all partition existed */
	for (i = 1; i < EMB_PARTITION_INFO_COUNT; i++) {
		if (pEmb[i].PartitionSize != 0) {
			switch (pEmb[i].EmbType) {
			case EMBTYPE_FDT:
			case EMBTYPE_RTOS:
			case EMBTYPE_UBOOT:
				//check this type exist in all-in-one
				np_get_input.id = i;
				np_get_input.mem = mem_in;
				if (nvtpack_check_partition(pEmb[i], &np_get_input, &mem_out) != NVTPACK_ER_SUCCESS) {
					printf("RUNFW boot: need partition[%d]\n", i);
					return -1;
				}
				break;
			}
		}
	}

	/* Enum all partition to do necessary handling */
	np_enum_input.mem = mem_in;
	np_enum_input.p_user_data = pEmb;
	/* Loading images */
	np_enum_input.fp_enum = nvt_on_rtos_partition_enum_copy_to_dest;

	if (nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
		printf("failed sanity.\r\n");
		return -1;
	}

	/* Make sure images had been already copied */
	flush_dcache_all();

	return 0;
}


#ifdef CONFIG_NVT_LINUX_AUTODETECT
static int nvt_getfdt_rtos_addr_size(ulong addr, u32 *fdt_rtos_addr, u32 *fdt_rtos_len)
{
	int len;
	int nodeoffset; /* next node offset from libfdt */
	const u32 *nodep; /* property node pointer */


	nodeoffset = fdt_path_offset((const void *)addr, "/nvt_memory_cfg/rtos");
	if (nodeoffset < 0) {
		*fdt_rtos_addr = 0;
		*fdt_rtos_len = 0;
		return -1;
	}

	nodep = fdt_getprop((const void *)addr, nodeoffset, "reg", &len);
	if (len == 0) {
		*fdt_rtos_addr = 0;
		*fdt_rtos_len = 0;
		return -1;
	}

	*fdt_rtos_addr = be32_to_cpu(nodep[0]);
	*fdt_rtos_len = be32_to_cpu(nodep[1]);
	return 0;
}

static int nvt_boot_linux_bin_auto(void)
{
	char cmd[256] = {0};
	int ret = 0;
	image_header_t *hdr;
	uint size;

	loff_t part_off = 0, part_size = 0;
	u32 align_size = 0;
	u64 align_off = 0;
	get_part("linux", &part_off, &part_size);


	if (nvt_detect_fw_tbin()) {
		/* Check fw is valid */
		nvt_runfw_bin_chk_valid((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE);
		/* Copy linux binary to destination address */
		ret = nvt_runfw_bin_unpack_to_dest((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE);
		if (ret < 0) {
			return ret;
		}
	} else {




		// get uImage size firstly
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND)) || defined(CONFIG_NVT_SPI_NOR_NAND)
		sprintf(cmd, "nand read 0x%x 0x%llx 0x%x", CONFIG_LINUX_SDRAM_START, part_off, sizeof(image_header_t));
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
		align_size = ALIGN_CEIL(sizeof(image_header_t), ARCH_DMA_MINALIGN);
		sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", CONFIG_LINUX_SDRAM_START, part_off, align_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)  || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
		align_off = ALIGN_CEIL(part_off, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
		sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", CONFIG_LINUX_SDRAM_START, align_off, 2);
#endif
		run_command(cmd, 0);
		hdr = (image_header_t *)CONFIG_LINUX_SDRAM_START;
		size = image_get_data_size(hdr) + sizeof(image_header_t);
		// Reading Linux kernel image
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND)) || defined(CONFIG_NVT_SPI_NOR_NAND)
		sprintf(cmd, "nand read 0x%x 0x%llx 0x%x", CONFIG_LINUX_SDRAM_START, part_off, size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
		align_size = ALIGN_CEIL(size, ARCH_DMA_MINALIGN);
		sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", CONFIG_LINUX_SDRAM_START, part_off, align_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)|| (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
		align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
		sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", CONFIG_LINUX_SDRAM_START, align_off, align_size);
#endif
		run_command(cmd, 0);
	}

	return 0;
}

static u32 nvt_get_32bits_data(u_int32_t addr, bool is_little_endian)
{
	u_int32_t value = 0;
	u_int8_t *pb = (u_int8_t *)addr;
	if (is_little_endian) {
		if (addr & 0x3) { //NOT word aligned
			value = (*pb);
			value |= (*(pb + 1)) << 8;
			value |= (*(pb + 2)) << 16;
			value |= (*(pb + 3)) << 24;
		} else {
			value = *(u_int32_t *)addr;
		}
	} else {
		value = (*pb) << 24;
		value |= (*(pb + 1)) << 16;
		value |= (*(pb + 2)) << 8;
		value |= (*(pb + 3));
	}
	return value;
}

static int nvt_boot_rtos_bin_auto(void)
{
	u32 rtos_addr = 0, rtos_size = 0;
	char cmd[256] = {0};
	int ret = 0;
	unsigned int tmp_addr = 0;
	u_int32_t codesize = 0;
	u32 align_size = 0;
	char *pCodeInfo = NULL;
	BININFO *pbininfo = NULL;
	SHMINFO *p_shminfo = (SHMINFO *)CONFIG_SMEM_SDRAM_BASE;
#if defined(CONFIG_NVT_SPI_NOR) || defined(CONFIG_NVT_IVOT_EMMC)
	loff_t part_off = 0, part_size = 0;
#endif

	nvt_getfdt_rtos_addr_size((ulong)nvt_fdt_buffer, &rtos_addr, &rtos_size);

	if (nvt_detect_fw_tbin()) {
		/* Check fw is valid */
		nvt_runfw_bin_chk_valid((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE);
		/* Copy itron binary to destination address */
		ret = nvt_runfw_bin_unpack_to_dest_rtos((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE);;
		if (ret < 0) {
			return ret;
		}
	} else {
		// get rtos image size firstly
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND))
		sprintf(cmd, "nand read 0x%x rtos 0x500", rtos_addr);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
		get_part("rtos", &part_off, &part_size);
		sprintf(cmd, "sf read 0x%x 0x%llx 0x500 ", rtos_addr, part_off);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
		get_part("rtos", &part_off, &part_size);
		sprintf(cmd, "mmc read 0x%x 0x%llx 1", rtos_addr, part_off / MMC_MAX_BLOCK_LEN);
#endif
		printf("=>run cmd : %s\n", cmd);
		run_command(cmd, 0);
		// Reading rtos image
		NVTPACK_BFC_HDR *pbfc = (NVTPACK_BFC_HDR *)rtos_addr;
		if (pbfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			printf("Compressed rtos\n");
			/* rtos image size will be put in the end of rtos region */
			size_t firmware_size = cpu_to_be32(pbfc->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
			tmp_addr = rtos_addr + rtos_size - firmware_size;
			if ((firmware_size + cpu_to_be32(pbfc->uiSizeUnComp)) > rtos_size) {
				printf("%s Attention!!! rtos starting address at 0x%08x with size 0x%08x (uncompressed) is larger than temp buffer address!! 0x%08x %s\r\n",
					   ANSI_COLOR_RED, rtos_addr, cpu_to_be32(pbfc->uiSizeUnComp), tmp_addr, ANSI_COLOR_RESET);
				printf("%s It can't decompress %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
				while (1);
			}
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND))
			sprintf(cmd, "nand read 0x%x rtos 0x%x ", tmp_addr, firmware_size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
			sprintf(cmd, "sf read 0x%x 0x%llx 0x%x ", tmp_addr, part_off, firmware_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
			align_size = ALIGN_CEIL(firmware_size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
			sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", tmp_addr, part_off / MMC_MAX_BLOCK_LEN, align_size);
#endif
			printf("=>run cmd : %s\n", cmd);
			run_command(cmd, 0);

			debug("auto boot bfc UnCompsz:%d Compsz:%d Algorithm: %x \n", cpu_to_be32(pbfc->uiSizeUnComp), cpu_to_be32(pbfc->uiSizeComp), cpu_to_be32(pbfc->uiAlgorithm));
			if ((cpu_to_be32(pbfc->uiAlgorithm) & 0xFF) == 11) {
				/* lzma compressed image*/
				size_t Compsz = cpu_to_be32(pbfc->uiSizeComp);
				size_t Uncompsz = cpu_to_be32(pbfc->uiSizeUnComp);
				printf("Decompressed lzma rtos\n");
				flush_dcache_range((u32)tmp_addr,
								   (u32)(tmp_addr + firmware_size));
				ret = lzmaBuffToBuffDecompress((unsigned char *)rtos_addr, &Uncompsz, (unsigned char *)(tmp_addr + sizeof(NVTPACK_BFC_HDR)), Compsz);
				if (ret != 0) {
					printf("Decompressed lzma fail\n");
				}
				flush_dcache_range((u32)rtos_addr, (u32)(rtos_addr + Uncompsz));
			} else {
				/* lz77 compressed image*/
				lz_uncompress((unsigned char *)(tmp_addr + sizeof(NVTPACK_BFC_HDR)), (unsigned char *)rtos_addr, cpu_to_be32(pbfc->uiSizeComp));
				flush_dcache_range((u32)rtos_addr,
								   (u32)(rtos_addr + rtos_size));
			}
		} else {
			int i;
			pbininfo = (BININFO *)(rtos_addr + BIN_INFO_OFFSET_RTOS);
			printf("Non-compressed rtos\n");

			if (pbininfo->head.Resv1[HEADINFO_RESV_IDX_BOOT_FLAG] & BOOT_FLAG_PARTLOAD_EN) {
				// fllush whole rtos memory to invalidate cache on partial load
				flush_dcache_range((u32)rtos_addr, (u32)(rtos_addr + rtos_size));
				// preload some to get part-1 size for partial load and partial compressed load
				u32 preload_size = ALIGN_CEIL(FW_PART1_SIZE_OFFSET, _EMBMEM_BLK_SIZE_);
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND))
				sprintf(cmd, "nand read 0x%x rtos 0x%x ", rtos_addr, preload_size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
				align_size = ALIGN_CEIL(preload_size, ARCH_DMA_MINALIGN);
				sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", rtos_addr, part_off, align_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
				align_size = ALIGN_CEIL(preload_size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
				sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", rtos_addr, part_off / MMC_MAX_BLOCK_LEN, align_size);
#endif
				run_command(cmd, 0);
				//partial-load or partial-compressed-load
				u32 part1_size = *(u32 *)(rtos_addr + FW_PART1_SIZE_OFFSET);
				printf("part1_size=%08X,preload_size=%08X\r\n", part1_size, preload_size);
				//align to block size
				part1_size = ALIGN_CEIL(part1_size, _EMBMEM_BLK_SIZE_);
				printf("part1_size_aligned=%08X\r\n", part1_size);
				//sprintf(cmd, "nand read 0x%x 0x%x 0x%x", (unsigned int)dst_addr, base_ofs+ofs, size);
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND))
				align_size = part1_size;
				sprintf(cmd, "nand read 0x%x rtos 0x%x ", rtos_addr, align_size);
				//update LdLoadSize
				p_shminfo->boot.LdLoadSize = align_size;
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
				align_size = ALIGN_CEIL(part1_size, ARCH_DMA_MINALIGN);
				sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", rtos_addr, part_off, align_size);
				//update LdLoadSize
				p_shminfo->boot.LdLoadSize = align_size;
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
				align_size = ALIGN_CEIL(part1_size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
				sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", rtos_addr, part_off / MMC_MAX_BLOCK_LEN, align_size);
				//update LdLoadSize
				p_shminfo->boot.LdLoadSize = align_size * MMC_MAX_BLOCK_LEN;
#endif

				flush_dcache_range((u32)p_shminfo, (u32)p_shminfo + sizeof(SHMINFO));
			} else {
				//full load
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND))
				sprintf(cmd, "nand read 0x%x rtos 0x%x ", rtos_addr, pbininfo->head.BinLength);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
				sprintf(cmd, "sf read 0x%x 0x%llx 0x%x ", rtos_addr, part_off, pbininfo->head.BinLength);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
				sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", rtos_addr, part_off / MMC_MAX_BLOCK_LEN, pbininfo->head.BinLength / MMC_MAX_BLOCK_LEN);
#endif
				//update LdLoadSize
				p_shminfo->boot.LdLoadSize = pbininfo->head.BinLength;
				flush_dcache_range((u32)p_shminfo, (u32)p_shminfo + sizeof(SHMINFO));
			}
			printf("=>run cmd : %s\n", cmd);
			run_command(cmd, 0);
			// fllush whole rtos memory to invalidate cache
			flush_dcache_range((u32)rtos_addr, (u32)(rtos_addr + rtos_size));
		}
	}

	pCodeInfo = (char *)(rtos_addr + RTOS_CODEINFO_OFFSET);
	pbininfo = (BININFO *)(rtos_addr + BIN_INFO_OFFSET_RTOS);

	if (strncmp(pCodeInfo, "CODEINFO", 8) != 0) {
		printf("invalid CODEINFO\r\n");
		return -1;
	}

	/* dram partition rtos address should be the same with headinfo address */
	if (rtos_addr != pbininfo->head.CodeEntry) {
		printf("dram partition rtos addr (%08X) != headinfo(%08X)\r\n"
			   , rtos_addr
			   , pbininfo->head.CodeEntry);
		return -1;
	}

	/* To check if code size is larger than rtos_addr + rtos_size (Not image size) */
	codesize = nvt_get_32bits_data((u_int32_t)(&pCodeInfo[RTOS_CODEINFO_SUB_ZI_LIMIT]), true) - rtos_addr;
	if (codesize > rtos_size) {
		printf("uBoot uiCodeSize(%08X) > dram partition rtos size(%08X)\r\n"
			   , codesize
			   , rtos_size);
		return -1;
	}

	printf("rtos starting.... 0x%08x\n", rtos_addr);
	{
		typedef void (*BRANCH_CB)(void);
		BRANCH_CB p_func = (BRANCH_CB)rtos_addr;
		p_func();
	}

	return 0;
}

#ifdef CONFIG_NVT_LINUX_RAMDISK_BOOT



static int nvt_boot_rootfs_bin_auto(void)
{

	int  nodeoffset = 0;
	int nextoffset = 0;
	const unsigned long long *val = NULL;
	u64 ramdisk_partition_addr;
	int len = 0;
	char cmd[256] = {0};
	char tmp_translate[64] = {0};
	unsigned int ramdisk_partition_addr_32 = 0;
	image_header_t *hdr;
	unsigned int hdr_tmp = 0;
	//get rootfs partition info from fdt
#if defined(CONFIG_NVT_IVOT_EMMC)
	char path[128] = {0};
	sprintf(path, "/mmc@%x/partition_rootfs", IOADDR_SDIO3_REG_BASE);
	nodeoffset = fdt_path_offset((void *)nvt_fdt_buffer, path);
#elif defined(CONFIG_NVT_SPI_NOR)
	nodeoffset = fdt_path_offset((void *)nvt_fdt_buffer, "/nor/partition_rootfs");
#else
	nodeoffset = fdt_path_offset((void *)nvt_fdt_buffer, "/nand/partition_rootfs");
#endif

	if (nodeoffset < 0) {
		printf("nvt_boot_rootfs_bin_auto get fdt information (partition_rootfs) error\n");
		return -1;
	}

	val = (const unsigned long long *)fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "reg", &len);
	if (len <= 0) {
		printf("get fdt partition_rootfs reg value error\n");
		return -1;

	}
	ramdisk_partition_addr = be64_to_cpu(val[0]);
	//sprintf(&hdr_tmp,"%x",ramdisk_partition_addr);
	//hdr = (image_header_t *)hdr_tmp;
	sprintf(tmp_translate, "%llx", ramdisk_partition_addr);
	ramdisk_partition_addr_32 = atoi(tmp_translate);

	printf("ramdisk_partition_addr_32:%u\n", ramdisk_partition_addr_32);


	nvt_ramdisk_addr = (ulong)memalign(CONFIG_SYS_CACHELINE_SIZE, 0x20000);
	if (!nvt_ramdisk_addr) {
		printf("nvt_ramdisk_addr alloc fail\n");
		return -1;
	}
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND)) || defined(CONFIG_NVT_SPI_NOR_NAND)
	sprintf(cmd, "nand read 0x%x 0x%llx 0x%x", nvt_ramdisk_addr, ramdisk_partition_addr, sizeof(image_header_t));
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
	u32 align_size = 0;
	align_size = ALIGN_CEIL(sizeof(image_header_t), ARCH_DMA_MINALIGN);
	sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", nvt_ramdisk_addr, ramdisk_partition_addr, align_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)  || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
	u64 align_off = ALIGN_CEIL(ramdisk_partition_addr, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
	sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", nvt_ramdisk_addr, align_off, 2);
#endif

	run_command(cmd, 0);


	hdr = (image_header_t *)nvt_ramdisk_addr;
	nvt_ramdisk_size = image_get_data_size(hdr) + sizeof(image_header_t);
	nvt_ramdisk_size = ALIGN_CEIL(nvt_ramdisk_size, 4096);
	if(nvt_ramdisk_addr)
	    free((void *)nvt_ramdisk_addr);
	nvt_ramdisk_addr = _BOARD_LINUX_ADDR_ + _BOARD_LINUX_SIZE_ - nvt_ramdisk_size - 0x10000;
	//nvt_ramdisk_size = ALIGN_CEIL(p_mem->len, 4096);
	printf("ramdisk_addr:%x\n", nvt_ramdisk_addr);
	printf("ramdisk_size:%d\n", nvt_ramdisk_size);


#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NAND)) || defined(CONFIG_NVT_SPI_NOR_NAND)
	sprintf(cmd, "nand read 0x%x 0x%llx 0x%x", nvt_ramdisk_addr, ramdisk_partition_addr, nvt_ramdisk_size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_SPI_NOR))
	align_size = ALIGN_CEIL(nvt_ramdisk_size, ARCH_DMA_MINALIGN);
	sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", nvt_ramdisk_addr, ramdisk_partition_addr, align_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)|| (defined(CONFIG_NVT_LINUX_RAMDISK_BOOT) && defined(CONFIG_NVT_IVOT_EMMC))
	u32 align_size = 0;
	align_size = ALIGN_CEIL(nvt_ramdisk_size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
	align_off = ALIGN_CEIL(ramdisk_partition_addr, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
	sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", nvt_ramdisk_addr, align_off, align_size);
#else
	printf("can not get ramdisk from flash/emmc, please check flash type or boot type\n");

#endif
	run_command(cmd, 0);



	return 0;
}
#endif
static int nvt_boot_fw_init_detect(void)
{
	u32 rtos_addr = 0, rtos_size = 0;
	printf("NVT firmware boot.....\n");

	if (nvt_getfdt_rtos_addr_size((ulong)nvt_fdt_buffer, &rtos_addr, &rtos_size) != 0 || rtos_size == 0) {
		nvt_boot_linux_bin_auto();
#ifdef CONFIG_NVT_LINUX_RAMDISK_BOOT
		if (!nvt_detect_fw_tbin()) {
			nvt_boot_rootfs_bin_auto();
		}
#endif
	} else {
		nvt_boot_rtos_bin_auto();
	}

	return 0;
}
#else /* !CONFIG_NVT_LINUX_AUTODETECT */

static int nvt_boot_linux_bin(void)
{
	char cmd[256] = {0};

#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	sprintf(cmd, "nand read 0x%x linux ", CONFIG_LINUX_SDRAM_START);
	run_command(cmd, 0);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
	loff_t part_off = 0, part_size = 0;
	/* Get linux partition offset and size */
	get_part("linux", &part_off, &part_size);
	sprintf(cmd, "sf read 0x%x 0x%llx 0x%llx", CONFIG_LINUX_SDRAM_START, part_off, part_size);
	ret = run_command(cmd, 0);
#elif defined(CONFIG_NVT_LINUX_SD_BOOT) /* CONFIG_NVT_LINUX_SD_BOOT */
	sprintf(cmd, "fatload mmc 0:1 0x%x uImage.bin ", CONFIG_LINUX_SDRAM_START);
	run_command(cmd, 0);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) /* CONFIG_NVT_LINUX_EMMC_BOOT */
	loff_t part_off = 0, part_size = 0;
	/* Get linux partition offset and size */
	get_part("linux", &part_off, &part_size);
	sprintf(cmd, "mmc read 0x%x 0x%llx 0x%llx ", CONFIG_LINUX_SDRAM_START, part_off, part_size);
	run_command(cmd, 0);
#else /* !defined(CONFIG_NVT_LINUX_SPINAND_BOOT) && !defined(CONFIG_NVT_LINUX_SD_BOOT) */
	int ret = 0;
	/* Check fw is valid */
	nvt_runfw_bin_chk_valid((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE);
	/* Copy linux binary to destination address */
	ret = nvt_runfw_bin_unpack_to_dest((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE);
	if (ret < 0) {
		return ret;
	}
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */
}

static int nvt_boot_fw_init(void)
{
	char cmd[256] = {0};

	printf("NVT firmware boot.....\n");

#ifndef _NVT_LINUX_SMP_ON_

#endif /* !_NVT_LINUX_SMP_ON_ */
#ifdef _CPU2_NONE_
	/* Core1 will turn off core2 if core2 is not linux */
	while (1);
#endif /* _CPU2_NONE_ */
	nvt_boot_linux_bin();

	return 0;
}
#endif /* CONFIG_NVT_LINUX_AUTODETECT */
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

int do_nvt_boot_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	char cmdline[CONFIG_CMDLINE_SIZE] = {0};
	char cmd[CONFIG_CMDLINE_SIZE] = {0};
	char buf[CONFIG_CMDLINE_SIZE] = {0};

#ifdef CONFIG_NVT_IVOT_SOC_UART_UPGRADE_SUPPORT
	return 0;
#endif
	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline
	 */
	sprintf(buf, "%s ", getenv("bootargs"));
	strcat(cmdline, buf);

	/*
	 * Loading linux kernel
	 */
#ifdef  CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
#ifdef CONFIG_NVT_LINUX_AUTODETECT
	ret = nvt_boot_fw_init_detect();
	if (ret < 0) {
		printf("boot firmware init failed\n");
		return ret;
	}
#else /* !CONFIG_NVT_LINUX_AUTODETECT */
	ret = nvt_boot_fw_init();
	if (ret < 0) {
		printf("boot firmware init failed\n");
		return ret;
	}
#endif /* CONFIG_NVT_LINUX_AUTODETECT */
#else /* !CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
	/* FIXME: To do customized boot */

#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
	/* Remove timer 0 firstly in 321 codebase */
#ifndef CONFIG_ARCH_NA51068_A32
	nvt_tm0_cnt_end = get_nvt_timer0_cnt();
	/* boot time recording */
	sprintf(buf, "bootts=%lu,%lu resume_addr=0x%08lx user_debug=0xff ", nvt_tm0_cnt_beg, nvt_tm0_cnt_end, nvt_shminfo_comm_core2_start);
	strcat(cmdline, buf);
#endif
#ifdef CONFIG_MS_NVR
    char *p = strstr(getenv("mtdparts"), "hinand");
    sprintf(buf, "mtdparts=%s;%s", getenv("nor_mtdparts"), strstr(getenv("mtdparts"), "spi_nand.0"));
	strcat(cmdline, buf);
#endif
	setenv("bootargs", cmdline);
	/* To assign relocated fdt address */
	sprintf(buf, "0x%08x ", _BOARD_LINUX_MAXBLK_ADDR_ + _BOARD_LINUX_MAXBLK_SIZE_);
	setenv("fdt_high", buf);

	/* The following will setup the lmb memory parameters for bootm cmd */
	sprintf(buf, "0x%08x ", _BOARD_LINUX_ADDR_ + _BOARD_LINUX_SIZE_);
	setenv("bootm_size", buf);
	setenv("bootm_mapsize", buf);
	sprintf(buf, "0x%08x ", _BOARD_LINUX_ADDR_);
	setenv("bootm_low", buf);

	/*
	 * Issue boot command
	 */
#ifdef  CONFIG_NVT_LINUX_RAMDISK_BOOT
	sprintf(buf, "0x%08x ", nvt_ramdisk_addr + nvt_ramdisk_size);
	setenv("initrd_high", buf);
	if (nvt_ramdisk_addr == 0) {
		printf("Stop boot because your ramdisk address is wrong!!! addr: 0x%08x \n", nvt_ramdisk_addr);
		while(1);
	}
	sprintf(cmd, "bootm %x %lx %lx", CONFIG_LINUX_SDRAM_START, nvt_ramdisk_addr, (ulong)nvt_fdt_buffer);
	printf("%s\n", cmd);
#else
	printf("%s uImage is at %x, uboot fdt image is at %lx, loader tmp fdt address is at %x %s\n", ANSI_COLOR_YELLOW, CONFIG_LINUX_SDRAM_START, (ulong)nvt_fdt_buffer, nvt_readl((ulong)nvt_shminfo_boot_fdt_addr), ANSI_COLOR_RESET);
	sprintf(cmd, "bootm %x - %lx", CONFIG_LINUX_SDRAM_START, (ulong)nvt_fdt_buffer);
#endif
#ifndef CONFIG_ARCH_NA51068_A32
	printf("%s Uboot boot time: \n\tstart:\t%lu us\n\tending:\t%lu us %s\r\n", ANSI_COLOR_GREEN, nvt_tm0_cnt_beg, nvt_tm0_cnt_end, ANSI_COLOR_RESET);
#endif
	run_command(cmd, 0);
	return 0;
}

U_BOOT_CMD(
	nvt_boot,   1,  0,  do_nvt_boot_cmd,
	"To do nvt platform boot init.", "\n"
);

static int do_nvt_update_addr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("0x%x\n", CONFIG_NVT_RUNFW_SDRAM_BASE);
	return 0;
}

U_BOOT_CMD(
	nvt_update_addr,	1,		1,	do_nvt_update_addr,
	"DRAM address that image can be put",
	""
);

#ifdef  CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
int do_nvt_update_all_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr = 0, size = 0;
	int ret = 0;

	if (argc != 3) {
		return CMD_RET_USAGE;
	}

	size = cmd_get_data_size(argv[0], 4);
	if (size < 0) {
		return 1;
	}

	addr = simple_strtoul(argv[1], NULL, 16);
	size = simple_strtoul(argv[2], NULL, 10);

	printf("%s addr: 0x%08lx, size: 0x%08lx(%lu) bytes %s\r\n", ANSI_COLOR_YELLOW, addr, size, size, ANSI_COLOR_RESET);

	ret = nvt_process_all_in_one(addr, size, 0);
	if (ret < 0) {
		printf("Update nvt all-in-one image failed.\n");
		return ret;
	}

	return 0;
}

U_BOOT_CMD(
	nvt_update_all, 3,  0,  do_nvt_update_all_cmd,
	"To Update all-in-one image from memory address and size \n",
	"[address(hex)][size(dec)] \n"
);
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
static UINT8 car_in[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
//static UINT8 car_in[16] = {0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x12, 0x11, 0x10, 0x09, 0x16, 0x15, 0x14, 0x13};
//static UINT8  car_in[16] = {0xb1, 0xbe, 0x28, 0x1a, 0x3e, 0xe0, 0xd3, 0x2e, 0xf8, 0xb1, 0xe0, 0x0b, 0x4a, 0x9e, 0x38, 0x13};
//static __packed __aligned(64) UINT8  output[16];
//static __packed __aligned(64) UINT8  output2[16];
//volatile UINT32 test = 1;
int do_nvt_encrypt_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int index_cnt;

	UINT32 test_buf;
	UINT32 test_buf2;

	UINT32 test_buf_align;
	UINT32 test_buf2_align;

	UINT8  *output;
	UINT8  *output2;

	UINT32      key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
	CRYPT_OP    crypt_op_param;

	if (strncmp(argv[1], "0", 1) == 0) {
		key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
	} else {
		return CMD_RET_USAGE;
	}
	test_buf = (UINT32)malloc(16 + 64);
	test_buf_align = (UINT32)((test_buf + 63) & 0xFFFFFFC0);

	test_buf2 = (UINT32)malloc(16 + 64);
	test_buf2_align = (UINT32)((test_buf + 63) & 0xFFFFFFC0);

	output = (UINT8 *)test_buf_align;
	output2 = (UINT8 *)test_buf2_align;
	crypt_op_param.op_mode = CRYPTO_EBC;
	crypt_op_param.en_de_crypt = CRYPTO_ENCRYPT;
	crypt_op_param.src_addr = (UINT32)car_in;
	crypt_op_param.dst_addr = (UINT32)output;
	crypt_op_param.length   = 16;
	if (crypto_data_operation(key_set, crypt_op_param) != 0) {
		printf("Encrypt operation fail [%d] set\r\n", (int)(key_set + 1));
	} else {
		printf("Encrypt operation success [%d] set\r\n", (int)(key_set + 1));
		printf("Source =>\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", car_in[index_cnt]);
		}
		printf("\r\n");
		printf("Destination =>\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", output[index_cnt]);
		}
		printf("\r\n");
	}

	crypt_op_param.en_de_crypt = CRYPTO_DECRYPT;
	crypt_op_param.src_addr = (UINT32)output;
	crypt_op_param.dst_addr = (UINT32)output2;
	crypt_op_param.length   = 16;

	if (crypto_data_operation(key_set, crypt_op_param) != 0) {
		printf("Decrypt operation fail [%d] set\r\n", (int)(key_set + 1));
	} else {
		printf("Decrypt operation success [%d] set\r\n", (int)(key_set + 1));
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", output2[index_cnt]);
		}
		printf("\r\n");
	}


	printf("do_nvt_encrypt_cmd sample code done\r\n");

	if (test_buf) {
		free((char *)test_buf);
	}

	if (test_buf2) {
		free((char *)test_buf2);
	}

	return 0;
}
U_BOOT_CMD(
	nvt_encrypt,    2,  0,  do_nvt_encrypt_cmd,
	"To encrypt via key manager sample",
	"[keyset(0~3)]\n"
);




static UINT8 key_sample[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
static UINT8 encrypt_sample[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
static UINT8 key_sample0[16] = {0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x12, 0x11, 0x10, 0x09, 0x16, 0x15, 0x14, 0x13};
static UINT8 key_sample1[16] = {0x24, 0xaa, 0x0d, 0x9b, 0xf1, 0xae, 0x31, 0xb4, 0x28, 0x51, 0xe4, 0xc4, 0xd1, 0x71, 0x1d, 0x1e};
static UINT8 key_sample2[16] = {0x95, 0x4d, 0x81, 0xc5, 0x5a, 0xcc, 0x2b, 0xe5, 0xdd, 0xc8, 0x74, 0xc3, 0x9f, 0xaf, 0xcf, 0x5c};
static UINT8 key_sample3[16] = {0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x12, 0x11, 0x10, 0x09, 0x16, 0x15, 0x14, 0x13};

int do_nvt_write_key_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	INT32		result;
	int         index_cnt;
	UINT32      test_buf;
	UINT32      test_buf_align;


	UINT32      test_buf2;
	UINT32      test_buf2_align;

	UINT8  		pKey[16];

	UINT8       *output;
	UINT8       *output2;
	UINT32      key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
	CRYPT_OP    crypt_op_param;

	if (strncmp(argv[1], "0", 1) == 0) {
		key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample3, 16);
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample1, 16);
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample2, 16);
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample3, 16);
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
		memcpy((void *)pKey, (void *)key_sample3, 16);
	} else {
		return CMD_RET_USAGE;
	}

	result = otp_write_key(key_set, pKey);

	if (result < 0) {
		printf("Write [%d] key operation fail [%d] \r\n", (int)(key_set), result);
		return 0;
	} else {
		printf("Write [%d] key operation success\r\n", (int)(key_set));
	}


	test_buf = (UINT32)malloc(16 + 64);
	test_buf_align = (UINT32)((test_buf + 63) & 0xFFFFFFC0);
	output = (UINT8 *)test_buf_align;

	test_buf2 = (UINT32)malloc(16 + 64);
	test_buf2_align = (UINT32)((test_buf2 + 63) & 0xFFFFFFC0);
	output2 = (UINT8 *)test_buf2_align;

	memset((char *)test_buf2_align, 0x0, 16);
	memset((char *)test_buf_align, 0x0, 16);

	crypt_op_param.op_mode = CRYPTO_EBC;
	crypt_op_param.en_de_crypt = CRYPTO_ENCRYPT;
	crypt_op_param.src_addr = (UINT32)encrypt_sample;
	crypt_op_param.dst_addr = (UINT32)output;   //<<<-------------(1)
	crypt_op_param.length   = 16;

	//Encrypt via key manager by using specific key set
	if (crypto_data_operation(key_set, crypt_op_param) != 0) {
		printf("Encrypt operation fail [%d] set\r\n", (int)(key_set));
		return 0;
	} else {
		printf("Encrypt operation success [%d] set\r\n", (int)(key_set));
		printf("Source =>\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", pKey[index_cnt]);
		}
		printf("\r\n");
		printf("Destination =>\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", output[index_cnt]);
		}
		printf("\r\n");
	}

	//Encrypt via fill key by using CPU
	crypt_op_param.dst_addr = (UINT32)output2;  ////<<<-------------(2)
	crypto_data_operation_by_key(pKey, crypt_op_param);

	printf("Verification via CPU fill key=>\r\n");
	for (index_cnt = 0; index_cnt < 16; index_cnt++) {
		printf("%2x ", output2[index_cnt]);
	}
	printf("\r\n");


	//(1) vs (2) result should the same
	if (memcmp((void *)output2, (void *)output, 16) != 0) {
		printf("write key fail [%d] set\r\n", (int)(key_set + 1));
		printf("OTP operation\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", output[index_cnt]);
		}

		printf("\r\n");
		printf("Fill key operation\r\n");
		for (index_cnt = 0; index_cnt < 16; index_cnt++) {
			printf("%2x ", output2[index_cnt]);
		}
	} else {
		printf("write key success [%d] set\r\n", (int)(key_set + 1));
	}

	return 0;
}
U_BOOT_CMD(
	nvt_write_key,  6,  0,  do_nvt_write_key_cmd,
	"To write key via OTP sample",
	"[Option] key set (0~4)=> 0 is dedicate for secure boot\n"
	"[0] => 1st key set (0 is dedicate for secure boot)\n"
	"[1] => 2nd key set\n"
	"[2] => 3rd key set\n"
	"[3] => 4th key set\n"
	"[4] => 5th key set\n"
);

int do_nvt_read_lock_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	UINT32      key_set_read_lock = SECUREBOOT_1ST_KEY_SET_READ_LOCK;

	if (strncmp(argv[1], "0", 1) == 0) {
		key_set_read_lock = SECUREBOOT_1ST_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "1", 1) == 0) {
		key_set_read_lock = SECUREBOOT_2ND_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "2", 1) == 0) {
		key_set_read_lock = SECUREBOOT_3RD_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "3", 1) == 0) {
		key_set_read_lock = SECUREBOOT_4TH_KEY_SET_READ_LOCK;
	} else if (strncmp(argv[1], "4", 1) == 0) {
		key_set_read_lock = SECUREBOOT_5TH_KEY_SET_READ_LOCK;
	} else {
		return CMD_RET_USAGE;
	}

	enable_secure_boot(key_set_read_lock);
	return 0;
}
U_BOOT_CMD(
	nvt_read_lock,  6,  0,  do_nvt_read_lock_cmd,
	"To let key set X un-readable",
	"[Option] key set (0~4)=> 0 is dedicate for secure boot\n"
	"[0] => 1st key set (0 is dedicate for secure boot)\n"
	"[1] => 2nd key set\n"
	"[2] => 3rd key set\n"
	"[3] => 4th key set\n"
	"[4] => 5th key set\n"
);

int do_nvt_secure_en_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (!strncmp(argv[1], "secure_en", 9)) {
		printf("=>[secure_en]\r\n");
		efuse_secure_en();
	} else if (!strncmp(argv[1], "data_encrypt", 12)) {
		printf("=>[efuse_data_area_encrypt_en]\r\n");
		efuse_data_area_encrypt_en();
	} else if (!strncmp(argv[1], "rsa_en", 6)) {
		printf("=>[efuse_signature_rsa_en]\r\n");
		efuse_signature_rsa_en();
	} else if (!strncmp(argv[1], "rsa_chk_en", 10)) {
		printf("=>[efuse_signature_rsa_chksum_en]\r\n");
		efuse_signature_rsa_chksum_en();
	}  else if (!strncmp(argv[1], "jtag_disable", 12)) {
		printf("=>[jtag_disable]\r\n");
		efuse_jtag_dis();
	} else if (!strncmp(argv[1], "uniqueue_id", 11)) {
		UINT32 h, l;
		printf("=>[uniqueue_id]=>");
		if(efuse_get_unique_id(&l, &h) < 0) {
			printf("unique ID[0x%08x][0x%08x] error\r\n", (int)h, (int)l);
		} else {
			printf("unique ID[0x%08x][0x%08x] success\r\n", (int)h, (int)l);
		}

	} else if (!strncmp(argv[1], "quary", 5)) {
		printf("=>[quary]\r\n");
		printf("             is_secure_enable()=%d\r\n", is_secure_enable());
		printf("       is_data_area_encrypted()=%d\r\n", is_data_area_encrypted());
		printf("             is_signature_rsa()=%d\r\n", is_signature_rsa());
		printf("is_signature_rsa_chsum_enable()=%d\r\n", is_signature_rsa_chsum_enable());
		printf("           is_JTAG_DISABLE_en()=%d\r\n", is_JTAG_DISABLE_en());
		printf("        is_1st_key_programmed()=%d\r\n", is_1st_key_programmed());
		printf("        is_2nd_key_programmed()=%d\r\n", is_2nd_key_programmed());
		printf("        is_3rd_key_programmed()=%d\r\n", is_3rd_key_programmed());
		printf("        is_4th_key_programmed()=%d\r\n", is_4th_key_programmed());
		printf("        is_5th_key_programmed()=%d\r\n", is_5th_key_programmed());
		printf("        is_1st_key_read_lock()=%d\r\n", is_1st_key_read_lock());
		printf("        is_2nd_key_read_lock()=%d\r\n", is_2nd_key_read_lock());
		printf("        is_3rd_key_read_lock()=%d\r\n", is_3rd_key_read_lock());
		printf("        is_4th_key_read_lock()=%d\r\n", is_4th_key_read_lock());
		printf("        is_5th_key_read_lock()=%d\r\n", is_5th_key_read_lock());
	} else if (!strncmp(argv[1], "power_trim_data", 15)) {
		BOOL 	is_found;
		INT32	code;
		UINT32	power_trim;
		printf("=>[power trim]\r\n");

		code = otp_key_manager(7);

		if(code == -33) {
			printf("Read power trim error\r\n");
		} else {
			power_trim = 0;
	    	is_found = extract_trim_valid(code, &power_trim);
	    	if(is_found) {
				printf("Read power trim = 0x%08x\r\n", power_trim);
	    	} else {
				printf("Read power trim = 0x%08x(NULL)\r\n", power_trim);
	    	}
	    }

	} else if (!strncmp(argv[1], "dump_leakage_data", 17)) {
		UINT32 leakage = 1;
		UINT32 duty;

		printf("[CPU][1200MHz]\r\n");
		for(leakage=1; leakage <= LEAKAGE_MAX; leakage++) {
			duty = extract_trim_to_pwm_duty(PWM1_CPU_POWER, leakage, 1200);
			printf("	=>efuse[%d] -> duty = [%02d]\r\n", leakage, duty);
		}

		printf("[CPU][1400MHz]\r\n");
		for(leakage=1; leakage <= LEAKAGE_MAX; leakage++) {
			duty = extract_trim_to_pwm_duty(PWM1_CPU_POWER, leakage, 1400);
			printf("	=>efuse[%d] -> duty = [%02d]\r\n", leakage, duty);
		}

		printf("[CORE][2400MHz]\r\n");
		for(leakage=1; leakage <= LEAKAGE_MAX; leakage++) {
			duty = extract_trim_to_pwm_duty(PWM0_CORE_POWER, leakage, 2400);
			printf("	=>efuse[%d] -> duty = [%02d]\r\n", leakage, duty);
		}

		printf("[CORE][2666MHz]\r\n");
		for(leakage=1; leakage <= LEAKAGE_MAX; leakage++) {
			duty = extract_trim_to_pwm_duty(PWM0_CORE_POWER, leakage, 2666);
			printf("	=>efuse[%d] -> duty = [%02d]\r\n", leakage, duty);
		}

	} else {
		return CMD_RET_USAGE;
	}
	return 0;
}


U_BOOT_CMD(
	nvt_secure_en,  2,  0,  do_nvt_secure_en_cmd,
	"secure enable related API",
	"[Option] \n"
	"              [secure_en]\n"
	"              [data_encrypt]\n"
	"              [rsa_en]\n"
	"              [rsa_chk_en]\n"
	"              [jtag_disable]\n"
	"              [quary]\n"
	"              [uniqueue_id]\n"
	"              [power_trim_data]\n"
	"              [dump_leakage_data]\n"
);

int do_nvt_jtag_dis_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	efuse_jtag_dis();
	return 0;
}


U_BOOT_CMD(
	nvt_jtag_disable,  1,  0,  do_nvt_jtag_dis_cmd,
	"jtag disable",
	"After jtag disable, JTAG will disable forever\n"
);

