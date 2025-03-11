/*
 * ***************************************************************
 * Filename:        disk_nas.c
 * Created at:      2017.05.10
 * Description:     nas mangement API
 * Author:          zbing
 * Copyright (C)    milesight
 * Modify:            Solin 2019.04.04
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
#include <ctype.h>

#include "msfs_disk.h"
#include "MFdisk.h"
#include "MFlog.h"

#define MSDISK0     "MDISK0.bin"
#define MSDISK1     "MDISK1.bin"
#define MSINDEX00   "MINDEX00.bin"
#define MSINDEX01   "MINDEX01.bin"
#define MSINDEX10   "MINDEX10.bin"
#define MSINDEX11   "MINDEX11.bin"
#define MSLOG       "MLOG.bin"

#define NAS_RM_ERR_MAX  (10) // only for remove dir/file count errors. solin
#define CMD_MOUNT_NFS   "mount -t nfs -o nolock,noac,soft,proto=udp,timeo=7,retrans=3 %s %s"
#define CMD_MOUNT_CIFS  "mount -t cifs -o noac,soft,iocharset=utf8,username=%s,password=%s %s %s"

#define NAS_MIN_SIZE    (20*1024*1024*1024LLU)
#define NAS_REMAINDER_SIZE  (150*1024*1024)
typedef struct nas_priv {
    struct diskObj *parent;
    RWLOCK_BOJECT rwlock;

    MF_BOOL bsubdir;
    MF_S8 subdir[DEV_HOST_LEN];

    MF_BOOL bprivver;
    MF_S8 privdir[DEV_HOST_LEN];
    MF_S8 dir[DEV_HOST_LEN];
    MF_S32 type;
    MF_S8 user[64];
    MF_S8 password[64];
} nas_priv;

static MF_S32
nas_is_mount(MF_S8 *mnt)
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

static MF_S8 *
nas_replace_slash(MF_S8 *model)
{
    MF_S8 *tmodel = model;
    while (*tmodel) {
        if ((*tmodel) != '-' && !isalnum(*tmodel)) {
            *tmodel = '-';
        }
        tmodel++;
    };
    return model;
}

static MF_S32
nas_rm_dir(MF_S8 *dir_full_path, MF_S32 *err, MF_BOOL bself)
{
    MF_S8 sub_path[1024];
    struct dirent *dir;
    struct stat st;
    DIR *dirp = opendir(dir_full_path);

    regex_t reg, hreg;
    char match[32] = "^m[0-9]{7}[.]{1}mp4$";
    char hmatch[64] = "^M(DISK0|DISK1|INDEX00|INDEX01|INDEX10|INDEX11|LOG)[.]{1}bin$";

    if (!dirp || !err) {
        return MF_FAILURE;
    }

    regcomp(&reg, match, REG_EXTENDED | REG_NOSUB);
    regcomp(&hreg, hmatch, REG_EXTENDED | REG_NOSUB);

    while ((dir = readdir(dirp)) != NULL) {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
            continue;
        }

        if (regexec(&reg, dir->d_name, sizeof(dir->d_name), NULL, 0) != 0 &&
            regexec(&hreg, dir->d_name, sizeof(dir->d_name), NULL, 0) != 0) {
            continue;
        }

        memset(sub_path, 0, sizeof(sub_path));
        sprintf(sub_path, "%s/%s", dir_full_path, dir->d_name);
        if (lstat(sub_path, &st) == -1) {
            MFwarm("%s no exists or nfs offline!", sub_path);
            (*err)++;
            if ((*err) > NAS_RM_ERR_MAX) {
                closedir(dirp);
                regfree(&reg);
                regfree(&hreg);
                return MF_FAILURE;
            } else {
                continue;
            }
        }
        /* save remove directory */
        /*if (S_ISDIR(st.st_mode))
        {
            if (nas_rm_dir(sub_path, err, bself) == MF_FAILURE) // remove dir file recyle
            {
                closedir(dirp);
                regfree(&reg);
                regfree(&hreg);
                return MF_FAILURE;
            }
            rmdir(sub_path);
        }
        else */
        if (S_ISREG(st.st_mode)) {
            unlink(sub_path);    //  remove common file by unlink
        } else {
            continue;
        }
    }
    if (bself) { //delete dir itself.
        if (rmdir(dir_full_path) == -1) {
            closedir(dirp);
            regfree(&reg);
            regfree(&hreg);
            return MF_FAILURE;
        }
    }
    closedir(dirp);
    regfree(&reg);
    regfree(&hreg);
    return 0;
}

MF_S32
nas_rm_all(MF_S8 *file_path, MF_BOOL bself)
{
    struct stat st;
    MF_S32 err = 0;

    if (!file_path) {
        return MF_FAILURE;
    }
    if (lstat(file_path, &st) == -1) {
        return MF_FAILURE;
    }
    if (S_ISREG(st.st_mode)) {
        if (unlink(file_path) == -1) {
            return MF_FAILURE;
        }
    } else if (S_ISDIR(st.st_mode)) {
        if (strcmp(file_path, ".") == 0 || strcmp(file_path, "..") == 0) {
            return MF_FAILURE;
        }
        if (nas_rm_dir(file_path, &err, bself) == -1) { //delete all the files in dir.
            return MF_FAILURE;
        }
    }
    return MF_SUCCESS;
}

static inline MF_S8 *
nas_get_mnt(struct nas_priv *priv)
{
    MF_S8 *mnt = NULL;
    ms_rwlock_rdlock(&priv->rwlock);
    if (priv->bprivver) {
        mnt = priv->dir;
    } else {
        if (priv->bsubdir) {
            mnt = priv->subdir;
        } else {
            mnt = priv->parent->mnt;
        }
    }
    ms_rwlock_unlock(&priv->rwlock);
    return mnt;
}

static MF_S32
nas_set_head_info(struct diskObj *pstDev, MF_S8 *fheader)
{
    MF_S8 fileName[128];
    MF_S32 fd;
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 res = MF_FAILURE;

    snprintf(fileName, sizeof(fileName), "%s/%s", nas_get_mnt(pstDev->pstPrivate), fheader);
    fd = disk_open(fileName, MF_NO);
    if (fd > 0) {
        res = disk_write(fd, pstDev->pstHeader, size, pstDev->sector);
        if (res < 0) {
            MFerr("disk_write failed");
        }
        disk_close(fd);
    }

    return res < 0 ? MF_FAILURE : MF_SUCCESS;
}

static MF_S32
nas_get_head_info(struct diskObj *pstDev, MF_S8 *fheader)
{
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 fd = -1;
    MF_S8 fileName[128];

    snprintf(fileName, sizeof(fileName), "%s/%s", nas_get_mnt(pstDev->pstPrivate), fheader);
    if (access(fileName, F_OK)) {
        return MF_FAILURE;
    }

    fd = disk_open(fileName, MF_NO);
    if (fd <= 0) {
        return MF_FAILURE;
    }

    pstDev->sector = 0;
    if (disk_read(fd, pstDev->pstHeader, size, pstDev->sector) < 0) {
        disk_close(fd);
        MFerr("disk_read failed");
        return MF_FAILURE;
    }

    disk_close(fd);
    return MF_SUCCESS;
}

static MF_S32
nas_header_check(struct diskObj *pstDev)
{
    MF_S32 res = MF_FAILURE;
    if (!pstDev) {
        return res;
    }
    if (nas_get_head_info(pstDev, MSDISK0) == MF_SUCCESS) {
        res = MF_SUCCESS;
        if ((strcmp(pstDev->pstHeader->magic, DISK_MAGIC) == 0) &&
            (strcmp(pstDev->pstHeader->version, DISK_VERSION) == 0) &&
            (pstDev->pstHeader->checkSum != 0) &&
            pstDev->pstHeader->checkSum == \
            check_sum(pstDev->pstHeader, MF_POS(pstDev->pstHeader, checkSum))) {
            pstDev->enState = DISK_STATE_NORMAL;
            return MF_SUCCESS;
        } else {
            pstDev->enState = DISK_STATE_UNFORMATTED;
        }
    }
    if (nas_get_head_info(pstDev, MSDISK1) == MF_SUCCESS) {
        res = MF_SUCCESS;
        if ((strcmp(pstDev->pstHeader->magic, DISK_MAGIC) == 0) &&
            (strcmp(pstDev->pstHeader->version, DISK_VERSION) == 0) &&
            (pstDev->pstHeader->checkSum != 0) &&
            pstDev->pstHeader->checkSum == \
            check_sum(pstDev->pstHeader, MF_POS(pstDev->pstHeader, checkSum))) {
            MFinfo("set head 0 !!!");
            pstDev->enState = DISK_STATE_NORMAL;
            /* save msdisk1 header to msdisk0 */
            nas_set_head_info(pstDev, MSDISK0);
        } else {
            pstDev->enState = DISK_STATE_UNFORMATTED;
            res = MF_UNKNOWN;
        }
    }
    return res;
}

static MF_S32
nas_mount_access(MF_S8 *mnt)
{
    if (access(mnt, F_OK | R_OK | W_OK)) {
        if (mkdir(mnt, 0775) != 0) {
            MFwarm("mkdir [%s] failed. \n", mnt);
            return MF_FAILURE;
        }
        if (access(mnt, F_OK | R_OK | W_OK)) {
            MFwarm("access [%s] failed. \n", mnt);
            return MF_FAILURE;
        }
    }
    return MF_SUCCESS;
}

static MF_S32
nas_umount_access(MF_S8 *mnt)
{
    if (umount(mnt) < 0) {
        MFinfo("umount path :%s", mnt);
        return MF_FAILURE;
    }

    if (access(mnt, F_OK) == 0) { // exist
        MFinfo("has mount point %s, will delete it.", mnt);
        if (rmdir(mnt) != 0) {
            MFerr("rmdir [%s] failed. \n", mnt);
            return MF_FAILURE ;
        }
        if (access(mnt, F_OK) == 0) { //still exist,  rmdir mnt failed.
            MFerr("still exist mnt [%s], failed. \n", mnt);
            return MF_FAILURE;
        }
    }
    return MF_SUCCESS;
}

static MF_S32
nas_mount_predir(struct nas_priv *pstPrivate)
{
    MF_S8 *end   = NULL;
    MF_S8 *colon = NULL;
    MF_S8 mnt[DEV_HOST_LEN] = {0};
    MF_S8 host[DEV_HOST_LEN] = {0};
    MF_S8 cmd[DEV_HOST_LEN * 2] = {0};

    struct nas_priv *priv = pstPrivate;

    colon = strchr(priv->parent->host, ':');
    if (!colon) {
        return MF_FAILURE;
    }

    colon++;
    end = strrchr(colon + 1, '/');
    if (!end) {
        return MF_FAILURE;
    }

    snprintf(host, (end - priv->parent->host + 1), "%s", priv->parent->host);
    snprintf(cmd, sizeof(cmd), "mount -t nfs -o nolock,noac,soft,proto=tcp,timeo=5,retrans=2	%s %s 2>/dev/null", host,
             priv->parent->mnt);
    ms_system(cmd);
    MFinfo("%s", cmd);
    end++;

    if (nas_is_mount(priv->parent->mnt) == MF_FALSE) {
        return MF_FAILURE;
    }

    snprintf(mnt, sizeof(mnt), "%s/%s", priv->parent->mnt, end);
    MFinfo("new mnt - %s", mnt);
    if (nas_mount_access(mnt)) {
        if (umount(priv->parent->mnt) < 0) {
            MFinfo("umount path fail :%s", priv->parent->mnt);
        }
        return MF_FAILURE;
    }
    //sprintf(priv->parent->mnt, "%s", mnt); // do not update parent->mnt
    ms_rwlock_wrlock(&priv->rwlock);
    priv->bsubdir = MF_TRUE;
    snprintf(priv->subdir, sizeof(priv->subdir), "%s", mnt);
    ms_rwlock_unlock(&priv->rwlock);
    MFinfo("last %s", priv->subdir);

    return MF_SUCCESS;
}

static void
nas_set_privdir(struct nas_priv *priv, MF_S8 *mnt)
{
    ms_rwlock_wrlock(&priv->rwlock);
    priv->bprivver = MF_TRUE;
    snprintf(priv->dir, sizeof(priv->dir), "%s", mnt);
    ms_rwlock_unlock(&priv->rwlock);
}

static MF_S32
nas_check_privdir(struct nas_priv *priv)
{
    MF_S8 mnt[DEV_HOST_LEN] = {0};

    snprintf(mnt, sizeof(mnt), "%s/%s", nas_get_mnt(priv), priv->privdir);
    if (!access(mnt, F_OK)) {
        nas_set_privdir(priv, mnt);
    }
    return MF_SUCCESS;
}

static MF_S32
nas_create_privdir(struct nas_priv *priv)
{
    if (priv->bprivver) {
        return MF_SUCCESS;
    }

    MF_S8 mnt[DEV_HOST_LEN] = {0};
    snprintf(mnt, sizeof(mnt), "%s/%s", nas_get_mnt(priv), priv->privdir);
    if (access(mnt, F_OK) && mkdir(mnt, 0775) != 0) {
        MFwarm("mkdir [%s] failed. \n", mnt);
        return MF_FAILURE;
    }

    nas_set_privdir(priv, mnt);
    return MF_SUCCESS;
}

static MF_S32
nas_size(void *pstPrivate)
{
    struct nas_priv *priv = (struct nas_priv *)pstPrivate;
    struct statfs sfs;

    if (statfs(nas_get_mnt(priv), &sfs)) {
        perror("statfs");
        priv->parent->capacity = 0;
        priv->parent->free = 0;
        return MF_FAILURE;
    }
    priv->parent->free     = (MF_U64)sfs.f_bavail * sfs.f_bsize;
    priv->parent->capacity = priv->parent->free;
#ifdef DEBUG_REC
    priv->parent->capacity = (MF_U64)1 * 1024 * 1024 * 1024;
    priv->parent->free = 0;
#endif
    if (priv->parent->enState == DISK_STATE_NORMAL) {
        MF_U64 surplus = 0;
//        priv->parent->free = mf_retr_disk_free_size(priv->parent);
        priv->parent->capacity = priv->parent->pstHeader->recordLen;
        if (priv->parent->free > priv->parent->pstHeader->extendLen) {
            priv->parent->free -= priv->parent->pstHeader->extendLen;
        } else {
            priv->parent->free = 0;
        }

        surplus = priv->parent->capacity - priv->parent->fileNum * priv->parent->pstHeader->perFileSize;
        if (!priv->type) {
            priv->parent->free = MF_MIN(surplus, priv->parent->free);
        } else {
            priv->parent->free = surplus;
        }
    }

    return MF_SUCCESS;
}

static MF_S32
nas_status(void *pstPrivate)
{
    struct nas_priv *priv = (struct nas_priv *)pstPrivate;
    MF_S32 res = MF_FAILURE;

    if (priv->type && !nas_is_mount(priv->parent->mnt)) {
        priv->parent->enState = DISK_STATE_OFFLINE;
        return MF_SUCCESS;
    }

    if ((res = nas_header_check(priv->parent)) == MF_FAILURE || res == MF_UNKNOWN) {
        priv->parent->enState = DISK_STATE_UNFORMATTED;
        goto EXIT;
    }
    if (!priv->parent->tree && priv->parent->enState == DISK_STATE_NORMAL) {
        MF_S8 fileName[128];
        if (priv->parent->pstHeader->index0Status) {
            sprintf(fileName, "%s/%s", nas_get_mnt(priv), MSINDEX01);
        } else {
            sprintf(fileName, "%s/%s", nas_get_mnt(priv), MSINDEX00);
        }
        priv->parent->tree = mf_index_fetch(fileName,
                                            priv->parent->pstHeader->index00Off,
                                            priv->parent);
    }
    if (!mf_index_is_valid(priv->parent->tree)) {
        priv->parent->enState = DISK_STATE_UNFORMATTED;
        // do not send error, avoid nas mounting fail alway .solin
        //mf_disk_event_notify(MSFS_EVENT_DISK_ERR, priv->parent, MF_NO);
    }
EXIT:
    return MF_SUCCESS;
}
static MF_S32
nas_mount_nfs(nas_priv *priv)
{
    MF_S32 ret = MF_FAILURE;
    MF_S8 cmd[DEV_HOST_LEN * 2] = {0};
    
    snprintf(cmd, sizeof(cmd), CMD_MOUNT_NFS, priv->parent->host, priv->parent->mnt);
    MFinfo("nas mount nas. cmd:%s", cmd);
    ms_system(cmd);
    if (nas_is_mount(priv->parent->mnt) == MF_FALSE) {
        nas_mount_predir(priv);
    }
    ret = nas_is_mount(priv->parent->mnt);
    if (ret == MF_TRUE) {
        nas_check_privdir(priv);
    }

    return ret;
}

static MF_S32
nas_mount_cifs(nas_priv *priv)
{
    MF_S32 ret = MF_FAILURE;
    MF_S8 cmd[DEV_HOST_LEN * 2] = {0};
    
    snprintf(cmd, sizeof(cmd), CMD_MOUNT_CIFS, priv->user, priv->password, priv->parent->host, priv->parent->mnt);
    MFinfo("nas mount cifs. cmd:%s", cmd);
    ms_system(cmd);
    if (nas_is_mount(priv->parent->mnt) == MF_FALSE) {
        nas_mount_predir(priv);
    }
    ret = nas_is_mount(priv->parent->mnt);
    if (ret == MF_TRUE) {
        nas_check_privdir(priv);
    }

    return ret;
}


static MF_S32
nas_mount(void *pstPrivate)
{
    MF_S32 ret = MF_FAILURE;
    struct nas_priv *priv = (struct nas_priv *)pstPrivate;

    if (!priv) {
        MFwarm("nas mount priv is NULL.");
        return MF_FAILURE;
    }
    if (!strlen(priv->parent->mnt)) {
        MF_U8 out [MD5_LEN] = {0};
        MD5string(priv->parent->host, out);
        snprintf(priv->parent->mnt, sizeof(priv->parent->mnt), "/mnt/nas_%s", out);
    }

    if (nas_mount_access(priv->parent->mnt)) {
        MFwarm("nas mount %s access failed.", priv->parent->mnt);
        return MF_FAILURE;
    }
    if (nas_is_mount(priv->parent->mnt) == MF_TRUE) {
        nas_check_privdir(priv);
        MFinfo("nas is already mount.");
        return MF_SUCCESS;
    }

    MFinfo("nas mount start type:%s mnt:%s name:%s host:%s user:%s password:%s", priv->type?"cifs":"nfs",
        priv->parent->mnt, priv->parent->name, priv->parent->host,  priv->user, priv->password);
    if (!priv->type) {
        ret = nas_mount_nfs(priv);
    } else {
        ret = nas_mount_cifs(priv);
    }

    MFinfo("nas mount end ret=%d .", ret);
    return ret;
}

static MF_S32
nas_umount(void *pstPrivate)
{
    struct nas_priv *priv = (struct nas_priv *)pstPrivate;
    MF_S32 res;

    if (!priv) {
        return MF_FAILURE;
    }
    if (priv->parent->tree) {
        mf_index_release(priv->parent->tree);
        priv->parent->tree = NULL;
    }

    res = nas_umount_access(priv->parent->mnt);
    return res;
}

static MF_S32
nas_vendor(void *pstPrivate)
{
    struct nas_priv *priv = (struct nas_priv *)pstPrivate;

    strcpy(priv->parent->vendor, priv->parent->alias);
    return MF_SUCCESS;
}

static MF_S32
nas_file_open(void *pstPrivate, int fileNo, MF_BOOL bO_DIRECT)
{
    struct nas_priv *priv = (struct nas_priv *)pstPrivate;
    MF_S8 fileName[128];

    sprintf(fileName, "%s/m%07d.mp4", nas_get_mnt(priv), fileNo);
    return disk_open(fileName, MF_NO);
}

static MF_S32
nas_file_close(void *pstPrivate, int fd)
{
    disk_close(fd);
    return MF_SUCCESS;
}

static MF_S32
nas_file_write(void *pstPrivate,
               MF_S32 fd,
               const void *buf,
               MF_S32 count,
               off64_t offset)
{
    MF_S32 res;
    MF_U8 TryAagin = 5;
//    struct nas_priv *priv = (struct nas_priv *)pstPrivate;

    do {
//        if (priv->parent->enState != DISK_STATE_NORMAL) {
//            MFerr("disk state != DISK_STATE_NORMAL");
//            return MF_FAILURE;
//        }
        res = disk_write(fd, buf, count, offset);
        TryAagin--;
        if (res < 0) {
            if ((res == -EAGAIN || res == -EIO) &&
                TryAagin) {
                usleep(100000);
                MFshow("try again[%d]", TryAagin);
                continue;
            } else {
                MFerr("disk_write failed[%d]", res);
                break;
            }
        }
        break;
    } while (TryAagin);

    return res;
}

static MF_S32
nas_file_read(void *pstPrivate,
              MF_S32 fd,
              void *buf,
              MF_S32 count,
              off64_t offset)
{
    MF_S32 res;
    MF_U8 TryAagin = 3;
//    struct nas_priv *priv = (struct nas_priv *)pstPrivate;

    do {
//        if (priv->parent->enState != DISK_STATE_NORMAL) {
//            MFerr("disk state != DISK_STATE_NORMAL");
//            return MF_FAILURE;
//        }
        res = disk_read(fd, buf, count, offset);
        TryAagin--;
        if (res < 0) {
            if ((res == -EAGAIN || res == -EIO) && 
                TryAagin) {
                usleep(100000);
                MFshow("try again[%d]", TryAagin);
                continue;
            } else {
                MFerr("disk_read failed[%d]", res);
                break;
            }
        }
        break;
    } while (TryAagin);

    return res;
}

static MF_S32
nas_format(void *pstPrivate, void *argv, MF_U64 quota)
{
    /*
    * disk data file
    * 's' indicate a sector (eg 512)
    *
    *|HEADER0|-|LOG|-|HEADER1|-|INDEX0|INDEX1|-|N*FILE|

    */
    /*
    * file data structure
    *                 data ---->                       <----info
    *  start   |-----AVdata---|--infoN~info0|--infoN~info0|segment0~segment1023--|Tag-resverve(5MB)--|fileHeader(512B)--|  end
    */
    struct bplus_key stKey;
    struct bplus_data stData;
    struct nas_priv *priv = (struct nas_priv *)pstPrivate;
    struct diskObj *pstDev = priv->parent;
    FORMAT_EN ftype = *(FORMAT_EN *)argv;
    uuid_t uu;
    MF_S32 i;
    MF_S32 res;
    MF_S32 fd = -1;
    MF_S8 fileName[128] = {0};

    // percent time
    int start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    if (ftype != FORMAT_TYPE_MS) {
        return MF_FAILURE;
    }

    if (pstDev->tree) {
        mf_index_release(pstDev->tree);
        pstDev->tree = NULL;
    }

    MFinfo("rm all %s", nas_get_mnt(pstPrivate));
    res = nas_rm_all(nas_get_mnt(pstPrivate), MF_FALSE);// remove all files
    if (res != MF_SUCCESS || pstDev->bFmtExit == MF_YES) {
        return MF_FAILURE;
    }

    if (nas_create_privdir(pstPrivate)) {
        return MF_FAILURE;
    }

    res = nas_size(pstPrivate);// re-update size
    if (res != MF_SUCCESS || pstDev->bFmtExit == MF_YES) {
        return MF_FAILURE;
    }

    if (pstDev->free < NAS_MIN_SIZE) {
        MFerr("nas capacity too small\n");
        return MF_NAS_LOW_CAP;
    }

    /*disk header info init */
    strcpy(pstDev->pstHeader->magic, DISK_MAGIC);
    strcpy(pstDev->pstHeader->version, DISK_VERSION);
    pstDev->pstHeader->size           = MF_POS(pstDev->pstHeader, checkSum);
    pstDev->pstHeader->attr           = 0;
    pstDev->pstHeader->logStatus      = 0;
    pstDev->pstHeader->capacity       = pstDev->free;//pstDev->capacity; // use real available capacity
    pstDev->pstHeader->mainLogOff     = 0;
    pstDev->pstHeader->mainLogLen     = HEADER_MAINLOGLEN;
    pstDev->pstHeader->curLogOff      = 0;
    pstDev->pstHeader->curLogLen      = HEADER_CURLOGLEN;
    pstDev->pstHeader->index00Off     = 0;
    pstDev->pstHeader->index01Off     = 0;
    pstDev->pstHeader->index10Off     = 0;
    pstDev->pstHeader->index11Off     = 0;
    pstDev->pstHeader->index0Status   = 0;
    pstDev->pstHeader->index1Status   = 0;
    pstDev->pstHeader->perFileSize    = FILE_PER_SIZE;
    pstDev->pstHeader->recordOff      = 0;
    pstDev->pstHeader->fileCount      = (pstDev->pstHeader->capacity - NAS_REMAINDER_SIZE) / pstDev->pstHeader->perFileSize -
                                        1;
    pstDev->pstHeader->recordLen      = pstDev->pstHeader->fileCount * pstDev->pstHeader->perFileSize;
    pstDev->pstHeader->extendOff      = 0;
    pstDev->pstHeader->extendLen      = pstDev->pstHeader->capacity - NAS_REMAINDER_SIZE - pstDev->pstHeader->recordLen;
    pstDev->pstHeader->restoreTimes   = time(0);
    pstDev->pstHeader->restoreCounts  = 0;

    /* create index by B+tree */
    sprintf(fileName, "%s/%s", nas_get_mnt(pstPrivate), MSINDEX00);
    pstDev->tree = mf_index_rebuild(pstDev->tree, fileName,
                                    pstDev->pstHeader->index00Off,
                                    pstDev,
                                    MF_YES);
    if (pstDev->tree == NULL || pstDev->bFmtExit == MF_YES) {
        return MF_FAILURE;
    }
    pstDev->tree->owner = pstDev;

    MFinfo("format step 1");
    mf_index_key_init(&stKey, TYPE_RECORD);
    MFinfo(" NAS filecount %d\n", pstDev->pstHeader->fileCount);
    for (i = 0; i < pstDev->pstHeader->fileCount; i++) {
        if (pstDev->bFmtExit == MF_YES) {
            return MF_FAILURE;
        }

//        stKey.stRecord.chnId = i % 64;
        stKey.stRecord.fileNo = i;
        stKey.stRecord.fileOffset = 0;
//        stKey.stRecord.startTime = time(0);
        stData.stRecord.fileOffset = stKey.stRecord.fileOffset;
        res = mf_index_key_insert(pstDev->tree, &stKey, &stData);
        if (res != MF_SUCCESS || pstDev->bFmtExit == MF_YES) {
            MFerr("insert key failed! res[%d]", res);
            return res;
        }
        pre = gap * ((i + 1) / (pstDev->pstHeader->fileCount * 1.0));
        if (abs(pre - percent) >= 1) {
            percent = pre;
            mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, pstDev->port, start + percent, pstDev->enType);
        }
        fd = nas_file_open(pstPrivate, stKey.stRecord.fileNo, MF_NO);
        if (fd > 0) {
            if (!priv->type) {
                ftruncate(fd, FILE_PER_SIZE);
            }
            nas_file_close(pstPrivate, fd);
            fd = -1;
        }
//        mf_index_data_init(pstDev->tree, &stKey);
        // TODO:
    }
    pstDev->pstHeader->index00Len = mf_index_size(pstDev->tree);
    pstDev->free = pstDev->pstHeader->recordLen;

    mf_index_key_backup(pstDev->tree);

    /*create a uuid  string */
    MFinfo("format step 2");
    uuid_generate(uu);
    uuid_unparse(uu, pstDev->pstHeader->uuid);

    /* check header sum */
    MFinfo("format step 3");
    pstDev->pstHeader->checkSum = check_sum(pstDev->pstHeader, pstDev->pstHeader->size);

    if (pstDev->bFmtExit == MF_YES) {
        return MF_FAILURE;
    }

    res = nas_set_head_info(pstDev, MSDISK0);
    if (res != MF_SUCCESS || pstDev->bFmtExit == MF_YES) {
        return MF_FAILURE;
    }
    res = nas_set_head_info(pstDev, MSDISK1);
    if (res != MF_SUCCESS || pstDev->bFmtExit == MF_YES) {
        return MF_FAILURE;
    }

    MFinfo("format step 4");
    pstDev->ops->disk_log_fomat(pstDev);
    MFinfo("format step done");
    return MF_SUCCESS;
}

MF_S32
nas_log_init(void *pdisk)
{
    MF_S8 logName[128];
    MF_S32 fd, exist = MF_NO;
    struct diskObj *disk = (struct diskObj *)pdisk;

    sprintf(logName, "%s/%s", nas_get_mnt(disk->pstPrivate), MSLOG);
    if (access(logName, F_OK) == 0) {
        exist = MF_YES;
    }

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

MF_S32
nas_log_fomat(void *nas_disk)
{
    int fd = -1;
    int ret = 0;
    MF_S8 logName[128];
    struct diskObj *disk = (struct diskObj *)nas_disk;

    sprintf(logName, "%s/%s", nas_get_mnt(disk->pstPrivate), MSLOG);
    fd = disk_open(logName, MF_NO);
    if (fd < 0) {
        ret = MF_FAILURE;
    } else {
        ret = mf_log_disk_format(fd, disk);
        disk_close(fd);
    }
    return ret;
}

static MF_S32
nas_index_backup(void *index)
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
        MF_S32 backupFd = -1;
        MF_S8 fileName[128];

        if (pstDev->pstHeader->index1Status) {
            sprintf(fileName, "%s/%s", nas_get_mnt(pstDev->pstPrivate), MSINDEX11);
        } else {
            sprintf(fileName, "%s/%s", nas_get_mnt(pstDev->pstPrivate), MSINDEX10);
        }

        mf_index_set_crc(tree, crc);
        backupFd = disk_open(fileName, MF_NO);
        if (backupFd > 0) {
            boot->stInfo.crc = crc;
            res = disk_write(backupFd, out, size, backupOff);
            if (res < 0) {
                MFerr("disk_write failed");
            }
            disk_close(backupFd);
        }
    }

    ms_free(out);
    return res;
}

static MF_S32
nas_index_recover(void *index)
{
    struct bplus_tree *tree = index;
    struct diskObj *pstDev = tree->owner;
    struct bplus_boot *boot;
    MF_S32 backupFd = -1;
    MF_U64 backupOff = pstDev->pstHeader->index10Off;
    MF_U64 recoverOff = pstDev->pstHeader->index00Off;
    MF_S32 res = MF_SUCCESS;
    MF_S8 fileName[128];

    if (pstDev->pstHeader->index1Status) {
        sprintf(fileName, "%s/%s", nas_get_mnt(pstDev->pstPrivate), MSINDEX11);
    } else {
        sprintf(fileName, "%s/%s", nas_get_mnt(pstDev->pstPrivate), MSINDEX10);
    }
    backupFd = disk_open(fileName, MF_NO);
    if (backupFd < 0) {
        MFerr("open file :%s failed", fileName);
        return MF_FAILURE;
    }

    //reopen MSINDEX0.bin
    if (pstDev->pstHeader->index0Status) {
        sprintf(fileName, "%s/%s", nas_get_mnt(pstDev->pstPrivate), MSINDEX01);
    } else {
        sprintf(fileName, "%s/%s", nas_get_mnt(pstDev->pstPrivate), MSINDEX00);
    }

    if (tree->fd) {
        disk_close(tree->fd);
    }
    tree->fd = disk_open(fileName, MF_NO);
    if (tree->fd < 0) {
        disk_close(backupFd);
        MFerr("open file :%s failed", fileName);
        return MF_FAILURE;
    }

    boot = ms_malloc(MAX_BLOCK_SIZE);
    res = disk_read(backupFd, boot, MAX_BLOCK_SIZE, backupOff);
    if (res < 0) {
        ms_free(boot);
        MFerr("disk_read failed");
        return MF_FAILURE;
    }

    if (boot->stInfo.magic == BPTREE_MAGIC
        &&  boot->stInfo.enState == STATE_DONE) {
        MF_U32 fileSize = (MF_U32)(boot->stInfo.file_size - recoverOff);
        MF_S8 *out = ms_malloc(fileSize);
        res = disk_read(backupFd, out, fileSize, backupOff);
        if (res < 0) {
            ms_free(boot);
            ms_free(out);
            disk_close(backupFd);
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
            disk_close(backupFd);
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
    disk_close(backupFd);

    return res;
}

static MF_S32
nas_index_restore(void *pstPrivate)
{
    struct diskObj *pstDev = ((struct nas_priv *)pstPrivate)->parent;
    struct bplus_tree *tree;
    struct bplus_key stKey;
    struct bplus_data stData;
    struct mf_file stFile;
    MF_S8 fileName[128];
    MF_S32 i;
    MF_S32 res;
    MF_S32 start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    if (strcmp(pstDev->pstHeader->magic, DISK_MAGIC) != 0) {
        MFerr("disk magic is not[%s]\n", DISK_MAGIC);
        return MF_FAILURE;
    }

    /* rebulid index tree */
    MFinfo("step 1:  rebulid index tree\n");

    if (pstDev->pstHeader->index0Status) {
        sprintf(fileName, "%s/%s", nas_get_mnt(pstPrivate), MSINDEX00);
    } else {
        sprintf(fileName, "%s/%s", nas_get_mnt(pstPrivate), MSINDEX01);
    }

    tree = mf_index_rebuild(NULL, fileName,
                            pstDev->pstHeader->index01Off,
                            pstDev,
                            MF_YES);
    if (tree == NULL) {
        return MF_FAILURE;
    }

    for (i = 0; i < pstDev->pstHeader->fileCount; i++) {
        mf_index_key_init(&stKey, TYPE_RECORD);
        stKey.stRecord.fileNo       = i;
        stKey.stRecord.fileOffset   = 0;
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

    nas_set_head_info(pstDev, MSDISK0);
    nas_set_head_info(pstDev, MSDISK1);
    /* backup index tree */
    MFinfo("step 4: backup index tree\n");
    mf_index_key_backup(pstDev->tree);

    return MF_SUCCESS;
}



static struct disk_ops nas_ops = {
    .disk_format        = nas_format,
    .disk_mount         = nas_mount,
    .disk_umount        = nas_umount,
    .disk_size          = nas_size,
    .disk_vendor        = nas_vendor,
    .disk_status        = nas_status,
    .disk_file_open     = nas_file_open,
    .disk_file_close    = nas_file_close,
    .disk_file_read     = nas_file_read,
    .disk_file_write    = nas_file_write,
    .disk_log_fomat     = nas_log_fomat,
    .disk_log_init      = nas_log_init,
    .disk_index_backup  = nas_index_backup,
    .disk_index_recover = nas_index_recover,
    .disk_index_restore = nas_index_restore,
};

MF_S32
disk_nas_register(struct diskObj *pstDiskObj, MF_S8 *host, void *params)
{
    struct nas_priv *pstPrivate;
    pstDiskObj->ops = &nas_ops;
    NasInfoStr *tmp = (NasInfoStr *)params;

    pstPrivate = ms_calloc(1, sizeof(struct nas_priv));
    pstPrivate->parent   = pstDiskObj;
    pstPrivate->bprivver = MF_FALSE;
    strcpy(pstPrivate->parent->host, host);
    snprintf(pstPrivate->privdir, sizeof(pstPrivate->privdir), "%s_%s",
             nas_replace_slash(mf_dev_model()), mf_dev_mmac());
    if (tmp) {
        pstPrivate->type = tmp->mounttype;
        snprintf(pstPrivate->user, sizeof(pstPrivate->user), "%s", tmp->user);
        snprintf(pstPrivate->password, sizeof(pstPrivate->password), "%s", tmp->password);
    }
    pstDiskObj->pstPrivate = pstPrivate;
    ms_rwlock_init(&pstPrivate->rwlock);

    return MF_SUCCESS;
}

MF_S32
disk_nas_unregister(struct diskObj *pstDiskObj)
{
    struct nas_priv *pstPrivate = (struct nas_priv *)pstDiskObj->pstPrivate;
    if (pstPrivate) {
        ms_rwlock_uninit(&pstPrivate->rwlock);
        ms_free(pstPrivate);
    }
    return MF_SUCCESS;
}

