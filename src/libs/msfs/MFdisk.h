/*
 * ***************************************************************
 * Filename:        MFdisk.h
 * Created at:      2017.05.10
 * Description:     msfs api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFDISK_H__
#define __MFDISK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "MFindex.h"
#include "MFrecord.h"
#include "MFretrieve.h"
#include "disk.h"

#define CMD_FIND_NAME_BY_PORT (1)
#define CMD_FIND_ALIAS_BY_PORT (2)
#define CMD_FIND_PORT_BY_NAME  (3)
#define CMD_FIND_PORT_BY_ALIAS (4)

#define RAID_OP_CREATE (1)
#define RAID_OP_DEL  (2)
#define RAID_OP_REBUILD (3)

#define UUID_KEY_BIT        (2)
#define UUID_LEN_BIT        (1)
#define UUID_PRIVATE_BIT    (0)

#define HEADER1_OFFSET(sector)    ((66 * (sector)) + (HEADER_MAINLOGLEN + HEADER_CURLOGLEN))

#define RULE_FILE_PATH "/opt/app/rules/"

typedef struct disk_header {
    MF_U32  size;
    MF_S8   magic[32];
    MF_S8   version[32];//"2017.05.20"
    MF_U32  attr;
    MF_U32  sector;
    MF_U64  capacity;
    MF_U64  mainLogOff;
    MF_U64  mainLogLen;
    MF_U64  curLogOff;
    MF_U64  curLogLen;
    MF_U32  logStatus;
    MF_U32  fileCount;
    MF_U64  perFileSize;
    MF_U64  recordOff;
    MF_U64  recordLen;
    MF_U64  index00Off;
    MF_U64  index01Off;
    MF_U64  index00Len;
    MF_U64  index01Len;
    MF_U64  index10Off;
    MF_U64  index11Off;
    MF_U64  index10Len;
    MF_U64  index11Len;
    MF_U64  extendOff;
    MF_U64  extendLen;
    MF_U32  index0Status;
    MF_U32  index1Status;
    MF_U32  restoreCounts;
    MF_U32  restoreTimes;
    MF_S8   uuid[37];
    MF_U8   reserved[223];// reserved for extra datas. fill up 512
    MF_U32  checkSum;
} disk_header;

typedef struct stscan {
    MF_S8       name[DEV_NAME_LEN];
    struct list_head node;
} stscan;

typedef struct port_grp {
    RW_EN     enRw;
    MF_U8     gId;
} port_grp;

typedef struct grp_chn {
    MF_U64      chnMask; //0-63
    MF_U64      chnMask1; //64-127
} grp_chn;

typedef enum {
    QUOTA_NORMAL    = 0UL,
    QUOTA_PIC_FULL  = (1UL << 0),
    QUOTA_VID_FULL  = (1UL << 1),
    QUOTA_ALL_FULL  = ~0UL,
} QUOTA_STATE_EN;

typedef struct chn_qta_t {
    MF_U8           chnId;
    MF_U32          vidQta;
    MF_U32          vidUsd;
    MF_U32          picQta;
    MF_U32          picUsd;
} chn_qta_t;

typedef struct mf_grp {
    MF_BOOL bEnable;
    struct port_grp pg[MAX_DISK_PORT_NUM + 1];
    struct grp_chn  gn[MAX_DISK_GROUP_NUM + 1];
} mf_grp;

typedef struct mf_qta {
    MF_BOOL bEnable;
    struct chn_qta_t cq[MAX_REC_CHN_NUM];
} mf_qta;

typedef struct load_s {
    MF_U64      capacity;
    MF_U64      free;
    MF_U8       port;
    MF_U8       group;
    RW_EN       enRw;
    TYPE_EN     enType;
    STATE_EN    enState;
    struct diskObj *pstObj;
    MF_S8       name[DEV_NAME_LEN];
    MF_S8       vendor[DEV_NAME_LEN];
    MF_S8       host[DEV_HOST_LEN];
    struct list_head node;
} load_s;

typedef struct ioReq {
    MF_U8   port;
    MF_BOOL bDone;
    off64_t fileOffset;
    MUTEX_OBJECT  *pMutex;
    MF_COND_T *pCond;
    struct list_head node;
} ioReq;

ERROR_E
mf_disk_error(MF_S32 fd, MF_U64 offset, MF_S32 count, MF_U32 sector);
MF_S32
mf_disk_format(MF_U8 port, FORMAT_EN enFormat, MF_U64 quota);
MF_S32
mf_disk_restore(MF_U8 port);
MF_S32
mf_disk_restoreEx(MF_U8 port);
MF_S32
mf_disk_attribute(struct diskObj *disk);
MF_BOOL
mf_disk_is_exist(MF_U8 port);
void
mf_bkp_put_format();
void
mf_bkp_get_format();
struct list_head *
mf_disk_get_rd_list(MF_BOOL bTry);
struct list_head *
mf_disk_get_wr_list(MF_BOOL bTry);
void
mf_disk_put_rw_list(struct list_head *list);
struct list_head *
mf_disk_get_loading_list();
void
mf_disk_put_loading_list(struct list_head *list);
struct list_head *
mf_disk_get_format_list();
void
mf_disk_put_format_list(struct list_head *list);
MF_BOOL
mf_disk_can_write(struct diskObj *disk, MF_U8 chnId);
void
mf_disk_port_permission(MF_U8 port, RW_EN enRw);
void
mf_disk_index_dump(MF_U8 portId);
void
mf_disk_file_dump(MF_U8 portId, MF_U8 chnid);
void
mf_disk_port_bind_group(MF_U8 port, MF_U8 group);
void
mf_disk_chn_bind_group(MF_U64 chnMask, MF_U8 group);
void
mf_disk_group_set_enable(MF_BOOL bEnable);
void
mf_disk_quota_set_enable(MF_BOOL bEnable);
MF_BOOL
mf_disk_quota_get_status(void);
void
mf_disk_qta_grp_info(struct mf_grp **grp, struct mf_qta **qta);
MF_S32
mf_disk_set_chn_quota(MF_U8 chnId, MF_U32 vidQta, MF_U32 picQta);
MF_S32
mf_disk_get_chn_quota(MF_U8 chnId, quota_info_t *pstQta);
void
mf_disk_qta_chn_refresh(MF_U8 chnId, struct list_head *list);
void
mf_disk_set_loop(MF_BOOL bLoop);
void
mf_disk_go_private(MF_U8 port, MF_BOOL bEnable);
void
mf_disk_set_esata_usage(MF_BOOL brec);
MF_S32
mf_disk_nas_add(MF_S8 *name, MF_U8 port, MF_S8 *host, MF_S32 type, MF_S8 *user, MF_S8 *password);
void
mf_disk_bad_reload(struct list_head *head);
MF_S32
mf_disk_nas_del(MF_U8 port);
MF_S32
mf_disk_nas_rename(MF_S8 *name, MF_U8 port);
MF_U32
mf_disk_nas_get_dir(MF_S8 *hostIp, struct list_head *list);
void
mf_disk_nas_put_dir(struct list_head *list);
MF_S32
mf_disk_nand_add(MF_S8 *name, MF_U8 port, MF_S8 *host);
MF_S32
mf_disk_nand_del(MF_U8 port);
void
mf_disk_event_get();
void
mf_disk_event_cb(MODULE_E from, EVENT_E event, void *argv);
void
mf_disk_event_notify(EVENT_E event, void *argv, MF_BOOL bBlock);
void
mf_disk_dbg_switch(MF_BOOL bDebug);
void
mf_disk_dbg_show_mem_usage();
void
mf_disk_init(DISK_USER_CB user_cb, disk_param *para);
void
mf_disk_deinit();
MF_BOOL
mf_disk_Udisk_protected(struct diskObj *pdisk);
MF_S32
mf_disk_get_Udisk(MF_S8 *mnt);
void
mf_disk_put_Udisk(MF_S8 *mnt);
MF_S32
mf_find_name_port(MF_S8 *name, MF_U8 *portp, MF_U8 cmd_type);
MF_S32
mf_raid_update_local_inraid(struct diskObj *pos);
void
mf_raid_add_update_notify(MF_U8 port, EVENT_E event);
void
mf_raid_fault_update_disk(MF_S8 *name);
void
mf_raid_info_add(MF_U8 port, MF_S8 *vendor);
void
mf_raid_info_get(struct diskObj *pstObj);
void
mf_raid_info_dels();
MF_S32
mf_raid_rename(MF_S8 *name, MF_U8 port);
void
mf_raid_deal_create_failed(struct raid_op_t *raid);
MF_S32
mf_raid_create(struct raid_op_t *raid);
MF_S32
mf_raid_del(int raid_port);
MF_S32
mf_raid_rebuild(struct raid_op_t *raid);
void
mf_get_component_size_raid(MF_S32 *raidport, MF_U64 *rsize);
MF_S32
mf_raid_rebuild_progress(MF_S32 raid_port);
MF_S32
mf_disk_raid_create(MF_S8 *name);
MF_S32
mf_disk_raid_add(MF_S8 *name);
MF_S32
mf_disk_raid_del(struct diskObj *pstDev);
void
mf_disk_set_raid_mode(MF_BOOL bmode);
MF_BOOL
mf_disk_get_raid_mode();
MF_S32
mf_raid_create_update_local(raid_op_t *raid);
void
mf_raid_assemble_update_local(MF_S8 *name);
void
mf_check_raid_status();
void
mf_disk_usb_get_mntpath(MF_U8 port, MF_S8 *mntpath);
//MF_S32
//mf_scan_raid(void);
void
mf_disk_notify_bar(PROGRESS_BAR_E enBar, MF_U8 port, MF_S32 percent, TYPE_EN type);
MF_S32
mf_disk_set_head_info(MF_S8 *devname);
struct raid_info *
mf_disk_get_raid_priv(void *pstPriv);
MF_S32
mf_write_super_magic(char *devname, MF_U32 magic_num);
MF_S32
mf_read_super_magic(char *devname, MF_U32 *magic_num);
MF_S32
mf_create_global_spare(MF_S32 disk_port);
MF_S32
mf_remove_global_spare(MF_S32 disk_port);
MF_S32
mf_disk_io_status(int fd);
void
mf_disk_io_request(struct ioReq *pstIOReq);
void
mf_disk_scan(void);
MF_S32
mf_disk_dev_loading(struct diskObj *pstDev, MF_BOOL);
MF_BOOL
mf_disk_is_ready();
void
mf_disk_ready_notify();
MF_S32
mf_disk_op_smart(MF_S8 *pdevpath, HDD_ATTR *attr, int test_type);
MF_S32
mf_disk_get_smart_attr(HDD_ATTR *attr, MF_U8 port);
MF_S32
mf_disk_del_smart_attr_list(HDD_ATTR *attr);

DISK_SMART_RESULT
mf_disk_get_disk_attr_status(MF_S8 *pdevpath);

void 
mf_disk_set_health_temperature(MF_U8 portId, MF_S32 temperature);

void
mf_disk_set_health_status(MF_U8 portId, MF_S32 status);

void
mf_disk_set_health_enable(MF_U64 enableMask);

MF_S8 *
mf_dev_mmac();
MF_S8 *
mf_dev_model();

MF_BOOL
mf_disk_is_can_record();


#ifdef __cplusplus
}
#endif

#endif

