/**
    NVT utilities for pice customization

    @file       nvt_pcie_boot_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/libfdt.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/IOAddress.h>
#include "nvt_ivot_pack.h"
#include <malloc.h>
#include <mapmem.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <fs.h>
#include <nand.h>
#include <mmc.h>
#include <spi_flash.h>
#include <fdt_support.h>
#include <asm/arch/nvt_pcie.h>
#include <environment.h>
#include <mapmem.h>

#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
#include <asm/nvt-common/nvt_ivot_aes_smc.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>
#endif

#define EP_LOADER_SIZE 0x10000
/* This buffer is a tmperoary buffer for put uboot + eploader */
#define TMP_BUFFER_SIZE 0x100000
#define CMD_BUFFER_SIZE 255

/**************************************************************************
 *
 *
 * Novatek common function
 *
 *
 **************************************************************************/

#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
static void get_bininfo_size_offset(unsigned char* bininfo,unsigned int* size,unsigned int *offset)
{

	        *size = bininfo[4] | bininfo[5]<<8 | bininfo[6] << 16 | bininfo[7] << 24;
		        *offset = bininfo[0] | bininfo[1] << 8 | bininfo[2] << 16 | bininfo[3] <<24;
}
#endif

static int nvt_common_get_linux_mem(const void *fdt_addr, u64 *addr, u64 *size)
{
	int  nodeoffset, nextoffset;
	const char *ptr = NULL;
	u32 *val = NULL;
	int addr_cells, size_cells;

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/memory");
	if (nodeoffset > 0) {
		addr_cells = fdt_address_cells(fdt_addr, nodeoffset);
		size_cells = fdt_size_cells(fdt_addr, nodeoffset);

		val = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "reg", NULL);
		if (val != NULL) {
			*addr = fdt_read_number(val, addr_cells);
			*size = fdt_read_number(val + addr_cells, size_cells);
			nvt_dbg(MSG, "EP Linux memory address : 0x%lx, size : 0x%lx\n", *addr, *size);
		} else {
			nvt_dbg(ERR, "can not find reg on device tree\n");
			return -1;
		}
	}
	return 0;
}

/**************************************************************************
 *
 *
 * Novatek RC boot EP Flow
 *
 *
 **************************************************************************/
static int nvt_rc_boot_copy_from_storage(unsigned long dest_addr, unsigned long src_addr, unsigned long size)
{
	char cmd[CMD_BUFFER_SIZE];
	u32 align_size = 0;
	align_size = ALIGN_CEIL(size, _EMBMEM_BLK_SIZE_);
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) && defined(CONFIG_NVT_SPI_NAND)) \
	|| defined(CONFIG_NVT_LINUX_SPINORNAND_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) && defined(CONFIG_NVT_SPI_NOR_NAND))
	sprintf(cmd, "nand read 0x%lx 0x%lx 0x%lx", dest_addr, src_addr, align_size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) && defined(CONFIG_NVT_SPI_NOR))
	align_size = ALIGN_CEIL(align_size, ARCH_DMA_MINALIGN);
	sprintf(cmd, "sf read 0x%lx 0x%lx 0x%lx", dest_addr, src_addr, align_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) && defined(CONFIG_NVT_IVOT_EMMC))
	unsigned long align_off;
	align_size = ALIGN_CEIL(align_size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
	align_off = ALIGN_CEIL(src_addr, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
	sprintf(cmd, "mmc read 0x%lx 0x%lx 0x%lx", dest_addr, align_off, align_size);
#else
	nvt_dbg(ERR, "A.bin can not set EMBMEM_NONE on nvt_info.dtsi\n");
#endif
	nvt_dbg(MSG, "%s\n", cmd);
	run_command(cmd, 0);
	flush_cache(round_down((unsigned long)dest_addr, CONFIG_SYS_CACHELINE_SIZE), round_up(align_size, CONFIG_SYS_CACHELINE_SIZE));

	return 0;
}

static int nvt_rc_boot_set_ep_image_addr(void *fdt_addr, unsigned int ep_addr, char *name)
{
	int  nodeoffset, ret = -1;
	const char *ptr = NULL;
	u64 *val = NULL;
	
	if (fdt_addr == NULL || name == NULL) {
		nvt_dbg(ERR, "Wrong input fdt_addr : %s, name : %s\n", (char*)fdt_addr, name);
		return ret;
	}

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/ep_info");
	if (nodeoffset > 0) {
		ret = fdt_setprop_u32(fdt_addr, nodeoffset, name, ep_addr);
	} else {
		nvt_dbg(ERR, "can not find /ep_info on device tree\n");
		return ret;
	}
	return ret;
}

static int nvt_rc_boot_get_epfdt(unsigned long *rcfdt_addr, unsigned long *epfdt_addr, unsigned long *epfdt_size)
{
	int ret;
	ulong rc_size;

	/* 1. Verify RC FDT and Get Size firstly */
	if ((ret = fdt_check_header((void *)*rcfdt_addr) != 0)) {
		nvt_dbg(ERR, "invalid fdt header, addr=0x%08lx er = %d \n", (unsigned long)_BOARD_LINUXTMP_ADDR_, ret);
		return ret;
	}
	rc_size = (ulong)fdt_totalsize((void *)*rcfdt_addr);
	if (rc_size <= 0) {
		nvt_dbg(ERR, "invalid fdt size, addr=0x%08lx er = %d \n", (unsigned long)_BOARD_LINUXTMP_ADDR_, ret);
		return ret;
	}

	/* 2. Verify EP FDT (fdt address: RC FDT + RC FDT Size) and Get Size */
	*epfdt_addr = (unsigned long)((unsigned long)*rcfdt_addr + (unsigned long)rc_size);
	if ((ret = fdt_check_header((void *)*epfdt_addr) != 0)) {
		nvt_dbg(ERR, "invalid ep fdt header, addr=0x%08lx er = %d\n", epfdt_addr, ret);
		return ret;
	}

	*epfdt_size = (ulong)fdt_totalsize((void *)*epfdt_addr);
	if (*epfdt_size <= 0) {
		nvt_dbg(ERR, "invalid ep fdt size, addr=0x%08lx er = %d \n", (unsigned long)epfdt_addr, ret);
		return ret;
	}

	return ret;

}

static int nvt_rc_boot_eploader_copy(bool tbin, unsigned long *src_addr, unsigned size, bool preloader)
{
	int ret;
	HEADINFO *head_info;
	char cmd[CMD_BUFFER_SIZE];
	unsigned long ep_addr;
	char *buffer = malloc(TMP_BUFFER_SIZE);
	if (buffer == NULL) {
		nvt_dbg(ERR, ": malloc fail\n");
		return -1;
	}

	/* 1. Copy to tmp buffer */
	if (tbin) {
		if (size > TMP_BUFFER_SIZE) {
			nvt_dbg(ERR, "Uboot size(0x%lx) > TMP_BUFFER_SIZE(0x%lx), Stop to boot\n", size, TMP_BUFFER_SIZE);
			goto out;
		}
		memcpy((void *)buffer, src_addr, size);
	} else {
		size = 2 * _EMBMEM_BLK_SIZE_;
		if (size > TMP_BUFFER_SIZE) {
			nvt_dbg(ERR, "Uboot size(0x%lx) > TMP_BUFFER_SIZE(0x%lx), Stop to boot\n", size, TMP_BUFFER_SIZE);
			goto out;
		}
		nvt_rc_boot_copy_from_storage((unsigned long)buffer, (unsigned long)src_addr, size);
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
		head_info = (HEADINFO *)(buffer);
#else
		head_info = (HEADINFO *)(buffer + 0x300);
#endif


		/* TODO multiple EP flow */
		if (preloader) {
			/* Uboot + EP Loader + EP Pre-loader */
			size = head_info->BinLength + (EP_LOADER_SIZE * 2);
		} else {
			size = head_info->BinLength + EP_LOADER_SIZE;
		}
		nvt_rc_boot_copy_from_storage((unsigned long)buffer, (unsigned long)src_addr, size);
		if (size > TMP_BUFFER_SIZE) {
			nvt_dbg(ERR, "Uboot size(0x%lx) > TMP_BUFFER_SIZE(0x%lx), Stop to boot\n", size, TMP_BUFFER_SIZE);
			goto out;
		}
	}
	/* 2. Parsing NVT header to get real image size */
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
	head_info = (HEADINFO *)(buffer);
#else
	head_info = (HEADINFO *)(buffer + 0x300);
#endif
	if (preloader) {
		/* Uboot + EP Loader + EP Pre-loader */
		ep_addr = (unsigned long)buffer + head_info->BinLength + EP_LOADER_SIZE;
	} else {
		ep_addr = (unsigned long)buffer + head_info->BinLength;
	}
	nvt_dbg(MSG, ": EP loader addr = 0x%lx\n", ep_addr);

	/* 3. Copy EP Loader to EP SRAM */
	sprintf(cmd, "nvt_pcie_copy_sram 0x%lx", ep_addr);
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		goto out;
	}
out:
	if (buffer) {
		free(buffer);
	}
	return ret;
}

static int nvt_rc_boot_ep_loader_process_tbin(unsigned int id, NVTPACK_MEM* p_mem, void* p_user_data)
{
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	unsigned long src_addr = (unsigned long)p_mem->p_data;
	unsigned long size = p_mem->len;
	int ret = 0;

	switch(pEmb[id].EmbType)
	{
		case EMBTYPE_UBOOT:
			ret = nvt_rc_boot_eploader_copy(true, (unsigned long *)src_addr, size, false);
			break;
		default:
			break;
	}
	return ret;
}

static int nvt_rc_boot_ep_loader_process_abin(void* p_user_data)
{
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	int ret = -1, id;

	for (id = 0; id < EMB_PARTITION_INFO_COUNT; id++) {
		if (pEmb[id].EmbType == EMBTYPE_UBOOT) {
			unsigned long src_addr = pEmb[id].PartitionOffset;
			unsigned long size = pEmb[id].PartitionSize;
			ret = nvt_rc_boot_eploader_copy(false, (unsigned long *)src_addr, size, false);
			break;
		}
	}
	return ret;
}

static int nvt_rc_boot_ep_partition_enum_copy_abin(void* p_user_data)
{
	int ret = 0, id;
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	unsigned long map_base_addr = simple_strtoul(env_get("nvt_pcie_ep0_mau_base"),NULL,16);
	unsigned long src_addr;
	unsigned long size;
	unsigned long pcie_dest_addr;
	static unsigned long epfdt_addr = 0;
	static unsigned long epfdt_size = 0;
	HEADINFO *head_info;
	image_header_t *hdr;

	char label[CMD_BUFFER_SIZE];
	char cmd[CMD_BUFFER_SIZE];


	for (id = 0; id < EMB_PARTITION_INFO_COUNT; id++) {
		src_addr = pEmb[id].PartitionOffset;
		size = pEmb[id].PartitionSize;
		switch(pEmb[id].EmbType)
		{
			case EMBTYPE_FDT:
				nvt_dbg(MSG, "Copy EP DTS : \n");

				if (size > _BOARD_FDT_SIZE_) {
					nvt_dbg(ERR, "FDT Partition size(0x%lx) > BOARD_FDT_SIZE(0x%lx)\n", size, (unsigned long)_BOARD_FDT_SIZE_);
					return -1;
				}

				/* 1. Copy all DTS partition to RC BOARD_FDT_ADDR */
				pcie_dest_addr = (unsigned long)_BOARD_FDT_ADDR_;
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);

				/* 2. Get EP DTS address and size */
				if (nvt_rc_boot_get_epfdt(&pcie_dest_addr, &epfdt_addr, &epfdt_size) < 0)
					return -1;

				/* 3. Copy to destination */
				src_addr = epfdt_addr;
				pcie_dest_addr = (unsigned long)_BOARD_FDT_ADDR_ + map_base_addr;
				memcpy((void*)pcie_dest_addr, (void*)src_addr, epfdt_size);

				break;

			case EMBTYPE_ATF:
				nvt_dbg(MSG, "Copy EP ATF : \n");
				pcie_dest_addr = map_base_addr + (unsigned long)_BOARD_ATF_ADDR_;
				size = 2 * _EMBMEM_BLK_SIZE_;
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
				head_info = (HEADINFO *)(pcie_dest_addr);
#else
				head_info = (HEADINFO *)(pcie_dest_addr + 0x350);
#endif

				size = head_info->BinLength;
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);
				break;

#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
			case EMBTYPE_TEEOS:
				nvt_dbg(MSG, "Copy EP OPTEE : \n");
				pcie_dest_addr = map_base_addr + (unsigned long)_BOARD_TEEOS_ADDR_;
				size = 2 * _EMBMEM_BLK_SIZE_;
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
				head_info = (HEADINFO *)(pcie_dest_addr);
#else
				head_info = (HEADINFO *)(pcie_dest_addr + 0x350);
#endif
				size = head_info->BinLength;
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);
				break;
#endif
			case EMBTYPE_UBOOT:
				nvt_dbg(MSG, "Copy EP UBOOT : \n");
				pcie_dest_addr = map_base_addr + (unsigned long)_BOARD_UBOOT_ADDR_;
				size = 2 * _EMBMEM_BLK_SIZE_;
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
				head_info = (HEADINFO *)(pcie_dest_addr);
#else
				head_info = (HEADINFO *)(pcie_dest_addr + 0x300);
#endif

				size = head_info->BinLength;
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);
				break;
			case EMBTYPE_LINUX:
				nvt_dbg(MSG, "Copy EP LINUX : \n");
				pcie_dest_addr = map_base_addr + (unsigned long)_BOARD_LINUXTMP_ADDR_;

#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
				size = sizeof(HEADINFO);
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);

				head_info = (HEADINFO *)(pcie_dest_addr);
				size = head_info->BinLength;
#else
				size = sizeof(image_header_t);
				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);

				hdr = (image_header_t *)pcie_dest_addr;
				size = image_get_data_size(hdr) + sizeof(image_header_t);
#endif

				pcie_dest_addr = map_base_addr + (unsigned long)CONFIG_UBOOT_SDRAM_BASE - ALIGN_CEIL(size, _EMBMEM_BLK_SIZE_);
				ret = nvt_rc_boot_set_ep_image_addr((void *)epfdt_addr, (unsigned int)(CONFIG_UBOOT_SDRAM_BASE - ALIGN_CEIL(size, _EMBMEM_BLK_SIZE_)), "linux_addr");
				if (ret < 0) {
					return	ret;
				}

				nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);
				break;
			case EMBTYPE_ROOTFS:
				if (nvt_runfw_bin_eprootfs_chk_exist(nvt_fdt_buffer, id) == true) {
					nvt_dbg(MSG, "Copy EP-ROOTFS : \n");

					/* 1. Get Header from storage */
					pcie_dest_addr = map_base_addr + (unsigned long)(_BOARD_LINUXTMP_ADDR_);
					size = sizeof(image_header_t);
					nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);

					/* 2. Get Ramdisk size */
					hdr = (image_header_t *)pcie_dest_addr;
					size = image_get_data_size(hdr) + sizeof(image_header_t);
					size = ALIGN_CEIL(size, 4096);

					/* 3. Get EP linux range */
					u64 linux_addr, linux_size;
					ret = nvt_common_get_linux_mem((void *)epfdt_addr, &linux_addr, &linux_size);
					if (ret < 0) {
						return	ret;
					}

					/* 4. Set EP Ramdisk address */
					ret = nvt_rc_boot_set_ep_image_addr((void *)epfdt_addr, (unsigned int)(linux_addr + linux_size - size - 0x10000), "ramdisk_addr");
					if (ret < 0) {
						return	ret;
					}

					/* 5. Update EP DTS to EP DRAM */
					pcie_dest_addr = (unsigned long)_BOARD_FDT_ADDR_ + map_base_addr;
					memcpy((void*)pcie_dest_addr, (void*)epfdt_addr, epfdt_size);


					/* 6. Copy EP-rootfs to EP DRAM */
					pcie_dest_addr = map_base_addr + (unsigned long)(linux_addr + linux_size - size - 0x10000);
					nvt_rc_boot_copy_from_storage(pcie_dest_addr, src_addr, size);

					break;
				} else {
					continue;
				}
			default:
				continue;
		}
	}
	return 0;
}

static int nvt_rc_boot_ep_partition_enum_copy_tbin(unsigned int id, NVTPACK_MEM* p_mem, void* p_user_data)
{
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	unsigned long dest_addr;
	unsigned long src_addr = (unsigned long)p_mem->p_data;
	unsigned long size = p_mem->len;
	static unsigned long epfdt_addr = 0;
	static unsigned long epfdt_size = 0;

	int ret = 0;
	char cmd[CMD_BUFFER_SIZE];

	switch(pEmb[id].EmbType)
	{
		case EMBTYPE_FDT:
			nvt_dbg(MSG, "Copy EP DTS : 0x%lx\n",src_addr);
			if (nvt_rc_boot_get_epfdt(&src_addr, &epfdt_addr, &epfdt_size) < 0)
				return -1;
			dest_addr = (unsigned long)_BOARD_FDT_ADDR_;
			src_addr = epfdt_addr;
			size = epfdt_size;
			break;

		case EMBTYPE_ATF:
			nvt_dbg(MSG, "Copy EP ATF : \n");
			dest_addr = (unsigned long)_BOARD_ATF_ADDR_;
			break;

#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
          	case EMBTYPE_TEEOS:
			nvt_dbg(MSG, "Copy EP OPTEE : \n");
			dest_addr = (unsigned long)_BOARD_TEEOS_ADDR_;
			break;
#endif
		case EMBTYPE_UBOOT:
			nvt_dbg(MSG, "Copy EP UBOOT : \n");
			dest_addr = (unsigned long)_BOARD_UBOOT_ADDR_;
			break;
		case EMBTYPE_LINUX:
			nvt_dbg(MSG, "Copy EP LINUX : \n");
			dest_addr = (unsigned long)CONFIG_UBOOT_SDRAM_BASE - ALIGN_CEIL(size, _EMBMEM_BLK_SIZE_);
			ret = nvt_rc_boot_set_ep_image_addr((void *)epfdt_addr, (unsigned int)(CONFIG_UBOOT_SDRAM_BASE - ALIGN_CEIL(size, _EMBMEM_BLK_SIZE_)), "linux_addr");
			break;
		case EMBTYPE_ROOTFS:
			if (nvt_runfw_bin_eprootfs_chk_exist(nvt_fdt_buffer, id) == true) {
				nvt_dbg(MSG, "Copy EP-ROOTFS : \n");
#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
				src_addr += 64;
#endif
				/* 1. Get EP-rootfs size from Header */
				image_header_t *hdr = (image_header_t *)src_addr;
				size = image_get_data_size(hdr) + sizeof(image_header_t);
				size = ALIGN_CEIL(size, 4096);

				/* 2. Get EP linux range */
				u64 linux_addr, linux_size;
				unsigned long map_base_addr = simple_strtoul(env_get("nvt_pcie_ep0_mau_base"),NULL,16);
				ret = nvt_common_get_linux_mem((void *)epfdt_addr, &linux_addr, &linux_size);
				if (ret < 0) {
					return	ret;
				}

				/* 3. Set EP Ramdisk address */
				dest_addr = (unsigned long)(linux_addr + linux_size - size - 0x10000);
				ret = nvt_rc_boot_set_ep_image_addr((void *)epfdt_addr, (unsigned int)dest_addr, "ramdisk_addr");
				if (ret < 0) {
					return	ret;
				}

				/* 4. Copy to destination */
				unsigned long pcie_dest_addr = (unsigned long)_BOARD_FDT_ADDR_ + map_base_addr;
				memcpy((void*)pcie_dest_addr, (void*)epfdt_addr, epfdt_size);

				break;
			} else {
				return 0;
			}
		default:
			return 0;
	}

	/* Copy to EP DRAM */
	sprintf(cmd, "nvt_pcie_copy 0x%lx 0x%lx 0x%lx", src_addr, dest_addr, size);
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}
}

static int nvt_rc_boot_set_ep_preloader(void)
{
	EMB_PARTITION* pEmb = (EMB_PARTITION*)emb_partition_info_data_curr;
	int ret = -1, id;

	for (id = 0; id < EMB_PARTITION_INFO_COUNT; id++) {
		if (pEmb[id].EmbType == EMBTYPE_UBOOT) {
			unsigned long src_addr = pEmb[id].PartitionOffset;
			unsigned long size = pEmb[id].PartitionSize;
			ret = nvt_rc_boot_eploader_copy(false, (unsigned long *)src_addr, size, true);
			break;
		}
	}
	return ret;
}


/**
 * nvt_rc_boot_rc_boot_flow - The main flow for RC boot EP
 * @tbin: a flag for T.bin
 *
 * Init PCIE and create mapping table
 * Set communication register to idle
 * Get EP loader and copy to EP DRAM
 * Boot up EP CPU
 * Wait EP DRAM config done
 * Get all partition and copy to EP DRAM
 * Release EP CPU
 *
 * Returns 0 on success, < 0 otherwise
 */
static int nvt_rc_boot_flow(bool tbin)
{
	int ret;
	int i;
	char cmd[CMD_BUFFER_SIZE];
        EMB_PARTITION *pEmbCurr = emb_partition_info_data_curr;
	NVTPACK_ENUM_PARTITION_INPUT np_enum_input;
	/* Init PCIE and create mapping table */
	sprintf(cmd, "nvt_pcie_init");
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}

	/* 1. Get EP loader and copy to EP SRAM */
	if (tbin) {
		memset(&np_enum_input, 0, sizeof(np_enum_input));
		np_enum_input.mem = (NVTPACK_MEM){(void*)CONFIG_NVT_RUNFW_SDRAM_BASE, 0};;
		np_enum_input.p_user_data = pEmbCurr;
		np_enum_input.fp_enum = nvt_rc_boot_ep_loader_process_tbin;

		if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
			printf("failed with rc copy ep loader process.\r\n");
			return -1;
		}
	} else {
		ret = nvt_rc_boot_ep_loader_process_abin(pEmbCurr);
		if (ret < 0) {
			nvt_dbg(ERR, "nvt_rc_boot_ep_loader_process_abin fail\n");
			return ret;
		}
	}
	/* 2. Boot up EP CPU */
	sprintf(cmd, "nvt_pcie_boot");
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}

	/* Set communication register to idle */
	sprintf(cmd, "nvt_pcie_epstatus set idle");
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}

	/* 3. Wait EP DRAM config done */
	i = 0;
	sprintf(cmd, "nvt_pcie_epstatus get");
	ret = run_command(cmd, 0);
	while ((ret != 1) && (i < 200)) {   // 0 = ep not boot, so we should copy ep loader and boot ep cpu
		nvt_dbg(MSG, "Wait EP DRAM config done ret = %d\n");
		udelay(50000);
		ret = run_command(cmd, 0);
		i++;
	}
	if (i >= 200) {
		nvt_dbg(ERR, "Wait EP DRAM config done Time out !!!");
		return -1;
	}
	/* 4. Copy all EP image to EP DRAM */

	if (tbin) {
		memset(&np_enum_input, 0, sizeof(np_enum_input));
		np_enum_input.mem = (NVTPACK_MEM){(void*)CONFIG_NVT_RUNFW_SDRAM_BASE, 0};;
		np_enum_input.p_user_data = pEmbCurr;
		np_enum_input.fp_enum = nvt_rc_boot_ep_partition_enum_copy_tbin;

		if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
			printf("failed with copy other images.\r\n");
			return -1;
		}

	} else {
		ret = nvt_rc_boot_ep_partition_enum_copy_abin(pEmbCurr);
		if (ret < 0) {
			nvt_dbg(ERR, "nvt_rc_boot_ep_partition_enum_copy_abin fail\n");
			return ret;
		}

	}
	/* 5. Issue cmd to EP CPU starting boot */
	sprintf(cmd, "nvt_pcie_epstatus set start 0x%lx", _BOARD_FDT_ADDR_);
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}
	return 0;
}

/**************************************************************************
 *
 *
 * Novatek EP Boot Flow
 *
 *
 **************************************************************************/

static int nvt_ep_boot_set_env_by_dts(const void *fdt_addr)
{
	int  nodeoffset, nextoffset;
	const void *val = NULL;
	const char *name;

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/ep_info");
	if (nodeoffset > 0) {
		nodeoffset = fdt_first_subnode((const void*)fdt_addr, nodeoffset);
		fdt_for_each_property_offset(nextoffset, fdt_addr, nodeoffset) {
			val = fdt_getprop_by_offset(fdt_addr, nextoffset, &name, NULL);
			if (name != NULL && val != NULL) {
				nvt_dbg(MSG, "Set \"%s = %s\" to env\n",name,val);
				env_set(name, (void *)val);
			}
		}
	} else {
		nvt_dbg(ERR, "can not find /ep_info on device tree\n");
		return -1;
	}
	return 0;
}

static int nvt_ep_boot_get_ep_image_addr(const void *fdt_addr, unsigned int *ep_ramdisk_addr, char *name)
{
	int  nodeoffset, nextoffset;
	const char *ptr = NULL;
	u32 *val = NULL;

	if (ep_ramdisk_addr == NULL || name == NULL) {
		return -1;
	}

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/ep_info");
	if (nodeoffset > 0) {
		val = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, name, NULL);
		if (val != NULL) {
			*ep_ramdisk_addr = (unsigned long)fdt32_to_cpu(val[0]);
		} else {
			nvt_dbg(ERR, "can not find %s on device tree\n", name);
			return -1;
		}
	} else {
		nvt_dbg(ERR, "can not find /ep_info on device tree\n");
		return -1;
	}
	return 0;
}

static int nvt_ep_boot_get_ep_info(const void *fdt_addr, u32 *data)
{
	int  nodeoffset, nextoffset;
	const char *ptr = NULL;
	u32 *val = NULL;

	if (data == NULL) {
		return -1;
	}

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/ep_info");
	if (nodeoffset > 0) {
		val = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "enable", NULL);
		if (val != NULL) {
			*data = (unsigned int)fdt32_to_cpu(val[0]);
		}
	} else {
		nvt_dbg(ERR, "can not find /ep_info on device tree\n");
		return -1;
	}
	return 0;
}

#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
static int nvt_ep_decrypt_linux(unsigned long addr)
{
	int ret = 0;
	unsigned char test_aes_key[16]={0x04,0x03,0x02,0x01,0x08,0x07,0x06,0x05,0x12,0x11,0x10,0x09,0x16,0x15,0x14,0x013};
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_DATA_ENCRYPTED;
	if (1 != nvt_ivot_optee_efuse_operation(&efuse_data)) {
		printf("data encrypt not enable\n");
		return 0;
	}

	HEADINFO *headinfo = (HEADINFO *)addr;
	unsigned int crypto_offset = 0;
	unsigned int crypto_size = 0;
	unsigned char *encrypt_buf = NULL;
	unsigned char * bininfo = headinfo->BinInfo_3;
	get_bininfo_size_offset(headinfo->BinInfo_3, &crypto_size, &crypto_offset);

	encrypt_buf = (unsigned char *)(addr + crypto_offset);

	unsigned int total_size = headinfo->BinLength;

	unsigned int decrypt_size = total_size - crypto_offset;
	NVT_SMC_AES_DATA aes_data={0};
	aes_data.crypto_type = NVT_SMC_AES_CRYPTO_DECRYPTION;
	aes_data.aes_mode = NVT_SMC_AES_MODE_CBC;
	aes_data.key_buf = (unsigned char *)test_aes_key;
	memset(aes_data.IV,0,16);
	aes_data.key_size = 16;
	aes_data.input_data = (unsigned char *)encrypt_buf;
	aes_data.input_size = crypto_size;
	aes_data.output_data = (unsigned char *)addr;
	ret = nvt_ivot_optee_aes_operation(&aes_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_aes_operation fail\r\n",ret);
		return -1;
	}
	if(decrypt_size > crypto_size) {
		//there are some data not encrypt, need copy to buf
		unsigned char * tmp = (unsigned char *)addr;
		memcpy(&tmp[crypto_size], &encrypt_buf[crypto_size], (decrypt_size - crypto_size));

	}

	if(ret != 0) {
		printf("%s fail ret:%d\n", __func__, ret);
		return -1;
	}

	return 0;
}
#endif

static int nvt_ep_boot_prepare_env_and_boot(u64 *linux_addr, u64 *linux_size)
{
	char buf[CMD_BUFFER_SIZE], cmd[CMD_BUFFER_SIZE];
	u64 kernel_size;
	unsigned int nvt_ep_boot_ramdisk_addr, nvt_ep_boot_ramdisk_size, size;
	unsigned int nvt_ep_boot_linux_addr;
	image_header_t *hdr;
	int ret;

	/* To set the uboot env runtime. */
	ret = nvt_ep_boot_set_env_by_dts(nvt_fdt_buffer);
	if (ret < 0) {
		return ret;
	}
	/* To get the ramdisk address runtime. */
	ret = nvt_ep_boot_get_ep_image_addr(nvt_fdt_buffer, &nvt_ep_boot_ramdisk_addr, "ramdisk_addr");
	if (ret < 0) {
		return ret;
	}
	/* Get Ramdisk size and address */
	hdr = (image_header_t *)(unsigned long)nvt_ep_boot_ramdisk_addr;
	size = image_get_data_size(hdr) + sizeof(image_header_t);
	nvt_ep_boot_ramdisk_size = ALIGN_CEIL(size, 4096);


	sprintf(buf, "0x%08x ", nvt_ep_boot_ramdisk_addr + nvt_ep_boot_ramdisk_size);
	env_set("initrd_high", buf);

	/* To assign relocated fdt address */
	sprintf(buf, "0x%08x ", *linux_addr + *linux_size);
	env_set("fdt_high", buf);

	/* The following will setup the lmb memory parameters for bootm cmd */
	sprintf(buf, "0x%08x ", *linux_addr + *linux_size);
	env_set("bootm_size", buf);
	env_set("bootm_mapsize", buf);

	sprintf(buf, "0x%08x ", *linux_addr);
	env_set("bootm_low", buf);
	env_set("kernel_comp_addr_r", buf);

	/* Get kernel size */
	ret = nvt_ep_boot_get_ep_image_addr(nvt_fdt_buffer, &nvt_ep_boot_linux_addr, "linux_addr");
	if (ret < 0) {
		return ret;
	}

#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
	HEADINFO *headinfo = (HEADINFO *)(unsigned long)nvt_ep_boot_linux_addr;
	kernel_size = headinfo->BinLength;
	ret = nvt_ep_decrypt_linux(nvt_ep_boot_linux_addr);
	if (ret < 0){
		return ret;
	}
#else
	hdr = (image_header_t *)(unsigned long)nvt_ep_boot_linux_addr;
	kernel_size = image_get_data_size(hdr);

#endif
	sprintf(buf, "0x%x", kernel_size);
	env_set("kernel_comp_size", buf);
	run_command("pri", 0);

	sprintf(cmd, "booti");

	sprintf(cmd, "%s %x %lx %lx", cmd, nvt_ep_boot_linux_addr + sizeof(image_header_t), (unsigned long)nvt_ep_boot_ramdisk_addr, (unsigned long)nvt_fdt_buffer);
	printf("%s\n", cmd);
	run_command(cmd, 0);

	return 0;
}

static int nvt_ep_boot_flow()
{
	int ret;
	u64 linux_addr, linux_size;
	u32 data;

	/* init env */
	set_default_env(NULL, 0);
#ifdef CONFIG_OF_CONTROL
	env_set_hex("fdtcontroladdr",
			(unsigned long)map_to_sysmem(gd->fdt_blob));
#endif

	/* Verify EP device tree */
	ret = nvt_ep_boot_get_ep_info(nvt_fdt_buffer, &data);
	if (ret < 0) {
		return ret;
	}

	if (data == 0) {
		nvt_dbg(ERR, "!!! Stop Boot EP !!!\n");
		return -1;
	} else {
		/* normal boot ep */
	}

	/* Get linux memory address and size from device tree*/
	ret = nvt_common_get_linux_mem(nvt_fdt_buffer, &linux_addr, &linux_size);
	if (ret != 0) {
		return ret;
	}

	/* Prepare env for boot to linux and boot */
	ret = nvt_ep_boot_prepare_env_and_boot(&linux_addr, &linux_size);

	return ret;
}

/**************************************************************************
 *
 *
 * Novatek RC Boot EP Entry
 *
 *
 **************************************************************************/

int do_nvt_pcie(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = -1;
#ifdef CONFIG_NVT_PCIE_DAUGHTER_BRD
	ret = nvt_ep_boot_flow();
#endif
	ret = run_command("nvt_pcie_getid", 0);
	if (ret == NVT_PCIE_EP) {
		ret = nvt_ep_boot_flow();
#ifdef CONFIG_NVT_PCIE_CASCADE
	} else if (ret == NVT_PCIE_RC){
		if (argc == 2) {
			if (strncmp(argv[1], "preloader", strlen("preloader")) == 0) {
				printf("Copy preloader to EP SRAM\n");
				ret = nvt_rc_boot_set_ep_preloader();
				return ret;
			} else {
				printf("Unknow parameter !!!\n");
			}
		}	
		if (nvt_detect_fw_tbin()) {
			ret = nvt_rc_boot_flow(true);
		} else {
			ret = nvt_rc_boot_flow(false);
		}
#else
		nvt_dbg(ERR, "Not support boot ep on uboot \n");
		ret = -1;
#endif
	}

	return ret;
}

U_BOOT_CMD(
	nvt_pcie, 2,    1,  do_nvt_pcie,
	"nvt RC boot EP",
	""
);
