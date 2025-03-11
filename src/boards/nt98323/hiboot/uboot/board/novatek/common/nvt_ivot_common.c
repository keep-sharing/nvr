/**
    NVT update handling api

    @file       nvt_ivot_fw_update_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <jffs2/jffs2.h>
#include <mmc.h>
#include <nand.h>
#include <fdt_support.h>
#include <libfdt.h>
#include <linux/ctype.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <asm/nvt-common/modelext/emb_partition_info.h>
#include <asm/nvt-common/shm_info.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/IOAddress.h>
#include "nvt_ivot_pack.h"
#ifdef CONFIG_MS_NVR
#include <asm/io.h>
#include <asm/gpio.h>
#endif
unsigned long nvt_tm0_cnt_beg = 0;
unsigned long nvt_tm0_cnt_end = 0;
unsigned long nvt_shminfo_comm_uboot_boot_func = 0;
unsigned long nvt_shminfo_comm_core1_start = 0;
unsigned long nvt_shminfo_comm_core2_start = 0;
unsigned long nvt_shminfo_comm_itron_comp_addr = 0;
unsigned long nvt_shminfo_comm_itron_comp_len = 0;
unsigned long nvt_shminfo_comm_fw_update_addr = 0;
unsigned long nvt_shminfo_comm_fw_update_len = 0;
unsigned int nvt_shminfo_boot_fdt_addr = 0;
unsigned long nvt_ramdisk_addr = 0;
unsigned long nvt_ramdisk_size = 0;
uint8_t *nvt_fdt_buffer = NULL;
extern HEADINFO gHeadInfo;

void nvt_shminfo_init(void)
{
	SHMINFO *p_shminfo;
	//MODELEXT_HEADER *header;

	//p_bininfo = (BININFO *)modelext_get_cfg((unsigned char*)CONFIG_SMEM_SDRAM_BASE, MODELEXT_TYPE_BIN_INFO, &header);
	p_shminfo = (SHMINFO *)CONFIG_SMEM_SDRAM_BASE;

	#ifdef CONFIG_NVT_FPGA_EMULATION
	p_shminfo->boot.fdt_addr = (UINT32)_BOARD_FDT_ADDR_;
	memset(p_shminfo->boot.LdInfo_1, 0, sizeof(p_shminfo->boot.LdInfo_1));
	memcpy(p_shminfo->boot.LdInfo_1, "LD_NVT", 6);
	p_shminfo->boot.LdCtrl2 |= LDCF_BOOT_CARD;
	p_shminfo->comm.Resv[0] = BOOT_REASON_NORMAL;
	p_shminfo->comm.Resv[1] = 0; // COMM_CORE1_START
	p_shminfo->comm.Resv[2] = 0; // COMM_CORE2_START
	p_shminfo->comm.Resv[3] = 0; // COMM_UITRON_COMP_ADDR
	p_shminfo->comm.Resv[4] = 0; // COMM_UITRON_COMP_LEN
	#endif /* CONFIG_NVT_FPGA_EMULATION */

#ifdef CONFIG_NVT_IVOT_SOC_UART_UPGRADE_SUPPORT
	if (strncmp(p_shminfo->boot.LdInfo_1, "LD_NVT", 6) != 0) {


	    if (gHeadInfo.ModelextAddr != 0) {
		    printf("%sAttention!!!! Please update to latest version loader%s", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		    while(1);
	    }
	    /* uart upgrade use non all in one */
	    p_shminfo->boot.fdt_addr = CONFIG_UBOOT_SDRAM_BASE + gHeadInfo.BinLength;
	    nvt_shminfo_boot_fdt_addr = (unsigned int)&p_shminfo->boot.fdt_addr;
	    nvt_fdt_buffer = malloc(FDT_SIZE);

	    return;
	}

#elif CONFIG_NVT_IVOT_SOC_UTILS
	if (strncmp(p_shminfo->boot.LdInfo_1, "LD_NVT", 6) != 0) {
		printf("%sAttention!!!! Please update to latest version loader%s", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		while(1);
	}
#endif


	nvt_shminfo_comm_uboot_boot_func = (unsigned long)&p_shminfo->comm.Resv[0];
	nvt_shminfo_comm_core1_start = (unsigned long)&p_shminfo->comm.Resv[1];
	nvt_shminfo_comm_core2_start = (unsigned long)&p_shminfo->comm.Resv[2];
	nvt_shminfo_comm_itron_comp_addr = (unsigned long)&p_shminfo->comm.Resv[3];
	nvt_shminfo_comm_itron_comp_len = (unsigned long)&p_shminfo->comm.Resv[4];
	nvt_shminfo_comm_fw_update_addr = (unsigned long)&p_shminfo->comm.Resv[5];
	nvt_shminfo_comm_fw_update_len = (unsigned long)&p_shminfo->comm.Resv[6];
	nvt_shminfo_boot_fdt_addr = (unsigned int)&p_shminfo->boot.fdt_addr;
	nvt_fdt_buffer = malloc(FDT_SIZE);
}

int nvt_fdt_init(bool reload)
{
	int ret;
#if 1
	if (reload) {
		ulong fdt_real_size, alloc_size = SZ_512K;
		ulong preload_size = ALIGN_CEIL(sizeof(struct fdt_header), _EMBMEM_BLK_SIZE_);
		ulong tmp_addr  = (ulong)memalign(CONFIG_SYS_CACHELINE_SIZE, alloc_size);

		if (!tmp_addr) {
			printf("fdt malloc fail\n");
			return -1;
		}
		if (preload_size > alloc_size) {
			printf("preload_size 0x%x > 0x%x\n", alloc_size);
			return -1;
		}

		memset((unsigned char *)tmp_addr, 0, alloc_size);
		//read first block to get fdt size
	#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
		char command[128];
		#if defined(CONFIG_NVT_SPI_NOR_NAND)
		sprintf(command, "sf read 0x%lx 0x%x 0x%x", tmp_addr, CONFIG_MODELEXT_FLASH_BASE, preload_size);
		ret = run_command(command, 0);
		#elif defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_SPI_NAND) && defined(CONFIG_NVT_LINUX_RAMDISK_BOOT))
		sprintf(command, "nand read %lx %x %x", tmp_addr, CONFIG_MODELEXT_FLASH_BASE, preload_size);
		ret = run_command(command, 0);
		#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_SPI_NOR) && defined(CONFIG_NVT_LINUX_RAMDISK_BOOT))
		sprintf(command, "sf read 0x%lx 0x%x 0x%x", tmp_addr, CONFIG_MODELEXT_FLASH_BASE, preload_size);
		ret = run_command(command, 0);
		#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_IVOT_EMMC) && defined(CONFIG_NVT_LINUX_RAMDISK_BOOT))
		/* MMC read should use block number unit */
		sprintf(command, "mmc read 0x%lx 0x%x 0x%x", tmp_addr, CONFIG_MODELEXT_FLASH_BASE, ALIGN_CEIL(preload_size,MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN);
		ret = run_command(command, 0);
		#elif defined(CONFIG_NVT_LINUX_SD_BOOT) || defined(CONFIG_NVT_LINUX_RAMDISK_BOOT)
		sprintf(command, "fatload mmc 0:1 0x%lx %s", tmp_addr, get_nvt_bin_name(NVT_BIN_NAME_TYPE_MODELEXT));
		ret = run_command(command, 0);
		#else
		/* All-in-one SD boot */
		#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */

		if (ret) {
			printf("fdt init fail return %d\n", ret);
			free((void*)tmp_addr);
			return ret;
		}
		if ((ret = fdt_check_header((void *)tmp_addr)) != 0) {
			printf("invalid fdt header, addr=0x%08X er = %d \n", (unsigned int)tmp_addr, ret);
			free((void*)tmp_addr);
			return ret;
		}
		fdt_real_size = (ulong)fdt_totalsize(tmp_addr);
		debug("fdt size = %d\n", (unsigned int)fdt_real_size);

		if (fdt_real_size > alloc_size) {
			printf("fdt_real_size 0x%x > 0x%x\n", alloc_size);
			free((void*)tmp_addr);
			return -1;
		}

		//read remain size
		if (fdt_real_size > preload_size) {
			#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
			sprintf(command, "nand read %lx %x %x", tmp_addr+preload_size, CONFIG_MODELEXT_FLASH_BASE+preload_size, fdt_real_size-preload_size);
			ret = run_command(command, 0);
			#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
			sprintf(command, "sf read 0x%lx 0x%x 0x%x", tmp_addr+preload_size, CONFIG_MODELEXT_FLASH_BASE+preload_size, fdt_real_size-preload_size);
			ret = run_command(command, 0);
			#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
			/* MMC read should use block number unit */
			ulong num = ALIGN_CEIL(fdt_real_size, MMC_MAX_BLOCK_LEN)/MMC_MAX_BLOCK_LEN;
			if ((num * MMC_MAX_BLOCK_LEN) > alloc_size) {
				printf("fdt_real_size 0x%x > 0x%x\n", num*MMC_MAX_BLOCK_LEN, alloc_size);
				free((void*)tmp_addr);
				return -1;
			}
			sprintf(command, "mmc read 0x%lx 0x%x 0x%x", tmp_addr, CONFIG_MODELEXT_FLASH_BASE, num);
			ret = run_command(command, 0);
			#elif defined(CONFIG_NVT_LINUX_SD_BOOT) || defined(CONFIG_NVT_LINUX_RAMDISK_BOOT)
			sprintf(command, "fatload mmc 0:1 0x%lx %s", tmp_addr+preload_size, get_nvt_bin_name(NVT_BIN_NAME_TYPE_MODELEXT));
			ret = run_command(command, 0);
			#else
			/* All-in-one SD boot */
			#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */
		}
		// fdt in flash, maybe its size larger than loader passed one
		if (fdt_totalsize(nvt_fdt_buffer) < fdt_real_size) {
			free(nvt_fdt_buffer);
			nvt_fdt_buffer = malloc(fdt_real_size);
			if (nvt_fdt_buffer == NULL) {
				nvt_dbg(ERR, "%sfailed to alloc (%08X) bytes for fdt %s", ANSI_COLOR_RED, fdt_real_size, ANSI_COLOR_RESET);
				free((void*)tmp_addr);
				while(1);
			}
		}
		memcpy(nvt_fdt_buffer, (void*)tmp_addr, fdt_real_size);
	#else /* !CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
		/* FIXME: To do customized boot */

		ret = 0;

	#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
		free((void*)tmp_addr);
		if (ret) {
			printf("fdt init fail return %d\n", ret);
			return ret;
		}
	} else {
#ifdef CONFIG_NVT_IVOT_SOC_UTILS
		ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
#else
		ulong fdt_addr  = (ulong)memalign(CONFIG_SYS_CACHELINE_SIZE, _EMBMEM_BLK_SIZE_);

#ifdef CONFIG_NVT_IVOT_SOC_COMMON_UTILS
		nvt_load_combine_img(fdt_addr);
		nvt_writel(fdt_addr, (ulong)nvt_shminfo_boot_fdt_addr);
#endif
#endif
		if ((ret = fdt_check_header((void *)fdt_addr)) != 0) {
			printf("invalid fdt header, addr=0x%08X er = %d \n", (unsigned int)fdt_addr, ret);
			return ret;
		}
		/* Copy fdt data from loader fdt area to uboot fdt area */
		memcpy(nvt_fdt_buffer, (void*)fdt_addr, (ulong)fdt_totalsize((void*)fdt_addr));
	}

	if ((ret = fdt_check_full((void *)nvt_fdt_buffer, fdt_totalsize((void *)nvt_fdt_buffer))) != 0) {
		printf("invalid fdt format\n");
		return ret;
	}

#else
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);

	if (reload) {
		memset((unsigned char *)fdt_addr, 0, alloc_size);
	#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
		char command[128];
		#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
		sprintf(command, "nand read %lx %x %x", fdt_addr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_SMEM_SDRAM_SIZE);
		ret = run_command(command, 0);
		#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
		sprintf(command, "sf read 0x%lx 0x%x 0x%x", fdt_addr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_SMEM_SDRAM_SIZE);
		ret = run_command(command, 0);
		#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
		/* MMC read should use block number unit */
		sprintf(command, "mmc read 0x%lx 0x%x 0x%x", fdt_addr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_SMEM_SDRAM_SIZE/MMC_MAX_BLOCK_LEN);
		ret = run_command(command, 0);
		#elif defined(CONFIG_NVT_LINUX_SD_BOOT)
		sprintf(command, "fatload mmc 0:1 0x%lx %s", fdt_addr, get_nvt_bin_name(NVT_BIN_NAME_TYPE_MODELEXT));
		ret = run_command(command, 0);
		#else
		/* All-in-one SD boot */
		#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */
	#else /* !CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
		/* FIXME: To do customized boot */

	#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
		if (ret) {
			printf("fdt init fail return %d\n", ret);
			return ret;
		}
	}

	/* Copy fdt data from loader fdt area to uboot fdt area */
	memcpy(nvt_fdt_buffer, (void*)fdt_addr, FDT_SIZE);
#endif
	return 0;
}

/*
 * Return 0
 *		fdt
 * else
 *		none
 */
int nvt_check_isfdt(ulong addr)
{
	int  nodeoffset, ret;
	#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	char path[128] = {0};
	#endif

	ret = fdt_check_header((void *)addr);
	if (ret < 0) {
		debug("fdt address %lx is not valid with error code: %d\n", addr, ret);
		return -1;
	}

#if 0 //there are fdt.system and fdt.application, so the the existing of '/nand' is not necessary
	#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	sprintf(path, "/mmc@%x", IOADDR_SDIO3_REG_BASE);
	nodeoffset = fdt_path_offset((void*)addr, path);

	#else
	nodeoffset = fdt_path_offset((void*)addr, "/nand");
	#endif
	if (nodeoffset < 0)
		return -1;
	else
		return 0;
#endif

	return 0;
}

void nvt_print_system_info(void)
{
	printf("\r%s \
			\r CONFIG_MEM_SIZE              =      0x%08x \n \
			\r CONFIG_NVT_UIMAGE_SIZE       =      0x%08x \n \
			\r CONFIG_UBOOT_SDRAM_BASE      =      0x%08x \n \
			\r CONFIG_UBOOT_SDRAM_SIZE      =      0x%08x \n \
			\r CONFIG_LINUX_SDRAM_BASE      =      0x%08x \n \
			\r CONFIG_LINUX_SDRAM_SIZE      =      0x%08x \n \
			\r CONFIG_LINUX_SDRAM_START     =      0x%08x %s \n", ANSI_COLOR_YELLOW, CONFIG_MEM_SIZE,
														CONFIG_NVT_UIMAGE_SIZE, CONFIG_UBOOT_SDRAM_BASE,
														CONFIG_UBOOT_SDRAM_SIZE, CONFIG_LINUX_SDRAM_BASE,
														CONFIG_LINUX_SDRAM_SIZE, CONFIG_LINUX_SDRAM_START,
														ANSI_COLOR_RESET);
}

/*
 * to parsing necessary dts info
 * format: phandle is /nand, /nor and 
 *
 */
int nvt_dts_config_parsing(void)
{
	int  nodeoffset = -1, nextoffset, subnode, ret;
	const char *ptr = NULL;
	char *endptr = NULL;
	u64 addr, size;
	const unsigned long long *val = NULL;
	unsigned int idx = 0;
	char cmd[512];
	char buf[80];

	memset(cmd, 0, 512);
	memset(buf, 0, 80);
	/* We will find the partition table node firstly */
	sprintf(cmd, "/nand");
	nodeoffset = fdt_path_offset((void*)nvt_fdt_buffer, cmd);
	subnode = fdt_first_subnode((void*)nvt_fdt_buffer, nodeoffset);
	ptr = fdt_get_name((const void*)nvt_fdt_buffer, subnode, NULL);
	if (ptr != NULL && strncmp(ptr, "partition_", 10) == 0) {
		sprintf(cmd, "nand0=spi_nand.0");
		ret = setenv("mtdids", cmd);
		if (ret) {
			nvt_dbg(ERR, "%s: error set\n", __func__);
			return ret;
		}

		sprintf(cmd, "mtdparts=spi_nand.0:");
		goto nvt_dts_cfg_getnode;
	}

	sprintf(cmd, "/nor");
	nodeoffset = fdt_path_offset((void*)nvt_fdt_buffer, cmd);
	subnode = fdt_first_subnode((void*)nvt_fdt_buffer, nodeoffset);
	ptr = fdt_get_name((const void*)nvt_fdt_buffer, subnode, NULL);
	if (ptr != NULL && strncmp(ptr, "partition_", 10) == 0) {
		sprintf(cmd, "nor0=spi_nor.0");
		ret = setenv("mtdids", cmd);
		if (ret) {
			nvt_dbg(ERR, "%s: error set\n", __func__);
			return ret;
		}

		sprintf(cmd, "mtdparts=spi_nor.0:");
		goto nvt_dts_cfg_getnode;
	}

	sprintf(cmd, "/emmc");
	nodeoffset = fdt_path_offset((void*)nvt_fdt_buffer, cmd);
	subnode = fdt_first_subnode((void*)nvt_fdt_buffer, nodeoffset);
	ptr = fdt_get_name((const void*)nvt_fdt_buffer, subnode, NULL);
	if (nodeoffset < 0 || strncmp(ptr, "partition_", 10) != 0)
		return -1;
	else
		sprintf(cmd, "mtdparts=emmc:");

nvt_dts_cfg_getnode:
	fdt_for_each_subnode(subnode, nvt_fdt_buffer, nodeoffset) {
		/* Got every subnode */
		ptr = fdt_getprop(nvt_fdt_buffer, subnode, "label", NULL);
		val = (const unsigned long long *)fdt_getprop(nvt_fdt_buffer, subnode, "reg", NULL);
		addr = be64_to_cpu(val[0]);
		size = be64_to_cpu(val[1]);
		if (ptr != NULL && strncmp(ptr, "all", 3) != 0) {
			nvt_dbg(FUNC, "Label: %s PartitionOffset=%llx PartitionSize=%llx\n", ptr, addr, size);
			sprintf(buf, "0x%llx@0x%llx(%s),", size, addr, ptr);
			strcat(cmd, buf);
		}
	}

	/* To handle uboot mtd env config */
	cmd[strlen(cmd) - 1] = '\0';
	ret = setenv("mtdparts", cmd);

	return 0;
}

#ifdef CONFIG_NVT_LINUX_EMMC_BOOT
lbaint_t nvt_mmc_sparse_write(struct sparse_storage *info, lbaint_t blk, lbaint_t blkcnt, const void *buffer)
{
	char command[128];

	sprintf(command, "mmc write 0x%lx 0x%lx 0x%lx", (unsigned long)buffer, blk, blkcnt);
	run_command(command, 0);
	return blkcnt;
}

static lbaint_t nvt_mmc_sparse_read(struct sparse_storage *info, lbaint_t blk, lbaint_t blkcnt, const void *buffer)
{
	char command[128];

	sprintf(command, "mmc read 0x%lx 0x%lx 0x%lx", (unsigned long)buffer, blk, blkcnt);
	run_command(command, 0);
	return blkcnt;
}

lbaint_t nvt_mmc_sparse_reserve(struct sparse_storage *info, lbaint_t blk, lbaint_t blkcnt)
{
	return blkcnt;
}

void nvt_mmc_sparse_msg_print(const char *str, char *response)
{
	nvt_dbg(IND, "%s\n", str);
	return;
}

int nvt_sparse_image_update(u32 addr, u64 off, u64 size, u64 part_size)
{
	char command[128];
	u64 align_off = ALIGN_CEIL(off, MMC_MAX_BLOCK_LEN);
	/* Using block unit */
	align_off /= MMC_MAX_BLOCK_LEN;
	u32 align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN);
	/* Using block unit */
	align_size /= MMC_MAX_BLOCK_LEN;
	if (is_sparse_image((void*)addr)) {
		struct sparse_storage sparse;
		sparse.blksz = MMC_MAX_BLOCK_LEN;
		sparse.start = (lbaint_t)align_off;
		/* It's used to check if update size is larger than partition size */
		sparse.size = (lbaint_t)part_size/MMC_MAX_BLOCK_LEN;
		sparse.read = NULL;
		sparse.write = nvt_mmc_sparse_write;
		sparse.reserve = nvt_mmc_sparse_reserve;
		printf("Flashing sparse ext4 image at offset 0x%lx\n", sparse.start);
		write_sparse_image(&sparse, "Rootfs.ext4", (void *)addr, align_size);
	} else {
		printf("Flashing raw ext4 or fat image at offset 0x%llx\n", align_off);
		sprintf(command, "mmc write 0x%x 0x%llx 0x%x", addr, align_off, align_size);
		run_command(command, 0);
	}

	return 0;
}

int nvt_sparse_image_readback(u32 addr, u64 off, u64 size, u64 part_size)
{
	char command[128];
	int ret = 0;
	u64 align_off = ALIGN_CEIL(off, MMC_MAX_BLOCK_LEN);
	/* Using block unit */
	align_off /= MMC_MAX_BLOCK_LEN;
	u32 align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN);
	/* Using block unit */
	align_size /= MMC_MAX_BLOCK_LEN;
	if (is_sparse_image((void*)addr)) {
			struct sparse_storage sparse;
			sparse.blksz = MMC_MAX_BLOCK_LEN;
			sparse.start = (lbaint_t)align_off;
			sparse.size = (lbaint_t)part_size/MMC_MAX_BLOCK_LEN;
			sparse.write = NULL;
			sparse.read = nvt_mmc_sparse_read;
			sparse.reserve = nvt_mmc_sparse_reserve;
			printf("Read sparse ext4 image at offset 0x%lx\n", sparse.start);
			read_sparse_image(&sparse, "Rootfs.ext4", (void *)addr, align_size);
	} else {
			printf("Read raw ext4 image at offset 0x%llx\n", off);
			sprintf(command, "mmc read 0x%llx 0x%llx 0x%llx", addr, align_off, align_size);
			run_command(command, 0);
	}

	return 0;
}
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */

void nvt_ivot_set_cpuclk(void)
{
	int  nodeoffset = -1, subnode = -1, ret = -1;
	u32 *cell = NULL;
	u32 cpu_freq = 0;
	char cmd[512];
	char buf[80];

	memset(cmd, 0, 512);
	memset(buf, 0, 80);

	/* We will find the cpu clock node firstly */
	sprintf(cmd, "/cpus");
	nodeoffset = fdt_path_offset((void*)nvt_fdt_buffer, cmd);
	subnode = fdt_first_subnode((void*)nvt_fdt_buffer, nodeoffset);
	cell = (u32*)fdt_getprop((const void*)nvt_fdt_buffer, subnode, "clock-frequency", NULL);
	if (cell == NULL) {
            cpu_freq = 1400;
            printf("%sAttention!!!! DTS not find cpu clock-frequency, use default clock %d %s", ANSI_COLOR_RED, cpu_freq, ANSI_COLOR_RESET);
	} else {
            cpu_freq = __be32_to_cpu(cell[0]) / 1000000;
            printf("DTS find cpu freq clock %dMHz\n", cpu_freq);
	}

	sprintf(cmd, "nvt_cpu_freq %d", cpu_freq);
	ret = run_command(cmd, 0);

	if (ret) {
	    printf("run do_nvt_cpu_freq fail\n");
	}

	ret = run_command("nvt_ddr_power", 0);

	if (ret) {
	    printf("run do_nvt_ddr_power fail\n");
	}
}

#ifdef CONFIG_MS_NVR
#define VER_MSB       GPIO_2(19)
#define VER_LSB       GPIO_2(18)

enum ver_hw_en {
	HW_VER_NONE = 0,
	HW_VER_1009,
	HW_VER_5016,
};

NVT_HW_VER nvt_hw_ver = NVT_HW_VER_NONE;
int nvt_ivot_hw_version(void)
{
    unsigned int ver = 0;

    if (nvt_hw_ver != NVT_HW_VER_NONE) {
        return nvt_hw_ver;
    }
    
    gpio_direction_input(VER_MSB);
    gpio_direction_input(VER_LSB);
    ver = gpio_get_value(VER_LSB);
    ver |= (gpio_get_value(VER_MSB) << 1);
    
    switch (ver) {
        case 0x00:
            nvt_hw_ver = NVT_HW_VER_5016;
            printf("board hardware : 5016\n");
            break;
        case 0x02:
            nvt_hw_ver = NVT_HW_VER_1009;
            printf("board hardware : 1009\n");
            break;
        default:
            nvt_hw_ver = NVT_HW_VER_NONE;
            printf("board hardware : unknown(0x%x)\n", ver);
            break;
    }

    return nvt_hw_ver;
}
#endif

int nvt_board_init(void)
{
	int ret = 0;
#ifdef CONFIG_MS_NVR
    nvt_ivot_hw_version();
#endif
	ret = nvt_ivot_hw_init();
	if (ret < 0) {
		return -1;
	}

#ifndef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
#ifndef CONFIG_MS_NVR
	ret = nvt_dts_config_parsing();
	if (ret < 0)
		return -1;
#endif /* CONFIG_MS_NVR */
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

	return 0;
}
