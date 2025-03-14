/**
    NVT evb board file
    To handle na51xxx basic init.
    @file       na51xxxevb.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/


#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/armv8/mmu.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/na51090evb.h>
#include <linux/libfdt.h>

#ifdef CONFIG_NVT_PCIE
#include <cli.h>
#include <asm/arch/nvt_pcie.h>
#endif

DECLARE_GLOBAL_DATA_PTR;
extern int nvt_mmc_init(int id);
extern int na51090_eth_initialize(bd_t *bis);
const char *boardinfo[] = {
	"Board:"_CHIP_NAME_"EVB\n"
};

static struct mm_region na51090_a64_evb_mem_map[] = {
	{
	    .virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
	    .virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
			 /* TODO: Do we need these? */
			 /* | PTE_BLOCK_PXN | PTE_BLOCK_UXN */

	}, {
	    .virt = 0x2f0000000UL, /* For 0x2F000_0000 peri io */
		.phys = 0x2f0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
			 /* TODO: Do we need these? */
			 /* | PTE_BLOCK_PXN | PTE_BLOCK_UXN */

	}, {
	    .virt = 0x400000000UL, /* For 0x4_0000_0000 pcie space  */
		.phys = 0x400000000UL,
		.size = 0x1000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
			 /* TODO: Do we need these? */
			 /* | PTE_BLOCK_PXN | PTE_BLOCK_UXN */

	}, {

		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = na51090_a64_evb_mem_map;

void reset_cpu(unsigned long ignored)
{
	nvt_ivot_reset_cpu();
}


/**
 * @brief board_early_init_f
 *
 * @return 0
 */
int board_early_init_f(void)
{
	//nvt_ipc_init();

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r(void)
{
	return 0;
}
#endif

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	int ret = 0;
#ifndef CONFIG_ARMV8_SET_SMPEN
	unsigned long  cval = 0;
#endif

#ifdef CONFIG_NVT_IVOT_BOOT_FAST
	char command[100] = {0};
	sprintf(command, "nvt_uart_disable");
	ret = run_command(command, 0);
#endif
	nvt_tm0_cnt_beg = get_nvt_timer0_cnt();

	printf("Relocation to 0x%08lx, Offset is 0x%08lx sp at %08lx\n", gd->relocaddr, gd->reloc_off, gd->start_addr_sp);

	icache_disable();
	dcache_disable();

	invalidate_icache_all();
	invalidate_dcache_all();

	icache_enable();
	dcache_enable();

	nvt_shminfo_init();
	ret = nvt_fdt_init(false);
	if (ret < 0) {
		printf("fdt init fail\n");
	}

#ifdef CONFIG_ARMV8_MULTIENTRY
	/* Loader will use the register value as a pc address
	then slave core will jump to the address when register value is not equal to zero */
	nvt_writel(CONFIG_UBOOT_SDRAM_BASE + gd->reloc_off, LOADER_CPU_RELEASE_ADDR);
#endif


	ret = nvt_board_init_early();
	if (ret < 0) {
		printf("board init early fail\n");
		return 0;
	}

#ifndef CONFIG_NVT_PCIE_DAUGHTER_BRD
#ifdef CONFIG_NVT_PCIE
	/* check EP or RC */
	ret = run_command("nvt_pcie_getid", 0);
	if (ret == NVT_PCIE_EP) {
		/* If it is EP, start booti */
		run_command("nvt_pcie", 0);

		/* If nvt_pcie returns unexpectedly, enter the uboot shell */
		cli_loop();
		return 0;
	}
#endif
#endif

	nvt_print_system_info();

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret = 0;
#ifdef CONFIG_NVT_MMC
	int i;
	int CONFIG_NVT_MMC_CHANNEL = 0;

#ifdef CONFIG_NVT_MMC_CHANNEL_SDIO1
	CONFIG_NVT_MMC_CHANNEL |= 1 << 0;
#endif

#ifdef CONFIG_NVT_MMC_CHANNEL_SDIO2
	CONFIG_NVT_MMC_CHANNEL |= 1 << 1;
#endif

#ifdef CONFIG_NVT_MMC_CHANNEL_SDIO3
	CONFIG_NVT_MMC_CHANNEL |= 1 << 2;
#endif

	for (i = 0; i < CONFIG_NVT_MMC_MAX_NUM; i++) {
		if((CONFIG_NVT_MMC_CHANNEL >> i) & 0x1) {
			ret = nvt_mmc_init(i);
			if(ret)
				break;
		}
	}
#endif
	return ret;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_ETHNET
	rc = na51090_eth_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
static int nvt_handle_fw_abin(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};
	/* To handle firmware update */
	ret = nvt_fw_update(false);
	if (ret < 0) {
		switch (ret) {
		case ERR_NVT_UPDATE_FAILED:
			printf("%sUpdate fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			return -1;
			break;
		case ERR_NVT_UPDATE_OPENFAILED:
			printf("Open SD fail:%s No SD device? %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			break;
		case ERR_NVT_UPDATE_READ_FAILED:
			printf("%sRead SD fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			return -1;
			break;
		case ERR_NVT_UPDATE_NO_NEED:
			printf("%sNo need to update (%s) %s\r\n", ANSI_COLOR_YELLOW, get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW), ANSI_COLOR_RESET);
			break;
		default:
			break;
		}
		ret = nvt_fdt_init(true);
		if (ret < 0) {
			printf("modelext init fail\n");
		}
	} else{
		printf("%sUpdate successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	}
	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 */
#ifndef CONFIG_MS_NVR
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;
	env_set("bootargs",cmdline);
#endif
	return 0;
}

static int nvt_handle_fw_tbin(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};

	ret = nvt_fw_load_tbin();
	if (ret < 0) {
		switch (ret) {
		case ERR_NVT_UPDATE_OPENFAILED:
			printf("Open SD fail:%s No SD device? (%s) %s\r\n", ANSI_COLOR_YELLOW, get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW), ANSI_COLOR_RESET);
			break;
		case ERR_NVT_UPDATE_NO_NEED:
		case ERR_NVT_UPDATE_READ_FAILED:
			printf("%sRead SD fail (%s) %s\r\n", ANSI_COLOR_RED, get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW), ANSI_COLOR_RESET);
			return -1;
			break;
		default:
			break;
		}
	} else
		printf("%sLoad successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);

	ret = nvt_fdt_init(false);
	if (ret < 0) {
		printf("modelext init fail\n");
		return ret;
	}

	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 */
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;

	env_set("bootargs",cmdline);

	return 0;
}

static int nvt_handle_fw_update_fw(void)
{
	int ret = 0;
	unsigned long val = 0;

	printf("%sStarting to update firmware runtime %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	/* To handle firmware update */
	ret = nvt_fw_update(true);
	if (ret < 0) {
		printf("%sUpdate fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		val = (nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & ~COMM_UBOOT_BOOT_FUNC_BOOT_DONE_MASK) | COMM_UBOOT_BOOT_FUNC_BOOT_NG;
		nvt_writel(val, (ulong)nvt_shminfo_comm_uboot_boot_func);
		flush_dcache_all();
		return -1;
	} else {
		printf("%sUpdate firmware successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);

		// Update finished.
		val = (nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & ~COMM_UBOOT_BOOT_FUNC_BOOT_DONE_MASK) | COMM_UBOOT_BOOT_FUNC_BOOT_DONE;
		nvt_writel(val, (ulong)nvt_shminfo_comm_uboot_boot_func);
		flush_dcache_all();
		while(1) {
			// Waiting for itron trigger reboot.
			printf(".");
			mdelay(1000);
		}
	}

	return 0;
}

static int nvt_handle_update_fw_by_usb_eth(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};

	printf("%sStarting to update firmware from USB/ETH%s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	/* To handle firmware update */
	ret = nvt_fw_update(true);
	if (ret < 0) {
		printf("%sUpdate fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		return -1;
	} else {
		printf("%sUpdate firmware successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	}

	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 * Continue to boot
	 */
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;

	env_set("bootargs",cmdline);
	return 0;
}

static int nvt_handle_format_rw_rootfs(void)
{
	int ret = 0;
	ret = nvt_process_rootfs_format();

	return ret;
}

static int nvt_handle_recovery_sys(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};
	ret = nvt_process_sys_recovery();

	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 */
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;

	env_set("bootargs",cmdline);

	return ret;
}

#ifdef CONFIG_NVT_LINUX_AUTODETECT
static int nvt_handle_fw_auto(void)
{
	int ret = 0;

	unsigned long boot_reason = nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & COMM_UBOOT_BOOT_FUNC_BOOT_REASON_MASK;

	if (boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_UPDFIRM) {
		// Update by itron: firmware will be loaded by itron.
		ret = nvt_handle_fw_update_fw();
		if (ret < 0)
			return ret;
	} else if ((boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_UPD_FRM_USB) || \
				(boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_UPD_FRM_ETH)){
		// Update by USB/ETH: firmware will be loaded by loader.
		ret = nvt_handle_update_fw_by_usb_eth();
		if (ret < 0)
			return ret;
	} else if (nvt_detect_fw_tbin()) {
		ret = nvt_handle_fw_tbin();
		if (ret < 0)
			return ret;
		#ifndef CONFIG_NVT_LINUX_RAMDISK_BOOT
		char cmdline[512] = {0};
		char buf[255] = {0};
		/*
		 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
		 */
		sprintf(buf,"%s ",env_get("bootargs"));
		strcat(cmdline, buf);
		ret = nvt_part_config((char *)cmdline, NULL);
		if (ret < 0)
			return ret;
		env_set("bootargs",cmdline);
		#endif /* CONFIG_NVT_LINUX_RAMDISK_BOOT */
	} else if (boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_FORMAT_ROOTFS){
		// Format R/W rootfs, we should check if this partition is existed firstly.
		ret = nvt_handle_format_rw_rootfs();
		if (ret < 0)
			return ret;
	} else if (boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_RECOVERY_SYS){
		// Recovery system if loader send event to do system recovery. (EMMC boot only)
		ret = nvt_handle_recovery_sys();
		if (ret < 0)
			return ret;
	} else {
		ret = nvt_handle_fw_abin();
		if (ret < 0)
			return ret;
	}
	return 0;
}
#endif /* CONFIG_NVT_LINUX_AUTODETECT */
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

#if defined(CONFIG_NVT_IVOT_EMMC)
static int nvt_emmc_set_bootbus(void)
{
	int ret = 0;
	char command[128] = {0};
	int bus_width;
	unsigned int pinmux_value = nvt_readl((ulong)IOADDR_TOP_REG_BASE);

	if (pinmux_value & 8) {
		bus_width = 2; // 8 bit
		nvt_dbg(MSG, "Set EMMC boot bus to 8-bit mode\n");
	} else {
		bus_width = 1; // 4 bit
		nvt_dbg(MSG, "Set EMMC boot bus to 4-bit mode\n");
	}
	sprintf(command, "mmc bootbus %d %d 0 0", CONFIG_NVT_IVOT_EMMC, bus_width);
	ret = run_command(command, 0);
	if (ret < 0){
		nvt_dbg(ERR, "command : [%s] failed\n", command);
	}
	return ret;
}

static int nvt_emmc_init(void)
{
	int ret = 0;
	char command[128] = {0};

	/* Switch to emmc bus and user partition access config */
	sprintf(command, "mmc dev %d", CONFIG_NVT_IVOT_EMMC);
	ret = run_command(command, 0);
	if (ret < 0)
		return ret;

	ret = nvt_emmc_set_bootbus();
	if (ret < 0)
		return ret;

	sprintf(command, "mmc partconf %d 1 %d 0", CONFIG_NVT_IVOT_EMMC, CONFIG_NVT_IVOT_EMMC_BOOT_PART);
	ret = run_command(command, 0);
	return ret;
}
#elif defined(CONFIG_NVT_SPI_NOR) || defined(CONFIG_NVT_SPI_NOR_NAND)
static int nvt_norflash_init(void)
{
	int ret = 0;
	char command[128] = {0};

	sprintf(command, "sf probe");
	ret = run_command(command, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "nor flash init failed\n");
	}

	return ret;
}
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */
/**
 * @brief misc_init_r - To do nvt update and board init.
 *
 * @return 0
 */
int misc_init_r(void)
{
	int ret = 0;


#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	nvt_dbg(FUNC, "%sFirmware name: %s %s %s %s \n", ANSI_COLOR_YELLOW,
						  get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW),
						  get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW),
						  get_nvt_bin_name(NVT_BIN_NAME_TYPE_MODELEXT),
						  ANSI_COLOR_RESET);
	nvt_dbg(FUNC, "boot time: %lu(us) \n", get_nvt_timer0_cnt());
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

#if defined(CONFIG_NVT_IVOT_EMMC)
	ret = nvt_emmc_init();
	if (ret < 0)
		return 0;
#elif defined(CONFIG_NVT_SPI_NOR) || defined(CONFIG_NVT_SPI_NOR_NAND)
	ret = nvt_norflash_init();
	if (ret < 0)
		return 0;
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */

#ifndef CONFIG_NVT_PCIE_DAUGHTER_BRD
#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	#ifdef CONFIG_NVT_LINUX_AUTODETECT
	ret = nvt_handle_fw_auto();
	if (ret < 0)
		return 0;
	#else /* !CONFIG_NVT_LINUX_AUTODETECT */
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_LINUX_SPINORNAND_BOOT)
	ret = nvt_handle_fw_abin();
	if (ret < 0)
		return 0;
	#else /* !defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_LINUX_SPINORNAND_BOOT) */
	ret = nvt_handle_fw_tbin();
	if (ret < 0)
		return 0;
	#endif /* defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_LINUX_SPINORNAND_BOOT) */
	#endif /* CONFIG_NVT_LINUX_AUTODETECT */
#else /* !CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
	/* FIXME: To do customized boot */
	ret = nvt_fdt_init(true);
	if (ret < 0) {
		printf("modelext init fail\n");
		return 0;
	}
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
#endif /* !CONFIG_NVT_PCIE_DAUGHTER_BRD */

	/* Why to do this again?
	 * This is because modelext maybe updated, we should init again here.
	 */
	nvt_dbg(FUNC, "boot time: %lu(us) \n", get_nvt_timer0_cnt());
	ret = nvt_board_init();
	if (ret < 0) {
		printf("board init fail\n");
		return 0;
	}

	nvt_dbg(FUNC, "boot time: %lu(us) \n", get_nvt_timer0_cnt());

	return 0;
}

/*
 * dram init.
 */
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

phys_size_t get_effective_memsize(void)
{
	/* We should report the uboot dram size due to relocate
	 * address will be calculated by this size to get ram top address,
	 * #ifdef CONFIG_SYS_SDRAM_BASE
	 *   gd->ram_base = CONFIG_SYS_SDRAM_BASE;
	 * #endif
	 *   gd->ram_top = gd->ram_base + get_effective_memsize();
	 *   gd->ram_top = board_get_usable_ram_top(gd->mon_len);
	 *   gd->relocaddr = gd->ram_top;
	 */
	return CONFIG_SYS_SDRAM_SIZE;
}

/*
 * get_board_rev() - get board revision
 */
u32 get_board_rev(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}

