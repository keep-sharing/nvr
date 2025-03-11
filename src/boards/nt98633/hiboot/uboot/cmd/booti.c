// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <image.h>
#include <lmb.h>
#include <mapmem.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#if defined(CONFIG_ARM64) && defined(CONFIG_ARCH_NOVATEK)
#include <linux/libfdt.h>
#include <fdt_support.h>
#endif

DECLARE_GLOBAL_DATA_PTR;
/*
 * Image booting support
 */
static int booti_start(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[], bootm_headers_t *images)
{
	int ret;
	ulong ld;
	ulong relocated_addr;
	ulong image_size;
	uint8_t *temp;
	ulong dest;
	ulong dest_end;
	unsigned long comp_len;
	unsigned long decomp_len;
	int ctype;

	ret = do_bootm_states(cmdtp, flag, argc, argv, BOOTM_STATE_START,
			      images, 1);

	/* Setup Linux kernel Image entry point */
	if (!argc) {
		ld = load_addr;
		debug("*  kernel: default image load address = 0x%08lx\n",
				load_addr);
	} else {
		ld = simple_strtoul(argv[0], NULL, 16);
		debug("*  kernel: cmdline image address = 0x%08lx\n", ld);
	}

	temp = map_sysmem(ld, 0);
	ctype = image_decomp_type(temp, 2);
	if (ctype > 0) {
		dest = env_get_ulong("kernel_comp_addr_r", 16, 0);
		comp_len = env_get_ulong("kernel_comp_size", 16, 0);
		if (!dest) {
		    const char *str = env_get("kernel_comp_addr_r");
		    if (str == NULL) {
			    puts("kernel_comp_addr_r is not provided!\n");
			    return -EINVAL;
		    }
		}
		if (!comp_len) {
			puts("kernel_comp_size is not provided!\n");
			return -EINVAL;
		}

		printf("kernel image compression type %d size = 0x%08lx address = 0x%08lx\n",
			ctype, comp_len, (ulong)dest);
		decomp_len = comp_len * 10;
		ret = image_decomp(ctype, 0, ld, IH_TYPE_KERNEL,
				 (void *)dest, (void *)ld, comp_len,
				 decomp_len, &dest_end);
		if (ret)
			return ret;
		/* dest_end contains the uncompressed Image size */
		memmove((void *) ld, (void *)dest, dest_end);
	}
	unmap_sysmem((void *)ld);

	/* Prevent the kernel from starting beyond the memory range for fastboot */
#if defined(CONFIG_ARM64) && defined(CONFIG_ARCH_NOVATEK)
	int  nodeoffset;
	u32 *val = NULL;
	int addr_cells, size_cells;
	unsigned long addr = 0, size = 0;
	unsigned long fdt_addr = simple_strtoul(argv[2], NULL, 16);

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/memory");
	if (nodeoffset > 0) {
		addr_cells = fdt_address_cells((void *)fdt_addr, nodeoffset);
		size_cells = fdt_size_cells((void *)fdt_addr, nodeoffset);

		val = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "reg", NULL);
		if (val != NULL) {
			addr = fdt_read_number(val, addr_cells);
			size = fdt_read_number(val + addr_cells, size_cells);
		} else {
			printf("can not find reg on device tree\n");
			return -1;
		}
	} else {
		printf("can not find /memory on device tree\n");
		return -1;
	}

	printf("Boot Kernel from 0x%lx to 0x%lx\n", ld, addr);
	/* If the kernel image is not in the linux memory range, copy the kernel to the linux memory range */
	if ((ld < addr) || (ld >= (addr + size))) {
		/* If the kernel is nvt type image, the kernel size should be kernel_comp_size */
		if (ctype <= 0) {
			dest_end = env_get_ulong("kernel_comp_size", 16, 0);
		}
		printf("Copy Kernel to 0x%lx, size 0x%lx\n", addr, dest_end);
		memmove((void *)addr, (void *)ld, dest_end);
		ld = addr;
	}
#endif

	ret = booti_setup(ld, &relocated_addr, &image_size, false);
	if (ret != 0)
		return 1;

	/* Handle BOOTM_STATE_LOADOS */
	if (relocated_addr != ld) {
		debug("Moving Image from 0x%lx to 0x%lx\n", ld, relocated_addr);
		memmove((void *)relocated_addr, (void *)ld, image_size);
	}

	images->ep = relocated_addr;
	lmb_reserve(&images->lmb, images->ep, le32_to_cpu(image_size));

	/*
	 * Handle the BOOTM_STATE_FINDOTHER state ourselves as we do not
	 * have a header that provide this informaiton.
	 */
	if (bootm_find_images(flag, argc, argv))
		return 1;

	return 0;
}

int do_booti(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	/* Consume 'booti' */
	argc--; argv++;

	if (booti_start(cmdtp, flag, argc, argv, &images))
		return 1;

	/*
	 * We are doing the BOOTM_STATE_LOADOS state ourselves, so must
	 * disable interrupts ourselves
	 */
	bootm_disable_interrupts();

	images.os.os = IH_OS_LINUX;
	images.os.arch = IH_ARCH_ARM64;
	ret = do_bootm_states(cmdtp, flag, argc, argv,
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
			      BOOTM_STATE_RAMDISK |
#endif
			      BOOTM_STATE_OS_PREP | BOOTM_STATE_OS_FAKE_GO |
			      BOOTM_STATE_OS_GO,
			      &images, 1);

	return ret;
}

#ifdef CONFIG_SYS_LONGHELP
static char booti_help_text[] =
	"[addr [initrd[:size]] [fdt]]\n"
	"    - boot arm64 Linux Image stored in memory\n"
	"\tThe argument 'initrd' is optional and specifies the address\n"
	"\tof an initrd in memory. The optional parameter ':size' allows\n"
	"\tspecifying the size of a RAW initrd.\n"
#if defined(CONFIG_OF_LIBFDT)
	"\tSince booting a Linux kernel requires a flat device-tree, a\n"
	"\tthird argument providing the address of the device-tree blob\n"
	"\tis required. To boot a kernel with a device-tree blob but\n"
	"\twithout an initrd image, use a '-' for the initrd argument.\n"
#endif
	"";
#endif

U_BOOT_CMD(
	booti,	CONFIG_SYS_MAXARGS,	1,	do_booti,
	"boot arm64 Linux Image image from memory", booti_help_text
);
