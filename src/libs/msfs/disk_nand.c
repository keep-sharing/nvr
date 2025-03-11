/*
 * ***************************************************************
 * Filename:        disk_nand.c
 * Created at:      2018.09.06
 * Description:     nand flash API
 * Author:          zbing
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
#include <fcntl.h>
#include <errno.h>
#include <uuid/uuid.h>
#include <dirent.h>
#include <regex.h>

#include "msfs_disk.h"
#include "MFdisk.h"
#include "MFlog.h"

#define MSLOG       "MLOG.bin"

typedef struct nand_priv {
    struct diskObj *parent;
} nand_priv;

static MF_S32
nand_is_mount(MF_S8 *mnt)
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
nand_size(void *pstPrivate)
{
    struct nand_priv *priv = (struct nand_priv *)pstPrivate;
    struct statfs sfs;

    priv->parent->capacity = 0;
    priv->parent->free = 0;

    if (statfs(priv->parent->mnt, &sfs)) {
        perror("statfs");
        return MF_FAILURE;
    }
    priv->parent->capacity = (MF_U64)sfs.f_blocks * sfs.f_bsize;
    priv->parent->free = (MF_U64)sfs.f_bavail * sfs.f_bsize;

    return MF_SUCCESS;
}

static MF_S32
nand_status(void *pstPrivate)
{
    struct nand_priv *priv = (struct nand_priv *)pstPrivate;

    priv->parent->enState = DISK_STATE_NORMAL;
    priv->parent->tree = NULL;

    return MF_SUCCESS;
}

static MF_S32
nand_mount(void *pstPrivate)
{
    struct nand_priv *priv = (struct nand_priv *)pstPrivate;

    if (!priv) {
        return MF_FAILURE;
    }
    if (!strlen(priv->parent->mnt)) {
        sprintf(priv->parent->mnt, "/mnt/%s", priv->parent->alias);
    }

    if (access(priv->parent->mnt, F_OK)) {
        if (mkdir(priv->parent->mnt, 0775) != 0) {
            MFwarm("mkdir [%s] failed. \n", priv->parent->mnt);
            return MF_FAILURE;
        }
        if (access(priv->parent->mnt, F_OK)) {
            MFwarm("access [%s] failed. \n", priv->parent->mnt);
            return MF_FAILURE;
        }
    }

    if (nand_is_mount(priv->parent->mnt) == MF_TRUE) {
        return MF_SUCCESS;
    }

    return nand_is_mount(priv->parent->mnt) == MF_TRUE ? MF_SUCCESS : MF_FAILURE;
}

static MF_S32
nand_umount(void *pstPrivate)
{
    return MF_SUCCESS;
}

static MF_S32
nand_vendor(void *pstPrivate)
{
    struct nand_priv *priv = (struct nand_priv *)pstPrivate;
    strcpy(priv->parent->vendor, priv->parent->alias);

    return MF_SUCCESS;
}

MF_S32
nand_log_init(void *pdisk)
{
    MF_S8 logName[128];
    MF_S32 fd, exist = MF_NO;
    struct diskObj *disk = (struct diskObj *)pdisk;

    sprintf(logName, "%s/%s", disk->mnt, MSLOG);
    if (access(logName, F_OK) == 0) {
        exist = MF_YES;
    }

    disk->pstHeader->mainLogOff = 0;
    disk->pstHeader->mainLogLen = HEADER_NANDLOG_SIZE;

    fd = disk_open(logName, MF_NO);
    if (fd < 0) {
        fd = MF_FAILURE;
    } else {
        if (exist == MF_NO) {
            mf_log_disk_format(fd, disk);
        }
    }

    return fd;
}

static MF_S32
nand_file_open(void *pstPrivate, int fileNo, MF_BOOL bO_DIRECT)
{
    return -1;
}

static MF_S32
nand_file_close(void *pstPrivate, int fd)
{
    return disk_close(fd);
}

static MF_S32
nand_file_write(void *pstPrivate,
                MF_S32 fd,
                const void *buf,
                MF_S32 count,
                off64_t offset)
{
    MF_S32 res;
    res = disk_write(fd, buf, count, offset);
    if (res < 0) {
        MFerr("disk_write failed");
    }

    return res;
}

static MF_S32
nand_file_read(void *pstPrivate,
               MF_S32 fd,
               void *buf,
               MF_S32 count,
               off64_t offset)
{
    MF_S32 res;
    res = disk_read(fd, buf, count, offset);
    if (res < 0) {
        MFerr("disk_read failed");
    }

    return res;
}

MF_S32
nand_log_fomat(void *nand_disk)
{
    int fd = -1;
    int ret = 0;
    MF_S8 logName[128];
    struct diskObj *disk = (struct diskObj *)nand_disk;

    sprintf(logName, "%s/%s", disk->mnt, MSLOG);

    disk->pstHeader->mainLogOff = 0;
    disk->pstHeader->mainLogLen = HEADER_NANDLOG_SIZE;

    fd = disk_open(logName, MF_NO);
    if (fd < 0) {
        ret = MF_FAILURE;
    } else {
        ret = mf_log_disk_format(fd, disk);
        disk_close(fd);
    }
    return ret;
}


static struct disk_ops nand_ops = {
    .disk_mount         = nand_mount,
    .disk_umount        = nand_umount,
    .disk_size          = nand_size,
    .disk_vendor        = nand_vendor,
    .disk_status        = nand_status,
    .disk_file_open     = nand_file_open,
    .disk_file_close    = nand_file_close,
    .disk_file_read     = nand_file_read,
    .disk_file_write    = nand_file_write,
    .disk_log_fomat     = nand_log_fomat,
    .disk_log_init      = nand_log_init,
};

MF_S32
disk_nand_register(struct diskObj *pstDiskObj, MF_S8 *host)
{
    struct nand_priv *pstPrivate;
    pstDiskObj->ops = &nand_ops;

    pstPrivate = ms_calloc(1, sizeof(struct nand_priv));
    pstPrivate->parent = pstDiskObj;
    pstDiskObj->pstPrivate = pstPrivate;

    return MF_SUCCESS;
}

MF_S32
disk_nand_unregister(struct diskObj *pstDiskObj)
{
    if (pstDiskObj->pstPrivate) {
        ms_free(pstDiskObj->pstPrivate);
    }

    return MF_SUCCESS;
}


