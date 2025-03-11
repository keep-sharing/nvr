/*
 * ***************************************************************
 * Filename:        disk_local.c
 * Created at:      2017.05.10
 * Description:     local disk mangement API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <linux/fs.h>
#include <uuid/uuid.h>
#include <errno.h>

#include "msfs_disk.h"
#include "MFdisk.h"
#include "MFlog.h"

#define CAP_16T (16ULL*1024*1024*1024*1024)

typedef struct local_priv {
    struct diskObj *parent;
} local_priv;

static MF_S32
local_set_head0_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 res;

    if (fd <= 0) {
        return MF_FAILURE;
    }

    res = disk_write(fd, pstDev->pstHeader, size, pstDev->sector);
    if (res < 0) {
        MFerr("disk_write failed");
    }

    disk_close(fd);

    return res < 0 ? MF_FAILURE : MF_SUCCESS;
}

static MF_S32
local_get_head0_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);

    if (fd <= 0) {
        return MF_FAILURE;
    }
    if (ioctl(fd, BLKSSZGET, &pstDev->sector) < 0) {
        MFerr("[%s] ioctl BLKSSZGET err, errno = %d, strerr = %s", pstDev->name, errno, strerror(errno));
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
local_set_head1_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);
    MF_S32 res;

    if (fd <= 0) {
        return MF_FAILURE;
    }

    res = disk_write(fd, pstDev->pstHeader, size, HEADER1_OFFSET(pstDev->sector));
    if (res < 0) {
        MFerr("disk_write failed");
    }

    disk_close(fd);

    return res < 0 ? MF_FAILURE : MF_SUCCESS;
}

static MF_S32
local_get_head1_info(struct diskObj *pstDev)
{
    MF_S32 fd = disk_open(pstDev->name, MF_NO);
    MF_U32 size = ALIGN_UP(sizeof(struct disk_header), 512);

    if (fd <= 0) {
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
local_header_check(struct diskObj *pstDev)
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

    if (local_get_head0_info(pstDev) == MF_SUCCESS) {
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

    if (local_get_head1_info(pstDev) == MF_SUCCESS) {
        res = MF_SUCCESS;
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
            local_set_head0_info(pstDev);
        } else {
            pstDev->enState = DISK_STATE_UNFORMATTED;
        }
    }
    return res;
}

static MF_S32
local_reset_head0_info(struct diskObj *pstDev)
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
local_reset_head1_info(struct diskObj *pstDev)
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

MF_S32
local_reset_allhead_info(struct diskObj *pstDev)
{
    MF_S32 res = 0;

    if (disk_erase_MBR512(pstDev->name) == MF_FAILURE) {
        MFerr("erase mbr fail!");
        res = -1;
    }
    res |= local_reset_head0_info(pstDev);
    res |= local_reset_head1_info(pstDev);
    MFinfo("reset all head %s!! res:[%d]", pstDev->name, res);
    return res < 0 ? MF_FAILURE : MF_SUCCESS;
}

static MF_S32
local_format(void *pstPrivate, void *argv, MF_U64 quota)
{
    /*
    * disk data structure
    * 's' indicate a sector (eg 512)
    *
    *  start   -|HEADER0|-|MAIN+CUR LOG|-|HEADER1|-|INDEX0|INDEX1|-|FILE|-  end
    *           s      63*s     s     60M+15M       s     63*s     s    50M      50M     s  n*1G
    */
    /*
    * file data structure
    *                 data ---->                       <----info
    *  start   |-----AVdata---|--infoN~info0|--infoN~info0|segment0~segment1023--|Tag-resverve(5MB)--|fileHeader(512B)--|  end
    */
    struct bplus_key stKey;
    struct bplus_data stData;
    struct diskObj *pstDev = ((struct local_priv *)pstPrivate)->parent;
    FORMAT_EN ftype = *(FORMAT_EN *)argv;
    MF_U32 sector = pstDev->sector;
    uuid_t uu;
    MF_S32 i;
    MF_S32 res;
    MF_U32 magic_num = 0;
    MF_S8 *pKey = mf_dev_mmac();

    // percent time
    int start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    if (ftype != FORMAT_TYPE_MS) {
        return MF_FAILURE;
    }

    local_reset_allhead_info(pstDev);

    /*disk header info init */
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

    /* create index by B+tree */
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
//        stKey.stRecord.chnId = i % 64;
        stKey.stRecord.fileNo = i;
        stKey.stRecord.fileOffset = pstDev->pstHeader->recordOff + pstDev->pstHeader->perFileSize * i;
//        stKey.stRecord.startTime = time(0);
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

//        mf_index_data_init(pstDev->tree, &stKey);
        // TODO:
    }
    mf_index_key_backup(pstDev->tree);

    MFinfo("format step 2");
    pstDev->pstHeader->index00Len = mf_index_size(pstDev->tree);
    pstDev->free = pstDev->pstHeader->recordLen;

    MFinfo("format step 3");

    /*create a uuid  string */
    uuid_generate(uu);
    uuid_unparse(uu, pstDev->pstHeader->uuid);

    //insert private flag to uuid[UUID_PRIVATE_BIT] , default: disable
    pstDev->pstHeader->uuid[UUID_PRIVATE_BIT] = 'N';
    //insert key len to uuid[UUID_LEN_BIT]
    pstDev->pstHeader->uuid[UUID_LEN_BIT] = strlen(pKey);
    //insert key from uuid[UUID_KEY_BIT] to uuid[strlen(pKey) -1]
    strncpy(&pstDev->pstHeader->uuid[UUID_KEY_BIT],
            pKey, pstDev->pstHeader->uuid[UUID_LEN_BIT]);
    //key end bit '-'
    pstDev->pstHeader->uuid[UUID_KEY_BIT + pstDev->pstHeader->uuid[UUID_LEN_BIT]] = '-';

    MFinfo("format step 4");

    /* check header sum */
    pstDev->pstHeader->checkSum = check_sum(pstDev->pstHeader, pstDev->pstHeader->size);

    MFinfo("format step 5");
    local_set_head0_info(pstDev);// set header 0 info
    local_set_head1_info(pstDev);// set header 1 info
    pstDev->ops->disk_log_fomat(pstDev);
    if (mf_read_super_magic(pstDev->alias, &magic_num) == MF_SUCCESS) {
        if (magic_num == RAID_MAGIC) {
            raid_local_zero_superblock(pstDev->alias);    // clear global raid magic
        } else if (magic_num == GLOBAL_SPARE_MAGIC) { // clear global spare magic
            remove_global_spare(pstDev->port, pstDev->alias);
        }
    }
    MFinfo("format step done");

//    mf_index_dump(pstDev->tree);
    return MF_SUCCESS;
}

static MF_S32
local_mount(void *pstPrivate)
{
    return MF_SUCCESS;
}

static MF_S32
local_umount(void *pstPrivate)
{
    struct local_priv *priv = (struct local_priv *)pstPrivate;

    if (priv->parent->tree) {
        mf_index_release(priv->parent->tree);
    }
    priv->parent->tree = NULL;

    return MF_SUCCESS;
}

static MF_S32
local_size(void *pstPrivate)
{
    struct local_priv *priv = (struct local_priv *)pstPrivate;
    MF_S32  dev;

    if (!priv) {
        return MF_FAILURE;
    }
    if (!priv->parent) {
        return MF_FAILURE;
    }

    if ((dev = open(priv->parent->name, O_RDWR)) <= 0) {
        MFerr("[%s] open error!", priv->parent->name);
        return MF_FAILURE;
    }
    if (ioctl(dev, BLKGETSIZE64, &priv->parent->capacity) < 0) {
        MFerr("[%s] ioctl BLKGETSIZE64 ", priv->parent->name);
        close(dev);
        return MF_FAILURE;
    }
#ifdef DEBUG_REC
    priv->parent->capacity = 1024 * 1024 * 1024;
#endif

    close(dev);

    if (priv->parent->enType != DISK_TYPE_GLOBAL_SPARE &&
        priv->parent->enType != DISK_TYPE_IN_RAID &&
        priv->parent->enState == DISK_STATE_NORMAL) {
        priv->parent->free = mf_retr_disk_free_size(priv->parent);
    } else {
        priv->parent->free = 0;
    }

    return MF_SUCCESS;
}

static MF_S32
local_vendor(void *pstPrivate)
{
    struct local_priv *priv = (struct local_priv *)pstPrivate;
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
local_status(void *pstPrivate)
{
    struct local_priv *priv = (struct local_priv *)pstPrivate;
    MF_U32 magic_num = 0;

    if (mf_read_super_magic(priv->parent->alias, &magic_num) == MF_SUCCESS) {
        if (mf_disk_get_raid_mode() == MF_YES && magic_num == GLOBAL_SPARE_MAGIC) {
            priv->parent->enType =
                DISK_TYPE_GLOBAL_SPARE; //when a disk for global spare disk, it can not used for record....., so switch disk state to GLOBAL_STATE
            if (local_header_check(priv->parent) == MF_FAILURE) {
                priv->parent->enState = DISK_STATE_OFFLINE;
            }
            goto EXIT;
        } else if (mf_disk_get_raid_mode() == MF_YES && priv->parent->enType == DISK_TYPE_IN_RAID) {
            goto EXIT;
        } else {
            priv->parent->enType = DISK_TYPE_LOCAL;
        }
    } else {
        priv->parent->enType = DISK_TYPE_LOCAL;
    }
    if (local_header_check(priv->parent) == MF_FAILURE) {
        priv->parent->enState = DISK_STATE_OFFLINE;
        goto EXIT;
    }
    /*    if (strcmp(priv->parent->pstHeader->magic, DISK_MAGIC))
        {
            MFinfo("disk[%s] unformatted", priv->parent->name);
            priv->parent->enState = DISK_STATE_UNFORMATTED;
            goto EXIT;
        }
        if (strcmp(priv->parent->pstHeader->version, DISK_VERSION))
        {
            MFinfo("disk[%s] version dismatch", priv->parent->name);
            priv->parent->enState = DISK_STATE_UNFORMATTED;
            goto EXIT;
        }
        if (priv->parent->pstHeader->checkSum != \
            check_sum(priv->parent->pstHeader, MF_POS(priv->parent->pstHeader, checkSum)))
        {
            MFinfo("disk[%s] damage", priv->parent->name);
            priv->parent->enState = DISK_STATE_UNFORMATTED;
            goto EXIT;
        }
        priv->parent->enState = DISK_STATE_NORMAL;*/
    if (!priv->parent->tree && priv->parent->enState == DISK_STATE_NORMAL) {
        priv->parent->tree = mf_index_fetch(priv->parent->name,
                                            priv->parent->pstHeader->index00Off,
                                            priv->parent);
    }

    if (!priv->parent->tree) {
        priv->parent->enState = DISK_STATE_UNFORMATTED;
    } else if (priv->parent->enState != DISK_STATE_BAD && !mf_index_is_valid(priv->parent->tree)) {
        priv->parent->enState = DISK_STATE_BAD;
    }
EXIT:
    return MF_SUCCESS;
}

static MF_S32
local_file_open(void *pstPrivate, MF_S32 fileNo, MF_BOOL bO_DIRECT)
{
    struct local_priv *priv = (struct local_priv *)pstPrivate;
    return disk_open(priv->parent->name, bO_DIRECT);
}

static MF_S32
local_file_close(void *pstPrivate, MF_S32 fd)
{
    return  disk_close(fd);
}

static MF_S32
local_file_write(void *pstPrivate,
                 MF_S32 fd,
                 const void *buf,
                 MF_S32 count,
                 off64_t offset)
{
    MF_S32 res;
    struct local_priv *priv = (struct local_priv *)pstPrivate;
    MF_U32 sector = priv->parent->sector;

    res = disk_write(fd, buf, count, offset);
    if (res != count) {
        MFerr("disk_write failed");
        res = mf_disk_error(fd, offset, count, sector);
    }

    return res;
}

static MF_S32
local_file_read(void *pstPrivate,
                MF_S32 fd,
                void *buf,
                MF_S32 count,
                off64_t offset)
{
    MF_S32 res;
    struct local_priv *priv = (struct local_priv *)pstPrivate;
    MF_U32 sector = priv->parent->sector;

    res = disk_read(fd, buf, count, offset);

    if (res != count) {
        MFerr("disk_read failed");
        res = mf_disk_error(fd, offset, count, sector);
    }
    return res;
}

MF_S32
local_log_init(void *disk)
{
    return disk_open(((struct diskObj *)disk)->name, MF_NO);
}

static MF_S32
local_log_fomat(void *local_disk)
{
    int fd = -1;
    int ret = 0;
    struct diskObj *disk = (struct diskObj *)local_disk;

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
local_index_backup(void *index)
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
local_index_recover(void *index)
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
local_index_restore(void *pstPrivate)
{
    struct diskObj *pstDev = ((struct local_priv *)pstPrivate)->parent;
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

    local_set_head0_info(pstDev);// set header 0 info
    local_set_head1_info(pstDev);// set header 1 info

    /* backup index tree */
    MFinfo("step 4: backup index tree\n");
    mf_index_key_backup(pstDev->tree);

    return MF_SUCCESS;
}

static MF_S32
local_index_restoreEx(void *pstPrivate)
{
    struct diskObj *pstDev = ((struct local_priv *)pstPrivate)->parent;
    struct bplus_tree *tree;
    struct bplus_key stKey;
    struct bplus_data stData;
    struct mf_file stFile;
    MF_S32 i;
    MF_S32 res;
    MF_S32 start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    MF_U32 sector = 512;

    local_reset_allhead_info(pstDev);

    /*disk header info init */
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

    local_set_head0_info(pstDev);// set header 0 info
    local_set_head1_info(pstDev);// set header 1 info

    /* backup index tree */
    MFinfo("step 4: backup index tree\n");
    mf_index_key_backup(pstDev->tree);

    return MF_SUCCESS;
}

static MF_S32
local_go_private(void *pstPrivate, MF_BOOL bEnable)
{
    struct diskObj *pstDev = ((struct local_priv *)pstPrivate)->parent;
    MF_S32 res = MF_FAILURE;

    local_get_head0_info(pstDev);
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

    if (local_set_head0_info(pstDev) == MF_SUCCESS) { // set header 0 info
        res = local_set_head1_info(pstDev);    // set header 1 info
    }

    return res;
}

static struct disk_ops local_ops = {
    .disk_format        = local_format,
    .disk_mount         = local_mount,
    .disk_umount        = local_umount,
    .disk_size          = local_size,
    .disk_vendor        = local_vendor,
    .disk_status        = local_status,
    .disk_file_open     = local_file_open,
    .disk_file_close    = local_file_close,
    .disk_file_read     = local_file_read,
    .disk_file_write    = local_file_write,
    .disk_log_fomat     = local_log_fomat,
    .disk_log_init      = local_log_init,
    .disk_index_backup  = local_index_backup,
    .disk_index_recover = local_index_recover,
    .disk_index_restore = local_index_restore,
    .disk_go_private    = local_go_private,
    .disk_index_restoreEx = local_index_restoreEx,
};

MF_S32
disk_local_register(struct diskObj *pstDiskObj, MF_S8 *host)
{
    struct local_priv *pstPrivate;

    pstDiskObj->ops = &local_ops;

    pstPrivate = ms_calloc(1, sizeof(struct local_priv));
    pstPrivate->parent = pstDiskObj;
    pstDiskObj->pstPrivate = pstPrivate;

    return MF_SUCCESS;
}

MF_S32
disk_local_unregister(struct diskObj *pstDiskObj)
{
    if (pstDiskObj->pstPrivate) {
        ms_free(pstDiskObj->pstPrivate);
    }

    return MF_SUCCESS;
}

