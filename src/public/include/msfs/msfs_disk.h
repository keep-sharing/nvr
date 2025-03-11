/*
 * ***************************************************************
 * Filename:        msfs_disk.h
 * Created at:      2017.10.10
 * Description:     msfs disk api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MSFS_DISK_H__
#define __MSFS_DISK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"
#include "msdefs.h"
#include "msfs_notify.h"

#define DEV_NAME_LEN    (64)
#define DEV_HOST_LEN    (256)
#define DISK_MAGIC      "MSFS@MILESIGHT"     //msfs  disk header magic 
#define DISK_VERSION    "2018.08.21"
#define RPCMNT_NAMELEN  (255)
#define ALL_CHNID       (255)
#define RAID_DISK_MAGIC "RAID@MILESIGHT"    //msfs raid disk header magic, this disk status will change to be  unformatted


#define HEADER_MAINLOGLEN    (96 * 1024 * 1024) // 96M   //old:60M
#define HEADER_CURLOGLEN     (34 * 1024 * 1024) // 34M   //old:15M
#define HEADER_NANDLOG_SIZE  (1  * 1024 * 1024)


//#define RAID_SPARE_MAGIC
#define MAX_DISK_GROUP_NUM  (16)     //max group
#define MAX_DISK_LOCAL_NUM  (16)     //max  local disk num,  hard disk , local hard disk port from 1-16
#define MAX_DISK_NET_NUM    (8)      //nas  ipsan etc..., net disk port from 17-24
#define MAX_DISK_ESATA_NUM  (1)      //esata device, esata disk port from 25
#define MAX_DISK_RAID       (8)      // max raid num, raid disk port from 26-33
#define MAX_DISK_USB_NUM    (4)      //usb storage  backup, usb disk port form 34-37
#define MAX_DISK_NAND_NUM   (2)      //usb storage  backup, usb disk port form 38-39
#define MAX_DISK_NUM        (MAX_DISK_LOCAL_NUM+MAX_DISK_NET_NUM+MAX_DISK_ESATA_NUM+MAX_DISK_RAID+MAX_DISK_USB_NUM+MAX_DISK_NAND_NUM)
#define MAX_DISK_PORT_NUM   MAX_DISK_NUM
#define MAX_DB_DISK_NUM     (MAX_DISK_LOCAL_NUM+MAX_DISK_NET_NUM+MAX_DISK_ESATA_NUM+MAX_DISK_RAID)

#define DISK_NAND_START_PORT (MAX_DISK_LOCAL_NUM+MAX_DISK_NET_NUM+MAX_DISK_ESATA_NUM+MAX_DISK_RAID+MAX_DISK_USB_NUM+1)

#define MAX_DISK_HM_TEMPERATURE (720)

typedef enum DISK_ERR_CODE_E {
    MF_ERR_DISK_BUSY = 1,
    /* new added */
    ERR_DISK_BUTT

} DISK_ERR_CODE_E;

/* System define error code */
#define MF_ERR_DISK_BUSY                HI_DEF_ERR(MF_MOD_ID_DISK, MF_ERR_LEVEL_ERROR, MF_ERR_DISK_BUSY)

typedef enum {  // raid 0 1 5 10 supported,  raid 6 will  consider
    RAID_0 = 0,
    RAID_1 = 1,
    RAID_5 = 5,
    RAID_6 = 6,
    RAID_10 = 10,
} MD_LEVEL_EN;

typedef enum {     // whole raid array
    RAID_STATE_NONE,
    RAID_STATE_NOEXIST = 1,   // no raid exist
    RAID_STATE_OFFLINE,       // raid exist, but can not be used
    RAID_STATE_ACTIVE,        // raid is OK
    RAID_STATE_DEGRADE,       // raid can be used, but degrade
    RAID_STATE_RESYNC,        // raid resyncing
    RAID_STATE_RECOVERY,      // raid reciverying
    RAID_STATE_NOINIT,        // raid UNFORMATTED
    RAID_STATE_BUSY,          // raid is busy, this operate can be failed.
    RAID_STATE_FAIL,            // raid operate be failed.
} MD_STATE_EN;

typedef enum { //signle disk stat in raid array
    RAID_DISK_STATE_NOEXIST = 1,   // disk no exist in system
    RAID_DISK_STATE_SATA,          // disk not be used for raid array
    RAID_DISK_STATE_RAID,          // disk used for a raid array
    RAID_DISK_STATE_SPARE,         // disk used for a raid array, but only for spare/resync/recovery function
    RAID_DISK_STATE_FAULTY,        // disk exist in a raid array, but has error, can not be used.
    RAID_DISK_STATE_REMOVE,        // disk has be removed
} MD_DISK_STATE_EN;

typedef enum { // raid array task
    RAID_NO_TASK = 0,    // raid is normal,  can recording,  no fault, no resync, no recovery...
    RAID_TASK_RECOVERY, //raid is degrade or fault, recoverying.   Recoverying time is too long.
} MD_TASK_EN;

typedef enum { //single spare disk in a raid array
    SPACE_FAILED = -1,           //spare disk can not be used
    SPACE_OK = 0,                 //spare disk is normal,  can be used for recovery
    SPARE_IN_USE,               //spare disk is used for recoverying
} SPARE_STATE_EN;

typedef enum {
    DISK_R_ONLY = 0,    //disk only be read
    DISK_RW,           //disk can be read and write
} RW_EN;

typedef enum {     //supported disk type
    DISK_TYPE_UNKNOWN = 0,   //
    DISK_TYPE_LOCAL,         //local hard disk
    DISK_TYPE_USB,            //USB storge device
    DISK_TYPE_NAS,          //NAS network storge
    DISK_TYPE_RAID,         //RAID array
    DISK_TYPE_ESATA,
    DISK_TYPE_IPSAN,       //
    DISK_TYPE_GLOBAL_SPARE, //disk is used for global spare disk when raid degrade
    DISK_TYPE_IN_RAID,
    DISK_TYPE_NAND,        //nand flash
    DISK_TYPE_CIFS,
    DISK_TYPE_NONE,
} TYPE_EN;

typedef enum {   //disk status
    DISK_STATE_UNFORMATTED = 0,   //disk no be format
    DISK_STATE_NORMAL,          //disk is OK, has be formatted
    DISK_STATE_OFFLINE,          //disk is not exist
    DISK_STATE_FORMATING,        //disk is formatting
    DISK_STATE_BAD,              //disk has error,  may be can not be used.
    DISK_STATE_LOADING,          //loading ...
    DISK_STATE_NONE,
} STATE_EN;

// format type for disk
typedef enum {
    FORMAT_TYPE_NONE = 0,
    FORMAT_TYPE_MS = 1,
    FORMAT_EXT3_4_TYPE = 0xEF53,
    FORMAT_FAT32_TYPE = 0x4d44,        // fat32
    FORMAT_NTFS_TYPE = 0x5346544e,     // ntfs-3g for ntfs rw
    FORMAT_NTFS_FUSE_TYPE = 0x65735546,// exfat
} FORMAT_EN;

typedef enum {
    DISK_MODE_GROUP_OFF = 0,
    DISK_MODE_GROUP_ON,
    DISK_MODE_QUOTA_OFF,
    DISK_MODE_QUOTA_ON,
} MODE_EN;

typedef enum smart_test_status {
    STATUS_GOOD,    //Good
    STATUS_WARN,    //Warning
    STATUS_BAD, //Bad
    STATUS_FAIL
} SMART_TEST_STATUS;

typedef enum {
    SMART_NONE  = 0,
    SMART_PASS  = 1,
    SMART_WARN  = 2
} DISK_SMART_RESULT;
    

typedef void (* DISK_USER_CB)(EVENT_E, MF_U8);
typedef int (*DISK_NAS_CB)(MF_S8 *client);

//>>>>>>>>>>>>> smartctl disk
typedef struct smart_node {
    char name[30];
    char type[16];
    char update[16];
    char failed[16];
    char raw_val[24];
    int id;
    int flag;
    int val;
    int worst;
    int thresh;
    int status;     //lzm add
    struct list_head node;
} SMART_NODE;

typedef struct smart_attr {
    SMART_NODE *node;
    //struct smart_attr *next;
    int cnt;
} SMART_LIST;

typedef struct bblk_map_mem {
    char msg[128]; ///< check process message
    char res[128]; ///< bad block check result
    unsigned long long cur_blk; ///< current block
    unsigned long long tot_blk; ///< total blocks
    unsigned int err_read; ///< read error times
    unsigned int err_write; ///< write error times
    unsigned int err_corrupt; ///< corrupt error times
    unsigned int bb_count; ///< bad blocks
} BBLK_MAP_MEM;

typedef BBLK_MAP_MEM BBLK_ATTR;

typedef struct hdd_attr {
    struct list_head smartlist;///< smart attribute list
    int count; /// amount of smartlist node
    BBLK_ATTR blk; ///< bad block process information
    long uptime;///< smart used time
    int pass;///< smart pass flag
    int temp;///< smart temperature
    int status; //< smart test status
    int process; ///< smart test process
    //char devpath[16];///< device path
    //char mntpath[16]; ///< mounted path
    char res[16];///< smart test result
    char bblk_path[64];
} HDD_ATTR;

typedef enum smart_test_type {
    TEST_SHORT = 0, ///< brief test
    TEST_LONG, ///< detail test
    TEST_RES, ///< test result
    TEST_STATUS, ///< test status
    TEST_PROCESS, ///< test process
    TEST_STOP ///< stop test
} SMART_TEST_TYPE;

#define TEST_FAILED_STR "FAILED"
#define TEST_PASSED_STR "PASSED"

//<<<<<<<<<<<<<<<


//****  raid output info ****************/
struct raid_disk_info {   //single disk info in a raid array
    MF_U8  port;            //disk port  num in a raid array
    MF_S8 dev_path[16];     //disk name, /dev/sda  /dev/sdb .....
    MD_DISK_STATE_EN state; //faulty spare normal no_raid
};

struct raid_disk_list {  //all disks info in a raid array
    struct raid_disk_info dev[MAX_DISK_LOCAL_NUM];  ////all disks info in a raid array
    MF_S32 cnt;             //all disk nums in a raid array
};

struct raid_info {       //a raid array info
    MD_LEVEL_EN level;             //raid level 0 1 5 10
    MD_STATE_EN state;            //raid array stat
    MD_TASK_EN  task;             //raid array task, recovery...
    MF_S32 percent;         //raid recoverying process
    MF_U64 min_hdd_size; //add min hdd size for raid rebuild.
    struct raid_disk_list disk;  //all disks info in this raid arrsy
};
//******* raid output info END ********/

//****  raid input  info ****************/
typedef struct raid_op_t {
    MF_S8 raid_vendor[DEV_NAME_LEN];
    MF_U8 raid_port;
    MF_U8 raid_level;
    MF_U8 disk_num;
    MF_U8 disk_port[MAX_DISK_LOCAL_NUM];
} raid_op_t, REQ_RAID_T;
//****  raid input  info  END****************/

typedef struct disk_t {
    MF_BOOL     bLoop;
    MF_BOOL     bPrivate;
    MF_S32      port;    //local disk, nas, raid...  port num
    MF_S32      group;
    RW_EN       enRw;
    TYPE_EN     enType;
    STATE_EN    enState;
    MF_S32      busable;
    MF_U64      capacity;
    MF_U64      free;
    MF_S8       name[DEV_NAME_LEN];  // /dev/MF_local_xx /dev/MF_NAS_xx /dev/mdxx
    MF_S8       vendor[DEV_NAME_LEN];  //hard disk vendor info, raid, nas no vendor info, use special info instead
    MF_S8       host[DEV_HOST_LEN];  // for NAS host info,  192.168.8.100:/dir
    struct raid_info    raid;  //this raid private  data
    DISK_SMART_RESULT   smartTest;
} disk_t;

/*
* all initial data-metas in here for mod_disk
*/
typedef struct disk_param {
    MF_S8 chip_info[DEV_HOST_LEN];
    MF_S8 dev_model[MAX_LEN_32];
    MF_S8 dev_mmac[MAX_LEN_64];
} disk_param;

typedef struct port_info_t {
    MF_U8       id;
    RW_EN       enRw;
    MF_U8       group;
} port_info_t;

typedef struct group_info_t {
    MF_U8      id;
    MF_U64     chnMaskl;
    MF_U64     chnMaskh;
} group_info_t;

typedef struct quota_info_t {
    MF_S8   chnId;
    /* set val */
    MF_U32  vidQta;
    MF_U32  picQta;
    /* return val */
    MF_U32  vidUsd;
    MF_U32  picUsd;
} quota_info_t;

struct req_quota {
    int enable;
    int cnt;
    quota_info_t quota_conf[MAX_REAL_CAMERA];
};

typedef struct NasInfoStr{
	MF_U32	mounttype;
	MF_S8 	user[64]; 
	MF_S8 	password[64];
} NasInfoStr;

typedef struct nas_dir_t {  //nas directory
    MF_S8   dirName[RPCMNT_NAMELEN];  //directory name
    struct list_head node;   //nas info
} nas_dir_t;

typedef struct disk_nas_t { //a hostIP nas info
    MF_U32 dirNum;            //dircectory nums in this hostIP
    struct list_head list;  //nas info in this hostIP
} disk_nas_t;

typedef struct disk_info_t {
    struct disk_t astDisk;
    struct list_head node;
} disk_info_t;

typedef struct disk_attr_t {
    MF_U8 diskNum;
    struct list_head list;
} disk_attr_t;

typedef struct disk_port_vendor_t {
    MF_U8 port;
    MF_S8 vendor[DEV_NAME_LEN];
    struct list_head node;
} disk_port_vendor_t;

typedef struct disk_port_t {
    MF_U8 port;
    struct list_head node;
} disk_port_t;

typedef struct disk_temperature{
    time_t timestamp;
    int temperature;
} disk_temperature;

typedef struct disk_health_data_t{
    MF_S32                  port;
    MF_S8                   vendor[DEV_NAME_LEN];
    MF_BOOL                 supportHM; //
    MF_S32                  HMStatus;  //-1 other error;0 or 116 health ;101 电缆;105 震动
    MF_S32                  temperatureStatus;//0:normal 1:high 2 :low
    struct disk_temperature temperatureList[MAX_DISK_HM_TEMPERATURE];
    MF_BOOL                 HMEnable;
    MF_BOOL                 hasDisk;
} disk_health_data_t;

/**
 * \brief get string for type.
 * \param[in] TYPE_EN
 * \return char *.
 */
MF_S8 *
msfs_disk_get_strtype(TYPE_EN enType);

/**
 * \brief get string for status.
 * \param[in] enStat
 * \return char *.
 */
MF_S8 *
msfs_disk_get_strstatus(STATE_EN enStat);

/**
 * \brief set this disk port info.
 * \param[in] pstPort
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_set_port_info(struct port_info_t *pstPort);
/**
 * \brief set this disk group info.
 * \param[in] pstGrp
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_set_group_info(struct group_info_t *pstGrp);

/**
 * \brief get all port of disks.
 * \param[in] list_head
 * \return void.
 */
void
msfs_disk_get_pv(struct list_head *head);

/**
 * \brief put all port of disks.
 * \param[in] list_head
 * \return void.
 */
void
msfs_disk_put_pv(struct list_head *head);

/**
 * \brief get all bad port of disks.
 * \param[in] list_head
 * \return void.
 */
void
msfs_disk_get_bad_port(struct list_head *head);

/**
 * \brief put all port of disks.
 * \param[in] list_head
 * \return void.
 */
void
msfs_disk_put_bad_port(struct list_head *head);

/**
 * \brief get all disks info in system
 * \param[out] pstAttr
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_get_disk_info(struct disk_attr_t *pstAttr, MF_U8 port);

/**
 * \brief delete all disks info in system
 * \param[in] pstAttr
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */

MF_S32
msfs_disk_put_disk_info(struct disk_attr_t *pstAttr);

/**
 * \brief switch group or no group mode
 * \param[in] enMode
 * \return no
 */
void
msfs_disk_set_mode(MODE_EN enMode);

/**
 * \brief lock u disk fmtmutex
 * \param[in] mnt
 * \return no
 */
MF_S32
msfs_disk_get_Udisk(MF_S8 *mnt);

/**
 * \brief unlock u disk fmtmutex
 * \param[in] mnt
 * \return no
 */
void
msfs_disk_put_Udisk(MF_S8 *mnt);

/**
 * \brief add a nas, by nas hostIP, dir
 * \param[in] name
  * \param[in] port
   * \param[in] host
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_nas_add(MF_S8 *name, MF_U8 port, MF_S8 *host, MF_S32 type, MF_S8 *user, MF_S8 *password);
/**
 * \brief rename a nas, by nas port
 * \param[in] name
  * \param[in] port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_nas_rename(MF_S8 *name, MF_U8 port);
/**
 * \brief delete a nas, by a nas port, this port has exist in system
  * \param[in] port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_nas_del(MF_U8 port);
/**
 * \brief get all nas dir by a hostIP
  * \param[in] hostIp
  * \param[out] pstNas
 * \return number of searching.
 */
MF_U32
msfs_disk_nas_get_dir(MF_S8 *hostIp, struct disk_nas_t *pstNas);

/**
 * \brief delete this nas dir
  * \param[in] pstNas
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
void
msfs_disk_nas_put_dir(struct disk_nas_t *pstNas);

/**
 * \brief set recycle mode
  * \param[in] bLoop
 * \return no.
 */
void
msfs_disk_set_loop(MF_BOOL bLoop);

/**
 * \brief set disk private noly for one host
  * \param[in] port
  * \param[in] bEnable
 * \return no.
 */
void
msfs_disk_set_private(MF_U8 port, MF_BOOL bEnable);

/**
 * \brief set esata reuse function
  * \param[in] brec : MF_YES  use for recording, MF_NO use for backup
 * \return no.
 */
void
msfs_disk_set_esata_usage(MF_BOOL brec);//solin

/**
 * \brief set raid mode function
  * \param[in] mode : 0 disable, > 0 enable
 * \return no.
 */
void
msfs_disk_set_raid_mode(MF_BOOL bmode);//solin

/**
 * \brief format a disk by a port num
  * \param[in] port
 * \param[in] enFormat
 * \param[in] quota : only for NAS
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_format(MF_U8 port, FORMAT_EN enFormat, MF_U64 quota);

/**
 * \brief restore a disk which  has formated or damaged
  * \param[in] port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_restore(MF_U8 port);

/**
 * \brief restoreEx a disk which  has formated or damaged
  * \param[in] port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_restoreEx(MF_U8 port);

/**
 * \brief dump a disk's index tree to ram
  * \param[in]  portId
 * \return no.
 */
void
msfs_disk_index_dump(MF_U8 portId);

/**
 * \brief dump a disk's file detail to ram
  * \param[in]  portId
  * \param[in]  chnId
 * \return no.
 */
void
msfs_disk_file_dump(MF_U8 portId, MF_U8 chnId);

/**
 * \brief scan all disks, add disks to g_disk.list, include local disk and raid array
  * \param[in]  none
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
void
msfs_disk_scan(void);
/**
 * \brief g_disk list init
  * \param[in]  user_cb
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_disk_init(DISK_USER_CB user_cb, disk_param *para);
/**
 * \brief g_disk list delete
  * \param[]  no
 * \return no.
 */
void
msfs_disk_deinit();
/**
 * \brief add a raid vendor temporary
  * \param[in]  port, vendor
 * \return void
 */
MF_S32
msfs_raid_info_add(MF_U8 port, MF_S8 *vendor);
/**
 * \brief rename a raid vendor
  * \param[in]  vdneor, port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_raid_rename(MF_S8 *vendor, MF_U8 port);

void 
msfs_raid_deal_create_failed(struct raid_op_t *raid);

/**
 * \brief create a raid array
  * \param[in]  raid
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_raid_create(struct raid_op_t *raid);
/**
 * \brief delete a raid array
  * \param[in]  raid_port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_raid_del(int raid_port);
/**
 * \brief rebuild a raid array
  * \param[in]  raid_port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_raid_rebuild(struct raid_op_t *raid);
/**
 * \brief rebuild a raid array
  * \param[in]  raid_port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
void
msfs_get_component_size_raid(MF_S32 *raidport, MF_U64 *rsize);
/**
 * \brief update rebuild progress
  * \param[in]  raid_port
 * \return int percent
 */
MF_S32
msfs_raid_rebuild_progress(MF_S32 raid_port);

/**
 * \brief create a global spare disk
  * \param[in]  disk_port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_create_global_spare(MF_S32 disk_port);

/**
 * \brief delete a  global spare disk
  * \param[in]  disk_port
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_remove_global_spare(MF_S32 disk_port);

/**
 * \brief notify bar stat by port&percent
  * \param[in]  disk_port percent
 * \return none
 */
void
msfs_disk_notify_bar(PROGRESS_BAR_E enBar, MF_U8 port, MF_S32 percent, TYPE_EN type);

/**
 * \brief update local info to in raid
  * \param[in]  raid_op_t
 * \return long long    <0: param error;
 *                      =0: success
 *                      >0: port mask of disk reset success
 */
MF_S32
msfs_raid_create_update_local(raid_op_t *raid);

/**
 * \brief reload bad disk
  * \param[in]  void
 * \return none
 */
void
msfs_disk_bad_reload(struct list_head *head);

/**
 * \breif operat smartctl for disk
  * \param[in]  disk port hdd_attr type
 * \return int
 */
MF_S32
msfs_disk_op_smart(int port, HDD_ATTR *attr, int test_type);

/**
 * \breif get smartctl information about disk
  * \param[in]  data port
 * \return int
 */
MF_S32
msfs_disk_get_smart_attr(HDD_ATTR *attr, int port);

/**
 * \breif del smartctl information list about disk
  * \param[in]  data
 * \return int
 */
MF_S32
msfs_disk_del_smart_attr_list(HDD_ATTR *attr);

/**
 * \brief  Start receiving event of disk when all modules  loaded
 * \param[in] none
 * \return none
 */
void
msfs_disk_event_get();

/**
* \brief  disk debug switch.
* \param[in] bDebug MF_YES for enable, MF_NO for disable
* \return none
*/
void
msfs_disk_dbg_switch(MF_BOOL bDebug);

/**
* \brief show disk memory pool usage.
* \return none
*/
void
msfs_disk_dbg_mem_usage();

/**
* \brief  disk debug level.
* \param[in] enLevel.
* \return none
*/
void
msfs_disk_dbg_level(DBG_E enLevel);

/**
* \brief  disk debug hook (ex. ms_cli_output).
* \param[in] hook.
* \return none
*/
void
msfs_disk_dbg_print(void *print);

/**
* \brief  disk quota mode set chn.
* \param[in] hook.
* \return none
*/
MF_S32
msfs_disk_set_quota_info(quota_info_t *pstQta, MF_U32 chnNum);

MF_S32
msfs_disk_get_quota_info(MF_U8 chnId, quota_info_t *pstQta);

MF_BOOL
msfs_disk_get_quota_enable(void);

MF_BOOL
msfs_disk_is_can_record();

void
msfs_check_raid_status();

/**
 * \breif get health data about disk
  * \param[in]  data port
 * \return int
 */
MF_S32
msfs_disk_get_health_data(disk_health_data_t *health, int port);

/**
 * \breif get disk wwn
  * \param[in]  data port
 * \return int
 */
MF_S32
msfs_disk_get_wwn(char *wwn, int port);

/**
 * \brief mscli set  temperature
  * \param[in]  portId
  * \param[in]  temperature
 * \return no.
 */
void
msfs_disk_set_health_temperature(MF_U8 portId, MF_S32 temperature);

/**
 * \brief mscli set status
  * \param[in]  portId
  * \param[in]  status
 * \return no.
 */
void
msfs_disk_set_health_status(MF_U8 portId, MF_S32 status);

/**
 * \brief mscli set status
  * \param[in]  enableMask
 * \return no.
 */
void
msfs_disk_set_health_enable(MF_U64 enableMask);

#ifdef __cplusplus
}
#endif

#endif

