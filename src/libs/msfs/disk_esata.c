/*
 * ***************************************************************
 * Filename:        disk_esata.c
 * Created at:      2017.11.28
 * Description:     esata mangement API
 * Author:                  hugo
 * Copyright (C)    milesight
 * ***************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#include <errno.h>
#include <sys/vfs.h>
#include <dirent.h>

#include "msfs_disk.h"
#include "MFdisk.h"
#include "MFlog.h"


#define ESATA_MOUNT_POINT "/media/esata"

typedef struct esata_priv {
    struct diskObj *parent;
} esata_priv;

static MF_S32
esata_is_mount(MF_S8 *mnt)
{
    struct stat lst, st;
    MF_S8 buf[128];
    MF_S32 res;

    if (!strlen(mnt)) {
        MFerr("mnt is NULL");
        return MF_FALSE;
    }

    if (lstat(mnt, &lst) != 0) {
        MFerr("check mount: %s: %s\n", mnt, strerror(errno));
        return MF_FALSE;
    }

    if (!S_ISDIR(lst.st_mode)) {
        MFerr("S_ISDIR error. \n");
        return MF_FALSE;
    }

    memset(buf, 0, sizeof(buf));
    strncpy(buf, mnt, sizeof(buf) - 4);
    strcat(buf, "/..");
    if (stat(buf, &st) != 0) {
        MFerr("stat failed. \n");
        return MF_FALSE;
    }

    res = (lst.st_dev != st.st_dev) ||
          (lst.st_dev == st.st_dev && lst.st_ino == st.st_ino);

    return res ? MF_TRUE : MF_FALSE;
}

static MF_S32
esata_set_head0_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 res;

    if (fd < 0) {
        return MF_FAILURE;
    }

    res = disk_write(fd, pstDev->pstHeader, size, pstDev->sector);
    if (res < 0) {
        MFerr("disk_write failed");
    }
    disk_close(fd);

    return res;
}

static MF_S32
esata_get_head0_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);

    if (fd < 0) {
        return MF_FAILURE;
    }
    if (ioctl(fd, BLKSSZGET, &pstDev->sector) < 0) {
        MFerr("[%s] ioctl BLKSSZGET ", pstDev->name);
        disk_close(fd);
        return MF_FAILURE;
    }
    if (disk_read(fd, pstDev->pstHeader, size, pstDev->sector) < 0) {
        disk_close(fd);
        MFerr("disk_read failed");
        return MF_FAILURE;
    }
    pstDev->bPrivate = pstDev->pstHeader->uuid[UUID_PRIVATE_BIT] == 'Y' ? MF_YES : MF_NO;
    disk_close(fd);

    return MF_SUCCESS;
}

static MF_S32
esata_set_head1_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 res;

    if (fd < 0) {
        return MF_FAILURE;
    }

    res = disk_write(fd, pstDev->pstHeader, size, HEADER1_OFFSET(pstDev->sector));
    if (res < 0) {
        MFerr("disk_write failed");
    }
    disk_close(fd);

    return res;
}

static MF_S32
esata_get_head1_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);

    if (fd < 0) {
        return MF_FAILURE;
    }
    if (ioctl(fd, BLKSSZGET, &pstDev->sector) < 0) {
        MFerr("[%s] ioctl BLKSSZGET ", pstDev->name);
        disk_close(fd);
        return MF_FAILURE;
    }
    if (disk_read(fd, pstDev->pstHeader, size, HEADER1_OFFSET(pstDev->sector)) < 0) {
        disk_close(fd);
        MFerr("disk_read failed");
        return MF_FAILURE;
    }
    pstDev->bPrivate = pstDev->pstHeader->uuid[UUID_PRIVATE_BIT] == 'Y' ? MF_YES : MF_NO;
    disk_close(fd);

    return MF_SUCCESS;
}

static MF_S32
esata_header_check(struct diskObj *pstDev)
{
    MF_S32 res = MF_FAILURE;
    HDD_ATTR attr;

    if (!pstDev) {
        return res;
    }

    memset(&attr, 0, sizeof(attr));
    if (mf_disk_op_smart(pstDev->alias, &attr, TEST_RES) == MF_SUCCESS) {
        if (!strstr(attr.res, TEST_PASSED_STR)) {
            MFerr("disk smart check:%d %s", pstDev->port, attr.res);
            pstDev->enState = DISK_STATE_BAD;
            return MF_SUCCESS;
        }
    }

    if (disk_find_gpt_sig(pstDev->name, &pstDev->sector) == MF_SUCCESS) {
        pstDev->enState = DISK_STATE_UNFORMATTED;
        return MF_SUCCESS;
    }

    if (esata_get_head0_info(pstDev) == MF_SUCCESS) {
        res = MF_SUCCESS;
        if ((strcmp(pstDev->pstHeader->magic, DISK_MAGIC) == 0) &&
            (strcmp(pstDev->pstHeader->version, DISK_VERSION) == 0) &&
            (pstDev->pstHeader->checkSum != 0) &&
            pstDev->pstHeader->checkSum == \
            check_sum(pstDev->pstHeader, MF_POS(pstDev->pstHeader, checkSum))) {
            if (pstDev->pstHeader->uuid[UUID_PRIVATE_BIT] == 'Y' &&
                strncmp(&pstDev->pstHeader->uuid[UUID_KEY_BIT], mf_dev_mmac(), \
                        pstDev->pstHeader->uuid[UUID_LEN_BIT])) {
                pstDev->enState = DISK_STATE_UNFORMATTED;
            } else {
                pstDev->enState = DISK_STATE_NORMAL;
                pstDev->smartTest = mf_disk_get_disk_attr_status(pstDev->alias);
            }
            return MF_SUCCESS;
        } else {
            pstDev->enState = DISK_STATE_UNFORMATTED;
        }
    }

    if (esata_get_head1_info(pstDev) == MF_SUCCESS) {
        res = MF_SUCCESS;
        MFerr("head0 errno !");
        if ((strcmp(pstDev->pstHeader->magic, DISK_MAGIC) == 0) &&
            (strcmp(pstDev->pstHeader->version, DISK_VERSION) == 0) &&
            (pstDev->pstHeader->checkSum != 0) &&
            pstDev->pstHeader->checkSum == \
            check_sum(pstDev->pstHeader, MF_POS(pstDev->pstHeader, checkSum))) {
            MFinfo("set head 0 !!!");
            if (pstDev->pstHeader->uuid[UUID_PRIVATE_BIT] == 'Y' &&
                strncmp(&pstDev->pstHeader->uuid[UUID_KEY_BIT], mf_dev_mmac(), \
                        pstDev->pstHeader->uuid[UUID_LEN_BIT])) {
                pstDev->enState = DISK_STATE_UNFORMATTED;
            } else {
                pstDev->enState = DISK_STATE_NORMAL;
                pstDev->smartTest = mf_disk_get_disk_attr_status(pstDev->alias);
            }
            esata_set_head0_info(pstDev);
        } else {
            pstDev->enState = DISK_STATE_UNFORMATTED;
        }
    }
    return res;
}

#if 0
static MF_S32
esata_clr_head_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_U32 sector;
    disk_header header;
    MF_S32 res;

    if (fd < 0) {
        return MF_FAILURE;
    }
    if (ioctl(fd, BLKSSZGET, &sector) < 0) {
        MFerr("[%s] ioctl BLKSSZGET ", pstDev->name);
        disk_close(fd);
        return MF_FAILURE;
    }
    MFinfo("##########size %d pst %d", size, sector);
    memset(&header, 0, sizeof(disk_header));
    res = disk_write(fd, &header, size, sector);
    disk_close(fd);

    return res;
}
#endif

static MF_S32
esata_scan_parts(char *name, struct list_head *dlist)
{
    MF_S32 dcount = 0;
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    stscan *scan = NULL;
    char *dir_dev = "/dev";
    char dname[8];
    MF_S32 len = 0;

    if (!name || !dlist) {
        return dcount;
    }

    len = strlen(name);
    while (len > 0) {
        len--;
        if (name[len] == '_') {
            strcpy(dname, &name[len + 1]);
            break;
        }
    }
    MFinfo("[Solin] strok %s", dname);
    if (strlen(dname) == 0) {
        MFinfo("no find disk dev name ...\n");
        return dcount;
    }

    if ((dir = opendir(dir_dev)) == NULL) {
        MFinfo("Open dir %s error ...\n", dir_dev);
        return dcount;
    }

    while ((ptr = readdir(dir)) != NULL) { //MF** is link to /dev/sda[b,c,d...].
        if ((ptr->d_type == DT_BLK) && !(strstr(ptr->d_name, "MF_")) &&
            (strstr(ptr->d_name, dname)) && (strlen(ptr->d_name) != strlen(dname))) {
            dcount++;
            scan = ms_malloc(sizeof(stscan));
            sprintf(scan->name, "%s/%s", dir_dev, ptr->d_name);
            MFinfo("dcount:%d %s", dcount, scan->name);
            list_add_tail(&scan->node, dlist);
        }
    }
    closedir(dir);

    return dcount;
}

static MF_S32
esata_scan_get_part1(struct diskObj *disk, MF_S8 *tname)
{
    MF_S32 count = 0;
    stscan *pos = NULL, *n = NULL;
    struct list_head list;

    if (!disk || !tname) {
        return count;
    }

    // check disk partition
    INIT_LIST_HEAD(&list);
    count = esata_scan_parts(disk->name, &list);
    if (count == 1) {
        list_for_each_entry(pos, &list, node)
        strcpy(tname, pos->name);
    }
    list_for_each_entry_safe(pos, n, &list, node) {
        list_del(&pos->node);
        ms_free(pos);
    }

    return count;
}

static MF_S32
esata_gdisk(char *disk_path)
{
    FILE *fp = NULL;
    char cmd[64] = {0};
    char del[64] = "p\n1\nd\n1\nd\n2\nd\n3\nd\n4\nn\n\n\n\n0700\nw\ny\n";
    char *tdel = del;

    if (disk_path == NULL) {
        MFerr("disk_path is NULL. \n");
        return MF_FAILURE;
    }
    //MFinfo("\n[Solin] %s", del);
    snprintf(cmd, sizeof(cmd), "gdisk %s", disk_path);

    if ((fp = ms_vpopen(cmd, "w")) == NULL) {
        perror("popen gdisk failed.");
        return MF_FAILURE;
    }
    while (*tdel) {
        putc(*tdel, fp);
        tdel++;
        if (*tdel == '\n') {
            sleep(1);
        }
    }
    ms_vpclose(fp);
    sleep(1);

    return MF_SUCCESS;
}

static MF_S32
esata_mount(void *pstPrivate)
{
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;
    MF_S8   cmd[128];
    MF_S32  res = MF_SUCCESS;

    MF_S32 count;
    MF_S8 tname[DEV_NAME_LEN];

    if (priv->parent->bRec) { // local disk action
        return MF_SUCCESS;
    }
    if (priv->parent->enState == DISK_STATE_BAD ||
        priv->parent->enState == DISK_STATE_OFFLINE) {
        return MF_FAILURE;
    }
    if (!strlen(priv->parent->mnt)) {
        sprintf(priv->parent->mnt, ESATA_MOUNT_POINT);
    }
    // update esata info and dont support msfs bplus tree format
    if (esata_header_check(priv->parent) == MF_SUCCESS &&
        priv->parent->enState == DISK_STATE_NORMAL) {
        return MF_FAILURE;
    }

    if (access(priv->parent->mnt, F_OK)) {
        MFinfo("no mount point %d", res);
        res = mkdir(priv->parent->mnt, 0775);
        if (res != 0) {
            MFerr("mkdir [%s] failed. \n", priv->parent->mnt);
            return MF_FAILURE ;
        }

        if (access(priv->parent->mnt, F_OK)) {
            MFerr("access [%s] failed. \n", priv->parent->mnt);
            return MF_FAILURE ;
        }
    }

    if (esata_is_mount(priv->parent->mnt) == MF_TRUE) {
        return MF_SUCCESS;
    }

    count = esata_scan_get_part1(priv->parent, tname);
    if (count && strlen(tname)) {
        sprintf(cmd, "ntfs-3g %s %s", tname, priv->parent->mnt);
    } else {
        sprintf(cmd, "ntfs-3g %s %s", priv->parent->name, priv->parent->mnt);
    }

    MFinfo("mount cmd :%s", cmd);
    if (access(priv->parent->alias, F_OK) == 0) {
        res = ms_system(cmd);
    }
    if (esata_is_mount(priv->parent->mnt) == MF_FALSE) {
        MFinfo("try mount ntfs fail");
        /* Promblem Number: ID1044980     Author:wuzh,   Date:2021/4/21
           Description    : esata盘拔插变成只读盘：设置errors的处理方式为continue(默认方式为ro-remount) */
        if (count && strlen(tname)) {
            sprintf(cmd, "mount -o utf8=1,errors=continue %s %s", tname, priv->parent->mnt);
        } else {
            sprintf(cmd, "mount -o utf8=1,errors=continue %s %s", priv->parent->name, priv->parent->mnt);
        }
        if (access(priv->parent->alias, F_OK) == 0) {
            res = ms_system(cmd);
        }
    }
    res = esata_is_mount(priv->parent->mnt) == MF_TRUE ? MF_SUCCESS : MF_FAILURE;

    return res;
}

static MF_S32
esata_umount(void *pstPrivate)
{
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;
    MF_S32  res = MF_SUCCESS;

    if (priv->parent->bRec) { // locak disk
        if (priv->parent->tree) {
            mf_index_release(priv->parent->tree);
            priv->parent->tree = NULL;
        }

        res = MF_SUCCESS;
    } else {
        if (esata_is_mount(priv->parent->mnt) == MF_TRUE) { //has mount
            res = umount(priv->parent->mnt);
            /* Promblem Number: -      Author:wuzh,   Date:2021/4/1
               Description    : ntfs格式写文件时拔出esata盘umount会失败,强制umount */
            if (res < 0) {
                char cmd[64];
                sprintf(cmd, "umount -l %s", priv->parent->mnt);
                res = ms_system(cmd);
            }
            MFinfo("umount path :%s res:%d", priv->parent->mnt, res);
            res = esata_is_mount(priv->parent->mnt) == MF_FALSE ? MF_SUCCESS : MF_FAILURE;
        }

        if (access(priv->parent->mnt, F_OK) == 0) { // exist
            MFinfo("has mount point %s, will delete it.", priv->parent->mnt);
            res = rmdir(priv->parent->mnt);
            if (res != 0) {
                MFerr("rmdir [%s] failed. \n", priv->parent->mnt);
                return MF_FAILURE ;
            }

            if (access(priv->parent->mnt, F_OK) == 0) { //still exist,  rmdir mnt failed.
                MFerr("still exist mnt [%s], failed. \n", priv->parent->mnt);
                return MF_FAILURE;
            }
        }
    }

    return res;
}

static MF_S32
esata_reset_head0_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 res = MF_FAILURE;

    if (fd > 0) {
        memset(pstDev->pstHeader, 0, size);
        res = disk_write(fd, pstDev->pstHeader, size, pstDev->sector);
        if (res < 0) {
            MFerr("disk_write failed");
        }
        disk_close(fd);
    }

    return res;
}

static MF_S32
esata_reset_head1_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 res = MF_FAILURE;

    if (fd > 0) {
        memset(pstDev->pstHeader, 0, size);
        res = disk_write(fd, pstDev->pstHeader, size, HEADER1_OFFSET(pstDev->sector));
        if (res < 0) {
            MFerr("disk_write failed");
        }
        disk_close(fd);
    }

    return res;
}

static void
esata_reset_allhead_info(struct diskObj *pstDev)
{
    if (disk_erase_MBR512(pstDev->name) == MF_FAILURE) {
        MFerr("erase mbr fail!");
    }
    esata_reset_head0_info(pstDev);
    esata_reset_head1_info(pstDev);
    return ;
}

static MF_S32
esata_format(void *pstPrivate, void *argv, MF_U64 quota)
{
    if (pstPrivate == NULL || argv == NULL) {
        MFerr("pstPrivate or argv is NULL. \n");
        return MF_FAILURE;
    }

    struct esata_priv *priv = (struct esata_priv *)pstPrivate;
    MF_S8 cmd[128];
    MF_S32 res;
    FORMAT_EN ftype = *(FORMAT_EN *)argv;

    MF_S32 count = 0;
    MF_S8 tname[DEV_NAME_LEN];

    struct bplus_key stKey;
    struct bplus_data stData;
    struct diskObj *pstDev = priv->parent;
    MF_U32 sector = pstDev->sector;
    uuid_t uu;
    MF_S32 i;
    MF_S8 *pKey = mf_dev_mmac();

    // percent time
    int start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    if (priv->parent->bRec) { // format local disk
        if (ftype != FORMAT_TYPE_MS) {
            return MF_FAILURE;
        }

        MFinfo("esata format local");
        esata_reset_allhead_info(pstDev);

        //disk header info init
        strcpy(pstDev->pstHeader->magic, DISK_MAGIC);
        strcpy(pstDev->pstHeader->version, DISK_VERSION);
        pstDev->pstHeader->size           = MF_POS(pstDev->pstHeader, checkSum);
        pstDev->pstHeader->attr           = 0;
        pstDev->pstHeader->logStatus      = 0;
        pstDev->pstHeader->capacity       = pstDev->capacity;
        pstDev->pstHeader->sector         = sector;
        pstDev->pstHeader->mainLogOff     = 65 * sector;
        pstDev->pstHeader->mainLogLen     = HEADER_MAINLOGLEN;
        pstDev->pstHeader->curLogOff      = pstDev->pstHeader->mainLogOff + pstDev->pstHeader->mainLogLen;
        pstDev->pstHeader->curLogLen      = HEADER_CURLOGLEN;
        pstDev->pstHeader->index0Status   = 0;
        pstDev->pstHeader->index1Status   = 0;
        pstDev->pstHeader->index00Off     = pstDev->pstHeader->curLogOff + pstDev->pstHeader->curLogLen + 65 * sector;
        pstDev->pstHeader->index01Off     = pstDev->pstHeader->index00Off + 25 * 1024 * 1024;
        pstDev->pstHeader->index10Off     = pstDev->pstHeader->index01Off + 25 * 1024 * 1024;
        pstDev->pstHeader->index11Off     = pstDev->pstHeader->index10Off + 25 * 1024 * 1024;
        pstDev->pstHeader->perFileSize    = FILE_PER_SIZE;
        pstDev->pstHeader->recordOff      = pstDev->pstHeader->index11Off + 25 * 1024 * 1024 + 1 * sector;
        pstDev->pstHeader->fileCount      = (pstDev->pstHeader->capacity - pstDev->pstHeader->recordOff) /
                                            pstDev->pstHeader->perFileSize - 1;
        pstDev->pstHeader->recordLen      = pstDev->pstHeader->fileCount * pstDev->pstHeader->perFileSize;
        pstDev->pstHeader->extendOff      = pstDev->pstHeader->recordOff + pstDev->pstHeader->recordLen + 10 * sector;
        pstDev->pstHeader->extendLen      = pstDev->pstHeader->capacity - pstDev->pstHeader->extendOff;
        pstDev->pstHeader->restoreTimes   = time(0);
        pstDev->pstHeader->restoreCounts  = 0;

        // create index by B+tree
        pstDev->tree = mf_index_rebuild(pstDev->tree, pstDev->name,
                                        pstDev->pstHeader->index00Off,
                                        pstDev,
                                        MF_YES);
        if (pstDev->tree == NULL) {
            /* if index00 failed then try index01 */
            pstDev->tree = mf_index_rebuild(NULL, pstDev->name,
                                            pstDev->pstHeader->index01Off,
                                            pstDev,
                                            MF_YES);
            if (pstDev->tree == NULL) {
                return MF_FAILURE;
            }

            MF_U64 tmpOff = pstDev->pstHeader->index00Off;
            pstDev->pstHeader->index00Off = pstDev->pstHeader->index01Off;
            pstDev->pstHeader->index01Off = tmpOff;
            pstDev->pstHeader->index0Status = !pstDev->pstHeader->index0Status;

            tmpOff = pstDev->pstHeader->index10Off;
            pstDev->pstHeader->index10Off = pstDev->pstHeader->index11Off;
            pstDev->pstHeader->index11Off = tmpOff;
            pstDev->pstHeader->index1Status = !pstDev->pstHeader->index1Status;
        }

        pstDev->tree->owner = pstDev;
        mf_index_key_init(&stKey, TYPE_RECORD);
        MFinfo("format step 1");
        for (i = 0; i < pstDev->pstHeader->fileCount; i++) {
            //stKey.stRecord.chnId = i % 64;
            stKey.stRecord.fileNo = i;
            stKey.stRecord.fileOffset = pstDev->pstHeader->recordOff + pstDev->pstHeader->perFileSize * i;
            //stKey.stRecord.startTime = time(0);
            stData.stRecord.fileOffset = stKey.stRecord.fileOffset;
            res = mf_index_key_insert(pstDev->tree, &stKey, &stData);
            if (res != MF_SUCCESS) {
                MFerr("insert key failed! res[%d]", res);
                return res;
            }
            pre = gap * ((i + 1) / (pstDev->pstHeader->fileCount * 1.0));
            if (abs(pre - percent) >= 1) {
                percent = pre;
                mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, pstDev->port, start + percent, pstDev->enType);
            }

//          mf_index_data_init(pstDev->tree, &stKey);
            // TODO:
        }
        mf_index_key_backup(pstDev->tree);
        MFinfo("format step 2");
        pstDev->pstHeader->index00Len = mf_index_size(pstDev->tree);
        pstDev->free = pstDev->pstHeader->recordLen;

        //create a uuid  string
        MFinfo("format step 3");
        uuid_generate(uu);
        uuid_unparse(uu, pstDev->pstHeader->uuid);

        //insert private flag to uuid[UUID_PRIVATE_BIT] , default: disable
        pstDev->pstHeader->uuid[UUID_PRIVATE_BIT] = 'N';
        //insert mac len to uuid[UUID_LEN_BIT]
        pstDev->pstHeader->uuid[UUID_LEN_BIT] = strlen(pKey);
        //insert mac from uuid[UUID_KEY_BIT] to uuid[strlen(pKey) -1]
        strncpy(&pstDev->pstHeader->uuid[UUID_KEY_BIT],
                pKey, pstDev->pstHeader->uuid[UUID_LEN_BIT]);
        //key end bit '-'
        pstDev->pstHeader->uuid[UUID_KEY_BIT + pstDev->pstHeader->uuid[UUID_LEN_BIT]] = '-';

        // check header sum
        MFinfo("format step 4");
        pstDev->pstHeader->checkSum = check_sum(pstDev->pstHeader, pstDev->pstHeader->size);

        MFinfo("format step 5");
        esata_set_head0_info(pstDev);
        esata_set_head1_info(pstDev);
        pstDev->ops->disk_log_fomat(pstDev);
        MFinfo("format step done");

        return MF_SUCCESS;
    }

    // for backup disk
    if (priv->parent->enState == DISK_STATE_BAD) { // || priv->parent->enState == DISK_STATE_OFFLINE
        return MF_FAILURE;
    }
    if (ftype != FORMAT_FAT32_TYPE && ftype != FORMAT_NTFS_TYPE) {
        return MF_FAILURE;
    }
    if (esata_umount(pstPrivate) != MF_SUCCESS) {
        return MF_FAILURE;
    }

    esata_reset_allhead_info(pstDev);

    MFinfo("format type 0x%x", ftype);
    switch (ftype) {
        case FORMAT_FAT32_TYPE: { // fat32
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(20, 30), priv->parent->enType);
            esata_gdisk(priv->parent->name);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(50, 60), priv->parent->enType);
            count = esata_scan_get_part1(priv->parent, tname);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(60, 70), priv->parent->enType);
            if (count && strlen(tname)) {
                sprintf(cmd, "mkfs.vfat -F 32 -I %s", tname);
            } else {
                sprintf(cmd, "mkfs.vfat -F 32 -I %s", priv->parent->name);
            }

            MFinfo("esata format cmd :%s", cmd);
            res = ms_system(cmd);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(70, 75), priv->parent->enType);
            if (res == MF_SUCCESS) {
                res = esata_mount(pstPrivate);
            }
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(75, 80), priv->parent->enType);
        }
        break;
        //case FORMAT_NTFS_FUSE_TYPE:
        case FORMAT_NTFS_TYPE: {
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(20, 30), priv->parent->enType);
            esata_gdisk(priv->parent->name);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(50, 60), priv->parent->enType);
            count = esata_scan_get_part1(priv->parent, tname);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(60, 70), priv->parent->enType);
            if (count && strlen(tname)) {
                sprintf(cmd, "mkntfs -FQq %s", tname);
            } else { // format ntfs ### -F - force format### -Q - perform a quick format
                sprintf(cmd, "mkntfs -FQq %s", priv->parent->name);
            }

            MFinfo("esata format cmd :%s", cmd);
            res = ms_system(cmd);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(70, 75), priv->parent->enType);
            if (res == MF_SUCCESS) {
                res = esata_mount(pstPrivate);
            }
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(75, 80), priv->parent->enType);
        }
        break;
        default :
            res = MF_FAILURE;
            break;
    }

    if (res == MF_SUCCESS && (esata_is_mount(priv->parent->mnt) == MF_TRUE)) {
        res = MF_SUCCESS;
    } else {
        res = MF_FAILURE;
    }

    return res;
}

static MF_S32
esata_size(void *pstPrivate)
{
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;
    struct statfs sfs;
    MF_S32  dev;

    if (priv->parent->bRec == MF_NO) {
        if (statfs(priv->parent->mnt, &sfs)) {
            perror("statfs");
            return MF_FAILURE;
        }
        MFinfo("name : %s", priv->parent->name);
        priv->parent->sector = 512;
        priv->parent->capacity = (MF_U64)sfs.f_blocks * sfs.f_bsize;
        priv->parent->free = (MF_U64)sfs.f_bavail * sfs.f_bsize;

        return MF_SUCCESS;
    } else {
        if ((dev = open(priv->parent->name, O_RDWR)) <= 0) {
            MFerr("[%s] open error!", priv->parent->name);
            return MF_FAILURE;
        }
        if (ioctl(dev, BLKGETSIZE64, &priv->parent->capacity) < 0) {
            MFerr("[%s] ioctl BLKGETSIZE64 ", priv->parent->name);
            close(dev);
            return MF_FAILURE;
        }
        close(dev);

        if (priv->parent->enState == DISK_STATE_NORMAL) {
            priv->parent->free = mf_retr_disk_free_size(priv->parent);
        } else {
            priv->parent->free = 0;
        }
    }
    return MF_SUCCESS;
}

static MF_S32
esata_vendor(void *pstPrivate)
{
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;
    FILE *pfd;
    MF_S8 vendor[DEV_NAME_LEN];
    MF_S8 cmd[DEV_NAME_LEN * 2];
    MF_S8 alias[DEV_NAME_LEN];

    sscanf(priv->parent->alias, "%63[^0-9]", alias);
    snprintf(cmd, sizeof(cmd), "lsscsi | grep %s | awk '{$1=\"\";$2=\"\";$3=\"\";$(NF-1)=\"\";$(NF)=\"\"; print}'", alias);
    pfd = ms_vpopen(cmd, "r");
    if (pfd) {
        if (fgets(vendor, sizeof(vendor), pfd) != NULL) {
            MF_S8 *p = vendor;
            while (*p == ' ') {
                p++;
            }
            strcpy(priv->parent->vendor, p);
        }
        ms_vpclose(pfd);
    }

    return MF_SUCCESS;
}

static MF_S32
esata_status(void *pstPrivate)
{
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;

    if (priv->parent->bRec) { // local disk
        if (esata_header_check(priv->parent) == MF_FAILURE) {
            priv->parent->enState = DISK_STATE_OFFLINE;
            goto EXIT;
        }
        /*      if (strcmp(priv->parent->pstHeader->magic, DISK_MAGIC))
                {
                    priv->parent->enState = DISK_STATE_UNFORMATTED;
                    MFinfo("disk[%s] unformatted", priv->parent->name);
                    return MF_SUCCESS;
                }
                if (strcmp(priv->parent->pstHeader->version, DISK_VERSION))
                {
                    priv->parent->enState = DISK_STATE_UNFORMATTED;
                    MFinfo("disk[%s] version dismatch", priv->parent->name);
                    return MF_SUCCESS;
                }
                if (priv->parent->pstHeader->checkSum != \
                    check_sum(priv->parent->pstHeader, MF_POS(priv->parent->pstHeader, checkSum)))
                {
                    priv->parent->enState = DISK_STATE_UNFORMATTED;
                    MFinfo("disk[%s] damage", priv->parent->name);
                    return MF_SUCCESS;
                }
                priv->parent->enState = DISK_STATE_NORMAL;*/
        if (!priv->parent->tree && priv->parent->enState == DISK_STATE_NORMAL) {
            priv->parent->tree = mf_index_fetch(priv->parent->name,
                                                priv->parent->pstHeader->index00Off,
                                                priv->parent);
        }
        if (priv->parent->enState != DISK_STATE_BAD && !mf_index_is_valid(priv->parent->tree)) {
            priv->parent->enState = DISK_STATE_UNFORMATTED;
        }
        MFinfo("DISK_ACTION_LOCAL");

        goto EXIT;
    }

    if (esata_is_mount(priv->parent->mnt) == MF_FALSE) {
        priv->parent->enState = DISK_STATE_UNFORMATTED;
    } else {
        priv->parent->enState = DISK_STATE_NORMAL;
    }

    MFinfo("DISK_ACTION_BACKUP");

EXIT:
    return MF_SUCCESS;
}

static MF_S32
esata_file_open(void *pstPrivate, int fileNo, MF_BOOL bO_DIRECT)
{
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;

    return disk_open(priv->parent->name, bO_DIRECT);
}

static MF_S32
esata_file_close(void *pstPrivate, int fd)
{
    return  disk_close(fd);
}

static MF_S32
esata_file_write(void *pstPrivate,
                 MF_S32 fd,
                 const void *buf,
                 MF_S32 count,
                 off64_t offset)
{
    MF_S32 res;
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;
    MF_U32 sector = priv->parent->sector;

    res = disk_write(fd, buf, count, offset);

    if (res != count) {
        MFerr("disk_write failed");
        res = mf_disk_error(fd, offset, count, sector);
    }

    return res;
}

static MF_S32
esata_file_read(void *pstPrivate,
                MF_S32 fd,
                void *buf,
                MF_S32 count,
                off64_t offset)
{
    MF_S32 res;
    struct esata_priv *priv = (struct esata_priv *)pstPrivate;
    MF_U32 sector = priv->parent->sector;

    res = disk_read(fd, buf, count, offset);

    if (res != count) {
        MFerr("disk_read failed");
        res = mf_disk_error(fd, offset, count, sector);
    }
    return res;
}

MF_S32
esata_log_init(void *disk)
{
    return disk_open(((struct diskObj *)disk)->name, MF_NO);
}

static MF_S32
esata_log_fomat(void *esata_disk)
{
    int fd = -1;
    int ret = 0;
    struct diskObj *disk = (struct diskObj *)esata_disk;

    fd = disk_open(disk->name, MF_NO);
    if (fd < 0) {
        ret = MF_FAILURE;
    } else {
        ret = mf_log_disk_format(fd, disk);
        disk_close(fd);
    }
    return ret;
}

static MF_S32
esata_index_backup(void *index)
{
    struct bplus_tree *tree = index;
    struct diskObj *pstDev = tree->owner;
    MF_U32 size = mf_index_size(tree);
    MF_S8 *out = ms_malloc(size);
    MF_U64 backupOff = pstDev->pstHeader->index10Off;
    MF_U32 crc = 0;
    MF_S32 res;

    res = mf_index_read_out(tree, out, size);
    if (res < 0) {
        ms_free(out);
        return MF_FAILURE;
    }

    crc = mf_index_get_crc(tree, out, size);
    if (crc == 0) {
        ms_free(out);
        MFerr("CRC is bad");
        return MF_FAILURE;
    }

    struct bplus_boot *boot = (struct bplus_boot *)out;

    if (boot->stInfo.enState == STATE_DONE) {
        boot->stInfo.crc = crc;
        mf_index_set_crc(tree, crc);
        res = disk_write(tree->fd, out, size, backupOff);
        if (res < 0) {
            MFerr("disk_write failed");
        }
    }

    ms_free(out);

    return res;
}

static MF_S32
esata_index_recover(void *index)
{
    struct bplus_tree *tree = index;
    struct diskObj *pstDev = tree->owner;
    struct bplus_boot *boot = ms_malloc(MAX_BLOCK_SIZE);
    MF_U64 backupOff = pstDev->pstHeader->index10Off;
    MF_U64 recoverOff = pstDev->pstHeader->index00Off;
    MF_S32 res = MF_SUCCESS;

    res = disk_read(tree->fd, boot, MAX_BLOCK_SIZE, backupOff);
    if (res < 0) {
        ms_free(boot);
        MFerr("disk_read failed");
        return MF_FAILURE;
    }

    if (boot->stInfo.magic == BPTREE_MAGIC
        &&  boot->stInfo.enState == STATE_DONE) {

        MF_U32 fileSize = (MF_U32)(boot->stInfo.file_size - recoverOff);
        MF_S8 *out = ms_malloc(fileSize);
        res = disk_read(tree->fd, out, fileSize, backupOff);
        if (res < 0) {
            ms_free(boot);
            ms_free(out);
            MFerr("disk_read failed");
            return MF_FAILURE;
        }

        MF_U32 crc = 0;
        MF_U32 NodeSize = (MF_U32)(boot->stInfo.file_size - boot->stInfo.root);
        MF_S32 NodeOff = fileSize - NodeSize;
        if (NodeOff >= 0) {
            crc = mf_index_get_crc(NULL, out + NodeOff, NodeSize);
        } else {
            ms_free(boot);
            ms_free(out);
            MFerr("file size is bad");
            return MF_FAILURE;
        }

        if (crc == boot->stInfo.crc) {
            res = disk_write(tree->fd, out, fileSize, recoverOff);
            if (res < 0) {
                MFerr("disk_write failed");
            }
        } else {
            MFerr("CRC is dismatch [%x][%x]", crc, boot->stInfo.crc);
        }
        ms_free(out);
    }

    ms_free(boot);

    return res;
}

static MF_S32
esata_index_restore(void *pstPrivate)
{
    struct diskObj *pstDev = ((struct esata_priv *)pstPrivate)->parent;
    struct bplus_tree *tree;
    struct bplus_key stKey;
    struct bplus_data stData;
    struct mf_file stFile;
    MF_S32 i;
    MF_S32 res;
    MF_S32 start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    if (strcmp(pstDev->pstHeader->magic, DISK_MAGIC) != 0) {
        MFerr("disk magic is not[%s]\n", DISK_MAGIC);
        return MF_FAILURE;
    }

    /* rebulid index tree */
    MFinfo("step 1:  rebulid index tree\n");
    tree = mf_index_rebuild(NULL, pstDev->name,
                            pstDev->pstHeader->index01Off,
                            pstDev,
                            MF_YES);
    if (tree == NULL) {
        return MF_FAILURE;
    }

    for (i = 0; i < pstDev->pstHeader->fileCount; i++) {
        mf_index_key_init(&stKey, TYPE_RECORD);
        stKey.stRecord.fileNo       = i;
        stKey.stRecord.fileOffset   = pstDev->pstHeader->recordOff + pstDev->pstHeader->perFileSize * i;
        stData.stRecord.fileOffset  = stKey.stRecord.fileOffset;
        stFile.fileNo               = stKey.stRecord.fileNo;
        stFile.fileOffset           = stKey.stRecord.fileOffset;

        res = mf_retr_file_info_from_disk(pstDev, &stFile);
        if (res == MF_SUCCESS) {
            stKey.stRecord.chnId        = stFile.chnId;
            stKey.stRecord.type         = stFile.fileType;
            stKey.stRecord.state        = stFile.fileState;
            stKey.stRecord.segNumMsb    = stFile.segNum / 256;
            stKey.stRecord.segNumLsb    = stFile.segNum % 256;
            stKey.stRecord.event        = stFile.recEvent;
            stKey.stRecord.fileNo       = stFile.fileNo;
            stKey.stRecord.fileOffset   = stFile.fileOffset;
            stKey.stRecord.startTime    = stFile.recStartTime;
            stKey.stRecord.endTime      = stFile.recEndTime;
            stKey.stRecord.isLocked     = stFile.lockNum ? 1 : 0;
            stKey.stRecord.infoType     = stFile.infoTypes | stFile.infoSubTypes;
            if (!(stKey.stRecord.infoType & INFO_MAJOR_INIT)) { //Compatibility with older versions
                stKey.stRecord.infoType = 0xffffffff;
            }
        }
        res = mf_index_key_insert(tree, &stKey, &stData);
        if (res != MF_SUCCESS) {
            MFerr("insert key[%d] failed! res[%d]", stKey.stRecord.fileNo, res);
            mf_index_release(tree);
            return res;
        }

        pre = gap * ((i + 1) / (pstDev->pstHeader->fileCount * 1.0));
        if (abs(pre - percent) >= 1) {
            percent = pre;
            mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, pstDev->port, start + percent, pstDev->enType);
        }
    }

    /* update index tree */
    MFinfo("step 2: update index tree\n");
    mf_index_release(pstDev->tree);
    pstDev->tree = tree;

    /* update header */
    MFinfo("step 3: update header\n");
    MF_U64 tmpOff = pstDev->pstHeader->index00Off;
    pstDev->pstHeader->index00Off = pstDev->pstHeader->index01Off;
    pstDev->pstHeader->index01Off = tmpOff;
    pstDev->pstHeader->index00Len = mf_index_size(pstDev->tree);
    pstDev->pstHeader->index01Len = 0;
    pstDev->pstHeader->index0Status = !pstDev->pstHeader->index0Status;

    tmpOff = pstDev->pstHeader->index10Off;
    pstDev->pstHeader->index10Off = pstDev->pstHeader->index11Off;
    pstDev->pstHeader->index11Off = tmpOff;
    pstDev->pstHeader->index10Len = mf_index_size(pstDev->tree);
    pstDev->pstHeader->index11Len = 0;
    pstDev->pstHeader->index1Status = !pstDev->pstHeader->index1Status;


    pstDev->pstHeader->restoreTimes = time(0);
    pstDev->pstHeader->restoreCounts++;
    pstDev->pstHeader->checkSum = check_sum(pstDev->pstHeader, pstDev->pstHeader->size);

    esata_set_head0_info(pstDev);// set header 0 info
    esata_set_head1_info(pstDev);// set header 1 info

    /* backup index tree */
    MFinfo("step 4: backup index tree\n");
    mf_index_key_backup(pstDev->tree);

    return MF_SUCCESS;
}

static MF_S32
esata_index_restoreEx(void *pstPrivate)
{
    struct diskObj *pstDev = ((struct esata_priv *)pstPrivate)->parent;
    struct bplus_tree *tree;
    struct bplus_key stKey;
    struct bplus_data stData;
    struct mf_file stFile;
    MF_S32 i;
    MF_S32 res;
    MF_S32 start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    MF_U32 sector = 512;

    MFinfo("esata format local");
    esata_reset_allhead_info(pstDev);

    // disk header info init
    strcpy(pstDev->pstHeader->magic, DISK_MAGIC);
    strcpy(pstDev->pstHeader->version, DISK_VERSION);
    pstDev->pstHeader->size          = MF_POS(pstDev->pstHeader, checkSum);
    pstDev->pstHeader->attr          = 0;
    pstDev->pstHeader->logStatus     = 0;
    pstDev->pstHeader->capacity      = pstDev->capacity;
    pstDev->pstHeader->sector        = sector;
    pstDev->pstHeader->mainLogOff    = 65 * sector;
    pstDev->pstHeader->mainLogLen    = HEADER_MAINLOGLEN;
    pstDev->pstHeader->curLogOff     = pstDev->pstHeader->mainLogOff + pstDev->pstHeader->mainLogLen;
    pstDev->pstHeader->curLogLen     = HEADER_CURLOGLEN;
    pstDev->pstHeader->index0Status  = 0;
    pstDev->pstHeader->index1Status  = 0;
    pstDev->pstHeader->index00Off    = pstDev->pstHeader->curLogOff + pstDev->pstHeader->curLogLen + 65 * sector;
    pstDev->pstHeader->index01Off    = pstDev->pstHeader->index00Off + 25 * 1024 * 1024;
    pstDev->pstHeader->index10Off    = pstDev->pstHeader->index01Off + 25 * 1024 * 1024;
    pstDev->pstHeader->index11Off    = pstDev->pstHeader->index10Off + 25 * 1024 * 1024;
    pstDev->pstHeader->perFileSize   = FILE_PER_SIZE;
    pstDev->pstHeader->recordOff     = pstDev->pstHeader->index11Off + 25 * 1024 * 1024 + 1 * sector;
    pstDev->pstHeader->fileCount     = (pstDev->pstHeader->capacity - pstDev->pstHeader->recordOff) / pstDev->pstHeader->perFileSize - 1;
    pstDev->pstHeader->recordLen     = pstDev->pstHeader->fileCount * pstDev->pstHeader->perFileSize;
    pstDev->pstHeader->extendOff     = pstDev->pstHeader->recordOff + pstDev->pstHeader->recordLen + 10 * sector;
    pstDev->pstHeader->extendLen     = pstDev->pstHeader->capacity - pstDev->pstHeader->extendOff;
    pstDev->pstHeader->restoreTimes  = time(0);
    pstDev->pstHeader->restoreCounts = 0;

    /* rebulid index tree */
    MFinfo("step 1:  rebulid index tree\n");
    tree = mf_index_rebuild(NULL, pstDev->name,
                            pstDev->pstHeader->index01Off,
                            pstDev,
                            MF_YES);
    if (tree == NULL) {
        return MF_FAILURE;
    }

    for (i = 0; i < pstDev->pstHeader->fileCount; i++) {
        mf_index_key_init(&stKey, TYPE_RECORD);
        stKey.stRecord.fileNo       = i;
        stKey.stRecord.fileOffset   = pstDev->pstHeader->recordOff + pstDev->pstHeader->perFileSize * i;
        stData.stRecord.fileOffset  = stKey.stRecord.fileOffset;
        stFile.fileNo               = stKey.stRecord.fileNo;
        stFile.fileOffset           = stKey.stRecord.fileOffset;

        res = mf_retr_file_info_from_disk(pstDev, &stFile);
        if (res == MF_SUCCESS) {
            stKey.stRecord.chnId        = stFile.chnId;
            stKey.stRecord.type         = stFile.fileType;
            stKey.stRecord.state        = stFile.fileState;
            stKey.stRecord.segNumMsb    = stFile.segNum / 256;
            stKey.stRecord.segNumLsb    = stFile.segNum % 256;
            stKey.stRecord.event        = stFile.recEvent;
            stKey.stRecord.fileNo       = stFile.fileNo;
            stKey.stRecord.fileOffset   = stFile.fileOffset;
            stKey.stRecord.startTime    = stFile.recStartTime;
            stKey.stRecord.endTime      = stFile.recEndTime;
            stKey.stRecord.isLocked     = stFile.lockNum ? 1 : 0;
            stKey.stRecord.infoType     = stFile.infoTypes | stFile.infoSubTypes;
            if (!(stKey.stRecord.infoType & INFO_MAJOR_INIT)) { //Compatibility with older versions
                stKey.stRecord.infoType = 0xffffffff;
            }
        }
        res = mf_index_key_insert(tree, &stKey, &stData);
        if (res != MF_SUCCESS) {
            MFerr("insert key[%d] failed! res[%d]", stKey.stRecord.fileNo, res);
            mf_index_release(tree);
            return res;
        }

        pre = gap * ((i + 1) / (pstDev->pstHeader->fileCount * 1.0));
        if (abs(pre - percent) >= 1) {
            percent = pre;
            mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, pstDev->port, start + percent, pstDev->enType);
        }
    }

    /* update index tree */
    MFinfo("step 2: update index tree\n");
    mf_index_release(pstDev->tree);
    pstDev->tree = tree;

    /* update header */
    MFinfo("step 3: update header\n");
    MF_U64 tmpOff = pstDev->pstHeader->index00Off;
    pstDev->pstHeader->index00Off = pstDev->pstHeader->index01Off;
    pstDev->pstHeader->index01Off = tmpOff;
    pstDev->pstHeader->index00Len = mf_index_size(pstDev->tree);
    pstDev->pstHeader->index01Len = 0;
    pstDev->pstHeader->index0Status = !pstDev->pstHeader->index0Status;

    tmpOff = pstDev->pstHeader->index10Off;
    pstDev->pstHeader->index10Off = pstDev->pstHeader->index11Off;
    pstDev->pstHeader->index11Off = tmpOff;
    pstDev->pstHeader->index10Len = mf_index_size(pstDev->tree);
    pstDev->pstHeader->index11Len = 0;
    pstDev->pstHeader->index1Status = !pstDev->pstHeader->index1Status;


    pstDev->pstHeader->restoreTimes = time(0);
    pstDev->pstHeader->restoreCounts++;
    pstDev->pstHeader->checkSum = check_sum(pstDev->pstHeader, pstDev->pstHeader->size);

    esata_set_head0_info(pstDev);// set header 0 info
    esata_set_head1_info(pstDev);// set header 1 info

    /* backup index tree */
    MFinfo("step 4: backup index tree\n");
    mf_index_key_backup(pstDev->tree);

    return MF_SUCCESS;
}

static MF_S32
esata_go_private(void *pstPrivate, MF_BOOL bEnable)
{
    struct diskObj *pstDev = ((struct esata_priv *)pstPrivate)->parent;
    MF_S32 res = MF_FAILURE;

    esata_get_head0_info(pstDev);
    pstDev->pstHeader->uuid[UUID_PRIVATE_BIT] = bEnable ? 'Y' : 'N';
    if (bEnable) {
        MF_S8 *pKey = mf_dev_mmac();
        //insert mac len to uuid[UUID_LEN_BIT]
        pstDev->pstHeader->uuid[UUID_LEN_BIT] = strlen(pKey);
        //insert mac from uuid[UUID_KEY_BIT] to uuid[strlen(pKey) -1]
        strncpy(&pstDev->pstHeader->uuid[UUID_KEY_BIT],
                pKey, pstDev->pstHeader->uuid[UUID_LEN_BIT]);
    }

    pstDev->pstHeader->checkSum =
        check_sum(pstDev->pstHeader, pstDev->pstHeader->size);

    if (esata_set_head0_info(pstDev) == MF_SUCCESS) { // set header 0 info
        res = esata_set_head1_info(pstDev);    // set header 1 info
    }

    return res;
}

static struct disk_ops esata_ops = {
    .disk_format        = esata_format,
    .disk_mount         = esata_mount,
    .disk_umount        = esata_umount,
    .disk_size          = esata_size,
    .disk_vendor        = esata_vendor,
    .disk_status        = esata_status,
    .disk_file_open     = esata_file_open,
    .disk_file_close    = esata_file_close,
    .disk_file_read     = esata_file_read,
    .disk_file_write    = esata_file_write,
    .disk_log_fomat     = esata_log_fomat,
    .disk_log_init      = esata_log_init,
    .disk_index_backup  = esata_index_backup,
    .disk_index_recover = esata_index_recover,
    .disk_index_restore = esata_index_restore,
    .disk_go_private    = esata_go_private,
    .disk_index_restoreEx = esata_index_restoreEx,
};

MF_S32
disk_esata_register(struct diskObj *pstDiskObj, MF_S8 *host)
{
    struct esata_priv *pstPrivate;
    pstDiskObj->ops = &esata_ops;

    pstPrivate = ms_calloc(1, sizeof(struct esata_priv));
    pstPrivate->parent = pstDiskObj;
    pstDiskObj->pstPrivate = pstPrivate;

    return MF_SUCCESS;
}

MF_S32
disk_esata_unregister(struct diskObj *pstDiskObj)
{
    ms_free(pstDiskObj->pstPrivate);
    return MF_SUCCESS;
}

