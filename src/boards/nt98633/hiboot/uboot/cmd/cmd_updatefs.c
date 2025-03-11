#include <common.h>
#include <command.h>
#include <asm/nvt-common/nvt_common.h>


static int do_updatefs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char cmd[64];
    unsigned int addr = 0x10000000;

    if (argc != 3) {
        return -1;
    }

    sprintf(cmd,"tftp 0x%x %s", addr, argv[2]);
    run_command(cmd, 0);

    nvt_sparse_image_update(addr, 0x2480000, 0x6400000, 0x35800000);

    return 0;
}

U_BOOT_CMD(
    updatefs, 3, 0, do_updatefs,
    "download ext4fs",
    "updatefs fs1/2 filename"
);


