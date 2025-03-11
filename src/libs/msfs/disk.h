#ifndef __DISK_H__
#define __DISK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"
#include "bplustree.h"
#include "msfs_disk.h"
#include "msfs_notify.h"
#include "msfs_rec.h"


/*
* file data structure(1Gb)
*                 data ---->                       <----info
*  start   |-----AVdata---|--infoN~info0|--infoN~info0|segment0~segment1023--|Tag-resverve(5MB)--|fileHeader(512B)--|  end
*/

//#define DEBUG_REC

#define FILE_RESERVE_SIZE   (3*1024*1024)
#ifdef DEBUG_REC
#define FILE_PER_SIZE   (100*1024*1024)
#else
#define FILE_PER_SIZE   (1*1024*1024*1024)
#endif

#define FILE_HEAD_MAX_SIZE  (32*1024)

#define SEG_MAX_NUM (1024)
#define TAG_MAX_NUM (64)
#define SEG_HEAD_SIZE  (ALIGN_UP(sizeof(struct mf_segment), 512))
#define TAG_HEAD_SIZE  (ALIGN_UP(sizeof(struct mf_tag), 512))

#define SEG_HEAD_MAX_SIZE  (SEG_HEAD_SIZE * SEG_MAX_NUM)
#define TAG_HEAD_MAX_SIZE  (TAG_HEAD_SIZE * TAG_MAX_NUM)

#define FILE_HEAD_OFFSET (FILE_PER_SIZE - FILE_HEAD_MAX_SIZE)
#define SEG_HEAD_OFFSET  (FILE_HEAD_OFFSET - FILE_RESERVE_SIZE - SEG_HEAD_MAX_SIZE)
#define TAG_HEAD_OFFSET  (FILE_HEAD_OFFSET - TAG_HEAD_MAX_SIZE)

#define INFO_START_OFFSET  (SEG_HEAD_OFFSET)
#define INFO_DATA_MAX_SIZE (ALIGN_UP(sizeof(struct mf_info), 512))
#define INFO_SEG_MAX_SIZE (INFO_DATA_MAX_SIZE * 1024)
#define FRAME_HEAD_MAX_SIZE (ALIGN_UP(sizeof(struct mf_frame), 512))
#define FRAME_DATA_MAX_SIZE(size) (ALIGN_UP(size, 512))
#define FRAME_MAX_SIZE(size) (ALIGN_UP(sizeof(struct mf_info) + size ,512))

#define RAID_MAGIC_OFFSET  (8 << 9)

#define GLOBAL_SPARE_MAGIC (0x12345678)
#define RAID_MAGIC         (0xa92b4efc)
#define FILE_MAGIC         (0x19900808)
#define INVALID_MAGIC      (0)

#define DISK_LOAD_NUM_MAX        (4)

typedef enum {
    DISK_ERR_UNKNOWN = -1,
    DISK_ERR_READ_ONLY = -2,
    DISK_ERR_GROUP = -3,
    DISK_ERR_IO = -4,
    DISK_ERR_NO_SPACE = -5,
    DISK_ERR_BADBLOCK = -6,
    DISK_ERR_DEV_BAD = -7,
    DISK_ERR_QUOTA_FULL = -8,
} ERROR_E;

//// for disk error read/write from driver libata.h ////
enum {
    /* ATA device commands */
    ATA_CMD_DEV_RESET   = 0x08, /* ATAPI device reset */
    ATA_CMD_CHK_POWER   = 0xE5, /* check power mode */
    ATA_CMD_STANDBY     = 0xE2, /* place in standby power mode */
    ATA_CMD_IDLE        = 0xE3, /* place in idle power mode */
    ATA_CMD_EDD     = 0x90, /* execute device diagnostic */
    ATA_CMD_DOWNLOAD_MICRO  = 0x92,
    //ATA_CMD_NOP       = 0x00, /* driver shield ms_disk_drv.c */
    ATA_CMD_FLUSH       = 0xE7,
    ATA_CMD_FLUSH_EXT   = 0xEA,
    ATA_CMD_ID_ATA      = 0xEC,
    ATA_CMD_ID_ATAPI    = 0xA1,
    ATA_CMD_SERVICE     = 0xA2,
    ATA_CMD_READ        = 0xC8,
    ATA_CMD_READ_EXT    = 0x25,
    ATA_CMD_READ_QUEUED = 0x26,
    ATA_CMD_READ_STREAM_EXT = 0x2B,
    ATA_CMD_READ_STREAM_DMA_EXT = 0x2A,
    ATA_CMD_WRITE       = 0xCA,
    ATA_CMD_WRITE_EXT   = 0x35,
    ATA_CMD_WRITE_QUEUED    = 0x36,
    ATA_CMD_WRITE_STREAM_EXT = 0x3B,
    ATA_CMD_WRITE_STREAM_DMA_EXT = 0x3A,
    ATA_CMD_WRITE_FUA_EXT   = 0x3D,
    ATA_CMD_WRITE_QUEUED_FUA_EXT = 0x3E,
    ATA_CMD_FPDMA_READ  = 0x60,
    ATA_CMD_FPDMA_WRITE = 0x61,
    ATA_CMD_PIO_READ    = 0x20,
    ATA_CMD_PIO_READ_EXT    = 0x24,
    ATA_CMD_PIO_WRITE   = 0x30,
    ATA_CMD_PIO_WRITE_EXT   = 0x34,
    ATA_CMD_READ_MULTI  = 0xC4,
    ATA_CMD_READ_MULTI_EXT  = 0x29,
    ATA_CMD_WRITE_MULTI = 0xC5,
    ATA_CMD_WRITE_MULTI_EXT = 0x39,
    ATA_CMD_WRITE_MULTI_FUA_EXT = 0xCE,
    ATA_CMD_SET_FEATURES    = 0xEF,
    ATA_CMD_SET_MULTI   = 0xC6,
    ATA_CMD_PACKET      = 0xA0,
    ATA_CMD_VERIFY      = 0x40,
    ATA_CMD_VERIFY_EXT  = 0x42,
    ATA_CMD_WRITE_UNCORR_EXT = 0x45,
    ATA_CMD_STANDBYNOW1 = 0xE0,
    ATA_CMD_IDLEIMMEDIATE   = 0xE1,
    ATA_CMD_SLEEP       = 0xE6,
    ATA_CMD_INIT_DEV_PARAMS = 0x91,
    ATA_CMD_READ_NATIVE_MAX = 0xF8,
    ATA_CMD_READ_NATIVE_MAX_EXT = 0x27,
    ATA_CMD_SET_MAX     = 0xF9,
    ATA_CMD_SET_MAX_EXT = 0x37,
    ATA_CMD_READ_LOG_EXT    = 0x2F,
    ATA_CMD_WRITE_LOG_EXT   = 0x3F,
    ATA_CMD_READ_LOG_DMA_EXT = 0x47,
    ATA_CMD_WRITE_LOG_DMA_EXT = 0x57,
    ATA_CMD_TRUSTED_RCV = 0x5C,
    ATA_CMD_TRUSTED_RCV_DMA = 0x5D,
    ATA_CMD_TRUSTED_SND = 0x5E,
    ATA_CMD_TRUSTED_SND_DMA = 0x5F,
    ATA_CMD_PMP_READ    = 0xE4,
    ATA_CMD_PMP_WRITE   = 0xE8,
    ATA_CMD_CONF_OVERLAY    = 0xB1,
    ATA_CMD_SEC_SET_PASS    = 0xF1,
    ATA_CMD_SEC_UNLOCK  = 0xF2,
    ATA_CMD_SEC_ERASE_PREP  = 0xF3,
    ATA_CMD_SEC_ERASE_UNIT  = 0xF4,
    ATA_CMD_SEC_FREEZE_LOCK = 0xF5,
    ATA_CMD_SEC_DISABLE_PASS = 0xF6,
    ATA_CMD_CONFIG_STREAM   = 0x51,
    ATA_CMD_SMART       = 0xB0,
    ATA_CMD_MEDIA_LOCK  = 0xDE,
    ATA_CMD_MEDIA_UNLOCK    = 0xDF,
    ATA_CMD_DSM     = 0x06,
    ATA_CMD_CHK_MED_CRD_TYP = 0xD1,
    ATA_CMD_CFA_REQ_EXT_ERR = 0x03,
    ATA_CMD_CFA_WRITE_NE    = 0x38,
    ATA_CMD_CFA_TRANS_SECT  = 0x87,
    ATA_CMD_CFA_ERASE   = 0xC0,
    ATA_CMD_CFA_WRITE_MULT_NE = 0xCD,
    /* marked obsolete in the ATA/ATAPI-7 spec */
    ATA_CMD_RESTORE     = 0x10,
    /* READ/WRITE LONG (obsolete) */
    ATA_CMD_READ_LONG   = 0x22,
    ATA_CMD_READ_LONG_ONCE  = 0x23,
    ATA_CMD_WRITE_LONG  = 0x32,
    ATA_CMD_WRITE_LONG_ONCE = 0x33,
};//for disk err check

enum ata_feature_error {
    /* for ata res_feature with ms_disk_drv.h */
    ATA_ICRC        = (1 << 7), /* interface CRC error */
    ATA_UNC         = (1 << 6), /* uncorrectable media error */
    ATA_IDNF        = (1 << 4), /* ID not found */
    ATA_ABORTED     = (1 << 2), /* command aborted */

    /* for ata res_cmd with ms_disk_drv.h*/
    ATA_BUSY        = (1 << 7), /* BSY status bit */
    ATA_DRDY        = (1 << 6), /* device ready */
    ATA_DF          = (1 << 5), /* device fault */
    ATA_DRQ         = (1 << 3), /* data request i/o */
    ATA_ERR         = (1 << 0), /* have an error */
};

enum ata_completion_errors {
    AC_ERR_DEV      = (1 << 0), /* device reported error */
    AC_ERR_HSM      = (1 << 1), /* host state machine violation */
    AC_ERR_TIMEOUT      = (1 << 2), /* timeout */
    AC_ERR_MEDIA        = (1 << 3), /* media error */
    AC_ERR_ATA_BUS      = (1 << 4), /* ATA bus error */
    AC_ERR_HOST_BUS     = (1 << 5), /* host bus error */
    AC_ERR_SYSTEM       = (1 << 6), /* system error */
    AC_ERR_INVALID      = (1 << 7), /* invalid argument */
    AC_ERR_OTHER        = (1 << 8), /* unknown */
    AC_ERR_NODEV_HINT   = (1 << 9), /* polling device detection hint */
    AC_ERR_NCQ      = (1 << 10), /* marker for offending NCQ qc */
};

typedef struct disk_temperature_data{
    time_t timestamp;
    int temperature;
} disk_temperature_data;

typedef struct diskObj {
    MF_BOOL     bPrivate; // noly for ?
    MF_BOOL     bLoop;  // loop record ?
    MF_BOOL     bRec;   // support record ?
    MF_BOOL     bRetr;  // retrieve index ready ?
    MF_BOOL     bExit;  // exitting... ? -> loading
    MF_BOOL     bFmtExit; // exiting -> format
    MF_BOOL     bLastFull;
    MF_U8       port;
    MF_U8       group;
    RW_EN       enRw;
    TYPE_EN     enType;
    STATE_EN    enState;
    STATE_EN    enLastState;
    MF_U32      sector;
    MF_U64      capacity;
    MF_U64      free;
    MF_S32      fileNum;
    MF_PTS      startTime;
    MF_PTS      endTime;
    MF_S8       name[DEV_NAME_LEN];
    MF_S8       alias[DEV_NAME_LEN];
    MF_S8       vendor[DEV_NAME_LEN];
    MF_S8       mnt[DEV_NAME_LEN];
    MF_S8       host[DEV_HOST_LEN];
    void       *pstPrivate;
    void       *loading;
    MF_S32      loadNum;
    MF_S32      ipcNum;
    struct file_info   *pstFirstUsed;
    struct list_head   ipcHead;
    struct disk_ops *ops;
    struct disk_header *pstHeader;
//    struct disk_file   *pstFile;
    struct bplus_tree *tree;
    struct list_head node;
    MUTEX_OBJECT mutex;
    MUTEX_OBJECT infoMutex;
    MUTEX_OBJECT fmtMutex;
    MUTEX_OBJECT opsMutex;
    MUTEX_OBJECT waitMutex;
    MF_COND_T    waitCond;
    DISK_SMART_RESULT smartTest;

    MF_U32 streamType;          //码流类型 bit @FILE_TYPE_EN
    REC_EVENT_EN recType;       /*主码流*/ 
    STID_MAJOR_EN majorType;
    INFO_MINOR_EN minorType;
    REC_EVENT_EN recSubType;    /*次码流*/  
    STID_MAJOR_EN majorSubType;
    INFO_MINOR_EN minorSubType;
//disk health management
    MF_S8       wwn[17]; //World Wide Name
    MF_BOOL     supportHM;//
    MF_S32      avgTemperature;
    MF_S32      HMStatus;//-1 other error;0 or 116 health ;101 电缆;102 or 105 震动 
    MF_BOOL     bReadyHM;//0 not get wwn ;1 ready
    struct disk_temperature_data    logTemperature[MAX_DISK_HM_TEMPERATURE];
    MF_S32      lastTemperatureIndex; //max 719
} diskObj;

typedef struct disk_ops {
    MF_S32(*disk_format)(void *priv, void *argv, MF_U64 quota);
    MF_S32(*disk_mount)(void *priv);
    MF_S32(*disk_umount)(void *priv);
    MF_S32(*disk_size)(void *priv);
    MF_S32(*disk_vendor)(void *priv);
    MF_S32(*disk_status)(void *priv);
    MF_S32(*disk_file_open)(void *priv, MF_S32 fileNo, MF_BOOL bAIO);
    MF_S32(*disk_file_close)(void *priv, MF_S32 fd);
    MF_S32(*disk_file_read)(void *priv, MF_S32 fd, void *buf, MF_S32 count, off64_t offset);
    MF_S32(*disk_file_write)(void *priv, MF_S32 fd, const void *buf, MF_S32 count, off64_t offset);
    MF_S32(*disk_log_fomat)(void *disk);
    MF_S32(*disk_log_init)(void *disk);
    MF_S32(*disk_index_backup)(void *index);
    MF_S32(*disk_index_recover)(void *index);
    MF_S32(*disk_index_restore)(void *priv);
    MF_S32(*disk_go_private)(void *priv, MF_BOOL bEnable);
    MF_S32(*disk_index_restoreEx)(void *priv);
} disk_ops;

MF_S32
disk_local_register(struct diskObj *pstDiskObj, MF_S8 *host);
MF_S32
disk_local_unregister(struct diskObj *pstDiskObj);
MF_S32
disk_raid_register(struct diskObj *pstDiskObj, MF_S8 *host);
MF_S32
disk_raid_unregister(struct diskObj *pstDiskObj);
MF_S32
disk_usb_register(struct diskObj *pstDiskObj, MF_S8 *host);
MF_S32
disk_usb_unregister(struct diskObj *pstDiskObj);
MF_S32
disk_nas_register(struct diskObj *pstDiskObj, MF_S8 *host, void *params);
MF_S32
disk_nas_unregister(struct diskObj *pstDiskObj);
MF_S32
disk_esata_register(struct diskObj *pstDiskObj, MF_S8 *host);
MF_S32
disk_esata_unregister(struct diskObj *pstDiskObj);
MF_S32
disk_nand_register(struct diskObj *pstDiskObj, MF_S8 *host);
MF_S32
disk_nand_unregister(struct diskObj *pstDiskObj);
MF_S32
local_reset_allhead_info(struct diskObj *pstDev);
MF_S32
raid_create(struct raid_op_t *raid);
MF_S32
raid_del(struct diskObj *pstDev);
MF_S32
raid_rebuild(struct diskObj *pstDev, struct raid_op_t *raid);
MF_S32
raid_rebuild_progress(MF_S32 raid_port);
void
raid_local_zero_superblock(MF_S8 *alias);
MF_S32
raid_get_min_disk_size(MF_S32 *port, unsigned long long *sizep); //get raid min disk size
MF_S32
raid_assemble(void);
struct raid_info *
raid_get_priv_ptr(void *pstPriv);
MF_S32
write_super_magic(char *devname, MF_U32 magic_num);
MF_S32
read_super_magic(char *devname, MF_U32 *magic_num);
MF_S32
raid_set_disk_header_magic(void *pstPrivate);
MF_S32
create_global_spare(MF_S32 disk_port, MF_S8 *devname);
MF_S32
remove_global_spare(int disk_port, MF_S8 *devname);
MF_S32
check_raid_status();


#ifdef __cplusplus
}
#endif

#endif

