#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/nvt-common/nvt_common.h>

#define LINUX_FLASH_ADDR			0x050000
#define CONFIG_CUSTOM_BOOTARGS		CONFIG_BOOTARGS_COMMON "rootfstype=squashfs ro  mtdparts=spi_nor.0  root=/dev/mtdblock3 "

static unsigned int fdt_backup_addr;

void nvt_load_combine_img(ulong fdt_addr)
{
	unsigned int linux_image_size;
	unsigned char *tmp_addr;
	char command[128];

    sprintf(command, "sf probe 0");
	run_command(command, 0);

	tmp_addr = (unsigned char *)CONFIG_LINUX_SDRAM_START;
	sprintf(command, "sf read 0x%lx 0x%x 0x%x", tmp_addr, LINUX_FLASH_ADDR, 0x100);
	run_command(command, 0);

    linux_image_size = (tmp_addr[12] << 24) | (tmp_addr[13] << 16) | (tmp_addr[14] << 8) | tmp_addr[15];
    linux_image_size += 64;

    /* load FDT image */
    fdt_backup_addr = fdt_addr;
	sprintf(command, "sf read 0x%lx 0x%x 0x%x", fdt_addr, LINUX_FLASH_ADDR + linux_image_size, _EMBMEM_BLK_SIZE_);
	run_command(command, 0);

    /* load Linux image */
	sprintf(command, "sf read 0x%lx 0x%x 0x%x", CONFIG_LINUX_SDRAM_START + 0x100, LINUX_FLASH_ADDR + 0x100, linux_image_size - 0x100);
	run_command(command, 0);
}

int do_nvt_boot_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char buf[CONFIG_CMDLINE_SIZE] = {0};

	/* To assign relocated fdt address */
	sprintf(buf, "0x%08x ", _BOARD_LINUX_MAXBLK_ADDR_ + _BOARD_LINUX_MAXBLK_SIZE_);
	setenv("fdt_high", buf);

	/* The following will setup the lmb memory parameters for bootm cmd */
	sprintf(buf, "0x%08x ", _BOARD_LINUX_ADDR_ + _BOARD_LINUX_SIZE_);
	setenv("bootm_size", buf);
	setenv("bootm_mapsize", buf);
	sprintf(buf, "0x%08x ", _BOARD_LINUX_ADDR_);
	setenv("bootm_low", buf);

    setenv("bootargs", CONFIG_CUSTOM_BOOTARGS);

    sprintf(buf, "bootm %x - %lx", CONFIG_LINUX_SDRAM_START, fdt_backup_addr);
    run_command(buf, 0);

    return 0;
}

U_BOOT_CMD(
    nvt_boot,   1,  0,  do_nvt_boot_cmd,
    "To do nvt platform boot init.", "\n"
);