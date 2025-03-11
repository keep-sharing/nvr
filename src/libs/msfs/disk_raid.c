/*
 * ***************************************************************
 * Filename:        disk_raid.c
 * Created at:      2017.11.23
 * Description:     raid  mangement API
 * Author:          hugo
 * Copyright (C)    milesight
 * ***************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <uuid/uuid.h>
#include <mntent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <mntent.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>

#include "mf_type.h"
#include "msfs_disk.h"
#include "msfs_notify.h"
#include "MFdisk.h"
#include "MFlog.h"
#include "msstd.h"
#include "raid.h"
#include "utils.h"

typedef struct raid_priv {
    struct raid_info raid;
    struct diskObj *parent;
} raid_priv;


struct local_raid {
    int port;
    char dev_path[16];
    int level;
    int state;
    int task;
    int percent;
    int de_num; //raid degrade, how much disk faulty
    struct raid_disk_list disk;
};

static MF_S32
get_raid_info(int raid_port, struct local_raid *raid);
static MF_S32
get_dev_size(char *devname, unsigned long long *sizep);


static MF_S32
raid_set_head0_info(struct diskObj *pstDev)
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

    return res;
}

static MF_S32
raid_get_head0_info(struct diskObj *pstDev)
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
raid_set_head1_info(struct diskObj *pstDev)
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

    return res;
}

static MF_S32
raid_get_head1_info(struct diskObj *pstDev)
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

static DISK_SMART_RESULT raid_check_smart_result(struct raid_info *raid)
{
    int i;
    DISK_SMART_RESULT smartTest = SMART_PASS;

    for (i = 0; i < raid->disk.cnt; i++) {
        if (mf_disk_get_disk_attr_status(raid->disk.dev[i].dev_path) == SMART_WARN) {
            smartTest = SMART_WARN;
            MFshow("raid port:%d path:%s smart:%d\n", raid->disk.dev[i].port, raid->disk.dev[i].dev_path, smartTest);
            break;
        }
    }

    return smartTest;
}

static MF_S32
raid_header_check(struct diskObj *pstDev, struct raid_info *raid)
{
    MF_S32 res = MF_FAILURE;

    if (!pstDev || raid->state == RAID_STATE_OFFLINE) {
        return res;
    }
    if (raid_get_head0_info(pstDev) == MF_SUCCESS) {
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
                pstDev->smartTest = raid_check_smart_result(raid);
            }
            return MF_SUCCESS;
        } else {
            pstDev->enState = DISK_STATE_UNFORMATTED;
        }
    }

    if (raid_get_head1_info(pstDev) == MF_SUCCESS) {
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
                pstDev->smartTest = raid_check_smart_result(raid);
            }
            raid_set_head0_info(pstDev);
        } else {
            pstDev->enState = DISK_STATE_UNFORMATTED;
        }
    }
    return res;
}

static MF_S32
raid_reset_head_info(struct diskObj *pstDev)
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
raid_format(void *pstPrivate, void *argv, MF_U64 quota)
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
    struct diskObj *pstDev = ((struct raid_priv *)pstPrivate)->parent;
    MF_U32 sector = pstDev->sector;
    uuid_t uu;
    MF_S32 i;
    MF_S32 res;
    MF_S8 *pKey = mf_dev_mmac();

    // percent time
    int start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    raid_reset_head_info(pstDev);

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

    /*create a uuid  string */
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

    /* check header sum */
    MFinfo("format step 4");
    pstDev->pstHeader->checkSum = check_sum(pstDev->pstHeader, pstDev->pstHeader->size);

    MFinfo("format step 5");
    raid_set_head0_info(pstDev);
    raid_set_head1_info(pstDev);
    pstDev->ops->disk_log_fomat(pstDev);
    MFinfo("format step done");

    return MF_SUCCESS;
}

static MF_S32
raid_mount(void *pstPrivate)
{
    return MF_SUCCESS;
}

static MF_S32
raid_umount(void *pstPrivate)
{
    struct raid_priv *priv = (struct raid_priv *)pstPrivate;

    if (priv->parent->tree) {
        mf_index_release(priv->parent->tree);
        priv->parent->tree = NULL;
    }

    return MF_SUCCESS;
}

static MF_S32
raid_size(void *pstPrivate)
{
    struct raid_priv *priv = (struct raid_priv *)pstPrivate;
    MF_S32  dev;

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

    if (priv->parent->enState == DISK_STATE_NORMAL) {
        priv->parent->free = mf_retr_disk_free_size(priv->parent);
    } else {
        priv->parent->free = 0;
    }

    return MF_SUCCESS;
}

static MF_S32
raid_vendor(void *pstPrivate)
{
    struct raid_priv *priv = (struct raid_priv *)pstPrivate;

    if (strlen(priv->parent->vendor) <= 0) {
        strcpy(priv->parent->vendor, "RAID");
    }

    return MF_SUCCESS;
}

// get raid info -> g_disk.list -> raid.pstPrivate
static void
get_raid_priv(struct raid_priv *priv)
{
    if (!priv || !priv->parent) {
        return;
    }

    struct local_raid raid;
    unsigned long long ksize = 0;
    int port = priv->parent->port;
    MD_STATE_EN state;
    memset(&raid, 0, sizeof(struct local_raid));

/*
    【稳定性-老化：老化8064-UH替换4个10TB硬盘创建RAID挂载，出现Disk界面一直加载卡住问题】
    https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001127711

    if not unlock, then will be dead lock:
    1. dev_attribute(): ms_mutex_lock(&pstDev->mutex) -> mf_find_name_port(): DEV_FIND_BY_ALIAS(): ms_rwlock_rdlock(&g_disk.rwlock);
    2. record_fetch_file(): mf_disk_get_wr_list(): ms_rwlock_rdlock(&g_disk.rwlock); -> record_fetch_file(): ms_mutex_lock(&pstDisk->mutex);
*/
    ms_mutex_unlock(&priv->parent->mutex);
    state = get_raid_info(port, &raid);
    ms_mutex_lock(&priv->parent->mutex);
    if (state == RAID_STATE_NOEXIST) {
        priv->parent->enState = DISK_STATE_OFFLINE;
        priv->raid.state = RAID_STATE_OFFLINE;
    } else {
        if (!raid_get_min_disk_size(&raid.port, &ksize)) {
            priv->raid.min_hdd_size = ksize;
        } else {
            priv->raid.min_hdd_size = 0;
        }

        priv->raid.level = raid.level;
        priv->raid.percent = raid.percent;
        priv->raid.state = raid.state;
        priv->raid.task = raid.task;
        memcpy(&priv->raid.disk, &raid.disk, sizeof(raid.disk));

        if (priv->raid.state == RAID_STATE_OFFLINE) {
            priv->parent->enState = DISK_STATE_OFFLINE;
        }
    }
}


static MF_S32
raid_status(void *pstPrivate)
{
    struct raid_priv *priv = (struct raid_priv *)pstPrivate;

    //if raid is offline,  then get raid priv info.
    if (priv->parent) {
        get_raid_priv(priv);
    }

    if (raid_header_check(priv->parent, &priv->raid) == MF_FAILURE) {
        priv->parent->enState = DISK_STATE_OFFLINE;
        priv->raid.state = RAID_STATE_OFFLINE;
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
    if (!mf_index_is_valid(priv->parent->tree)) {
        priv->parent->enState = DISK_STATE_UNFORMATTED;
    }
EXIT:
    return MF_SUCCESS;
}

static MF_S32
raid_file_open(void *pstPrivate, int fileNo, MF_BOOL bO_DIRECT)
{
    struct raid_priv *priv = (struct raid_priv *)pstPrivate;

    return disk_open(priv->parent->name, bO_DIRECT);
}

static MF_S32
raid_file_close(void *pstPrivate, int fd)
{
    return  disk_close(fd);
}

static MF_S32
raid_file_write(void *pstPrivate,
                MF_S32 fd,
                const void *buf,
                MF_S32 count,
                off64_t offset)
{
    MF_S32 res;
    struct raid_priv *priv = (struct raid_priv *)pstPrivate;
    MF_U32 sector = priv->parent->sector;

    res = disk_write(fd, buf, count, offset);

    if (res != count) {
        MFerr("disk_write failed");
        res = mf_disk_error(fd, offset, count, sector);
    }

    return res;
}

static MF_S32
raid_file_read(void *pstPrivate,
               MF_S32 fd,
               void *buf,
               MF_S32 count,
               off64_t offset)
{
    MF_S32 res;
    struct raid_priv *priv = (struct raid_priv *)pstPrivate;
    MF_U32 sector = priv->parent->sector;

    res = disk_read(fd, buf, count, offset);

    if (res != count) {
        MFerr("disk_read failed");
        res = mf_disk_error(fd, offset, count, sector);
    }
    return res;
}

MF_S32
raid_log_init(void *disk)
{
    return disk_open(((struct diskObj *)disk)->name, MF_NO);
}

static MF_S32
raid_log_fomat(void *esata_disk)
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
raid_index_backup(void *index)
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
raid_index_recover(void *index)
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
raid_index_restore(void *pstPrivate)
{
    struct diskObj *pstDev = ((struct raid_priv *)pstPrivate)->parent;
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

    raid_set_head0_info(pstDev);// set header 0 info
    raid_set_head1_info(pstDev);// set header 1 info

    /* backup index tree */
    MFinfo("step 4: backup index tree\n");
    mf_index_key_backup(pstDev->tree);

    return MF_SUCCESS;
}

static MF_S32
raid_index_restoreEx(void *pstPrivate)
{
    struct diskObj *pstDev = ((struct raid_priv *)pstPrivate)->parent;
    struct bplus_tree *tree;
    struct bplus_key stKey;
    struct bplus_data stData;
    struct mf_file stFile;
    MF_S32 i;
    MF_S32 res;
    MF_S32 start = 20, end = 80, gap = end - start, percent = 0, pre = 0;

    MF_U32 sector = 512;

    raid_reset_head_info(pstDev);

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

    raid_set_head0_info(pstDev);// set header 0 info
    raid_set_head1_info(pstDev);// set header 1 info

    /* backup index tree */
    MFinfo("step 4: backup index tree\n");
    mf_index_key_backup(pstDev->tree);

    return MF_SUCCESS;
}

static MF_S32
raid_go_private(void *pstPrivate, MF_BOOL bEnable)
{
    struct diskObj *pstDev = ((struct raid_priv *)pstPrivate)->parent;
    MF_S32 res = MF_FAILURE;

    raid_get_head0_info(pstDev);
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

    if (raid_set_head0_info(pstDev) == MF_SUCCESS) { // set header 0 info
        res = raid_set_head1_info(pstDev);    // set header 1 info
    }

    return res;
}

static struct disk_ops raid_ops = {
    .disk_format        = raid_format,
    .disk_mount         = raid_mount,
    .disk_umount        = raid_umount,
    .disk_size          = raid_size,
    .disk_vendor        = raid_vendor,
    .disk_status        = raid_status,
    .disk_file_open     = raid_file_open,
    .disk_file_close    = raid_file_close,
    .disk_file_read     = raid_file_read,
    .disk_file_write    = raid_file_write,
    .disk_log_fomat     = raid_log_fomat,
    .disk_log_init      = raid_log_init,
    .disk_index_backup  = raid_index_backup,
    .disk_index_recover = raid_index_recover,
    .disk_index_restore = raid_index_restore,
    .disk_go_private    = raid_go_private,
    .disk_index_restoreEx = raid_index_restoreEx,
    //.disk_raid_op     = raid_operate;
};

MF_S32
disk_raid_register(struct diskObj *pstDiskObj, MF_S8 *host)
{
    struct raid_priv *pstPrivate;

    pstDiskObj->ops = &raid_ops;

    pstPrivate = ms_calloc(1, sizeof(struct raid_priv));
    pstPrivate->parent = pstDiskObj;
    pstDiskObj->pstPrivate = pstPrivate;

    return MF_SUCCESS;
}

MF_S32
disk_raid_unregister(struct diskObj *pstDiskObj)
{
    if (pstDiskObj->pstPrivate) {
        ms_free(pstDiskObj->pstPrivate);
    }

    return MF_SUCCESS;
}

/*************** raid API **************/

static MF_S32
enough(int level, int raid_disks, int layout, int clean, char *avail)
{
    int copies, first;
    int i;
    int avail_disks = 0;

    if (!avail) {
        MFerr("avail is NULL. \n");
        return -1;
    }

    for (i = 0; i < raid_disks; i++) {
        avail_disks += !!avail[i];
    }

    switch (level) {
        case 10: {
            copies = (layout & 255) * ((layout >> 8) & 255);
            first = 0;
            do {
                int n = copies;
                int cnt = 0;
                int this = first;
                while (n--) {
                    if (avail[this]) {
                        cnt++;
                    }
                    this = (this + 1) % raid_disks;
                }
                if (cnt == 0) {
                    return MF_SUCCESS;
                }
                first = (first + (layout & 255)) % raid_disks;
            } while (first != 0);
            return 1;
        }

        case LEVEL_MULTIPATH: {
            return avail_disks >= 1;
        }

        case LEVEL_LINEAR:
        case 0: {
            return avail_disks == raid_disks;
        }

        case 1: {
            return avail_disks >= 1;
        }

        case 4: {
            if (avail_disks == raid_disks - 1 &&
                !avail[raid_disks - 1])
                /* If just the parity device is missing, then we have enough, even if not clean */
            {
                return 1;
            }
            /* FALL THROUGH */
        }

        case 5: {
            if (clean) {
                return avail_disks >= raid_disks - 1;
            } else {
                return avail_disks >= raid_disks;
            }
        }

        case 6: {
            if (clean) {
                return avail_disks >= raid_disks - 2;
            } else {
                return avail_disks >= raid_disks;
            }
        }

        default:
            return MF_SUCCESS;
    }
}

MF_S32
raid_get_min_disk_size(MF_S32 *port, unsigned long long *sizep) //get raid min disk size
{
    FILE *fd = NULL;
    MF_S8 path[DEV_NAME_LEN] = {0};

    if (!sizep) {
        MFerr("sizep is NULL. \n");
        return -1;
    }

    sprintf(path, "/sys/devices/virtual/block/md%d/md/component_size", *port);
    fd = fopen(path, "r");
    if (fd == NULL) {
        return -1;
    }
    if (fscanf(fd, "%llu", sizep) == 0) {
        MFerr("cannot get size for %d: %s\n", *port, strerror(errno));
        fclose(fd);
        return -1;
    }
    fclose(fd);

    MFinfo("sizep %llu", *sizep);
    *sizep = (*sizep) * 1024;

    return MF_SUCCESS;
}

//raid module use this function in internal.
static MF_S32
get_md_array_status(char *mdname, int md_port, int *status)
{
    int fd = -1;
    int d = 0;
    int next = 0;
    int max_disks = MD_SB_DISKS; /* just a default */
    int avail_disks = 0;
    unsigned char port = 0;
    char *avail = NULL;
    char fname[1024] = {0};
    char buf[1024] = {0};
    DIR *dir = NULL;
    mdu_array_info_t array;
    mdu_disk_info_t *disks;
    mdu_disk_info_t disk;
    char diskpath[16] = {0};
    char tdisk[16] = {0};
    char *devp = NULL;

    if (!mdname || !status) {
        MFerr("mdname or status is NULL. \n");
        return -1;
    }

    fd = open(mdname, O_RDONLY);
    if (fd < 0) {
        MFerr("=== open mdname:[%s] failed. \n", mdname);
        return -1;
    }

    if (ioctl(fd, GET_ARRAY_INFO, &array) != 0) {
        *status = RAID_STATE_OFFLINE;
        close(fd);
        return 0;
    }

    disks = ms_malloc(max_disks * 2 * sizeof(mdu_disk_info_t));
    if (!disks) {
        MFerr("== ms_malloc disks failed. \n");
        close(fd);
        return -1;
    }

    for (d = 0; d < max_disks * 2; d++) {
        disks[d].state = (1 << MD_DISK_REMOVED);
        disks[d].major = disks[d].minor = 0;
        disks[d].number = -1;
        disks[d].raid_disk = d / 2;
    }

    next = array.raid_disks * 2;

    for (d = 0; d < max_disks; d++) {
        disk.number = d;
        if (ioctl(fd, GET_DISK_INFO, &disk) < 0) {
            if (d < array.raid_disks) {
                continue;
            }
        }

        if (disk.major == 0 && disk.minor == 0) {
            continue;
        }

        if (disk.raid_disk >= 0 && disk.raid_disk < array.raid_disks
            && disks[disk.raid_disk * 2].state == (1 << MD_DISK_REMOVED)) {
            disks[disk.raid_disk * 2] = disk;
        } else if (disk.raid_disk >= 0 && disk.raid_disk < array.raid_disks
                   && disks[disk.raid_disk * 2 + 1].state == (1 << MD_DISK_REMOVED)
                   && !(disk.state & (1 << MD_DISK_JOURNAL))) {
            disks[disk.raid_disk * 2 + 1] = disk;
        } else if (next < max_disks * 2) {
            disks[next++] = disk;
        }
    }

    avail = ms_calloc(array.raid_disks, 1);
    if (!avail) {
        close(fd);
        ms_free(disks);
        return -1;
    }

    for (d = 0; d < array.raid_disks; d++) {
        if ((disks[d * 2].state & (1 << MD_DISK_SYNC)) ||
            (disks[d * 2 + 1].state & (1 << MD_DISK_SYNC))) {
            sprintf(fname, "/sys/block/md%d/md/rd%d", md_port, d);

            if (readlink(fname, buf, sizeof(buf)) >= 0) {
                devp = strstr(buf, "-");
                sprintf(diskpath, "%s", devp + 1);
                sprintf(tdisk, "/dev/%s", diskpath);
                //MFinfo("disk %s", tdisk);
                sprintf(fname, "/sys/block/md%d/md/%s/block", md_port, buf);
                //MFinfo("fname %s", fname);
                dir = opendir(fname);
                if (dir != NULL &&
                    mf_find_name_port(tdisk, &port, CMD_FIND_PORT_BY_ALIAS) == MF_SUCCESS) {
                    avail_disks ++;
                    avail[d] = 1;
                    closedir(dir);
                }
            }
        }
    }

    if (array.raid_disks) {
        if (avail_disks == array.raid_disks) {
            *status = RAID_STATE_ACTIVE;
        } else if (!enough(array.level, array.raid_disks, array.layout, 1, avail)) {
            *status = RAID_STATE_OFFLINE;
        } else {
            *status = RAID_STATE_DEGRADE;
        }
    } else {
        *status = RAID_STATE_OFFLINE;
    }

    close(fd);
    ms_free(avail);
    ms_free(disks);

    return MF_SUCCESS;
}


static MF_S32
get_raid_info(int raid_port, struct local_raid *raid)
{
    FILE *fp = NULL;
    char line[256] = {0};
    int  find_raid_conf = 0;
    char *md_s = "md";
    char buf[1024] = {0};
    char dev_name[DEV_NAME_LEN] = {0};
    char *word[128] = {0};
    char *nextword = NULL;
    char *buffer = buf;
    char w[32] = {0};
    int  num = 0;
    int status = 0;
    int md_l = 0;

    if (raid == NULL) {
        MFerr("local raid is NULL. \n");
        return RAID_STATE_NOEXIST;
    }

    if ((fp = fopen("/proc/mdstat", "r")) == NULL) {
        return RAID_STATE_NOEXIST;
    }

    md_l = strlen(md_s);

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, md_s, md_l) == 0) {
            if (find_raid_conf == 1) {
                break;
            }

            // raid port only support  1-99.
            if ((isdigit(line[md_l])  && (!isdigit(line[md_l + 1])) && (line[md_l] - '0' == raid_port)) \
                || (isdigit(line[md_l]) && isdigit(line[md_l + 1]) &&
                    ((line[md_l] - '0') * 10 + (line[md_l + 1] - '0') == raid_port))) {
                find_raid_conf = 1;
                raid->port = raid_port;
                strncpy(buf, line, sizeof(line));
                continue;
            }
        }

        if (find_raid_conf) {
            strncat(buf, line, sizeof(line));
        }
    }
    fclose(fp);

    if (!find_raid_conf) {
        MFerr("=========== RAID_STATE_NOEXIST =========== \n");
        raid->state = RAID_STATE_NOEXIST; // the raid not found, should delte raid info
        return raid->state;
    }

    // raid exist
    raid->disk.cnt = 0;

    while ((word[num] = strtok_r(buffer, " ", &nextword)) != NULL) {
        int wlen = strlen(word[num]);
        strncpy(w, word[num], wlen);
        if (strncmp(word[num], md_s, md_l) == 0) {
            raid->port = atoi(word[num] + md_l);
            snprintf(raid->dev_path, sizeof(raid->dev_path), "/dev/%s", word[num]);

            if (!get_md_array_status(raid->dev_path, raid->port, &status)) {
                raid->state = status;
            } else {
                MFerr("=== get_md_array_status error, raid port:[%d],name:[%s]. \n", raid->port, raid->dev_path);
            }
        } else if (strncmp(word[num], "raid", 4) == 0) {
            raid->level = atoi(word[num] + 4);
        } else if (strcmp(word[num], "resync") == 0 || strcmp(word[num], "recovery") == 0) {
            raid->task = RAID_TASK_RECOVERY;//RAID_STATE_RESYNC;
            raid->state = RAID_STATE_RECOVERY;
        } else if (strstr(word[num], "(F)")) {
            MF_S8 path[16] = {0};
            MF_U8 port;
            strncpy(dev_name, word[num], strlen("sda"));
            sprintf(path, "/dev/%s", dev_name);
            if (mf_find_name_port(path, &port, CMD_FIND_PORT_BY_ALIAS) == MF_SUCCESS) {
                strcpy(raid->disk.dev[raid->disk.cnt].dev_path, path);
                raid->disk.dev[raid->disk.cnt].port = port;
                raid->disk.dev[raid->disk.cnt].state = RAID_DISK_STATE_FAULTY;
                raid->disk.cnt++;
                mf_raid_fault_update_disk(path);
            }
        } else if (strstr(word[num], "(S)")) {
            strncpy(dev_name, word[num], strlen("sda"));
            sprintf(raid->disk.dev[raid->disk.cnt].dev_path, "/dev/%s", dev_name);
            mf_find_name_port(raid->disk.dev[raid->disk.cnt].dev_path, &(raid->disk.dev[raid->disk.cnt].port),
                              CMD_FIND_PORT_BY_ALIAS);
            raid->disk.dev[raid->disk.cnt].state = RAID_DISK_STATE_SPARE;
            raid->disk.cnt++;
        } else if (strstr(word[num], "sd")) {
            struct stat stb;
            strncpy(dev_name, word[num], strlen("sda"));
            sprintf(raid->disk.dev[raid->disk.cnt].dev_path, "/dev/%s", dev_name);
            mf_find_name_port(raid->disk.dev[raid->disk.cnt].dev_path, &(raid->disk.dev[raid->disk.cnt].port),
                              CMD_FIND_PORT_BY_ALIAS);
            if (stat(raid->disk.dev[raid->disk.cnt].dev_path, &stb) != 0 || !S_ISBLK(stb.st_mode)) {
                MFerr("disk no exist name:[%s]. \n", raid->disk.dev[raid->disk.cnt].dev_path);
                raid->disk.dev[raid->disk.cnt].state = RAID_DISK_STATE_NOEXIST;
                if (raid->level == RAID_0) { //raid 0 can not less anyone disk
                    MFerr("======== raid 0 offline ============= \n");
                    raid->state = RAID_STATE_OFFLINE;
                }
            } else {
                raid->disk.dev[raid->disk.cnt].state = RAID_DISK_STATE_RAID;
            }
            raid->disk.cnt++;
        } else if (w[0] >= '0' && w[0] <= '9' && w[wlen - 1] == '%') {
            raid->percent = atoi(w);
        }

        num ++;
        buffer = NULL;
    }

    return raid->state;
}

static MF_S32
gdisk_fd_raid(char *disk_path)
{
    FILE *fp = NULL;
    char cmd[64] = {0};
    //char *del = "o\ny\nn\n\n\n\nfd00\nw\ny\n";
    char *del = "d\n1\nd\nw\ny\n";

    if (disk_path == NULL) {
        MFerr("disk_path is NULL. \n");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "gdisk %s", disk_path);

    if ((fp = ms_vpopen(cmd, "w")) == NULL) {
        perror("popen gdisk failed.");
        return -1;
    }
    while (*del) {
        putc(*del++, fp);
        if (*del == '\n') {
            sleep(1);
        }
    }
    ms_vpclose(fp);
    sleep(1);

    return MF_SUCCESS;
}


static MF_S32
make_raid(char *name, int level, struct raid_disk_list *disks)
{
    char cmd[1024] = {0};
    char disk_names[MAX_LEN_128] = {0};
    char disk_tname[MAX_LEN_128] = {0};

    if (disks == NULL) {
        MFerr("disks is NULL. \n");
        return -1;
    }

    int i = 0;
    for (i = 0; i < disks->cnt; i++) {
        snprintf(disk_tname, sizeof(disk_tname), "%s %s", disk_names, disks->dev[i].dev_path);
        snprintf(disk_names, sizeof(disk_names), "%s", mf_trim(disk_tname));
    }
    snprintf(cmd, sizeof(cmd), "mdadm -C %s --run --assume-clean  -l %d -n %d %s ", name, level, disks->cnt, disk_names);
    ms_system(cmd);
    return MF_SUCCESS;
}


MF_S32
raid_create(struct raid_op_t *raid)//(MF_U8 *disk_port, int disk_num, int  raid_level, int raid_port)
{
    int i = 0;
    char cmd[128] = {0};
    struct local_raid       traid;
    struct raid_disk_list   tdisk;

    int percent = 0;
    int per_gap = 0;
    MF_S32 errPort = 0;

    if (raid == NULL) {
        MFerr("raid  is NULL. \n");
        return MF_FAILURE;
    }

    MF_U8 disk_port[MAX_DISK_LOCAL_NUM] = {0};
    MF_U8 disk_num = raid->disk_num;
    MF_U8 raid_level = raid->raid_level;
    MF_U8 raid_port = raid->raid_port;

    memcpy(disk_port, raid->disk_port, sizeof(raid->disk_port));
    memset(&traid, 0, sizeof(struct raid_info));
    memset(&tdisk, 0, sizeof(struct raid_disk_list));

    if (((raid_level == RAID_0) && (disk_num < 2)) ||
        ((raid_level == RAID_1) && (disk_num != 2)) ||
        ((raid_level == RAID_5) && (disk_num < 3)) ||
        ((raid_level == RAID_6) && (disk_num < 4)) ||
        ((raid_level == RAID_10) && ((disk_num < 4) || (disk_num % 2)))) {
        MFerr("==== disk_num is not suitable for raid type. \n");
        return MF_FAILURE;
    }

    sprintf(traid.dev_path, "/dev/md%d", raid_port);

    if (access(traid.dev_path, F_OK) == 0) {
        if (get_raid_info(raid_port, &traid) == RAID_STATE_NOEXIST) {
            MFerr("raid %s exist, but /proc/mdstat no exist, so delete this raid. \n", traid.dev_path);
            sprintf(cmd, "mdadm -Ds %s", traid.dev_path);
            MFerr("cmd: %s", cmd);
            ms_system(cmd);
        } else {
            MFerr("raid: %s exist, can't create! \n", traid.dev_path);
            return MF_FAILURE;
        }
    }

    //need check local disk be used for recoding or playback ?

    percent = 5;
    per_gap = ((65 - percent) / disk_num);
    mf_disk_notify_bar(PROGRESS_BAR_RAID_CREATE, raid_port, percent, DISK_TYPE_RAID);

    for (i = 0; i < disk_num; i++) {
        if (mf_find_name_port(tdisk.dev[i].dev_path, &disk_port[i], CMD_FIND_ALIAS_BY_PORT) != MF_SUCCESS) {
            MFerr("mf_find_name_port fail, disk port %d, raid port:[%d]. \n", disk_port[i], raid_port);
            errPort |= (1 << disk_port[i]);
        }

        if (gdisk_fd_raid(tdisk.dev[i].dev_path)) {
            MFerr("gdisk_fd_raid fail, raid port:%d, disk port:%d,disk dev_path:[%s]. \n", raid_port, disk_port[i],
                  tdisk.dev[i].dev_path);
            errPort |= (1 << disk_port[i]);
        }
        if (!errPort) {
            mf_disk_notify_bar(PROGRESS_BAR_RAID_CREATE, raid_port,
                               mf_random(percent, percent + per_gap), DISK_TYPE_RAID);
            percent += per_gap;
        }

        tdisk.dev[i].port = disk_port[i];
        tdisk.dev[i].state = RAID_DISK_STATE_RAID;
        tdisk.cnt++;
    }
    if (errPort) {
        return errPort;
    }

    make_raid(traid.dev_path, raid_level, &tdisk);

    mf_disk_notify_bar(PROGRESS_BAR_RAID_CREATE, raid_port, mf_random(percent, 70), DISK_TYPE_RAID);

    if (get_raid_info(raid_port, &traid) == RAID_STATE_NOEXIST) {
        MFerr("create raid fail, port %d. \n", raid_port);
        return MF_FAILURE;
    }

    mf_disk_raid_create(traid.dev_path); // add raid to gdisk.list

    return MF_SUCCESS;
}


MF_S32
raid_del(struct diskObj *pstDev)
{
    struct local_raid raid;
    MF_S8 cmd[128] = {0};
    MF_S32 raid_port = -1;
    int i = 0;
    int fd = -1;

    if (pstDev) {
        raid_port = pstDev->port;
    } else {
        return RAID_STATE_NOEXIST;
    }

    if (get_raid_info(raid_port, &raid) == RAID_STATE_NOEXIST) {
        MFerr("==== delete raid failed, port %d not exist. \n", raid_port);
        return RAID_STATE_NOEXIST;
    }

    fd = disk_open(raid.dev_path, MF_NO);
    if (fd < 0) {
        return RAID_STATE_BUSY;
    }
    disk_close(fd);
    snprintf(cmd, sizeof(cmd), "mdadm -S /dev/md%d", raid_port);
    ms_system(cmd);

    if (get_raid_info(raid_port, &raid) != RAID_STATE_NOEXIST) {
        return RAID_STATE_BUSY;
    }

    for (i = 0; i < raid.disk.cnt; i++) {
        //for raid delete
        raid_local_zero_superblock(raid.disk.dev[i].dev_path);
    }

    return mf_disk_raid_del(pstDev);
}


static char abuf[4096 + 4096];

/* aligned read.
 * On devices with a 4K sector size, we need to read
 * the full sector and copy relevant bits into
 * the buffer
 */
static MF_S32
aread(int fd, void *buf, int len, int blk_sz)
{
    int bsize, iosize;
    char *b;
    int n;

    bsize = blk_sz;

    if (!bsize || bsize > 4096 || len > 4096) {
        if (!bsize) {
            MFerr("aread() called with invalid block size\n");
        }
        return -1;
    }
    b = ROUND_UP_PTR((char *)abuf, 4096);

    for (iosize = 0; iosize < len; iosize += bsize)
        ;
    n = read(fd, b, iosize);
    if (n <= 0) {
        return n;
    }
    lseek(fd, len - n, 1);
    if (n > len) {
        n = len;
    }
    memcpy(buf, b, n);
    return n;
}


/* Return size of device in bytes */
static MF_S32
get_dev_size(char *devname, unsigned long long *sizep)
{
    unsigned long long ldsize;
    struct stat st;

    int fd = 0;

    if (!devname || !sizep) {
        MFerr("devname or sizep is NULL. \n");
        return -1;
    }

    fd = open(devname, O_RDONLY);
    if (fd < 0) {
        MFerr("==get dev size,open [%s] failed. \n", devname);
        return -1;
    }

    if (fstat(fd, &st) != -1 && S_ISREG(st.st_mode)) {
        ldsize = (unsigned long long)st.st_size;
    } else if (ioctl(fd, BLKGETSIZE64, &ldsize) != 0) {
        close(fd);
        return -1;
    }
    *sizep = ldsize;
    close(fd);
    return MF_SUCCESS;
}


MF_S32
write_super_magic(char *devname, MF_U32 magic_num)
{
    int fd = -1;
    //unsigned long long offset = 0;

    if (devname == NULL) {
        MFinfo("devname is NULL. \n");
        return -1;
    }

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        MFinfo("Open dev [%s] failed. \n", devname);
        return -1;
    }

    //offset = (8 << 9);  //version 1.20

    if (lseek64(fd, RAID_MAGIC_OFFSET, 0) < 0LL) {
        MFinfo("Cannot seek to superblock on %s: %s\n", devname, strerror(errno));
        close(fd);
        return -1;
    }

    if (write(fd, &magic_num, sizeof(magic_num)) != sizeof(magic_num)) { // only write 4 bytes ???
        MFinfo("Cannot write superblock on %s\n", devname);
        close(fd);
        return -1;
    }

    close(fd);

    return MF_SUCCESS;
}

MF_S32
read_super_magic(char *devname, MF_U32 *magic_num)
{
    int fd = -1;

    if (devname == NULL || magic_num == NULL) {
        MFinfo("devname or magic_num is NULL. \n");
        return -1;
    }

    fd = open(devname, O_RDONLY);
    if (fd < 0) {
        MFinfo("open [%s]  failed. \n", devname);
        return -1;
    }

    //offset = MD_NEW_SIZE_SECTORS(sizep>>9); //version 0.90
    //offset *= 512;

    //offset = (8 << 9); //version 1.20

    if (lseek64(fd, RAID_MAGIC_OFFSET, 0) < 0LL) {
        MFinfo("Cannot seek to superblock on %s: %s\n", devname, strerror(errno));
        close(fd);
        return -1;
    }

    if (read(fd, magic_num, sizeof(*magic_num)) != sizeof(*magic_num)) { // only write 4 bytes, the same to raid magic
        MFinfo("Cannot read superblock on %s\n", devname);
        close(fd);
        return -1;
    }

    close(fd);

    return MF_SUCCESS;
}


MF_S32
create_global_spare(MF_S32 disk_port, MF_S8 *devname) // all raid disk must be formatted.
{
    if (devname == NULL) {
        MFerr("devname is NULL. \n");
        return -1;
    }

    if (write_super_magic(devname, GLOBAL_SPARE_MAGIC) != MF_SUCCESS) {
        MFerr("write_super_magic failed. \n");
        return -1;
    }

    return MF_SUCCESS;
}

MF_S32
remove_global_spare(int disk_port, MF_S8 *devname)
{
    if (devname == NULL) {
        MFerr("devname is NULL. \n");
        return MF_FAILURE;
    }

    if (write_super_magic(devname, INVALID_MAGIC) != MF_SUCCESS) {
        MFerr("write_super_magic failed. \n");
        return MF_FAILURE;
    }

    return MF_SUCCESS;
}

static MF_S32
add_raid_spare_disk(int raid_port, int disk_port)
{
    int i = 0;
    int ret = MF_FAILURE;
    char cmd[64] = {0};
    char dev_path[DEV_NAME_LEN] = {0};
    struct local_raid traid;

    memset(&traid, 0, sizeof(traid));
    if (get_raid_info(raid_port, &traid) == RAID_STATE_NOEXIST) {
        MFinfo("[Solin] no such raid with port %d \n", raid_port);
        return ret;
    }

    if (traid.level == RAID_0) {
        MFinfo("[Solin] raid level 0 can not add spare. \n");
        return ret;
    }

    if (mf_find_name_port(dev_path, (MF_U8 *)&disk_port, CMD_FIND_ALIAS_BY_PORT) != MF_SUCCESS) {
        MFinfo("[Solin] no such disk[%d] exists.\n", disk_port);
        return ret;
    }
    snprintf(cmd, sizeof(cmd), "mdadm %s -a %s", traid.dev_path, dev_path);
    MFinfo(" add raid spare disk cmd: [%s] ====== \n", cmd);
    ms_vsystem(cmd);

    if (get_raid_info(raid_port, &traid) != RAID_STATE_NOEXIST) {
        for (i = 0; i < traid.disk.cnt; i ++) {
            if (disk_port == traid.disk.dev[i].port && (traid.disk.dev[i].state == RAID_DISK_STATE_RAID \
                                                        || traid.disk.dev[i].state == RAID_DISK_STATE_SPARE)) {
                ret = MF_SUCCESS;
                break;
            }
        }
    }

    return ret;
}

static MF_S32
get_global_spare_list(struct spare_disk_list *list)
{
    int num = 0;
    unsigned long long sizep = 0;
    unsigned int super_magic = 0;
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;

    if (list == NULL) {
        MFinfo("Spare_disk_list is NULL. \n");
        return -1;
    }

    memset(list, 0, sizeof(struct spare_disk_list));

    struct list_head *head = mf_disk_get_rd_list(MF_NO);

    list_for_each_entry_safe(pos, n, head, node) {
        if (pos->enType != DISK_TYPE_LOCAL &&
            pos->enType != DISK_TYPE_GLOBAL_SPARE) { //only local disk can be used for global spare disk,  esata can not.
            continue;
        }

        MFinfo("====== get spare list, port:[%d], name:[%s, %s], type:[%d] ===== \n", pos->port, pos->alias, pos->name,
               pos->enType);

        if (get_dev_size(pos->alias, &sizep)) {
            MFerr("get dev size err. \n");
            continue;
        }

        //MFinfo("===== sizep:[%lld],i:[%d],cnt:[%d],dev:[%s] ========== \n", sizep, i, g_scsi_info.cnt, partion);

        if ((pos->enState != DISK_STATE_OFFLINE &&
             pos->enState != DISK_STATE_NONE) &&
            read_super_magic(pos->alias, &super_magic) == MF_SUCCESS) {
            if (super_magic == GLOBAL_SPARE_MAGIC) {
                strcpy(list->disk[num].dev_path, pos->alias);
                list->disk[num].port = pos->port;
                list->disk[num].size = sizep;
                num++;
                list->cnt = num;
                MFinfo("==== super_magic:[0x%x],list->cnt:[%d] ====== \n", super_magic, list->cnt);

                if (pos->enType == DISK_TYPE_LOCAL) {
                    pos->enType = DISK_TYPE_GLOBAL_SPARE;
                }
            }
        } else {
            MFerr(" ======== read super magic failed ======== \n");
        }
    }

    mf_disk_put_rw_list(head);

    return MF_SUCCESS;
}

static MF_S32
find_global_spare_disk(MF_S32 port) //now only support one disk spare
{
    unsigned long long ksize = 0;
    struct spare_disk_list list;
    int i = 0;

    if (!raid_get_min_disk_size(&port, &ksize)) {
        get_global_spare_list(&list);   //array.size<<10

        MFinfo("===== get_global_spare_list cnt:[%d] ==== \n", list.cnt);

        for (i = 0; i < list.cnt; i ++) {
            MFinfo("======i:[%d],list.disk[i].size:[%llu],ksize:[%llu]. \n", i, list.disk[i].size, ksize);
            if (list.disk[i].size >= ksize) {
                MFinfo("====== list.disk[i].port:[%d], list.cnt:[%d]. \n", list.disk[i].port, list.cnt);
                return list.disk[i].port;
            }
        }
    } else {
        MFinfo("====== get_raid_min_disk_size failed ======= \n");
    }

    return MF_FAILURE;
}

static MF_S32
get_raid_num(int *raid_port)
{
    FILE *fp = NULL;
    char line[256] = {0};
    int num = 0;

    if (!raid_port) {
        MFinfo("raid port array is NULL. \n");
        return -1;
    }

    if ((fp = fopen("/proc/mdstat", "r")) == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "md", 2) == 0) {
            if (isdigit(line[2]) && !isdigit(line[3])) { //0-9
                raid_port[num] = line[2] - '0';
                num++;
            } else if (isdigit(line[2]) && isdigit(line[3]) && !isdigit(line[4])) { //10-99
                raid_port[num] = (line[2] - '0') * 10 + (line[3] - '0');
                num++;
            }
        }
    }
    fclose(fp);

    return num;
}

void
raid_local_zero_superblock(MF_S8 *alias)
{
    MF_S8 cmd[128] = {0};

    if (!alias) {
        return ;
    }
    if (access(alias, F_OK) == 0) { //for raid delete
        snprintf(cmd, sizeof(cmd), "mdadm --misc --zero-superblock %s ", alias);
        ms_system(cmd);
    }
}

MF_S32
check_raid_status()
{
    int i = 0, res;
    int raid_num = 0;
    int disk_port = 0;
    int raid_port[16] = {0};
    char md_path[16] = {0};
    int fd = -1;
    STATE_EN temp;
    struct local_raid raid;
    mdu_bitmap_file_t bmf;
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct diskObj *pstDev = NULL;
    struct list_head *head = NULL;
    struct raid_info *raidInfo;

    if ((raid_num = get_raid_num(raid_port)) < 1) { //no raid found
        return -1;
    }

    for (i = 0; i < raid_num; i ++) {
        snprintf(md_path, sizeof(md_path), "/dev/md%d", raid_port[i]);
        fd = open(md_path, O_RDONLY);
        if (fd < 0) {
            continue;
        }
        ioctl(fd, GET_BITMAP_FILE, &bmf);
        close(fd);

        memset(&raid, 0, sizeof(raid));
        if (get_raid_info(raid_port[i], &raid) == RAID_STATE_DEGRADE && raid.task != RAID_TASK_RECOVERY) {
            MFinfo("== raid dev_path:[%s] degrade,raid.level:[%d] \n", md_path, raid.level);

            disk_port = find_global_spare_disk(raid.port);

            if (disk_port >= 0) {
                MFinfo("== disk_port:[%d]. \n", disk_port);
                res = add_raid_spare_disk(raid.port, disk_port);
                if (res == MF_SUCCESS) {
                    head = mf_disk_get_wr_list(MF_NO);
                    list_for_each_entry_safe(pos, n, head, node) {
                        if (disk_port == pos->port) {
                            mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pos, MF_YES);
                            mf_raid_update_local_inraid(pos);
                            break;
                        }
                    }
                    mf_disk_put_rw_list(head);
                }
            } else {
                MFinfo("== no global spare disk found. \n");
            }
        }

        head = mf_disk_get_rd_list(MF_NO);
        pstDev = NULL;
        list_for_each_entry_safe_reverse(pos, n, head, node) {
            if (raid_port[i] == pos->port) {
                pstDev = pos;
                break;
            }
        }
        mf_disk_put_rw_list(head);
        if (pstDev) {
            temp = pstDev->enState;
            mf_disk_attribute(pstDev);
            if ((temp != pstDev->enState) && (pstDev->enState == DISK_STATE_OFFLINE)) {
                mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pstDev, MF_YES);
            }

            if (pstDev->smartTest == SMART_WARN) {
                raidInfo = raid_get_priv_ptr(pstDev->pstPrivate);
                if (raidInfo) {
                    head = mf_disk_get_rd_list(MF_NO);
                    list_for_each_entry_safe(pos, n, head, node) {
                        for (i = 0; i < raidInfo->disk.cnt; i++) {
                            if (pos->port == raidInfo->disk.dev[i].port) {
                                if (!pos->smartTest) {
                                    pos->smartTest = mf_disk_get_disk_attr_status(pos->alias);
                                }
                            }
                        }
                    }
                    mf_disk_put_rw_list(head);
               }
            }
        }
        MFinfo("== raid_port:[%d], state:[%d], task:[%d], percent:[%d]. \n", raid_port[i], raid.state, raid.task, raid.percent);
    }
    MFinfo("== raid_port quit");

    return MF_SUCCESS;
}

MF_S32
raid_rebuild(struct diskObj *pstDev, struct raid_op_t *raidp)
{
    int res = MF_FAILURE;
    int disk_port = 0;
    int raid_ports[16] = {0};
    char md_path[16] = {0};
    int fd = -1;
    struct local_raid raid;
    mdu_bitmap_file_t bmf;
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *head = NULL;

    if ((get_raid_num(raid_ports)) < 1) { //no raid found
        MFinfo("get_raid_num fail !");
        return MF_FAILURE;
    }

    snprintf(md_path, sizeof(md_path), "/dev/md%d", raidp->raid_port);
    fd = open(md_path, O_RDONLY);
    if (fd < 0) {
        MFinfo("open fail !");
        return MF_FAILURE;
    }

    ioctl(fd, GET_BITMAP_FILE, &bmf);
    close(fd);

    memset(&raid, 0, sizeof(raid));
    if (get_raid_info(raidp->raid_port, &raid) == RAID_STATE_DEGRADE && raid.task != RAID_TASK_RECOVERY) {
        MFinfo("=== raid dev_path:[%s] degrade,raid.level:[%d] \n", md_path, raid.level);

        disk_port = (int)raidp->disk_port[0];//find_global_spare_disk(raid_port);

        if (disk_port >= 0) {
            MFinfo("===== disk_port:[%d]. \n", disk_port);
            res = add_raid_spare_disk(raidp->raid_port, disk_port);
            if (res == MF_SUCCESS) {
                head = mf_disk_get_wr_list(MF_NO);
                list_for_each_entry_safe(pos, n, head, node) {
                    if (disk_port == pos->port) {
                        mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pos, MF_YES);
                        pos->smartTest = mf_disk_get_disk_attr_status(pos->alias);
                        if (pos->smartTest == SMART_WARN) {
                            MFshow("rebuild port %d path:%s smart test is warning.", disk_port, pos->alias);
                            pstDev->smartTest = SMART_WARN;
                        }
                        mf_raid_update_local_inraid(pos);
                        break;
                    }
                }
                mf_disk_put_rw_list(head);
            }
        } else {
            MFinfo("===== no global spare disk found. \n");
        }
    } else if (raid.task == RAID_TASK_RECOVERY) {
        res = MF_SUCCESS;
    }

    MFinfo("== raid_port:[%d], state:[%d], task:[%d], percent:[%d], res:%d. \n",
           raidp->raid_port, raid.state, raid.task, raid.percent, res);

    return res;
}

MF_S32
raid_rebuild_progress(MF_S32 raid_port)
{
    int percent = 0;
    char md_path[16] = {0};
    int fd = -1;
    struct local_raid raid;

    snprintf(md_path, sizeof(md_path), "/dev/md%d", raid_port);
    fd = open(md_path, O_RDONLY);
    if (fd < 0) {
        return MF_FAILURE;
    }
    close(fd);

    memset(&raid, 0, sizeof(raid));
    if (get_raid_info(raid_port, &raid) == RAID_DISK_STATE_NOEXIST ||
        raid.state == RAID_STATE_OFFLINE || raid.state == RAID_STATE_FAIL ||
        raid.state == RAID_STATE_NOINIT) {
        return MF_FAILURE;
    }
    if (raid.task == RAID_TASK_RECOVERY) {
        //MFinfo("=== raid dev_path:[%s],raid.level:[%d],raid.percent[%d] \n", md_path, raid.level, raid.percent);
        percent = raid.percent < 0 ? 0 : raid.percent;
        percent = raid.percent > 100 ? 100 : raid.percent;
    } else {
        percent = 100;
    }

    return percent;
}

MF_S32
raid_find_member(struct assemble_info *raid_info) {

    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    int m = 0;
    int i = 0;
    int x = 0;
    int fd = -1;
    int blk_sz = 0;
    unsigned long long dsize;
    //unsigned long long sb_offset;
    MF_U8   md_port;
    MF_U8   uuid[16];

    struct list_head *head = NULL;
    struct mdp_superblock_1 *super = NULL;
    head = mf_disk_get_rd_list(MF_NO);

    list_for_each_entry_safe(pos, n, head, node) {
        if (pos->enType != DISK_TYPE_LOCAL) {
            continue;
        }

        fd = open(pos->alias, O_RDONLY);
        if (fd < 0) {
            MFerr("==get dev size,open [%s] failed. \n", pos->alias);
            continue;
        }

        if (ioctl(fd, BLKSSZGET, &blk_sz) != 0) {
            blk_sz = 512;
        }

        if (get_dev_size(pos->alias, &dsize)) {
            MFerr("get partion %s dev size err. \n", pos->alias);
            close(fd);
            continue;
        }

        dsize >>= 9;

        if (dsize < 24) {
            MFerr("%s is too small for md: size is %llu sectors.\n", pos->alias, dsize);
            close(fd);
            continue;
        }

        //sb_offset = 4*2; //1.2 version

        if (lseek64(fd, RAID_MAGIC_OFFSET, 0) < 0LL) {
            MFerr("Cannot seek to superblock on %s: %s\n", pos->alias, strerror(errno));
            close(fd);
            continue;
        }

        if (posix_memalign((void **)&super, 4096, SUPER1_SIZE) != 0) {
            MFerr("could not allocate superblock\n");
            close(fd);
            continue;
        }

        if (aread(fd, super, MAX_SB_SIZE, blk_sz) != MAX_SB_SIZE) {
            MFerr("Cannot read superblock on %s\n", pos->alias);
            ms_free(super);
            close(fd);
            continue;
        }

        if (super->magic != RAID_MAGIC) {
            ms_free(super);
            close(fd);
            continue;
        }

        char *mp = strchr(super->set_name, ':');
        if (!mp || atoi(mp + 1) == 0) {
            ms_free(super);
            close(fd);
            continue;
        }
        md_port = atoi(mp + 1);
        memcpy(uuid, super->set_uuid, sizeof(super->set_uuid));

        for (i = 0; i < m; i++) {
            if (md_port == raid_info[i].md_port) {
                for (x = 0; x < sizeof(super->set_uuid); x++) {
                    if (uuid[x] != raid_info[i].uuid[x]) {
                        break;
                    }
                }

                if (x == sizeof(super->set_uuid)) { // find already old
                    strcpy(raid_info[i].list[raid_info[i].cnt].dev_path, pos->alias);
                    raid_info[i].cnt++;
                    break;
                }
            }
        }

        if (i == m) { //new
            raid_info[m].md_port = md_port;
            memcpy(raid_info[m].uuid, uuid, sizeof(uuid));
            strcpy(raid_info[m].list[raid_info[m].cnt].dev_path, pos->alias);
            raid_info[m].cnt++;
            m++;
            raid_info[i].normalCnt = super->raid_disks;
        }

        ms_free(super);
        close(fd);
    }

    mf_disk_put_rw_list(head);
    return m;
}

MF_S32
raid_assemble(void)
{
    int i = 0;
    int m = 0;
    int k = 0;
    //unsigned long long sb_offset;
    MF_U8   md_port;
    MF_U8   uuid[16];
    char disk_names[MAX_LEN_128] = {0};
    char disk_tname[MAX_LEN_128] = {0};
    char cmd[1024] = {0};
    char md_name[16] = {0};
    struct local_raid raid;
    struct assemble_info amb_info[MAX_DISK_LOCAL_NUM];
    MF_BOOL raid_ready = MF_TRUE;
    int cnt = 60, max_check =3;

    memset(amb_info, 0, sizeof(amb_info));
    //7016
    for (i = 0; i < max_check * 60; i++) {
        if (cnt >= 60) { // 检测3次 每次间隔30s
            raid_ready = MF_TRUE;

            memset(amb_info, 0, sizeof(amb_info));
            m = raid_find_member(&amb_info);
            for (k = 0; k < m; k++) {
                if (amb_info[k].normalCnt != amb_info[k].cnt) {
                    raid_ready = MF_FALSE;
                }
            }
            if (raid_ready == MF_TRUE) {
                break;
            }
            cnt = 0;
        }
        cnt++;
        usleep(500 * 1000);
    }

    for (k = 0; k < m; k++) {
        memset(disk_names, 0, sizeof(disk_names));
        memset(md_name, 0, sizeof(md_name));
        memset(cmd, 0, sizeof(cmd));

        MFinfo("========= amb_info [%d], uuid:[0x%x,0x%x,0x%x,0x%x], md_port:[%d], disk_cnt:[%d]. \n\n", k, amb_info[k].uuid[0],
               \
               amb_info[k].uuid[1], amb_info[k].uuid[2], amb_info[k].uuid[3], amb_info[k].md_port, amb_info[k].cnt);

        for (i = 0; i < amb_info[k].cnt; i++) {
            snprintf(disk_tname, sizeof(disk_tname), "%s %s", disk_names, amb_info[k].list[i].dev_path);
            snprintf(disk_names, sizeof(disk_names), "%s", mf_trim(disk_tname));
            //mf_disk_set_head_magic(amb_info[k].list[i].dev_path, DISK_TYPE_RAID);
        }

        snprintf(md_name, sizeof(md_name), "/dev/md%d", amb_info[k].md_port);

        snprintf(cmd, sizeof(cmd), "mdadm --assemble --run %s %s ", md_name, disk_names);
        MFinfo("===== assemble:[%s] =======.\n", cmd);
        ms_system(cmd);

        //check raid assemble is success or fail.
        if (get_raid_info(amb_info[k].md_port, &raid) != RAID_STATE_NOEXIST) {
            mf_disk_raid_add(md_name);
            mf_raid_add_update_notify(amb_info[k].md_port, MSFS_EVENT_DISK_ADD);
            mf_raid_assemble_update_local(md_name);
        }
    }

    return MF_SUCCESS;
}

struct raid_info *
raid_get_priv_ptr(void *pstPriv)
{
    struct raid_priv *priv = (struct raid_priv *)pstPriv;

    return &priv->raid;
}

