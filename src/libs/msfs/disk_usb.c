/*
 * ***************************************************************
 * Filename:        disk_usb.c
 * Created at:      2017.05.10
 * Description:     usb mangement API
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
#include <sys/vfs.h>
#include <errno.h>
#include <dirent.h>

#include "msfs_disk.h"
#include "MFdisk.h"

#define USB_MOUNT_PREFIX "/media/usb"
#define USB_UMOUNT_TIME  (5)

#define USB_FSTYPE_NTFS "NTFS"
#define USB_FSTYPE_FAT32_WIN1 "mkdosfs"
#define USB_FSTYPE_FAT32_WIN2 "msdos"
#define USB_FSTYPE_FAT32_UNI "mkfs.fat"
#define USB_FSTYPE_FAT32_ULTRA "ultraiso"
#define USB_FSTYPE_EXFAT "EXFAT" // for next 9.0.2 version

#ifndef MS_DIRSYNC
    #define MS_DIRSYNC  (128) //for hi3536c only defined  S_WRITE = 128
#endif

/*
* KERNEL=="sd*[0-9]", KERNELS=="1-1", SUBSYSTEMS=="usb", DRIVERS=="usb", SYMLINK="MF_usb_34_%k"
* KERNEL=="sd*[0-9]", KERNELS=="1-2", SUBSYSTEMS=="usb", DRIVERS=="usb", SYMLINK="MF_usb_35_%k"
* KERNEL=="sd*[0-9]", KERNELS=="3-1", SUBSYSTEMS=="usb", DRIVERS=="usb", SYMLINK="MF_usb_36_%k"
* KERNEL=="sd*[0-9]", KERNELS=="4-1", SUBSYSTEMS=="usb", DRIVERS=="usb", SYMLINK="MF_usb_37_%k"
*/

typedef struct usb_priv {
    struct diskObj *parent;
} usb_priv;

/*****usb backup function
1. create new file directory
2. format
3. back up
******/


static MF_S32
usb_is_mount(MF_S8 *mnt)
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
usb_scan_parts(char *name, struct list_head *dlist)
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

//get first partition
static MF_S32
usb_scan_get_part1(struct diskObj *disk, MF_S8 *tname)
{
    MF_S32 count = 0;
    stscan *pos = NULL, *n = NULL;
    struct list_head list;

    if (!disk || !tname) {
        return count;
    }

    INIT_LIST_HEAD(&list);
    count = usb_scan_parts(disk->name, &list);
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
usb_gdisk(char *disk_path, struct diskObj *pstDev)
{
    FILE *fp = NULL;
    char cmd[64] = {0};
    char del[64] = "p\n1\nd\n1\nd\n2\nd\n3\nd\n4\nn\n\n\n\n0700\nw\ny\n";
    char *tdel = del;

    if (disk_path == NULL || !pstDev) {
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
    mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(30, 40), pstDev->enType);
    sleep(1);

    return MF_SUCCESS;
}

/*****usb backup function
1. create new file directory
2. format
3. back up
******/

static MF_S32
usb_mount_exec(MF_S8 *fdev, MF_S8 *ftarget, MF_S8 *ftype)
{
    MF_S32 res = MF_FAILURE;

    if (!fdev || !ftarget || !ftype) {
        return res;
    }
    if (access(fdev, F_OK) != 0) {
        return res;
    }

    // MS_MGC_VAL for old kernel.solin
    res = mount(fdev, ftarget, ftype, MS_MGC_VAL | MS_DIRSYNC, "utf8=1");
    if (res < 0) {
        //perror("mount fail");
        if (errno == EROFS) {
            res = mount(fdev, ftarget, ftype, MS_MGC_VAL | MS_RDONLY, "utf8=1");
        }
    }
    return res;
}

static MF_S32
usb_get_fstype(MF_S8 *dev)
{
    FILE *f = NULL;
    MF_S8 buf[16] = {0};

    if (!dev) {
        return MF_FAILURE;
    }
    if ((f = fopen(dev, "r")) == NULL) {
        return MF_FAILURE;
    }

    fread(buf, sizeof(buf) - 1, 1, f);
    fclose(f);

    MFinfo("dev[%s]", buf + 3);
    if (!strncmp(buf + 3, USB_FSTYPE_NTFS, strlen(USB_FSTYPE_NTFS))) {
        return FORMAT_NTFS_TYPE;
    }
    if (!strncmp(buf + 3, USB_FSTYPE_EXFAT, strlen(USB_FSTYPE_EXFAT))) {
        return FORMAT_NTFS_FUSE_TYPE;
    }
    if (!strncmp(buf + 3, USB_FSTYPE_FAT32_WIN1, strlen(USB_FSTYPE_FAT32_WIN1)) ||
        !strncmp(buf + 3, USB_FSTYPE_FAT32_UNI, strlen(USB_FSTYPE_FAT32_UNI)) ||
        !strncasecmp(buf + 3, USB_FSTYPE_FAT32_WIN2, strlen(USB_FSTYPE_FAT32_WIN2)) ||
        !strncasecmp(buf + 3, USB_FSTYPE_FAT32_ULTRA, strlen(USB_FSTYPE_FAT32_ULTRA))) {
        return FORMAT_FAT32_TYPE;
    }

    return MF_UNKNOWN;
}

static MF_S32
usb_mount(void *pstPrivate)
{
    struct usb_priv *priv = (struct usb_priv *)pstPrivate;
    MF_S8  cmd[128];
    MF_S32 res = MF_FAILURE;

    MF_S32 count;
    FILE *f = NULL;
    MF_S8 tname[DEV_NAME_LEN];

    if (strlen(priv->parent->mnt) <= 0) { //  find usb id name
        mf_disk_usb_get_mntpath(priv->parent->port, priv->parent->mnt);
        if (strlen(priv->parent->mnt) <= 0) {
            return MF_FAILURE;
        }
    }

    if (access(priv->parent->mnt, F_OK)) {
        MFinfo("no mount point %s", priv->parent->mnt);
        res = mkdir(priv->parent->mnt, 0775);
        if (res != 0) {
            MFerr("mkdir [%s] failed. \n", priv->parent->mnt);
            return MF_FAILURE ;
        }

        if (access(priv->parent->mnt, F_OK)) { // mkdir mnt failed.
            MFerr("access mnt [%s] failed. \n", priv->parent->mnt);
            return MF_FAILURE;
        }
    }

    if (usb_is_mount(priv->parent->mnt) == MF_TRUE) {
        return MF_SUCCESS;
    }

    count = usb_scan_get_part1(priv->parent, tname);
    if (count && strlen(tname)) {
        MFinfo("dev[%s]", tname);
    } else {
        sprintf(tname, "%s", priv->parent->name);
    }

    res = MF_FAILURE;
    switch (usb_get_fstype(tname)) {
        case FORMAT_NTFS_TYPE: {
            sprintf(cmd, "ntfsfix %s", tname);// check ntfs clusters is ok ?
            ms_system(cmd);

            sprintf(cmd, "ntfs-3g %s %s", tname, priv->parent->mnt);
            if (access(priv->parent->alias, F_OK) == 0) {
                res = ms_system(cmd);
            }
        }
        break;
        case FORMAT_NTFS_FUSE_TYPE: {
            sprintf(cmd, "fsck.exfat %s", tname);// check exfat clusters is ok ?
            ms_system(cmd);
            sprintf(cmd, "mount.exfat %s %s", tname, priv->parent->mnt);// check exfat clusters is ok ?
            if (access(priv->parent->alias, F_OK) == 0) {
                res = ms_system(cmd);
            }
        }
        break;
        case FORMAT_FAT32_TYPE: {
            //sprintf(cmd, "dosfsck -aw %s", tname);// check vfat clusters is ok ?
            //ms_system(cmd);

            if (access(priv->parent->alias, F_OK) == 0) {
                res = usb_mount_exec(tname, priv->parent->mnt, "vfat");
            }
        }
        break;
        default :
            if (res < 0) {
                res = usb_mount_exec(tname, priv->parent->mnt, "ext4");
            }
            if (res < 0) {
                res = usb_mount_exec(tname, priv->parent->mnt, "ext3");
            }
            if (res < 0) {
                res = usb_mount_exec(tname, priv->parent->mnt, "ext2");
            }
            break;
    }

    /*res = MF_FAILURE;
    if (access(priv->parent->alias, F_OK) == 0)
        res = ms_system(cmd);

    if (usb_is_mount(priv->parent->mnt) == MF_FALSE)
    {
        //sprintf(cmd, "mount -o utf8=1 %s %s", tname, priv->parent->mnt);//"vfat"
        sprintf(cmd, "dosfsck -a %s", tname);// check vfat clusters is ok ?
        ms_system(cmd);

        MFinfo("tname:%s mnt:%s", tname, priv->parent->mnt);
        res = usb_mount_exec(tname, priv->parent->mnt, "vfat");
    }
    else
    {
        // check disk is accessed
        sprintf(cmd, "%s/.mtest", priv->parent->mnt);
        MFinfo("check[%s]", cmd);
        if ((f = fopen(cmd, "wb")) != NULL)
        {
            fclose(f);
            unlink(cmd);
        }
        else
            return MF_FAILURE;
    }*/

    if (usb_is_mount(priv->parent->mnt) == MF_TRUE) {
        sprintf(cmd, "%s/.mtest", priv->parent->mnt);
        //MFinfo("check[%s]", cmd);
        if ((f = fopen(cmd, "wb")) != NULL) {
            fclose(f);
            unlink(cmd);
        } else {
            if (priv->parent->ops->disk_umount) {
                priv->parent->ops->disk_umount(priv);
            }
            return MF_FAILURE;
        }
    }

    res = usb_is_mount(priv->parent->mnt) == MF_TRUE ? MF_SUCCESS : MF_FAILURE;

    return res;
}

static MF_S32
usb_umount(void *pstPrivate)
{
    struct usb_priv *priv = (struct usb_priv *)pstPrivate;
    MF_S8 cmd[128];
    MF_S32 res = MF_SUCCESS;
    MF_S32 count = USB_UMOUNT_TIME;

    if (priv->parent->tree) {
        mf_index_release(priv->parent->tree);
        priv->parent->tree = NULL;
    }

    while ((res = usb_is_mount(priv->parent->mnt)) == MF_TRUE && count > 0) {
        sprintf(cmd, "umount %s", priv->parent->mnt);
        res = ms_system(cmd);
        usleep(1000);
        count--;
    }
    MFinfo("umount path :%s res:%d errno:%d", priv->parent->mnt, res, errno);
	
    if (res == MF_SUCCESS && remove(priv->parent->mnt) < 0) {
        MFinfo("DEL: %s NG", priv->parent->mnt);
    }

    // delete mnt dir
    /*if(access(priv->parent->mnt, F_OK) == 0)  // exist
    {
        MFinfo("has mount point %s, will delete it.", priv->parent->mnt);
        count = USB_UMOUNT_TIME;
        while((res = rmdir(priv->parent->mnt)) != 0 && count > 0)
        {
            usleep(1000);
            count--;
        }
        if(res != 0)
        {
            MFerr("rmdir [%s] failed.[errno:%d] \n", priv->parent->mnt, errno);
            return MF_FAILURE ;
        }

        if(access(priv->parent->mnt, F_OK) == 0) //still exist,  rmdir mnt failed.
        {
            MFerr("still exist mnt [%s], failed. \n", priv->parent->mnt);
            return MF_FAILURE;
        }
    }*/

    return res;
}

static MF_S32
usb_format(void *pstPrivate, void *argv, MF_U64 quota)
{
    struct usb_priv *priv = (struct usb_priv *)pstPrivate;
    MF_S8 cmd[128];
    MF_S32 res;
    FORMAT_EN ftype = *(FORMAT_EN *)argv;

    MF_S32 count;
    MF_S8 tname[DEV_NAME_LEN];

    if ((usb_is_mount(priv->parent->mnt) == MF_TRUE) && usb_umount(pstPrivate) != MF_SUCCESS) {
        MFinfo("mount->umount fail");
        return MF_FAILURE;
    }

    MFinfo("format type 0x%x", ftype);
    switch (ftype) {
        case FORMAT_FAT32_TYPE: { // fat32
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(20, 30), priv->parent->enType);
            usb_gdisk(priv->parent->name, priv->parent);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(50, 60), priv->parent->enType);
            count = usb_scan_get_part1(priv->parent, tname);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(60, 70), priv->parent->enType);
            if (count && strlen(tname)) {
                sprintf(cmd, "mkfs.vfat -F 32 -I %s", tname);
            } else {
                sprintf(cmd, "mkfs.vfat -F 32 -I %s", priv->parent->name);
            }

            MFinfo("usb format cmd :%s", cmd);
            res = ms_system(cmd);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(70, 75), priv->parent->enType);
            if (res == MF_SUCCESS) {
                res = usb_mount(pstPrivate);
            }
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(75, 80), priv->parent->enType);
        }
        break;
        case FORMAT_NTFS_FUSE_TYPE: {
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(20, 30), priv->parent->enType);
            usb_gdisk(priv->parent->name, priv->parent);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(40, 50), priv->parent->enType);
            count = usb_scan_get_part1(priv->parent, tname);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(60, 70), priv->parent->enType);
            // format ntfs ### -F - force format ### -Q - perform a quick format
            if (count && strlen(tname)) {
                sprintf(cmd, "mkfs.exfat %s", tname);
            } else {
                sprintf(cmd, "mkfs.exfat %s", priv->parent->name);
            }

            MFinfo("usb format cmd :%s", cmd);
            res = ms_system(cmd);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(70, 75), priv->parent->enType);
            if (res == MF_SUCCESS) {
                res = usb_mount(pstPrivate);
            }
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(75, 80), priv->parent->enType);
        }
        break;
        case FORMAT_NTFS_TYPE: {
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(20, 30), priv->parent->enType);
            usb_gdisk(priv->parent->name, priv->parent);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(40, 50), priv->parent->enType);
            count = usb_scan_get_part1(priv->parent, tname);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(60, 70), priv->parent->enType);
            // format ntfs ### -F - force format ### -Q - perform a quick format
            if (count && strlen(tname)) {
                sprintf(cmd, "mkntfs -FQq %s", tname);
            } else {
                sprintf(cmd, "mkntfs -FQq %s", priv->parent->name);
            }

            MFinfo("usb format cmd :%s", cmd);
            res = ms_system(cmd);
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(70, 75), priv->parent->enType);
            if (res == MF_SUCCESS) {
                res = usb_mount(pstPrivate);
            }
            mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(75, 80), priv->parent->enType);
        }
        break;
        default :
            res = MF_FAILURE;
            break;
    }

    if (res == MF_SUCCESS && (usb_is_mount(priv->parent->mnt) == MF_TRUE)) {
        res = MF_SUCCESS;
    } else {
        res = MF_FAILURE;
    }

    return res;
}

static MF_S32
usb_size(void *pstPrivate)
{
    struct usb_priv *priv = (struct usb_priv *)pstPrivate;
    struct statfs sfs;
    if (statfs(priv->parent->mnt, &sfs)) {
        perror("statfs");
        return MF_FAILURE;
    }
    priv->parent->sector = 512;
    priv->parent->capacity = (MF_U64)sfs.f_blocks * sfs.f_bsize;
    priv->parent->free = (MF_U64)sfs.f_bavail * sfs.f_bsize;

    return MF_SUCCESS;
}

static MF_S32
usb_vendor(void *pstPrivate)
{
    struct usb_priv *priv = (struct usb_priv *)pstPrivate;
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
usb_status(void *pstPrivate)
{
    struct usb_priv *priv = (struct usb_priv *)pstPrivate;
    MF_S32 res;

    res = usb_is_mount(priv->parent->mnt);
    if (res == MF_FALSE) { //no mount
        priv->parent->enState = DISK_STATE_OFFLINE;
    } else {
        priv->parent->enState = DISK_STATE_NORMAL;
    }

    return MF_SUCCESS;
}

static struct disk_ops usb_ops = {
    .disk_format    = usb_format,
    .disk_mount     = usb_mount,
    .disk_umount    = usb_umount,
    .disk_size      = usb_size,
    .disk_vendor    = usb_vendor,
    .disk_status    = usb_status,
};

MF_S32
disk_usb_register(struct diskObj *pstDiskObj, MF_S8 *host)
{
    struct usb_priv *pstPrivate;
    pstDiskObj->ops = &usb_ops;

    pstPrivate = ms_calloc(1, sizeof(struct usb_priv));
    pstPrivate->parent = pstDiskObj;
    pstPrivate->parent->bRec = MF_FALSE;// can`t record
    pstDiskObj->pstPrivate = pstPrivate;

    return MF_SUCCESS;
}

MF_S32
disk_usb_unregister(struct diskObj *pstDiskObj)
{
    ms_free(pstDiskObj->pstPrivate);
    return MF_SUCCESS;
}

