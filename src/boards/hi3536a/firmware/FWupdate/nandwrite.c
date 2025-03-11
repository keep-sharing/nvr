/* vi: set sw=4 ts=4: */
/*
 *  nandwrite.c
 *
 *  Ported to busybox from mtd-utils.
 *
 *  Copyright (C) 2000 Steven J. Hill (sjhill@realitydiluted.com)
 *        2003 Thomas Gleixner (tglx@linutronix.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Overview:
 *   This utility writes a binary image directly to a NAND flash
 *   chip or NAND chips contained in DoC devices. This is the
 *   "inverse operation" of nanddump.
 *
 * tglx: Major rewrite to handle bad blocks, write data with or without ECC
 *   write oob data only on request
 *
 * Bug/ToDo:
 */


#include "nandwrite.h"
#include "crc32.h"

#define VERSION_NANDWIRTE "1.0.1"
#define FS_TEMP_PATH    "/tmp/ext4fs.tar.tgz"
#define FS_MOUNT_PATH   "/mnt/nand4"

#define MAX_PAGE_SIZE   2048
#define MAX_OOB_SIZE    64
#define barrier() __asm__ __volatile__("":::"memory")
#define SET_PTR_TO_GLOBALS(x) do { \
        (*(struct globals**)&ptr_to_globals) = (void*)(x); \
        barrier(); \
    } while (0)

#define DEF_512M_CMDLINE    "mem=256M console=ttyAMA0,115200 ubi.mtd=fs1 root=ubi0:rootfs rw rootfstype=ubifs"
#define DEF_1G_CMDLINE      "mem=768M console=ttyAMA0,115200 ubi.mtd=fs1 root=ubi0:rootfs rw rootfstype=ubifs"

#define WRITE_THEN_READ_CHECK

struct globals {
    /* Buffer array used for writing data */
    unsigned char writebuf[MAX_PAGE_SIZE];
    unsigned char oobbuf[MAX_OOB_SIZE];
    unsigned char oobreadbuf[MAX_OOB_SIZE];
    /* oob layouts to pass into the kernel as default */
    struct nand_oobinfo none_oobinfo;
    struct nand_oobinfo jffs2_oobinfo;
    struct nand_oobinfo yaffs_oobinfo;
    struct nand_oobinfo autoplace_oobinfo;
};
struct globals *ptr_to_globals = NULL;

#define G (*ptr_to_globals)
#define INIT_G() do { \
        if(&G == NULL) \
            SET_PTR_TO_GLOBALS(xzalloc(sizeof(G))); \
        else \
            memset(&G, 0, sizeof(G)); \
        \
        G.none_oobinfo.useecc = MTD_NANDECC_OFF; \
        G.autoplace_oobinfo.useecc = MTD_NANDECC_AUTOPLACE; \
        \
        G.jffs2_oobinfo.useecc = MTD_NANDECC_PLACE; \
        G.jffs2_oobinfo.eccbytes = 6; \
        G.jffs2_oobinfo.eccpos[0] = 0; \
        G.jffs2_oobinfo.eccpos[1] = 1; \
        G.jffs2_oobinfo.eccpos[2] = 2; \
        G.jffs2_oobinfo.eccpos[3] = 3; \
        G.jffs2_oobinfo.eccpos[4] = 6; \
        G.jffs2_oobinfo.eccpos[5] = 7; \
        \
        G.yaffs_oobinfo.useecc = MTD_NANDECC_PLACE; \
        G.yaffs_oobinfo.eccbytes = 6; \
        G.yaffs_oobinfo.eccpos[0] = 8; \
        G.yaffs_oobinfo.eccpos[1] = 9; \
        G.yaffs_oobinfo.eccpos[2] = 10; \
        G.yaffs_oobinfo.eccpos[3] = 13; \
        G.yaffs_oobinfo.eccpos[4] = 14; \
        G.yaffs_oobinfo.eccpos[5] = 15; \
    } while (0)


char *g_part_str[] = {
    /*"nor",*/"uboot", "env", "logo", "ker1",
    "ker2", "fs1",  "fs2",  "cfg",
    "oem",  "rsd",  "all"
};


void *xzalloc(size_t size)
{
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

static inline unsigned int ver_str_to_ul(char *ver)
{
    unsigned int a, b, c, d;
    if (4 != sscanf(ver, "%2d.%2d.%2d.%2d", &a, &b, &c, &d)) {
        printf("uboot version format invalid, eg: xx.xx.xx.xx\n");
        return UPFW_SUCCESS;
    }

    return ((a << 24) | (b << 16) | (c << 8) | (d));
}

static void display_version(void)
{
    mtd_table_t table;
    int i;

    printf("Milesight update version: %s \n", VERSION_NANDWIRTE);
    if (get_env_mtd_table(&table) < 0) {
        fprintf(stderr, "flprog_get_part_table faild!\n");
    }
    for (i = 0; i < PART_MAX; i++) {
        if (table.part[i].size != 0) {
            if ((strstr(table.bootargs, table.part[i].name) != NULL)
                || (strstr(table.bootcmd, table.part[i].name) != NULL)) {
                printf("%s:(current)\n", table.part[i].name);
            } else if ((strstr(table.bootargs, "nfs") != NULL)
                       && (strstr(table.part[i].name, "fs") != NULL)) {
                printf("%s:(nfs)\n", table.part[i].name);
            } else {
                printf("%s:\n", table.part[i].name);
            }
            printf("\tversion = %d.%d.%d.%d\n",
                   (table.part[i].version >> 24) & 0xff,
                   (table.part[i].version >> 16) & 0xff,
                   (table.part[i].version >> 8) & 0xff,
                   (table.part[i].version) & 0xff);
            printf("\tsize    = 0x%08X (%d K)\n", table.part[i].size,
                   table.part[i].size / 1024);
            printf("\tdate    = %d/%d/%d\n",
                   (table.part[i].date >> 16) & 0xffff,
                   (table.part[i].date >> 8) & 0xff,
                   (table.part[i].date) & 0xff);
        }
    }
}


static int  mtdoffset = 0;
static int  quiet = 1;
static int  writeoob = 0;
//static int    autoplace = 0;
//static int    forcejffs2 = 0;
//static int    forceyaffs = 0;
//static int    forcelegacy = 0;
//static int    noecc = 0;
static int  blockalign = 1; /*default to using 16K block size */

#define MTD_INFO_FILE   "/proc/partitions"
//#define MTD_INFO_FMT  "%[^:]s %d %d \"%s\"\n"
#define MTD_INFO_FMT    "%d %d %d %s\n"


int set_env_partition(part_info_t *table)
{
    char envinfo[128];

    snprintf(envinfo, sizeof(envinfo),
             "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
             table->name, table->version, table->size, table->date);
    return hs_setenv(table->name, envinfo);
}

int get_env_mtd_table(mtd_table_t *table)
{
    char *envinfo;
    char *p;
    int i;

    memset(table, 0, sizeof(mtd_table_t));
    for (i = 0; i < PART_MAX; i++) {
        envinfo = hs_getenv(g_part_str[i]);

        if (envinfo != NULL) {
            printf("i=%d, g_part_str[i]=%s, envinfo=%s\n", i, g_part_str[i], envinfo);
            sscanf(envinfo,
                   "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
                   table->part[i].name,
                   &table->part[i].version, &table->part[i].size,
                   &table->part[i].date);
        } else {
            printf("[NULL]i=%d, g_part_str[i]=%s\n", i, g_part_str[i]);
        }
    }

    envinfo = hs_getenv("bootargs");
    if (envinfo != NULL) {
        memcpy(table->bootargs, envinfo, strlen(envinfo));
        table->bootargs[strlen(envinfo)] = '\0';
    }

    envinfo = hs_getenv("bootcmd");
    if (envinfo != NULL) {
        memcpy(table->bootcmd, envinfo, strlen(envinfo));
        table->bootcmd[strlen(envinfo)] = '\0';
    }

    //zbing 2018.11.7 fix uboot dont update when env clear;
    if ((table->part[PART_BLD].version >> 8 & 0xff) == 0) {
        envinfo = hs_getenv("ver");
        if (envinfo) {
            p = strstr(envinfo, "73.");
            if (p) {
                table->part[PART_BLD].version = ver_str_to_ul(p);
                set_env_partition(&table->part[PART_BLD]);
            }
        }
    }
    //zbing 2018.11.7 end
    return UPFW_SUCCESS;
}

int set_env_mtd_table(mtd_table_t *table)
{
    char envinfo[128];
    int i;
    int res = 0;

    memset(envinfo, 0, 128);
    for (i = 0; i < PART_MAX; i++) {
        if (strcmp(g_part_str[i], table->part[i].name) == 0) {
            sprintf(envinfo,
                    "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
                    table->part[i].name,
                    table->part[i].version, table->part[i].size,
                    table->part[i].date);
            res |= hs_setenv(table->part[i].name, envinfo);
        }
    }

    return res;
}

int set_env_cmdline(mtd_table_t *table)
{
    int rc = 0;

    if (strlen(table->bootargs) > 0) {
        rc = hs_setenv("bootargs", table->bootargs);
    }

    if (strlen(table->bootcmd) > 0) {
        rc = hs_setenv("bootcmd", table->bootcmd);
    }

    return rc;
}

int get_mtd_info(MTD_Handle mtd)
{
    int i;
    FILE *fp = NULL;
    char cmd[256];
    char mtd_name[32];
    char blk_name[32];
    char mtd_dev[64];
    char acTmp[1024];
    unsigned int size, erasesize;
    int major, minor;

    memset(mtd, 0, sizeof(IMG_MTD_INFO_T));
    for (i = PART_BLD; i < PART_MAX; i++) {
        memset(mtd_dev, 0, sizeof(mtd_dev));
        memset(mtd_name, 0, sizeof(mtd_name));
        memset(blk_name, 0, sizeof(blk_name));
        memset(cmd, 0, sizeof(cmd));
        memset(acTmp, 0, sizeof(acTmp));
        size = 0;
        erasesize = 0;
        strcpy(mtd_name, g_part_str[i]);
        strcpy(mtd->mtd_info[i].mtd_name, mtd_name);
        snprintf(blk_name, sizeof(blk_name), "mmcblk0p%d", i + 1);
        sprintf(cmd, "cat %s | grep %s \n", MTD_INFO_FILE, blk_name);
        fp = popen(cmd, "r");
        if (fp != NULL) {
            while (fgets(acTmp, sizeof(acTmp), fp) != NULL) {
                if (strstr(acTmp, blk_name) != NULL) {
                    sscanf(acTmp, MTD_INFO_FMT, &major, &minor, &size, mtd_dev);
                    size *= 2;
                    printf("get_mtd_info==mtd_dev=%s=size=%d===\n", mtd_dev, size);
                    break;
                }
            }
            pclose(fp);
            fp = NULL;
            sprintf(mtd->mtd_info[i].mtd_dev, "/dev/%s", mtd_dev);
            mtd->mtd_info[i].mtd_size = size;
        } else {
            PERR_RETURN(UPFW_FAILD, "popen "MTD_INFO_FILE);
        }

    }

    return UPFW_SUCCESS;
}

int display_info(void)
{
    int ret;

    display_version();
    printf("\nMore details:\n\n");
    printf("Mtd information: \n");
    system("cat /proc/partitions ");
    ret = hs_printenv();
    if (ret < 0) {
        ERR_RETURN(ret, "hs_printenv faild!\n");
    }

    return UPFW_SUCCESS;
}

int  set_env_boot(int bootid)
{
    char *cmdline = NULL;
    mtd_table_t mtd_table;

    if (get_env_mtd_table(&mtd_table) < 0) {
        ERR_RETURN(UPFW_FAILD, "get_env_mtd_table faild!\n");
    }

    cmdline = strstr(mtd_table.bootcmd, "ker");
    if (cmdline != NULL) {
        if (bootid) {
            *(cmdline + 3) = '0' + bootid;
        } else {
            *(cmdline + 3) = '3' - *(cmdline + 3) + '0';
        }
    } else {
        ERR_RETURN(UPFW_FAILD, "Can't find kernel partition!\n");
    }

    cmdline = strstr(mtd_table.bootargs, "=fs");
    if (cmdline == NULL) {
        memcpy(mtd_table.bootargs, DEF_512M_CMDLINE, sizeof(DEF_512M_CMDLINE));
        cmdline = strstr(mtd_table.bootargs, "=fs");
        if (cmdline == NULL) {
            ERR_RETURN(UPFW_FAILD, "Can't find filesys partition!\n");
        }
    }

    if (bootid) {
        *(cmdline + 3) = '0' + bootid;
    } else {
        *(cmdline + 3) = '3' - *(cmdline + 3) + '0';
    }

    printf("\tFilesys will switch to fs%c after reboot!\n", *(cmdline + 3));
    return set_env_cmdline(&mtd_table);
}

int set_cmdline_memory(int memsize)
{
    mtd_table_t mtd_table;
    char *cmdline = NULL;

    if (get_env_mtd_table(&mtd_table) < 0) {
        ERR_RETURN(UPFW_FAILD, "get_env_mtd_table faild!\n");
    }
    cmdline = strstr(mtd_table.bootargs, "mem=");
    if (cmdline != NULL) {
        switch (memsize) {
            case 256:
                cmdline[4] = '2';
                cmdline[5] = '5';
                cmdline[6] = '6';
                break;
            case 148:
                cmdline[4] = '1';
                cmdline[5] = '4';
                cmdline[6] = '8';
                break;
            case 128:
            default:
                cmdline[4] = '1';
                cmdline[5] = '2';
                cmdline[6] = '8';
                break;
        }
        return set_env_cmdline(&mtd_table);
    } else {
        ERR_RETURN(UPFW_FAILD, "unknow memory of env!\n");
    }

    return UPFW_SUCCESS;
}

int set_boot_state(int state)
{
    char *p = hs_getenv("bootState");
    if (state == 0) {
        if (p && strstr(p, "Done")) {
            return hs_setenv("bootState", "None");
        }
    } else {
        if (p && strstr(p, "None")) {
            return hs_setenv("bootState", "Done");
        }
    }
    return UPFW_SUCCESS;
}

int set_switch_filesys(int fs_id)
{
    switch (fs_id) {
        case 0:
        case 1:
        case 2:
            set_env_boot(fs_id);
            break;
        default:
            ERR_RETURN(UPFW_FAILD, "Unknown fs index, please choose 1 or 2\n");
    }
    sleep(3);
    return system("reboot");
}

int get_part_name(int part_index, char *part_name)
{
    if (part_index < PART_MAX) {
        strcpy(part_name, g_part_str[part_index]);
        return UPFW_SUCCESS;
    } else {
        return UPFW_FAILD;
    }
}

int set_ptb_cmdline(char cmdline[FLDEV_CMD_LINE_SIZE])
{
    return hs_setenv("bootargs", cmdline);
}

int set_ptb_sn(char *sn_data)
{
    return hs_setenv("sn", sn_data);
}

int set_ptb_bootdelay(uint8_t delay)
{
    char str[4];
    sprintf(str, "%d", delay);
    return hs_setenv("bootdelay", str);
}

// set auto_boot
int set_ptb_autoboot(uint8_t auto_boot)
{
    return UPFW_SUCCESS;
}

// set auto_download
int set_ptb_autodownload(uint8_t auto_dl)
{
    return UPFW_SUCCESS;
}

// set pri_addr
int set_ptb_pri_addr(uint32_t pri_addr)
{
    return UPFW_SUCCESS;
}

// set pri_file
int set_ptb_pri_file(char pri_file[SN_SIZE])
{
    return hs_setenv("bootfile", pri_file);
}

int set_ptb_tftpd_ip(char *tftpd)
{
    return hs_setenv("serverip", tftpd);
}

#if (ETH_INSTANCES >= 1)
int set_ptb_eth_mac(int eth_id, char *eth_mac)
{
    return hs_setenv("ethaddr", eth_mac);
}

int set_ptb_eth_ip(int eth_id, char *eth_ip)
{
    return hs_setenv("ipaddr", eth_ip);
}

int set_ptb_eth_netmask(int eth_id, char *eth_nm)
{
    return hs_setenv("netmask", eth_nm);
}

int set_ptb_eth_gateway(int eth_id, char *eth_gw)
{
    return hs_setenv("gateway", eth_gw);
}

int set_ptb_uboot_version(char *ver)
{
    mtd_table_t table;

    get_env_mtd_table(&table);
    table.part[PART_BLD].version = ver_str_to_ul(ver);

    return set_env_partition(&table.part[PART_BLD]);
}

#endif /* ETH_INSTANCES >= 1 */

#if (USE_WIFI >= 1)
int set_ptb_wifi_mac(int wifi_id, char *wifi_mac)
{
    return hs_setenv("wifimac", wifi_mac);
}

int set_ptb_wifi_ip(int wifi_id, char *wifi_ip)
{
    return hs_setenv("wifiip", wifi_ip);
}

int set_ptb_wifi_netmask(int wifi_id, char *wifi_nm)
{
    return hs_setenv("wifinm", wifi_nm);
}

int set_ptb_wifi_gateway(int wifi_id, char *wifi_gw)
{
    return hs_setenv("wifigw", wifi_gw);
}
#endif /* USE_WIFI >= 1 */

// set usb eth mac ip netmask gateway
#if (USE_USBETH >= 1)
int set_ptb_usbeth_mac(int usbeth_id, char *usbeth_mac)
{
    return UPFW_SUCCESS;
}

int set_ptb_usbeth_ip(int usbeth_id, char *usbeth_ip)
{
    return UPFW_SUCCESS;
}

int set_ptb_usbeth_netmask(int usbeth_id, char *usbeth_nm)
{
    return UPFW_SUCCESS;
}

int set_ptb_usbeth_gateway(int usbeth_id, char *usbeth_gw)
{
    return UPFW_SUCCESS;
}
#endif /* USE_USBETH >= 1 */


int clean_memory_and_sync()
{
    char cmd[128];
    strcpy(cmd, "echo 3 > /proc/sys/vm/drop_caches");
    if (system(cmd)) {
        ERR_RETURN(UPFW_FAILD, "clean memory error!\n");
    }

    system("sync");

    return UPFW_SUCCESS;
}


/* size of read/write buffer */
#define BUFSIZE (10 * 1024)

int spi_flash_write_file(char *img_name, IMG_PTB_INFO_T *part_info)
{
    struct mtd_info_user mtd;
    int  image_length = 0;
    struct erase_info_user erase;
    size_t size, written;
    int i, cnt;
    unsigned char src[BUFSIZE], dest[BUFSIZE];
    int dev_fd, ifd;
    mtd_table_t mtd_table;
    int part_index = -1;

    /* get some info about the flash device */
    dev_fd = open(part_info->mtd_dev, O_SYNC | O_RDWR);
    if (ioctl(dev_fd, MEMGETINFO, &mtd) < 0) {
        //DEBUG("ioctl(): %m\n");
        //log_printf (LOG_ERROR,"This doesn't seem to be a valid MTD flash device!\n");
        ERR_CLOSE_RETURN(dev_fd, UPFW_FAILD, "This doesn't seem to be a valid MTD flash device! \n");
    }

    /* Open the input file */
    if ((ifd = open(img_name, O_RDONLY)) == -1) {
        ERR_RETURN(UPFW_FAILD, "Open image failed ! \n");
    }

    // get image length
    image_length = part_info->fw_len;
    lseek(ifd, part_info->fw_start, SEEK_SET);  // set offset = fw_start
    //image_length = imglen;


    /* does it fit into the device/partition? */
    if (image_length > mtd.size) {
        printf("The image length > mtd size ! \n");
        close(dev_fd);
        close(ifd);
        return -1;
    }

    /*****************************************************
     * erase enough blocks so that we can write the file *
     *****************************************************/

    erase.start = 0;
    erase.length = (image_length + mtd.erasesize - 1) / mtd.erasesize;
    erase.length *= mtd.erasesize;

    /* if not, erase the whole chunk in one shot */
    if (ioctl(dev_fd, MEMERASE, &erase) < 0) {
        printf(
            "Fail erasing blocks from 0x%.8x-0x%.8x on %s \n",
            (unsigned int) erase.start, (unsigned int)(erase.start + erase.length), part_info->mtd_dev);
        //exit (EXIT_FAILURE);
        //return errno;
        close(dev_fd);
        close(ifd);
        return -1;

    }


    /**********************************
     * write the entire file to flash *
     **********************************/

    //if (flags & FLAG_VERBOSE) log_printf (LOG_NORMAL,"Writing data: 0k/%luk (0%%)",KB (filestat.st_size));
    size = image_length;
    i = BUFSIZE;
    written = 0;
    while (size) {
        if (size < BUFSIZE) {
            i = size;
        }

        cnt = read(ifd, src, i);
        if (cnt != i) {
            printf("Read image count error, result = [0x%x] ! \n", cnt);
            //return -1;
            goto closeall;
        }

        /* write to device */
        cnt = write(dev_fd, src, i);
        if (i != cnt) {
            printf("Write image count error, reault = [0x%x] ! \n", cnt);
            //return -1;
            goto closeall;
        }

        written += i;
        size -= i;
    }


    /**********************************
     * verify that flash == file data *
     **********************************/
    lseek(ifd, part_info->fw_start, SEEK_SET);
    lseek(dev_fd, 0L, SEEK_SET);

    size = image_length;
    i = BUFSIZE;
    written = 0;

    while (size) {
        if (size < BUFSIZE) {
            i = size;
        }

        /* read from filename */
        cnt = read(ifd, src, i);
        if (cnt != i) {
            printf(" Verify that flash, read image count error, result = [0x%x] ! \n", cnt);
            goto closeall;
        }

        /* read from device */
        cnt = read(dev_fd, dest, i);
        if (cnt != i) {
            printf(" Verify that flash, write image count error, result = [0x%x] ! \n", cnt);
            goto closeall;
        }

        /* compare buffers */
        if (memcmp(src, dest, i)) {
            printf("File does not seem to match flash data. \n");
            goto closeall;
        }

        written += i;
        size -= i;
    }

    /*if(size == 0)
    {
        printf(" verify that flash sucess !!! \n");
    }
    */


closeall:
    close(dev_fd);
    close(ifd);

    if (size > 0) {
        ERR_RETURN(UPFW_FAILD, "Data was only partially written due to error\n");
    }

    // Begin update ptb information
    for (cnt = 0; cnt < PART_MAX; cnt++) {
        if (strcmp(g_part_str[cnt], part_info->mtd_name) == 0) {
            part_index = cnt;
            break;
        }
    }
    if (cnt == PART_MAX || part_index == -1) {
        ERR_RETURN(UPFW_FAILD, "Not valid image, exit without update ptb!\n");
    }

    if (get_env_mtd_table(&mtd_table) < 0) {
        ERR_RETURN(UPFW_FAILD, "flprog_get_part_table faild!\n");
    }

    mtd_table.part[part_index].size = image_length;
    mtd_table.part[part_index].version = part_info->fw_version;
    mtd_table.part[part_index].date = part_info->fw_date;

    if (set_env_partition(&mtd_table.part[part_index]) < 0) {
        ERR_RETURN(UPFW_FAILD, "flprog_set_part_table faild!\n");
    }

    sync();

    return UPFW_SUCCESS;

}

int nand_flash_write_file(char *img_name, IMG_PTB_INFO_T *part_info)
{
    int ret = 0;
    uint8_t *img_buf = NULL;
    int ifd, fd;
    int image_length = 0;
    int cnt, part_index;
    mtd_table_t mtd_table;

    /* 1. Open the input file */
    if ((ifd = open(img_name, O_RDONLY)) == -1) {
        perror("open input file");
        ERR_RETURN(UPFW_FAILD, "img open faild!\n");
    }
    image_length = part_info->fw_len;
    lseek(ifd, part_info->fw_start, SEEK_SET);  // set offset = fw_start
    img_buf = malloc(image_length);
    if (read(ifd, img_buf, image_length) != image_length) {
        perror("read file failure");
        free(img_buf);
        close(ifd);
        ERR_RETURN(UPFW_FAILD, "read file failure!\n");
    }
    close(ifd);
	
    /* 2. Open the device & write device*/
    if ((fd = open(part_info->mtd_dev, O_RDWR)) == -1) {
        free(img_buf);
        PERR_RETURN(UPFW_FAILD, "open flash");
    }
    if (write(fd, img_buf, image_length) != image_length) {
        perror("write dev failure");
        free(img_buf);
        close(fd);
        PERR_RETURN(UPFW_FAILD, "write dev failure");
    }
    close(fd);
    sync();
    free(img_buf);

    //3. Begin update ptb information
    for (cnt = 0; cnt < PART_MAX; cnt++) {
        if (strcmp(g_part_str[cnt], part_info->mtd_name) == 0) {
            part_index = cnt;
            break;
        }
    }
    if (cnt == PART_MAX || part_index == -1) {
        ERR_RETURN(UPFW_FAILD, "Not valid image, exit without update ptb!\n");
    }
    if (get_env_mtd_table(&mtd_table) < 0) {
        ERR_RETURN(UPFW_FAILD, "get_env_mtd_table faild!\n");
    }
    mtd_table.part[part_index].size = image_length;
    mtd_table.part[part_index].version = part_info->fw_version;
    mtd_table.part[part_index].date = part_info->fw_date;
    if (set_env_partition(&mtd_table.part[part_index]) < 0) {
        ERR_RETURN(UPFW_FAILD, "set_env_partition faild!\n");
    }

    sync();

    return 0;
}

int nand_flash_write_fs_file(char *img_name, IMG_PTB_INFO_T *part_info)
{
    int ret = 0;
    uint8_t *img_buf = NULL;
    int ifd, ofd;
    int image_length = 0;
    char cmd[128];
    int cnt, part_index;
    mtd_table_t mtd_table;

    /*1. Open the input file */
    if ((ifd = open(img_name, O_RDONLY)) == -1) {
        perror("open input file");
        ERR_RETURN(UPFW_FAILD, "img open faild!\n");
    }
    image_length = part_info->fw_len;
    lseek(ifd, part_info->fw_start, SEEK_SET);  // set offset = fw_start
    img_buf = malloc(image_length);
    if (read(ifd, img_buf, image_length) != image_length) {
        perror("read file failure");
        free(img_buf);
        close(ifd);
        ERR_RETURN(UPFW_FAILD, "read file failure!\n");
    }
    close(ifd);

    /*2. save in file */
    if ((ofd = open(FS_TEMP_PATH, O_RDWR | O_CREAT)) == -1) {
        printf("open %s failure\n", FS_TEMP_PATH);
        free(img_buf);
        PERR_RETURN(UPFW_FAILD, "open extfs.tgz");
    }
    if (write(ofd, img_buf, image_length) != image_length) {
        printf("write %s failure\n", FS_TEMP_PATH);
        free(img_buf);
        close(ofd);
        PERR_RETURN(UPFW_FAILD, "write extfs.tgz failure");
    }
    close(ofd);
    sync();
    free(img_buf);

    /*3.1 Formatted mmc_partion */
    snprintf(cmd, sizeof(cmd), "mkfs.ext4 -F %s", part_info->mtd_dev);
    system(cmd);
    /*3.2 tar -C mmc_partion */
    snprintf(cmd, sizeof(cmd), "mount -t ext4 %s %s", part_info->mtd_dev, FS_MOUNT_PATH);
    system(cmd);
    clean_memory_and_sync();
    snprintf(cmd, sizeof(cmd), "tar -xzf %s -C %s", FS_TEMP_PATH, FS_MOUNT_PATH);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "rm -f %s", FS_TEMP_PATH);
    system(cmd);
    clean_memory_and_sync();

    //4. Begin update ptb information
    for (cnt = 0; cnt < PART_MAX; cnt++) {
        if (strcmp(g_part_str[cnt], part_info->mtd_name) == 0) {
            part_index = cnt;
            break;
        }
    }
    if (cnt == PART_MAX || part_index == -1) {
        ERR_RETURN(UPFW_FAILD, "Not valid image, exit without update ptb!\n");
    }

    if (get_env_mtd_table(&mtd_table) < 0) {
        ERR_RETURN(UPFW_FAILD, "get_env_mtd_table faild!\n");
    }

    mtd_table.part[part_index].size = image_length;
    mtd_table.part[part_index].version = part_info->fw_version;
    mtd_table.part[part_index].date = part_info->fw_date;

    if (set_env_partition(&mtd_table.part[part_index]) < 0) {
        ERR_RETURN(UPFW_FAILD, "set_env_partition faild!\n");
    }

    sync();

    return 0;
}
