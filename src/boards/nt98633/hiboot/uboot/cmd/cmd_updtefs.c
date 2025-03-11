#include <common.h>
#include <command.h>

static int do_updatefs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char cmd[64];
    unsigned int addr = 0x10000000;
    unsigned int emmcAddr = 0x12400;
    if (argc != 3) {
        return -1;
    }

    sprintf(cmd,"tftp 0x10000000 %s", argv[2]);
    run_command(cmd, 0);
    sprintf(cmd,"mmc erase.part %s", argv[1]);
    run_command(cmd, 0);
    
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0x1128", addr, emmcAddr);
    run_command(cmd, 0);
    
    addr += (0x1128 * 512);
    emmcAddr += 0x4000 //278528
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0x20", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0x20 * 512);
    emmcAddr += 0x40000//540672
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0x40000//802816
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0x1c8//803272
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0x3fe38//1064960
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0x40000//1064960
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0x40000//1327104
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0x1c8//1327560
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0x3fe38//1589248
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0xf", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0xf * 512);
    emmcAddr += 0xf60//1593184
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0x6b00", addr, emmcAddr);
    run_command(cmd, 0);

    addr += (0x6b00 * 512);
    emmcAddr += 0x6b00//1620576
    sprintf(cmd,"mmc write 1 0x%08x 0x%08x 0x14000", addr, emmcAddr);
    run_command(cmd, 0);
    printf("\n 11111\n")
    return 0;
}

U_BOOT_CMD(
    updatefs, 3, 0, do_updatefs,
    "download ext4fs",
    "updatefs fs1/2 filename"
);


