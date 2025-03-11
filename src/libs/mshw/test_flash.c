#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <mtd/mtd-abi.h>

#include "hardware.h"

typedef struct {
    char                dev[32];
    unsigned int        baseaddr;
    unsigned int        size;
    unsigned int        erasesize;
    char                name[32];
    unsigned int        blockNum;
    unsigned int        badblock[128];
} _mtd_device_t;
static _mtd_device_t g_mtd_device[16];

static int ms_badblock_check()
{
    int ret = 0;
    int fd;
    FILE *fp;
    int i = 0, j = 0;
    unsigned int start_addr = 0, mtdoffset = 0;
    int mtd_num = 0, total_bad_block = 0;
    char buf[128];
    char log[128];
    char mtd_dev[64];
    const char *MTD_FILE = "/proc/mtd";
    const char *MTD_FMT = "%[^:]: %x %x %s";
    _mtd_device_t *pMtdDev;

#if defined(_HI3536A_)
    hdinfo("emmc does not require bad block detection!\n");
    return 0;
#endif
    /*******parse /proc/mtd*******/
    fp = fopen(MTD_FILE, "r");
    if (!fp) {
        hderr("%s open failed\n", MTD_FILE);
        return -1;
    }

    mtd_num = 0;
    start_addr = 0;
    while (fgets(buf, 1024, fp)) {
        if (strstr(buf, "mtd")) {
            pMtdDev = &g_mtd_device[mtd_num++];
            sscanf(buf, MTD_FMT, mtd_dev, &pMtdDev->size, &pMtdDev->erasesize, pMtdDev->name);
            snprintf(pMtdDev->dev, sizeof(pMtdDev->dev), "/dev/%s", mtd_dev);
            pMtdDev->baseaddr = start_addr;
            start_addr += pMtdDev->size;
//          msprintf("%s, %08llx, %08x, %s", pMtdDev->dev, pMtdDev->size, pMtdDev->erasesize, pMtdDev->name);
        }
    }
    fclose(fp);

    /*******check bad block*******/
    for (i = 0; i < mtd_num; i++) {
        pMtdDev = &g_mtd_device[i];
        pMtdDev->blockNum = 0;

        if ((fd = open(pMtdDev->dev,  O_RDONLY)) == -1) {
            hderr("%s open failed\n", pMtdDev->dev);
            continue;
        }

        while (mtdoffset < pMtdDev->size) {
            loff_t offs = mtdoffset;

            ret = ioctl(fd, MEMGETBADBLOCK, &offs);
            if (ret < 0) {
                close(fd);
                hderr("ioctl MEMGETBADBLOCK failed, %s, error=%d\n", pMtdDev->dev, ret);
                break;
            }

            if (ret > 0) {
                pMtdDev->badblock[pMtdDev->blockNum++] = pMtdDev->baseaddr + mtdoffset;  //baseaddr + offaddr
            }

            mtdoffset += pMtdDev->erasesize;
        }
        mtdoffset = 0;
        close(fd);
    }

    /*******write file*******/
    for (i = 0; i < mtd_num; i++) {
        pMtdDev = &g_mtd_device[i];
        if (pMtdDev->blockNum > 0) {
            snprintf(log, sizeof(log), "%s (%s) : \n", pMtdDev->dev, pMtdDev->name);
            hdinfo(log);
            for (j = 0; j < pMtdDev->blockNum; j++) {
                snprintf(log, sizeof(log), "Bad block found at : %08x\n", pMtdDev->badblock[j]);
                hdinfo(log);
            }
            total_bad_block += pMtdDev->blockNum;
        }
    }
    snprintf(log, sizeof(log), "Total bad block : %d\n\n", total_bad_block);
    hdinfo(log);

    return 0;
}

static int ms_df_info()
{
    FILE *pfd = NULL;
    char buf[128];

    pfd = ms_vpopen("df", "r");
    if (pfd) {
        while (fgets(buf, sizeof(buf), pfd) != NULL) {
            hdinfo(buf);
        }
        ms_vpclose(pfd);
        return 0;
    }

    return -1;
}

static int test_flash_start(void *param)
{
    ms_df_info();
    ms_badblock_check();
    return 0;
}

static int test_stop()
{
    return 0;
}

static int test_find()
{
    return 1;
}

HD_MODULE_INFO(TEST_NAME_FLASH, test_find, test_flash_start, test_stop);

static int test_disk_start(void *param)
{
    FILE *pfd = NULL;
    char buf[128];

    pfd = ms_vpopen("ls /dev/MF*", "r");
    if (pfd) {
        while (fgets(buf, sizeof(buf), pfd) != NULL) {
            hdinfo(buf);
        }
        ms_vpclose(pfd);
        return 0;
    }

    return -1;
}

HD_MODULE_INFO(TEST_NAME_DISK, test_find, test_disk_start, test_stop);

