/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <linux/ctype.h>    /* isalpha, isdigit */
#include <u-boot/md5.h>


#define VER_NUM(main,sub0,sub1,sub2) ((main<<24)|(sub0<<16)|(sub1<<8)|(sub2))
#define VER_DATE(year,month,day)     ((year << 16) | (month << 8) | day)
#define LINK_STR(a,b,c,d)   a##.##b##.##c##.##d
#define VER_STR(a,b,c,d)    LINK_STR(a,b,c,d)

#define MD5_STR_LEN         33
#define DOWNLOAD_ADDR      (0x12000000)

#define BOLD "\033[1m"
#define DRED "\033[1;31m"
#define DGREEN "\033[1;32m"

#define NOCOLOR "</font></font></u></b>"
#define FNSTART printf("%s\n", __func__)
#define FIRM_ERR(fmt, args...) printf(DRED fmt NOCOLOR, ##args)
#define FIRM_SUC(fmt, args...) printf(DGREEN fmt NOCOLOR, ##args)


// must be same as firmware/fwupdate/updateFW.h >>begin
typedef enum {
    UPFW_BLD = 0,
    UPFW_KER,
    UPFW_FS,
    UPFW_CMD,
    UPFW_JPG,
    UPFW_BLD2,  // for another bld.
    UPFW_TOOL,
    UPFW_MAX,

} UPFW_INDEX_E;

typedef struct {
    unsigned int  fw_being;                 // image begin addr
    unsigned int  fw_len;                   // image lengths
    unsigned int  fw_version;               // image version
    unsigned int  fw_load_mem;              // image load memory
    unsigned int  fw_load_flag;             // image load flag
    unsigned int  fw_date;                  // image build date
    unsigned char fw_name[16];              // image name
    unsigned char fw_md5[MD5_STR_LEN];      // image md5 check number use hex
    unsigned int  fw_crc;                   // image crc32

} IMG_HEADER_T;


#define IMG_HEADER_SIZE     2048
#define HEADER_PAD_SIZE     ( IMG_HEADER_SIZE - sizeof(IMG_HEADER_T)*UPFW_MAX - 32 - 64)

typedef struct __upfw_head_t__ {
    IMG_HEADER_T img_head[UPFW_MAX];
    char img_date[32];
    char img_version[64];
    char img_rsv[HEADER_PAD_SIZE];

} UPFW_HEADER_T, *UPFW_header_handle;

typedef struct part_info_s {
    char         name[32];
    unsigned int size;
    unsigned int version;
    unsigned int date;

} part_info_t;
// must be same as firmware/fwupdate/updateFW.h >>end

const char *g_part_name[UPFW_MAX] = {
    [UPFW_BLD] = "uboot",
    [UPFW_KER] = "ker1",
    [UPFW_FS] = "fs1",
    [UPFW_JPG] = "logo"
};


#ifdef VERIFY_MD5
static int check_md5(ulong addr, ulong len, u8 md5_src[MD5_STR_LEN])
{
    unsigned int i;
    unsigned char md5_str[MD5_STR_LEN];
    u8 md5_des[16];

    md5((unsigned char *) addr, len, md5_des);
    sprintf((char *)md5_str,
            "%02x%02x%02x%02x"
            "%02x%02x%02x%02x"
            "%02x%02x%02x%02x"
            "%02x%02x%02x%02x",
            md5_des[0], md5_des[1], md5_des[2], md5_des[3],
            md5_des[4], md5_des[5], md5_des[6], md5_des[7],
            md5_des[8], md5_des[9], md5_des[10], md5_des[11],
            md5_des[12], md5_des[13], md5_des[14], md5_des[15]
           );

    for (i = 0; i < MD5_STR_LEN; i++) {
        if (md5_src[i] != md5_str[i]) {
            return -1;
        }
    }

    return 0;
}

#else
static int check_crc(ulong addr, ulong len, unsigned int crc_src)
{
    unsigned int crc_des = 0;

    crc_des = crc32(0, (const unsigned char *)addr, len);
    if (crc_des != crc_src) {
        return -1;
    }
    return 0;
}
#endif

static int update_write_nand(char *part_name, ulong addr, ulong len)
{
    char cmd_erase[CONFIG_SYS_CBSIZE];
    char cmd_write[CONFIG_SYS_CBSIZE];
    int rc = 0;

    sprintf(cmd_erase, "mmc erase.part %s", part_name);
    printf("cmd_erase=%s\n", cmd_erase);
    rc = run_command(cmd_erase, 0);
    if (rc < 0) {
        printf("mmc erase.part %s failed !\n", part_name);
        return -1;
    }

    if (!strcmp(part_name, "fs1") ||
        !strcmp(part_name, "fs2")) {
        sprintf(cmd_write, "mmc swrite 0x%lx %s", addr, part_name);  //write spare file
    } else {
        sprintf(cmd_write, "mmc write 0x%lx %s", addr, part_name);
    }

    printf("cmd_write=%s\n", cmd_write);
    rc = run_command(cmd_write, 0);
    if (rc < 0) {
        printf("nand write %s failed !\n", part_name);
        return -1;
    }

    return 0;
}

static int update_part_info(part_info_t *info)
{
    char envinfo[CONFIG_SYS_CBSIZE];
    int rc = 0;
    char *cmd = NULL, *p = NULL;
    char part2name[32] = {0};
    //char fs2name[32] = {0};

    if (strcmp(info->name, "ker1") == 0) {
        cmd = env_get("bootcmd");
        p = strstr(cmd, "ker");
        if (p != NULL) {
            *(p + 3) = '1';
        }

        sprintf(part2name, "ker%d", 2);

        sprintf(envinfo, "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
                info->name, info->version, info->size, info->date);
        rc = env_set(info->name, envinfo);

        sprintf(envinfo, "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
                part2name, info->version, info->size, info->date);
        rc = env_set(part2name, envinfo);
    } else if (strcmp(info->name, "fs1") == 0) {
        cmd = env_get("bootargs");
        p = strstr(cmd, "mmcblk1p");
        if (p != NULL) {
            *(p + 8) = '6';
        }

        sprintf(part2name, "fs%d", 2);

        sprintf(envinfo, "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
                info->name, info->version, info->size, info->date);
        rc = env_set(info->name, envinfo);

        sprintf(envinfo, "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
                part2name, info->version, info->size, info->date);
        rc = env_set(part2name, envinfo);
    } else {
        sprintf(envinfo, "name:%s\tver:0x%08x\tsize:%u\tdate:0x%08x ",
                info->name, info->version, info->size, info->date);
        rc = env_set(info->name, envinfo);
    }

    return rc;
}

static int version_to_number(char *ver)
{
    int ver1, ver_num = 0;
    char *cp = ver;
    char **endp = &ver;

    while (*cp != '\0') {
        /* check digit number */
        while (!isdigit(*cp)) {
            cp++;
            if (*cp == '\0') {
                return ver_num;
            }
        }

        /* transfer to number */
        ver1 = simple_strtol(cp, endp, 10);
        ver_num = (ver_num << 8) | ver1; // ver1|ver2|ver3|ver4 (4.2.4.9)
        cp = *endp;
    }
    return ver_num;
}

static int get_part_info(part_info_t *info)
{
    char *part = env_get(info->name);
    if (part != NULL) {
        char *ptmp = strstr(part, "ver:");
        if (ptmp != NULL) {
            info->version = simple_strtol(&ptmp[4], NULL, 16);
        }

        ptmp = strstr(part, "size:");
        if (ptmp != NULL) {
            info->size = simple_strtol(&ptmp[5], NULL, 10);
        }

        ptmp = strstr(part, "date:");
        if (ptmp != NULL) {
            info->date = simple_strtol(&ptmp[5], NULL, 16);
        }
    } else {
        return -1;
    }

    return 0;
}

#if 0
int uboot_update_partition(void)
{
    part_info_t part_info;
    int ver_num = version_to_number(MS_UBOOT_VERSION);
    char bupdate = 0;

    memset(&part_info, 0, sizeof(part_info_t));
    snprintf(part_info.name, sizeof(part_info.name), "%s", g_part_name[UPFW_BLD]);
    get_part_info(&part_info);
    if (part_info.date == 0) {
        part_info.date = 0x2160405; // 2016-04-05
        bupdate = 1;
    }
    if (part_info.version == 0 || part_info.version != ver_num) {
        part_info.version = ver_num;
        bupdate = 1;
    }
    if (bupdate) {
        update_part_info(&part_info);
        run_command("saveenv", 0);
    }

    return 0;
}
#endif

static int ddr_mem_verify(IMG_HEADER_T *img_info)
{
    int ddr_type, ddr_type2;

    ddr_type = (img_info->fw_version >> 8) & 0xff;
    ddr_type2 = MS_DDR_VERSION;

    if (ddr_type != ddr_type2) {
        printf("ddr not match===ddr_type=%d, ddr_type2=%d\n", ddr_type, ddr_type2);
        return -1;
    }

    return 0;
}

static int image_verify(UPFW_HEADER_T *upfw)
{
    int rc, i;
    ulong upload_addr, lens;
    int plat_version;

    printf(BOLD"\n>>>>>Verify %s begin<<<<<\n"NOCOLOR, env_get("bootfile"));
    for (i = 0; i < UPFW_MAX; i++) {
        if (i == UPFW_BLD) {
            plat_version = ((upfw->img_head[i].fw_version >> 24) & 0xff);
            if (plat_version != 78) { //nt98633 uboot_version is 78.X.X.X
                FIRM_ERR("check fw_img platform no match!\n");
                return -1;
            }
        }

        if ((strcmp((const char *)upfw->img_head[i].fw_name, "") != 0) &&
            (upfw->img_head[i].fw_being != 0) && (upfw->img_head[i].fw_len != 0)) {


            upload_addr = DOWNLOAD_ADDR + upfw->img_head[i].fw_being;
            lens = upfw->img_head[i].fw_len;
#ifdef VERIFY_MD5
            rc = check_md5(upload_addr, lens, upfw->img_head[i].fw_md5);
            if (rc < 0) {
                FIRM_ERR("check %s md5 no match! \n", upfw->img_head[i].fw_name);
                return -1;
            }
#else
            rc = check_crc(upload_addr, lens, upfw->img_head[i].fw_crc);
            if (rc < 0) {
                FIRM_ERR("check %s crc no match! \n", upfw->img_head[i].fw_name);
                return -1;
            }
#endif
        }
    }
    FIRM_SUC(">>>>>image verify success<<<<<\n\n");

    return 0;
}

#if 0
static int update_part_image(IMG_HEADER_T *image)
{
    int rc;
    part_info_t info;
    ulong upload_addr = 0;
    char *part_name;

    part_name = info.name;
    upload_addr = DOWNLOAD_ADDR + image->fw_being;

    if ((strcmp((char *)image->fw_name, "bld") == 0) ||
        (strcmp((char *)image->fw_name, "bld2") == 0) ||
        (strcmp((char *)image->fw_name, "bld3") == 0)) {
        if (ddr_mem_verify(image) != 0) {
            return 0;
        }
        strcpy(part_name, g_part_name[UPFW_BLD]);
    } else if (strcmp((char *)image->fw_name, "ker") == 0) {
        strcpy(part_name, g_part_name[UPFW_KER]);
    } else if (strcmp((char *)image->fw_name, "fs") == 0) {
        strcpy(part_name, g_part_name[UPFW_FS]);
    } else {
        return 0;
    }

    printf(BOLD">>>>>update %s begin<<<<<"NOCOLOR, image->fw_name);
    rc = update_write_nand(part_name, upload_addr, image->fw_len);
    if (rc < 0) {
        FIRM_ERR(">>>>>update %s failed !<<<<<\n", part_name);
        return -1;
    }

    if (!strcmp(part_name, "ker1")) {
        FIRM_SUC(">>>>>update ker1 success !<<<<<\n\n");
        rc = update_write_nand("ker2", upload_addr, image->fw_len);
        if (rc < 0) {
            FIRM_ERR(">>>>>update ker2 failed !<<<<<\n");
            return -1;
        }

        FIRM_SUC(">>>>>update ker2 success !<<<<<\n\n");
    }

    if (!strcmp(part_name, "fs1")) {
        FIRM_SUC(">>>>>update fs1 success !<<<<<\n\n");
        rc = update_write_nand("fs2", upload_addr, image->fw_len);
        if (rc < 0) {
            FIRM_ERR(">>>>>update fs2 failed !<<<<<\n");
            return -1;
        }


        FIRM_SUC(">>>>>update fs2 success !<<<<<\n\n");
    }

    info.size = image->fw_len;
    info.version = image->fw_version;
    info.date = image->fw_date;
    rc = update_part_info(&info);
    FIRM_SUC(">>>>>update %s success !<<<<<\n\n", image->fw_name);

    return rc;
}
#else
static int update_part_image(IMG_HEADER_T *image)
{
    int rc;
    part_info_t info;
    ulong upload_addr = 0;
    char *part_name;

    part_name = info.name;
    upload_addr = DOWNLOAD_ADDR + image->fw_being;

    if ((strcmp((char *)image->fw_name, "bld") == 0) ||
        (strcmp((char *)image->fw_name, "bld2") == 0) ||
        (strcmp((char *)image->fw_name, "bld3") == 0)) {
        if (ddr_mem_verify(image) != 0) {
            return 0;
        }
        strcpy(part_name, g_part_name[UPFW_BLD]);
    } else if ((strcmp((char *)image->fw_name, "ker") == 0) ||
            (strcmp((char *)image->fw_name, "ker2") == 0)) {
        strcpy(part_name, g_part_name[UPFW_KER]);
    } else if (strcmp((char *)image->fw_name, "fs") == 0) {
        strcpy(part_name, g_part_name[UPFW_FS]);
    } else if (strcmp((char *)image->fw_name, "jpg") == 0) {
        strcpy(part_name, g_part_name[UPFW_JPG]);
    } else {
        return 0;
    }

    printf(BOLD">>>>>update %s %sbegin<<<<<"NOCOLOR, image->fw_name, part_name);
    rc = update_write_nand(part_name, upload_addr, image->fw_len);
    if (rc < 0) {
        FIRM_ERR(">>>>>update %s failed !<<<<<\n", part_name);
        return -1;
    }

    if (!strcmp(part_name, "ker1")) {
        FIRM_SUC(">>>>>update ker1 success !<<<<<\n\n");
        rc = update_write_nand("ker2", upload_addr, image->fw_len);
        if (rc < 0) {
            FIRM_ERR(">>>>>update ker2 failed !<<<<<\n");
            return -1;
        }

        FIRM_SUC(">>>>>update ker2 success !<<<<<\n\n");
    }

    if (!strcmp(part_name, "fs1")) {
        FIRM_SUC(">>>>>update fs1 success !<<<<<\n\n");
        rc = update_write_nand("fs2", upload_addr, image->fw_len);
        if (rc < 0) {
            FIRM_ERR(">>>>>update fs2 failed !<<<<<\n");
            return -1;
        }


        FIRM_SUC(">>>>>update fs2 success !<<<<<\n\n");
    }

    info.size = image->fw_len;
    info.version = image->fw_version;
    info.date = image->fw_date;
    rc = update_part_info(&info);
    FIRM_SUC(">>>>>update %s success !<<<<<\n\n", image->fw_name);

    return rc;
}

#endif

static int tftp_load_image(ulong upload_addr)
{
    int rc;
    char cmd[CONFIG_SYS_CBSIZE];

    sprintf(cmd, "tftp 0x%lx %s ", upload_addr, env_get("bootfile"));
    rc = run_command(cmd, 0);

    return rc;
}

int do_download(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int rc;
    UPFW_HEADER_T *image_header;
    int i;

    rc = tftp_load_image(DOWNLOAD_ADDR);
    if (rc < 0) {
        return -1;
    }

    image_header = (UPFW_HEADER_T *)DOWNLOAD_ADDR;
    rc = image_verify(image_header);
    if (rc < 0) {
        FIRM_ERR("Verify %s failed!\n", env_get("bootfile"));
        return -1;
    }

    for (i = 0; i < UPFW_MAX; i++) {
        if (MS_DDR_VERSION == 2) {//2G ddr,跳过BLD2
            if((i == UPFW_BLD2)) {
                continue;
            }
        } else if (MS_DDR_VERSION == 3) {//4g ddr,跳过BLD
            if((i == UPFW_BLD)) {
                continue;
            }
        }
        rc = update_part_image(&image_header->img_head[i]);
        if (rc < 0) {
            FIRM_ERR("Write %s failed !\n", image_header->img_head[i].fw_name);
            return -1;
        }
    }

    env_set("factory_date",image_header->img_date);
    env_set("factory_version",image_header->img_version);

    run_command("saveenv", 0);
    run_command("reset", 0);

    return rc;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
    download, 1, 0, do_download,
    "Auto download firmware image from network to flash, then auto boot",
    ""
);

