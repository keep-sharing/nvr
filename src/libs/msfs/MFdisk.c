/*
 * ***************************************************************
 * Filename:        MFdisk.c
 * Created at:      2017.05.10
 * Description:     disk mangement API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/statfs.h>
#include <rpc/rpc.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <rpc/clnt.h>
#include <mntent.h>

#include "mf_type.h"
#include "msdefs.h"
#include "uv.h"
#include "disk.h"
#include "msstd.h"
#include "MFdisk.h"
#include "MFlog.h"
#include "MFindex.h"
#include "MFrecord.h"
#include "MFcommon.h"
#include "MFmemory.h"
#include "msfs_disk.h"
#include "msfs_notify.h"
#include "ms_disk_drv.h"

#define RPCMNT_EXPORT   5
#define RPCPROG_MNT 100005
#define RPCMNT_PATHLEN 1024
#define RPC_TIMEOUT (2), (0) // 4s timeout
#define RPC_RECONNE_TOTAL (3)
#define RPC_NAS_ERR_TOTAL (60) //reconnect after 300 times when read/write err almost 60s
#define NOTY_INTERVAL (70)
#define MAX_HEALTH_TASK 20
static int diskHealthTemperature[MAX_DISK_PORT_NUM];
static int diskHealthstatus[MAX_DISK_PORT_NUM];
static int healthCheckedCount = 5;
static sem_t healthTaskSemaphore;
static int healthTaskCount = 0;
diskObj *helathTaskQueue[MAX_HEALTH_TASK];
struct grouplist {
    struct grouplist *gr_next;
    MF_S8  gr_name[RPCMNT_NAMELEN + 1];
};

struct exportslist {
    struct exportslist *ex_next;
    struct grouplist *ex_groups;
    MF_S8  ex_dirp[RPCMNT_PATHLEN + 1];
};

typedef struct scan {
    MF_S8       name[DEV_NAME_LEN];
    struct list_head node;
} scan;

typedef struct raid_info_temp {
    MF_U8 port;
    MF_S8 vendor[DEV_NAME_LEN];
    struct list_head node;
} raid_info_temp;

typedef struct disk {
    MF_BOOL          bDebug;
    MF_BOOL          bLoop;
    MF_BOOL          bEsataUse;
    MF_BOOL          bRaidMode;
    MF_BOOL          bReady;
    MF_BOOL          bNotify;
    struct mf_mem_t *pstMem;
    struct mf_grp    stGroup;
    struct mf_qta    stQuota;
    TASK_HANDLE      pHotplug;
    TASK_HANDLE      pState;
    TASK_HANDLE      pIOTask;
    MF_BOOL          bRunHotplug;
    MF_BOOL          bRunState;
    MF_BOOL          bRunIOTask;
    RWLOCK_BOJECT    rwlock;
    MUTEX_OBJECT     loadingMutex;
    MUTEX_OBJECT     loadSyncMutex;
    struct list_head list;
    struct list_head loadingList;
    MF_U64           loadSyncPort;
    BARRIER_OBJECT   loadBarrier;

    //usb
    MF_U8            usbindex;
    MUTEX_OBJECT     usbMutex;

    //format
    struct list_head listFormat;
    MUTEX_OBJECT     listfmtMutex;

    //TASK_HANDLE      pClients;
    MF_BOOL          bRunClients;
    MUTEX_OBJECT     cmutex;
    DISK_USER_CB     user_cb;
    struct list_head clist;
    struct list_head clist_bk;

    //device info
    MF_S8 devModel[MAX_LEN_32];
    MF_S8 devMmac[MAX_LEN_32];

    struct list_head raidInfoList;
    MUTEX_OBJECT     raidinfoMutex;

    MUTEX_OBJECT     fmtbkpmutex;//formate for mas

    //io request
    MUTEX_OBJECT     IOMutex;
    struct list_head IOList[MAX_DISK_PORT_NUM + 1];
    off64_t IONowOffset[MAX_DISK_PORT_NUM + 1];

    MF_BOOL          bCanRecord;

    //Health Management
    TASK_HANDLE      pHMTask;
    TASK_HANDLE      pHMSTATUSTask;
    MF_BOOL          bRunHMTask;
    MF_U64           HMEnableMask;
} disk;

// nfs / ipsan detect
typedef struct rpc_client {
    MF_U8 port;
    MF_S8 ipaddr[DEV_NAME_LEN];
    MF_S8 name[MAX_LEN_128];
    MF_S8 host[DEV_HOST_LEN];// include ip & path
    MF_BOOL onState;
    MF_S32 mntstat;
    CLIENT *clnt;
    MF_S32 result;

    // for thread
    TASK_HANDLE pClients;
    MF_BOOL bRunClients;
    EVENT_E event;
    MF_BOOL cremov_bk;

    // for task exit when sync index.solin
    struct diskObj *pdisk;
    MUTEX_OBJECT     mutex;

    struct list_head node;

    //for cifs
    MF_S32  type;
    MF_S8   user[64];
    MF_S8   password[64];
} rpc_client;
struct disk g_disk;

/* for usb mount&umount.solin */
#define USB_MAX_PORT   (MAX_DISK_USB_NUM)
#define USB_DISK_PORT  (MAX_DISK_LOCAL_NUM+MAX_DISK_NET_NUM+MAX_DISK_ESATA_NUM+MAX_DISK_RAID+MAX_DISK_USB_NUM)
#define USB_START_PORT (USB_DISK_PORT - MAX_DISK_USB_NUM + 1)
struct USB_ALIA_TABLE {
    MF_S32 port;
    MF_S8 idname[14];
};
static struct USB_ALIA_TABLE usb_table[USB_MAX_PORT] = {
    {34, "/media/usb1_1"},
    {35, "/media/usb1_2"},
    {36, "/media/usb3_1"},
    {37, "/media/usb4_1"},
};

static MF_S32 g_disk_drv_fd = -1;
static inline void disk_driver_open(void)
{
    if (g_disk_drv_fd < 0) {
        g_disk_drv_fd = open(DISK_DRV_PATH, O_RDWR);
        if (g_disk_drv_fd < 0) {
            MFerr(" %s open fail.", DISK_DRV_PATH);
        } else {
            MFinfo(" %s open OK.", DISK_DRV_PATH);
        }
    }
}

static inline void disk_driver_close(void)
{
    if (g_disk_drv_fd >= 0) {
        close(g_disk_drv_fd);
    }
}


#define DEV_FIND_BY_PORT(portID, pstDev) \
    { \
        struct diskObj* pos = NULL; \
        struct diskObj* n = NULL; \
        ms_rwlock_rdlock(&g_disk.rwlock); \
        list_for_each_entry_safe(pos, n, &g_disk.list, node) \
        { \
            if (portID == pos->port) \
            { \
                pstDev = pos; \
                break; \
            } \
        } \
        ms_rwlock_unlock(&g_disk.rwlock); \
    }

#define DEV_FIND_BY_NAME(devname, pstDev) \
    { \
        struct diskObj* pos = NULL; \
        struct diskObj* n = NULL; \
        ms_rwlock_rdlock(&g_disk.rwlock); \
        list_for_each_entry_safe(pos, n, &g_disk.list, node) \
        { \
            if (!strcmp(pos->name, devname)) \
            { \
                pstDev = pos; \
                break; \
            } \
        } \
        ms_rwlock_unlock(&g_disk.rwlock); \
    }

#define DEV_FIND_BY_ALIAS(devname, pstDev) \
    { \
        struct diskObj* pos = NULL; \
        struct diskObj* n = NULL; \
        ms_rwlock_rdlock(&g_disk.rwlock); \
        list_for_each_entry_safe(pos, n, &g_disk.list, node) \
        { \
            if (!strcmp(pos->alias, devname)) \
            { \
                pstDev = pos; \
                break; \
            } \
        } \
        ms_rwlock_unlock(&g_disk.rwlock); \
    }

#define DEV_FIND_BY_MNT(devname, pstDev) \
    { \
        struct diskObj* pos = NULL; \
        struct diskObj* n = NULL; \
        ms_rwlock_rdlock(&g_disk.rwlock); \
        list_for_each_entry_safe(pos, n, &g_disk.list, node) \
        { \
            if (strlen(pos->mnt) && strstr(devname, pos->mnt)) \
            { \
                pstDev = pos; \
                break; \
            } \
        } \
        ms_rwlock_unlock(&g_disk.rwlock); \
    }


#define DEV_FIND_AND_PRINT() \
    { \
        struct diskObj* pos = NULL; \
        struct diskObj* n = NULL; \
        ms_rwlock_rdlock(&g_disk.rwlock); \
        list_for_each_entry_safe(pos, n, &g_disk.list, node) \
        { \
            MFinfo("current dev :%s ", pos->name); \
        } \
        ms_rwlock_unlock(&g_disk.rwlock); \
    }

struct disk_scan_ { //reference ms_disk_drv.c driver
    unsigned int port;
    char name[DISK_NAME_LEN];
    //signed int hub;//save extend
    struct list_head node;
};
static struct disk_scan_ g_dscan_;

//unreconiged disk solution - see /etc/udev/rules.d/11-usb-hotplug.rules.
#if 0//defined(_HI3536_)
/*
* MS-N8064-UPH
*
* HUB 2:
* "0:3:0:0""MF_local_1_"
* "0:0:0:0""MF_local_2_"
* "0:2:0:0""MF_local_3_"
* "0:1:0:0""MF_local_4_"
* "0:4:0:0""MF_esata_25_"
* "1:3:0:0""MF_local_5_"
* "1:0:0:0""MF_local_6_"
* "1:2:0:0""MF_local_7_"
* "1:1:0:0""MF_local_8_"
*/
static struct disk_scan g_dscan[] = {
    { 0 << 24 | 3 << 16 | 0 << 8 | 0, "MF_local_1_"},
    { 0 << 24 | 0 << 16 | 0 << 8 | 0, "MF_local_2_"},
    { 0 << 24 | 2 << 16 | 0 << 8 | 0, "MF_local_3_"},
    { 0 << 24 | 1 << 16 | 0 << 8 | 0, "MF_local_4_"},
    { 0 << 24 | 4 << 16 | 0 << 8 | 0, "MF_esata_25_"},
    { 1 << 24 | 3 << 16 | 0 << 8 | 0, "MF_local_5_"},
    { 1 << 24 | 0 << 16 | 0 << 8 | 0, "MF_local_6_"},
    { 1 << 24 | 2 << 16 | 0 << 8 | 0, "MF_local_7_"},
    { 1 << 24 | 1 << 16 | 0 << 8 | 0, "MF_local_8_"},
    { 0, ""},
};
//#elif 0//defined(_HI3798_)

/*
* MS-N5016-UPT
*
* only one hub
* "0:2:0:0"MF_local_1_"
* "0:3:0:0""MF_local_2_"
*/
static struct disk_scan g_dscan[] = {
    { 0 << 24 | 2 << 16 | 0 << 8 | 0, "MF_local_1_"},
    { 0 << 24 | 3 << 16 | 0 << 8 | 0, "MF_local_2_"},
    { 0, ""},
};
//#else

//static struct disk_scan g_dscan[] = {
//  { 0, ""},
//};
#endif

static MF_S32 disk_sdinfo_ioctl(void);
static MF_S32 client_task_simple(struct rpc_client *client);


static void
disk_mem_init(struct disk *pstDisk)
{
    struct mem_pool_t *pstMpool = mf_comm_mem_get();

    if (pstMpool->diskSize) {
        pstDisk->pstMem = mf_mem_pool_create("mfdisk", pstMpool->diskSize);
    }
}

static void
disk_mem_uninit(struct disk *pstDisk)
{
    if (pstDisk->pstMem) {
        mf_mem_state(pstDisk->pstMem, MF_YES);
        mf_mem_pool_destory(pstDisk->pstMem);
    }
}

static inline void *
disk_mem_alloc(MF_U32 size)
{
    void *p;

    if (g_disk.pstMem) {
        p = mf_mem_alloc(g_disk.pstMem, size);
    } else {
        p = ms_malloc(size);
    }

    return p;
}

static inline void *
disk_mem_calloc(MF_U32 n, MF_U32 size)
{
    void *p;

    if (g_disk.pstMem) {
        p = mf_mem_calloc(g_disk.pstMem, n, size);
    } else {
        p = ms_calloc(n, size);
    }

    return p;
}

static inline void
disk_mem_free(void *p)
{
    if (g_disk.pstMem) {
        mf_mem_free(g_disk.pstMem, p);
    } else {
        ms_free(p);
    }
}

static void
disk_user_notify(EVENT_E event, struct diskObj *disk)
{
    if (g_disk.user_cb) {
        if (disk) {
            g_disk.user_cb(event, disk->port);
        } else {
            g_disk.user_cb(event, 0XFF);
        }
    }
}

static MF_S32
nas_exports(XDR *xdrsp, struct exportslist **exp)
{
    struct exportslist *ep;
    struct grouplist *gp;
    MF_S32 bool, grpbool;
    MF_S8 *strp;

    *exp = (struct exportslist *)0;
    if (!xdr_bool(xdrsp, &bool)) {
        return (0);
    }
    while (bool) {
        ep = (struct exportslist *)disk_mem_calloc(1, sizeof(struct exportslist));
        if (ep == NULL) {
            return (0);
        }
        ep->ex_groups = (struct grouplist *)0;
        strp = ep->ex_dirp;
        if (!xdr_string(xdrsp, &strp, RPCMNT_PATHLEN)) {
            return (0);
        }
        if (!xdr_bool(xdrsp, &grpbool)) {
            return (0);
        }
        while (grpbool) {
            gp = (struct grouplist *)disk_mem_calloc(1, sizeof(struct grouplist));
            if (gp == NULL) {
                return (0);
            }
            strp = gp->gr_name;
            if (!xdr_string(xdrsp, &strp, RPCMNT_NAMELEN)) {
                return (0);
            }
            gp->gr_next = ep->ex_groups;
            ep->ex_groups = gp;
            if (!xdr_bool(xdrsp, &grpbool)) {
                return (0);
            }
        }
        ep->ex_next = *exp;
        *exp = ep;
        if (!xdr_bool(xdrsp, &bool)) {
            return (0);
        }
    }
    return (1);
}

static MF_U32
nas_directory_scan(MF_S8 *host, struct list_head *list)
{
    struct exportslist *exports;
    struct exportslist *exp;
    struct grouplist *grp;
    struct grouplist *grpTmp;
    struct nas_dir_t *dir;
    MF_S32 estat;
    MF_U32  dirNum = 0;

    if ((estat = callrpc(host, RPCPROG_MNT, 1,
                         RPCMNT_EXPORT, (xdrproc_t)xdr_void, (MF_S8 *)0,
                         (xdrproc_t)nas_exports, (MF_S8 *)&exports)) != 0) {
        clnt_perrno(estat);
        fprintf(stderr, ": Can't do Exports rpc\n");
        return dirNum;
    }
    printf("Exports list on %s:\n", host);
    exp = exports;
    while (exp) {
        dir = disk_mem_calloc(1, sizeof(struct nas_dir_t));
        strcpy(dir->dirName, exp->ex_dirp);
        dirNum++;
        MFinfo("%s", dir->dirName);
        list_add(&dir->node, list);
        grp = exp->ex_groups;
        if (grp == NULL) {
            printf(" Everyone\n"); // allow space
        } else {
            while (grp) {
                grpTmp = grp;
                grp = grp->gr_next;
                disk_mem_free(grpTmp);
            }
            printf("\n");
        }
        struct exportslist *expTmp;
        expTmp = exp;
        exp = exp->ex_next;
        disk_mem_free(expTmp);
    }

    return dirNum;
}

static inline MF_S32
nas_split_ip_path(const MF_S8 *host, MF_S8 *ip, MF_S8 *path)// path: 128 length, ip: 64 length
{
    MF_S8 *div = ":";
    MF_S8 *tpath = NULL;

    if (!host || strlen(host) <= 0) {
        return MF_FAILURE;
    }
    tpath = strstr(host, div);
    if (!tpath) {
        return MF_FAILURE;
    }

    if (ip && !path && (tpath - host) > 0) {
        strncpy(ip, host, tpath - host);
        return MF_SUCCESS;
    } else if (path && !ip && strlen(tpath + 1) > 0) {
        strcpy(path, tpath + 1);
        return MF_SUCCESS;
    } else if (ip && path && (tpath - host) > 0 && strlen(tpath + 1) > 0) {
        strncpy(ip, host, tpath - host);
        strcpy(path, tpath + 1);
        return MF_SUCCESS;
    }

    return MF_FAILURE;
}

/*
* valid client port with mutex .solin
*/
static MF_S32
client_is_online(MF_U8 port)
{
    struct rpc_client *pclnt = NULL, *n = NULL;

    ms_mutex_lock(&g_disk.cmutex);
    list_for_each_entry_safe(pclnt, n, &g_disk.clist, node) {
        if (pclnt->port == port) {
            MFinfo("client exist %d", port);
            ms_mutex_unlock(&g_disk.cmutex);
            return MF_FAILURE;
        }
    }
    ms_mutex_unlock(&g_disk.cmutex);
    return MF_SUCCESS;
}

/*
* add client with mutex .solin
*/
static void
client_list_add(struct rpc_client *pClient)
{
    ms_mutex_lock(&g_disk.cmutex);
    if (list_empty(&g_disk.clist)) {
        list_add(&pClient->node, &g_disk.clist);
    } else {
        list_add_tail(&pClient->node, &g_disk.clist);
    }
    ms_mutex_unlock(&g_disk.cmutex);
    MFinfo("add client");
}

static void
client_list_del(struct rpc_client *pClient)
{
    MFinfo("del client %d from %s", pClient->port, pClient->cremov_bk ? "SUB" : "MAIN");
    list_del(&pClient->node);
}

static MF_S32
client_create_client(MF_S32 type, MF_S8 *devname, MF_U8 port, MF_S8 *host, MF_S8 *user, MF_S8 *password)
{
    struct rpc_client *client;
    MF_S8 *p = NULL;

    if (!host || port <= 0 || !devname) {
        return MF_FAILURE;
    }
    /* if new client is exists or not */
    if (client_is_online(port) != MF_SUCCESS) {
        return MF_FAILURE;
    }
    if (*(host + strlen(host) - 1) == '/') { // clear last  '/'
        *(host + strlen(host) - 1) = '\0';
    }

    client = disk_mem_calloc(1, sizeof(struct rpc_client));
    assert(client != NULL);

    client->port      = port;
    client->mntstat   = MF_FAILURE;
    //client->pdisk = NULL;
    strncpy(client->host, host, sizeof(client->host));
    strncpy(client->name, devname, sizeof(client->name));
    ms_mutex_init(&client->mutex);

    //for thread
    client->event       = MSFS_EVENT_NONE;
    client->result      = MF_FAILURE;
    client->bRunClients = MF_NO;

    //for cifs
    client->type        = type;
    if (user) {
        snprintf(client->user, sizeof(client->user), "%s", user);
    } else {
        client->user[0] = '\0';
    }
    if (password) {
        snprintf(client->password, sizeof(client->password), "%s", password);
    } else {
        client->password[0] = '\0';
    }

    //get ip addr frome host string
    if (!type && nas_split_ip_path(host, client->ipaddr, NULL) != MF_SUCCESS) {
        MFerr("[Solin] nas host err! (%s)", host);
        disk_mem_free(client);
        return MF_FAILURE;
    } else if (type) {
        if (strlen(host) <= 2) {
            MFerr("cifs host err! (%s)", host);
            disk_mem_free(client);
            return MF_FAILURE;
        }
        p = strstr(host + 2, "/");
        if (!p) {
            MFerr("cifs host err! (%s)", host);
            disk_mem_free(client);
            return MF_FAILURE;
        }
        strncpy(client->ipaddr, host + 2, p - host - 2);
    }
    MFinfo("add client port[%d]:[%s]ip", client->port, client->ipaddr);

    client_list_add(client);
    client_task_simple(client);
    return MF_SUCCESS;
}

static MF_S32
client_remove_by_port(MF_U8 port)
{
    struct rpc_client *client = NULL, *n = NULL;

    ms_mutex_lock(&g_disk.cmutex);
    list_for_each_entry_safe(client, n, &g_disk.clist, node) {
        if (client->port == port) {
            ms_mutex_lock(&client->mutex);
            client->bRunClients = MF_NO;
            client->event = MSFS_EVENT_NONE;
            //client->port = MAX_DISK_PORT_NUM + 1;
            client_list_del(client);
            list_add(&client->node, &g_disk.clist_bk);
            client->cremov_bk = MF_YES;
            ms_mutex_unlock(&client->mutex);
            MFinfo("remove client %d task !", port);
            break;
        }
    }
    ms_mutex_unlock(&g_disk.cmutex);
    return MF_SUCCESS;
}

static void
client_update_event(MF_U8 port, EVENT_E event)
{
    struct rpc_client *client = NULL;

    ms_mutex_lock(&g_disk.cmutex);
    list_for_each_entry(client, &g_disk.clist, node) {
        if (client->port == port) {
            ms_mutex_lock(&client->mutex);
            client->event = event;
            ms_mutex_unlock(&client->mutex);
            MFinfo("set client %d event %d !", port, event);
            break;
        }
    }
    ms_mutex_unlock(&g_disk.cmutex);
    return ;
}

static MF_U8
get_group_by_port(MF_U8 port)
{
    return g_disk.stGroup.pg[port].gId;
}

static int output_execute_Info(const char *cmd, char *string, int len)
{
    FILE *fp;
    MF_S32 ret = 0;
    MF_S8 buf[MAX_LEN_2048] = {0};

    if (cmd == NULL || string == NULL) {
        return -1; 
    }

    memset(string, 0, len);
    int stringLen = 0;

    if ((fp = ms_vpopen(cmd, "r")) != NULL) {
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            int bufLen = strlen(buf);
            if (stringLen + bufLen >= len - 1) {
                bufLen = len - 1 - stringLen;
            }
            strncat(string, buf, bufLen);
            stringLen += bufLen;

            if (stringLen >= len - 1) {
                break;
            }
        }
        ms_vpclose(fp);
    }

    return ret;
}

static void
dev_get_wwn(struct diskObj *pstDev) 
{
    MF_S8 buf[MAX_LEN_1024] = {0};
    MF_S8 cmd[64] = {0};
    MF_S8 *startWWN = NULL;
    MF_S32  index = 0;

    snprintf(cmd, sizeof(cmd), "smartctl -i %s", pstDev->alias);
    output_execute_Info(cmd, buf, sizeof(buf));
    startWWN = strstr(buf, "LU WWN Device Id: ");
    if (startWWN != NULL) {
        startWWN += strlen("LU WWN Device Id: ");
        while (*startWWN != '\0' && *startWWN != '\n' && index < sizeof(pstDev->wwn) - 1) {
            if (*startWWN != ' ' ) {
                pstDev->wwn[index] = *startWWN;
                index++;
            }
            startWWN++;
        }
        pstDev->wwn[index] = '\0';
    }
    msprintf("dev_get_wwn dev:%s wwn:%s",pstDev->alias, pstDev->wwn);
}

static void
dev_get_temperature_log(struct diskObj *pstDev) 
{
    MF_S8  cmd[128] = {0};
    MF_S8  path[64] = {0};
    MF_S8  line[256];

    snprintf(cmd, sizeof(cmd), "find /mnt/nand3/ -name 'diskPort%d_*' ! -name 'diskPort%d_%s' -delete", pstDev->port, pstDev->port, pstDev->wwn);
    ms_system(cmd);

    snprintf(cmd, sizeof(cmd), "find /mnt/nand3/ -name 'HMdiskPort%d_*' ! -name 'HMdiskPort%d_%s.csv' -delete", pstDev->port, pstDev->port, pstDev->wwn);
    ms_system(cmd);

    snprintf(path, sizeof(path), "/mnt/nand3/diskPort%d_%s", pstDev->port, pstDev->wwn);
    FILE* file = fopen(path, "r");
    
    if (file != NULL) {
        time_t     currentTime = time(NULL);
        struct tm *timeInfo    = localtime(&currentTime);
        timeInfo->tm_min       = 0;
        timeInfo->tm_sec       = 0;
        time_t hourTimestamp   = mktime(timeInfo);
        time_t earlier_time = hourTimestamp - (720 * 3600);//720h

        while (fgets(line, sizeof(line), file) != NULL && pstDev->lastTemperatureIndex < MAX_DISK_HM_TEMPERATURE - 1) {
            MF_S8* timestampStr = strtok(line, ";");
            MF_S8* valueStr = strtok(NULL, "\n");
            time_t timestamp = (time_t) strtol(timestampStr, NULL, 10);
            if (timestamp <= earlier_time) {
                continue;
            }
            pstDev->logTemperature[pstDev->lastTemperatureIndex].timestamp = timestamp;
            pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature = strtol(valueStr, NULL, 10);
            
            pstDev->lastTemperatureIndex++;
        }
        msprintf("gsjt dev_get_temperature_log port:%d index:%d", pstDev->port, pstDev->lastTemperatureIndex);
        
        fclose(file);
    } else {
        msprintf("Failed to open the file.\n");
    }
    pstDev->bReadyHM = MF_TRUE;
}

static MF_S32
dev_get_type(MF_S8 *name, struct diskObj *pstDev)
{
    MF_S8 type[16] = {0};
    MF_S8 dev[33] = {0};
    MF_S32  port;
    MF_S8 *raid_s = "/dev/md";
    MF_S8 m_type = 0;

    if (strncmp(name, raid_s, strlen(raid_s)) == 0) { // raid node -----> /dev/mdxx, xx for raid port number
        port = atoi(name + strlen(raid_s));
        m_type = DISK_TYPE_RAID;
    } else if (3 != sscanf(name, "%*[^_]_%15[^_]_%d_%32[^_]", type, &port, dev)) {
        MFerr("dev_get_attr failed [%s][%s][%d]!\n", type, dev, port);
        return MF_FAILURE;
    }
    MFinfo("[%s][%s][%d], name:[%s]!\n", type, dev, port, name);

    strcpy(pstDev->name, name);
    sprintf(pstDev->alias, "/dev/%s", dev);
    pstDev->port    = port;
    pstDev->group   = get_group_by_port(port);
    pstDev->enRw    = g_disk.stGroup.pg[port].enRw;
    pstDev->bLoop   = g_disk.bLoop;

    if (strstr(type, "usb")) {
        pstDev->enType = DISK_TYPE_USB;
    } else if (strstr(type, "local")) {
        pstDev->enType = DISK_TYPE_LOCAL;
    } else if (strstr(type, "nfs")) {
        strcpy(pstDev->alias, dev);
        pstDev->enType = DISK_TYPE_NAS;
    } else if (strstr(type, "cifs")) {
        strcpy(pstDev->alias, dev);
        pstDev->enType = DISK_TYPE_CIFS;
    } else if (strstr(type, "esata")) { // for esate type //solin
        pstDev->enType = DISK_TYPE_ESATA;
    } else if (strstr(type, "nand")) {
        strcpy(pstDev->alias, dev);
        pstDev->enType = DISK_TYPE_NAND;
    } else if (m_type == DISK_TYPE_RAID) { // for raid type
        pstDev->enType = DISK_TYPE_RAID;
    } else {
        pstDev->enType = DISK_TYPE_UNKNOWN;
    }

    return MF_SUCCESS;
}


static MF_S32
dev_update_status(struct diskObj *disk, MF_BOOL bNotify, EVENT_E event)
{
    if (disk == NULL) {
        MFerr("disk is NULL. \n");
        return MF_FAILURE;
    }

    if (disk->enType ==
        DISK_TYPE_GLOBAL_SPARE) { //when a disk for global spare disk, it can not used for record....., so switch disk state to GLOBAL_STATE
        disk->bRec = MF_FALSE;
    } else if (disk->enType == DISK_TYPE_IN_RAID) {
        disk->bRec = MF_FALSE;
    } else if (disk->enType == DISK_TYPE_USB) {
        disk->bRec = MF_FALSE;
    } else if (disk->enType == DISK_TYPE_NAND) {
        disk->bRec = MF_FALSE;
    } else if (disk->enType == DISK_TYPE_ESATA) {
        disk->bRec = g_disk.bEsataUse;
    } else { // local disk,  raid,  nas , nand is TRUE  default.
        disk->bRec = MF_TRUE;
    }

    if (bNotify) {
        mf_disk_event_notify(event, disk, MF_YES);
    }

    return MF_SUCCESS;
}

static void
dev_print_head_info(struct disk_header *pstHeader)
{
    MF_S8 stime [32];

    if (strcmp(pstHeader->magic, DISK_MAGIC)) {
        return;
    }
    time_to_string_local(pstHeader->restoreTimes, stime);
    MFprint("magic           :   %s\n", pstHeader->magic);
    MFprint("version         :   %s\n", pstHeader->version);
    MFprint("sector          :   %d\n", pstHeader->sector);
    MFprint("attr            :   %d\n", pstHeader->attr);
    MFprint("capacity        :   %lldM\n", pstHeader->capacity >> 20);
    MFprint("mainLogOff      :   %#llx\n", pstHeader->mainLogOff);
    MFprint("mainLogLen      :   %#llx\n", pstHeader->mainLogLen);
    MFprint("curLogOff       :   %#llx\n", pstHeader->curLogOff);
    MFprint("curLogLen       :   %#llx\n", pstHeader->curLogLen);
    MFprint("logStatus       :   %d\n", pstHeader->logStatus);
    MFprint("recordOff       :   %#llx\n", pstHeader->recordOff);
    MFprint("recordLen       :   %#llx\n", pstHeader->recordLen);
    MFprint("perFileSize     :   %#llx\n", pstHeader->perFileSize);
    MFprint("fileCount       :   %d\n", pstHeader->fileCount);
    MFprint("index00Off      :   %#llx\n", pstHeader->index00Off);
    MFprint("index00Len      :   %#llx\n", pstHeader->index00Len);
    MFprint("index01Off      :   %#llx\n", pstHeader->index01Off);
    MFprint("index01Len      :   %#llx\n", pstHeader->index01Len);
    MFprint("index10Off      :   %#llx\n", pstHeader->index10Off);
    MFprint("index10Len      :   %#llx\n", pstHeader->index10Len);
    MFprint("index11Off      :   %#llx\n", pstHeader->index11Off);
    MFprint("index11Len      :   %#llx\n", pstHeader->index11Len);
    MFprint("index0Status    :   %d\n", pstHeader->index0Status);
    MFprint("index1Status    :   %d\n", pstHeader->index1Status);
    MFprint("restoreCounts   :   %d\n", pstHeader->restoreCounts);
    MFprint("restoreTimes    :   %s\n", stime);
    MFprint("uuid            :   %s\n", pstHeader->uuid);
    MFprint("checkSum        :   %#02x\n", pstHeader->checkSum);
}

static MF_S32
dev_restore(struct diskObj *pstDev)
{
    MF_S32 res = MF_SUCCESS;

    if (pstDev->ops->disk_index_restore) {
        ms_mutex_lock(&pstDev->mutex);
        res = pstDev->ops->disk_index_restore(pstDev->pstPrivate);
        ms_mutex_unlock(&pstDev->mutex);
    }

    return res;
}

static MF_S32
dev_restoreEx(struct diskObj *pstDev)
{
    MF_S32 res = MF_SUCCESS;

    if (pstDev->ops->disk_index_restoreEx) {
        ms_mutex_lock(&pstDev->mutex);
        res = pstDev->ops->disk_index_restoreEx(pstDev->pstPrivate);
        ms_mutex_unlock(&pstDev->mutex);
    }

    return res;
}

static MF_S32
dev_format(struct diskObj *pstDev, FORMAT_EN enFormat, MF_U64 quota)
{
    MF_S32 res = MF_SUCCESS;

    if (pstDev->ops->disk_format) {
        ms_mutex_lock(&pstDev->mutex);
        pstDev->enState = DISK_STATE_FORMATING;
        res = pstDev->ops->disk_format(pstDev->pstPrivate, &enFormat, quota);
        ms_mutex_unlock(&pstDev->mutex);
    }

    return res;
}

static MF_S32
dev_mount(struct diskObj *pstDev)
{
    MF_S32 res = MF_SUCCESS;

    if (pstDev->ops->disk_mount) {
        res = pstDev->ops->disk_mount(pstDev->pstPrivate);
    }

    return res;
}

static MF_S32
dev_umount(struct diskObj *pstDev)
{
    MF_S32 res = MF_SUCCESS;

    msprintf("[wcm] disk_umount begin, lock begin");
    ms_mutex_lock(&pstDev->mutex);
    msprintf("[wcm] disk_umount begin, lock end");
    if (pstDev->ops->disk_umount) {
        res = pstDev->ops->disk_umount(pstDev->pstPrivate);
    }
    msprintf("[wcm] disk_umount end, unlock begin");
    ms_mutex_unlock(&pstDev->mutex);
    msprintf("[wcm] disk_umount end, unlock end");

    return res;
}

static MF_S32
dev_attribute(struct diskObj *pstDev)
{
    dev_update_status(pstDev, MF_NO, MSFS_EVENT_NONE);

    ms_mutex_lock(&pstDev->mutex);
    if (pstDev->ops->disk_status) {
        MFinfo("get disk[%d] status", pstDev->port);
        pstDev->ops->disk_status(pstDev->pstPrivate);
    }
    if (pstDev->ops->disk_size) {
        MFinfo("get disk[%d] size", pstDev->port);
        pstDev->ops->disk_size(pstDev->pstPrivate);
    }
    if (pstDev->ops->disk_vendor) {
        MFinfo("get disk[%d] vendor", pstDev->port);
        pstDev->ops->disk_vendor(pstDev->pstPrivate);
    }
    ms_mutex_unlock(&pstDev->mutex);

    MFinfo("name[%s] : vendor: %s", pstDev->name, pstDev->vendor);
    MFinfo("name[%s] : capacity = %lldM ", pstDev->name, pstDev->capacity / 1024 / 1024);
    MFinfo("name[%s] : free = %lldM ", pstDev->name, pstDev->free / 1024 / 1024);
    MFinfo("name[%s] : state = %d", pstDev->name, pstDev->enState);

//    dev_format(pstDev, FORMAT_TYPE_MS);
//    dev_print_head_info(pstDev->pstHeader);
//    mf_index_dump(pstDev->tree);

    return MF_SUCCESS;
}

static struct diskObj *
dev_init()
{
    struct diskObj *pstDev;

    pstDev = disk_mem_calloc(1, sizeof(struct diskObj));

    pstDev->pstHeader = disk_mem_calloc(1, ALIGN_UP(sizeof(struct disk_header), 512));
    assert(pstDev->pstHeader != NULL);

    ms_mutex_init(&pstDev->mutex);
    ms_mutex_init(&pstDev->infoMutex);
    ms_mutex_init(&pstDev->fmtMutex);
    ms_mutex_init(&pstDev->opsMutex);
    ms_mutex_init(&pstDev->waitMutex);
    mf_cond_init(&pstDev->waitCond);
    INIT_LIST_HEAD(&pstDev->ipcHead);
    pstDev->enRw = DISK_RW;
    pstDev->enLastState = DISK_STATE_NONE;
    pstDev->bLastFull = MF_NO;

    return pstDev;
}

static void
dev_deinit(struct diskObj *pstDev)
{
    ms_mutex_uninit(&pstDev->mutex);
    ms_mutex_uninit(&pstDev->infoMutex);
    ms_mutex_uninit(&pstDev->fmtMutex);
    ms_mutex_uninit(&pstDev->opsMutex);
    ms_mutex_uninit(&pstDev->waitMutex);
    mf_cond_uninit(&pstDev->waitCond);
    disk_mem_free(pstDev->pstHeader);
    disk_mem_free(pstDev);
}

static struct diskObj *
dev_register(MF_S8 *name, MF_S8 *host, void *params)
{
    struct diskObj *pstDev = NULL;
    struct diskObj *tpstDev = NULL;

    pstDev = dev_init();
    if (dev_get_type(name, pstDev) == MF_SUCCESS) {
        DEV_FIND_BY_PORT(pstDev->port, tpstDev);
        if (!tpstDev) {
            switch (pstDev->enType) {
                case DISK_TYPE_LOCAL:
                    disk_local_register(pstDev, host);
                    break;
                case DISK_TYPE_USB:
                    disk_usb_register(pstDev, host);
                    break;
                case DISK_TYPE_NAS:
                case DISK_TYPE_CIFS:
                    disk_nas_register(pstDev, host, params);
                    break;
                case DISK_TYPE_RAID:
                    disk_raid_register(pstDev, host);
                    break;
                case DISK_TYPE_ESATA:
                    disk_esata_register(pstDev, host);
                    pstDev->bRec = g_disk.bEsataUse;// init esata type after mscore init
                    break;
                case DISK_TYPE_NAND:
                    disk_nand_register(pstDev, host);
                    break;
                default:
                    break;
            }
            return pstDev;
        }
    }

    dev_deinit(pstDev);

    return NULL;
}

static void
dev_unregister(struct diskObj *pstDev)
{
    switch (pstDev->enType) {
        case DISK_TYPE_LOCAL:
            disk_local_unregister(pstDev);
            break;
        case DISK_TYPE_USB:
            disk_usb_unregister(pstDev);
            break;
        case DISK_TYPE_NAS:
        case DISK_TYPE_CIFS:
            disk_nas_unregister(pstDev);
            break;
        case DISK_TYPE_RAID:
            disk_raid_unregister(pstDev);
            break;
        case DISK_TYPE_NAND:
            disk_nand_unregister(pstDev);
            break;
        case DISK_TYPE_ESATA:
            disk_esata_unregister(pstDev);
            break;
        default:
            break;
    }
    dev_deinit(pstDev);
}

static load_s *
dev_loading(struct diskObj *disk, MF_BOOL isLoading)
{
    struct load_s *load = NULL;

    ms_mutex_lock(&g_disk.loadingMutex);
    if (isLoading == MF_YES) {
        load = disk_mem_calloc(1, sizeof(struct load_s));
        load->port      = disk->port;
        load->group     = disk->group;
        load->enType    = disk->enType;
        load->enRw      = disk->enRw;
        load->capacity  = disk->capacity;
        load->free      = disk->free;
        strncpy(load->name, disk->name, DEV_NAME_LEN);
        strncpy(load->vendor, disk->vendor, DEV_NAME_LEN);
        strncpy(load->host, disk->host, DEV_HOST_LEN);
        load->enState = DISK_STATE_LOADING;
        load->pstObj  = disk;
        list_add(&load->node, &g_disk.loadingList);
        disk->loading = load;
    } else {
        load = disk->loading;
        if (load) {
            list_del(&load->node);
            disk->loading = NULL;
            load->pstObj  = NULL;
            disk_mem_free(load);
            mf_disk_notify_bar(PROGRESS_BAR_DISK_LOADING, disk->port, 100, disk->enType);
        }
    }
    ms_mutex_unlock(&g_disk.loadingMutex);

    return load;
}


static inline MF_S32
dev_scan_begin(struct list_head *list)
{
    DIR *dir;
    struct dirent *ptr;
    MF_S8 name[DEV_NAME_LEN];
    struct scan *pstScan;
    char *dir_dev = "/dev";
    char *blk_dev = "MF";
    //char *blk_raid_dev = "md";
    MF_S32 count = 0;

    INIT_LIST_HEAD(list);

    if ((dir = opendir(dir_dev)) == NULL) {
        msprintf("Open dir %s error ...\n", dir_dev);
        return count;
    }

    while ((ptr = readdir(dir)) != NULL) { //MF** is link to /dev/sda[b,c,d...].
        if ((ptr->d_type == DT_LNK) && (strncmp(ptr->d_name, blk_dev, strlen(blk_dev)) == 0)) {
            //printf("\n ======= dir_dev:[%s], name:[%s], type:[%d]. \n", dir_dev, ptr->d_name, ptr->d_type);
            snprintf(name, sizeof(name), "%s/%s", dir_dev, ptr->d_name);
            pstScan = disk_mem_calloc(1, sizeof(struct scan));
            strcpy(pstScan->name, name);
            list_add(&pstScan->node, list);
            count++;
        }
//zbing delete 20180603 for  fixing raid add retr failed;
//      else if((ptr->d_type == DT_BLK) && (strncmp(ptr->d_name, blk_raid_dev, strlen(blk_raid_dev)) == 0))
//      {
//          //printf("\n ======= dir_dev:[%s], name:[%s], type:[%d]. \n", dir_dev, ptr->d_name, ptr->d_type);
//          snprintf(name, sizeof(name), "%s/%s", dir_dev, ptr->d_name);
//          pstScan = disk_mem_calloc(1, sizeof(struct scan));
//          strcpy(pstScan->name, name);
//          list_add(&pstScan->node, list);
//          count++;
//      }
    }

    closedir(dir);

    return count;
}

static void
dev_scan_end(struct list_head *list)
{
    struct scan *pos = NULL;
    struct scan *n = NULL;

    list_for_each_entry_safe(pos, n, list, node) {
        list_del(&pos->node);
        disk_mem_free(pos);
    }
}

static void
dev_wait_stop(struct diskObj *pstDev)
{
    ms_mutex_lock(&pstDev->waitMutex);
    mf_cond_signal(&pstDev->waitCond);
    ms_mutex_unlock(&pstDev->waitMutex);
}

static void
dev_wait_start(struct diskObj *pstDev, MF_U64 timeUs)
{
    ms_mutex_lock(&pstDev->waitMutex);
    if (timeUs) {
        mf_cond_timedwait(&pstDev->waitCond, &pstDev->waitMutex, timeUs);
    } else {
        mf_cond_wait(&pstDev->waitCond, &pstDev->waitMutex);
    }
    ms_mutex_unlock(&pstDev->waitMutex);
}

static void
dev_clear_bit_port(struct diskObj *pstDev)
{
    if (pstDev) {
        ms_mutex_lock(&g_disk.loadSyncMutex);
        g_disk.loadSyncPort &= ~(BIT1_LSHFT(pstDev->port - 1));
        ms_mutex_unlock(&g_disk.loadSyncMutex);
    }
}

static void
dev_loading_exit(MF_U8 port)
{
    struct load_s *load = NULL;
    struct load_s *m = NULL;

    ms_mutex_lock(&g_disk.loadingMutex);
    list_for_each_entry_safe(load, m, &g_disk.loadingList, node) {
        if (load->port == port) {
            load->pstObj->bExit = MF_YES;
        }
    }
    ms_mutex_unlock(&g_disk.loadingMutex);
}

static MF_BOOL
dev_is_alive(struct diskObj *pstDev)
{
    struct list_head list;
    struct scan *pos = NULL;
    MF_BOOL find = MF_NO;

    if (pstDev->enType == DISK_TYPE_NAS || pstDev->enType == DISK_TYPE_CIFS) {
        return MF_YES;
    }

    if (pstDev->bExit == MF_YES) {
        return MF_NO;
    }

    dev_scan_begin(&list);

    list_for_each_entry(pos, &list, node) {
        if (!strcmp(pos->name, pstDev->name)) {
            find = MF_YES;
        }
    }
    dev_scan_end(&list);
    return find;
}

static MF_S32
dev_retr_add(struct diskObj *pstDev)
{
    struct load_s *load = NULL;

    dev_update_status(pstDev, MF_NO, MSFS_EVENT_NONE);
    load = dev_loading(pstDev, MF_YES);
    mf_retr_disk_info_add(pstDev);

    if (pstDev->bExit != MF_YES && dev_is_alive(pstDev) == MF_YES) {
        return MF_SUCCESS;
    } else {
        ms_mutex_lock(&g_disk.loadingMutex);
        list_del(&load->node);
        load->pstObj = NULL;
        pstDev->loading = NULL;
        ms_mutex_unlock(&g_disk.loadingMutex);
        disk_mem_free(load);
        return MF_FAILURE;
    }
}

static void
dev_retr_del(struct diskObj *pstDev)
{
    mf_retr_disk_info_del(pstDev);
}

static void
dev_list_add(struct diskObj *pstDev)
{
    ms_rwlock_wrlock(&g_disk.rwlock);
    dev_loading(pstDev, MF_NO);
    LIST_INSERT_SORT_ASC(pstDev, &g_disk.list, port);
    ms_rwlock_unlock(&g_disk.rwlock);
    disk_user_notify(MSFS_EVENT_DISK_ADD, pstDev);
}

static void
dev_list_del(struct diskObj *pstDev)
{
    ms_rwlock_wrlock(&g_disk.rwlock);
    list_del(&pstDev->node);
    ms_rwlock_unlock(&g_disk.rwlock);
    disk_user_notify(MSFS_EVENT_DISK_DEL, pstDev);
}

static MF_S32
dev_load_sync_port(struct diskObj *pstDev)
{
    MF_S32 res = MF_FAILURE;
    if (!pstDev) {
        return res;
    }

    ms_mutex_lock(&g_disk.loadSyncMutex);
    MFdbg("byte-port[%d]-64 type[%d]", pstDev->port, pstDev->enType);
    if (!(g_disk.loadSyncPort & BIT1_LSHFT(pstDev->port - 1))) {
        g_disk.loadSyncPort |= BIT1_LSHFT(pstDev->port - 1);
        res = mf_disk_dev_loading(pstDev, MF_YES);
    } else {
        MFinfo("port[%d] ing ####", pstDev->port);
    }
    ms_mutex_unlock(&g_disk.loadSyncMutex);
    return res;
}

static struct diskObj *
dev_add_by_name(MF_S8 *name, MF_S8 *host, MF_S32 *mntstat, void *params)
{
    MF_S32 res = MF_SUCCESS;
    MF_S32 resmnt = MF_FAILURE;
    struct diskObj *pstDev = NULL;

    DEV_FIND_BY_NAME(name, pstDev);
    if (!pstDev) {
        pstDev = dev_register(name, host, params);
        if (pstDev) {
            resmnt = dev_mount(pstDev);// should remount usb
            if (pstDev->enType == DISK_TYPE_USB && resmnt == MF_FAILURE) {
                dev_umount(pstDev);
                resmnt = dev_mount(pstDev);
            }

            switch (pstDev->enType) {
                case DISK_TYPE_RAID: {
                    mf_raid_info_get(pstDev);
                    dev_attribute(pstDev);
                    dev_retr_add(pstDev);
                    dev_list_add(pstDev);
                }
                break;
                case DISK_TYPE_NAND: {
                    dev_attribute(pstDev);
                    dev_list_add(pstDev);
                }
                break;
                // nas can be break after
                case DISK_TYPE_CIFS:
                case DISK_TYPE_NAS: {
                    if (mntstat) {
                        *mntstat = resmnt;
                    }
                    if (resmnt == MF_FAILURE) {
                        MFinfo("mount fail");
                        dev_umount(pstDev);
                        dev_unregister(pstDev);
                        return NULL; // if mount failure, do not create disk
                    }
                }
                default: {
                    /* sync load port: LOCAL / USB / NAS / ESATA . */
                    msprintf("[wcm] dev_load_sync_port begin, pstDev = %p", pstDev);
                    res = dev_load_sync_port(pstDev);
                    msprintf("[wcm] dev_load_sync_port end, pstDev = %p, res = %d", pstDev, res);
                    if (res == MF_FAILURE) {
                        if (mntstat) {
                            *mntstat = MF_FAILURE;
                        }
                        dev_umount(pstDev);
                        dev_unregister(pstDev);
                        return NULL;
                    }
                    #if defined (_HI3536A_)
                    //disk health management init data, only support local or esata
                    msprintf("gsjt port:%d pstDev->enType:%d",pstDev->port,pstDev->enType);
                    if (pstDev->enType == DISK_TYPE_LOCAL || pstDev->enType == DISK_TYPE_ESATA || pstDev->enType == DISK_TYPE_IN_RAID) {
                        dev_get_wwn(pstDev);
                        dev_get_temperature_log(pstDev);
                    }
                    #endif
                }
                break;
            }
        }
    } else {
        /* re-update all attribute if disk is not error.solin */
        MFinfo("UPT: exist dev[%d]", pstDev->port);
        if (pstDev->enState != DISK_STATE_BAD) {
            dev_attribute(pstDev);
        }
    }

    return pstDev;
}

static struct diskObj *
dev_format_exit(MF_U8 port)
{
    struct diskObj *pos = NULL;
    ms_mutex_lock(&g_disk.listfmtMutex);
    list_for_each_entry(pos, &g_disk.listFormat, node) {
        if (port == pos->port) {
            pos->bFmtExit = MF_YES;
            MFdbg("port[%d]", port);
            ms_mutex_unlock(&g_disk.listfmtMutex);
            return pos;
        }
    }
    ms_mutex_unlock(&g_disk.listfmtMutex);
    return NULL;
}

static MF_S32
dev_remove_by_port(MF_U8 port)
{
    struct diskObj *pstDev = NULL;
    //exit format
    pstDev = dev_format_exit(port);
    if (!pstDev) {
        DEV_FIND_BY_PORT(port, pstDev);
    }

    if (pstDev) {
        ms_mutex_lock(&pstDev->fmtMutex);
        dev_update_status(pstDev, MF_YES, MSFS_EVENT_DISK_DEL);
        dev_umount(pstDev);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);
        dev_clear_bit_port(pstDev);
        ms_mutex_unlock(&pstDev->fmtMutex);
        dev_unregister(pstDev);
    }

    return MF_SUCCESS;
}

static void
dev_format_list_add(struct diskObj *pstDev)
{
    if (pstDev) {
        ms_mutex_lock(&g_disk.listfmtMutex);
        pstDev->bFmtExit = MF_NO;
        LIST_INSERT_SORT_ASC(pstDev, &g_disk.listFormat, port);
        ms_mutex_unlock(&g_disk.listfmtMutex);
    }
}

static void
dev_format_list_del(struct diskObj *pstDev)
{
    if (pstDev) {
        ms_mutex_lock(&g_disk.listfmtMutex);
        list_del(&pstDev->node);
        pstDev->bFmtExit = MF_NO;
        ms_mutex_unlock(&g_disk.listfmtMutex);
    }
}

#if defined (_HI3536_) || defined (_HI3536A_)
static MF_S32
dev_scan_raid(void)
{
    MFdbg("start raid scan\n");
    return raid_assemble();
}
#endif

static void
dev_scan_add(struct list_head *list)
{
    struct scan *pos = NULL;
    struct scan *n = NULL;

    list_for_each_entry_safe(pos, n, list, node) {
        MFdbg("ADD: dev name[%s]\n", pos->name);
        dev_add_by_name(pos->name, NULL, NULL, NULL);
    }
}

static void
dev_scan_remove(struct list_head *list)
{
    struct diskObj *Dpos = NULL;
    struct diskObj *d = NULL;
    struct scan *Spos = NULL;
    struct scan *s = NULL;
    MF_S32 find = 0;

    list_for_each_entry_safe(Dpos, d, &g_disk.list, node) {
        find = 0;
        list_for_each_entry_safe(Spos, s, list, node) {
            if (!strcmp(Dpos->name, Spos->name)) {
                find = 1;
                break;
            }
        }
        if (!find) {
            if (Dpos->enType == DISK_TYPE_NAS) {
                continue;
            }
            if (Dpos->enType == DISK_TYPE_CIFS) {
                continue;
            }
            if (Dpos->enType == DISK_TYPE_NAND) {
                continue;
            }
            if (Dpos->enType == DISK_TYPE_RAID) { // if exist, always online .solin 20180604
                continue;
            }

            dev_loading_exit(Dpos->port);
            dev_format_exit(Dpos->port);

            mf_bkp_get_format();
            ms_mutex_lock(&Dpos->fmtMutex);
            dev_update_status(Dpos, MF_YES, MSFS_EVENT_DISK_DEL);
            dev_umount(Dpos);
            dev_list_del(Dpos);
            dev_retr_del(Dpos);
            dev_clear_bit_port(Dpos);
            disk_user_notify(MSFS_EVENT_DISK_OFFLINE, Dpos);
            disk_user_notify(MSFS_EVENT_DISK_HEALTH_TEMPERATURE_NORMAL, Dpos);
            disk_user_notify(MSFS_EVENT_DISK_HEALTH_STATUS_NORMAL, Dpos);
            ms_mutex_unlock(&Dpos->fmtMutex);
            dev_unregister(Dpos);
            mf_bkp_put_format();
        }
    }
}

/* get cmd from ioctl only read/write err
*  fd: file descript
*  return: err count
*/
static MF_S32
disk_get_rwerr_cmd(sd_rwerr *rwerr)
{
    MF_S32 count = 0;

    if (!rwerr) {
        return count;
    }
    if (g_disk_drv_fd > 0) {
        count = ioctl(g_disk_drv_fd, IOSTRWERR, rwerr);
    }
    MFinfo(">>>>>>>>> disk_get_rwerr_cmd count:%d <<<<<<<<<\n", rwerr->count);
    return count;
}

static void
disk_list_init(struct disk *disk)
{
    MF_S32 i;

    INIT_LIST_HEAD(&disk->list);
    INIT_LIST_HEAD(&disk->loadingList);
    INIT_LIST_HEAD(&disk->listFormat);
    INIT_LIST_HEAD(&disk->clist);
    INIT_LIST_HEAD(&disk->clist_bk);
    INIT_LIST_HEAD(&disk->raidInfoList);
    for (i = 0; i <= MAX_DISK_PORT_NUM; i++) {
        INIT_LIST_HEAD(&disk->IOList[i]);
    }
}

static void
disk_list_deinit(struct disk *disk)
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;

    list_for_each_entry_safe(pos, n, &disk->list, node) {
        printf("######debug umount[%s] port[%d]\n", pos->name, pos->port);
        dev_umount(pos);
        dev_list_del(pos);
        dev_retr_del(pos);
        dev_unregister(pos);
    }

    struct load_s *load = NULL;
    struct load_s *m = NULL;

    list_for_each_entry_safe(load, m, &disk->loadingList, node) {
        pos = load->pstObj;
        if (pos) {
            pos->bExit = MF_YES;
            dev_wait_start(pos, 0);
        }
    }

    struct disk_scan_ *dpos = NULL, *dn = NULL;
    list_for_each_entry_safe(dpos, dn, &g_dscan_.node, node) {
        list_del(&dpos->node);
        disk_mem_free(dpos);
    }
}

static void
disk_rules_init(MF_S8 *chip_info)
{
    MF_S8 hdd[MAX_LEN_16] = {0};
    FILE *file = NULL;
    MF_S32 total = 0;
    MF_S32 host, chan, id, lum;
    MF_S8 prefix[MAX_LEN_16 + 1] = {0};
    struct disk_scan_ *dscan = NULL;

    if (!chip_info || !strlen(chip_info)) {
        return ;
    }

    strncpy(hdd, chip_info + MS_SERIAL_HDD_BIT, MS_SERIAL_HDD_LEN);
    sprintf(chip_info, "%s%s", RULE_FILE_PATH, hdd);
    file = fopen(chip_info, "r");
    if (!file) {
        perror(chip_info);
        return ;
    }

    if (fscanf(file, "total:%d", &total) > 0) {
        MFdbg("[Solin] total %d", total);
        while (!feof(file)) {
            if (fscanf(file, "%d:%d:%d:%d:%16s", &host, &chan, &id, &lum, prefix) <= 0) {
                break;
            }
            MFdbg("[Solin] %d:%d:%d:%d:%s", host, chan, id, lum, prefix);
            dscan = disk_mem_calloc(1, sizeof(struct disk_scan_));
            if (dscan) {
                dscan->port = host << 24 | chan << 16 | id << 8 | lum;
                strncpy(dscan->name, prefix, strlen(prefix));
                list_add_tail(&dscan->node, &g_dscan_.node);
            }
        }
    }
    fclose(file);
}

void
mf_disk_notify_bar(PROGRESS_BAR_E enBar, MF_U8 port, MF_S32 percent, TYPE_EN type)
{
    PROGRESS_BAR_T bar;
    memset(&bar, 0, sizeof(PROGRESS_BAR_T));
    bar.port = port;
    bar.percent = percent;
    bar.type = type;
    msfs_notify_progress_bar_state(enBar, &bar);
}

static void *
disk_task_dev_scan(void *argv)
{
    MF_S32 loadcount = 0;
    struct list_head list;

    MFinfo("disk device loading ......");

    // firstly, create symlink diskname by solin 20180402.
#ifndef SELECT_MODE
    disk_sdinfo_ioctl();
#endif
    //end

    loadcount = dev_scan_begin(&list);
    msprintf("[wcm] loadcount = %d", loadcount);
    ms_barrier_init(&g_disk.loadBarrier, loadcount + 1);
    dev_scan_add(&list);
    dev_scan_end(&list);

    ms_barrier_wait(&g_disk.loadBarrier);
    ms_barrier_uninit(&g_disk.loadBarrier);
    MFinfo("[Solin] wait barrier loadcount %d ok", loadcount);

//20171218 hugo add,  only 3536 have raid function, but 3798 platform/SDK also support raid,
//so if we want change 3798 to raid, then this code also will change.
#if defined (_HI3536_) || defined (_HI3536A_)
    if (g_disk.bRaidMode == MF_YES) {
        mf_comm_task_submit(dev_scan_raid, NULL);
    }
#endif

    mf_raid_info_dels();
    g_disk.bReady = MF_YES;

    MFinfo("MSFS_EVENT_DISK_ALL_OKAY !!!");
    mf_disk_event_notify(MSFS_EVENT_DISK_ALL_OKAY, NULL, MF_YES);
    disk_user_notify(MSFS_EVENT_DISK_ALL_OKAY, NULL);
    mf_record_update();

    return MF_SUCCESS;
}

static void *
disk_task_dev_loading(void *argv)
{
    struct diskObj *pstDev = argv;
    // MF_S8 taskName[64];
    TYPE_EN type = pstDev->enType;

    // sprintf(taskName, "dev[%d] loading", pstDev->port);
    // ms_task_set_name(taskName);

    MFinfo("dev port[%d] loading begin", pstDev->port);
    pstDev->loadNum++;
    dev_attribute(pstDev);
    if (dev_retr_add(pstDev) == MF_SUCCESS) {
        dev_list_add(pstDev);
        dev_update_status(pstDev, MF_YES, MSFS_EVENT_DISK_ADD);
        disk_user_notify(MSFS_EVENT_DISK_ADD, pstDev);
        dev_wait_stop(pstDev);
    } else {
        dev_umount(pstDev);
        dev_retr_del(pstDev);
        dev_clear_bit_port(pstDev);
        dev_wait_stop(pstDev);
        dev_unregister(pstDev);
    }

    if (g_disk.bReady == MF_NO && (type != DISK_TYPE_NAS)
        && (type != DISK_TYPE_NAND) && (type != DISK_TYPE_CIFS)) {
        // MFinfo("[Solin] wait barrier %s", taskName);
        MFinfo("[Solin] wait barrier port = %d", pstDev->port);
        ms_barrier_wait(&g_disk.loadBarrier);
    }
    MFinfo("dev port[%d] loading end", pstDev->port);

    return MF_SUCCESS;
}

MF_S32
mf_disk_dev_loading(struct diskObj *pstDev, MF_BOOL bAsync)
{
    struct load_s *pos = NULL;

    ms_mutex_lock(&g_disk.loadingMutex);
    list_for_each_entry(pos, &g_disk.loadingList, node) {
        if (pos->port == pstDev->port) {
            ms_mutex_unlock(&g_disk.loadingMutex);
            return MF_FAILURE;
        }
    }
    ms_mutex_unlock(&g_disk.loadingMutex);

    if (bAsync == MF_YES) {
        mf_comm_task_submit(disk_task_dev_loading, pstDev);
    } else {
        disk_task_dev_loading(pstDev);
    }

    return MF_SUCCESS;
}

MF_BOOL
mf_disk_is_ready()
{
    return g_disk.bReady;
}

void
mf_disk_ready_notify()
{
    if (g_disk.bReady) {
        mf_disk_event_notify(MSFS_EVENT_DISK_ALL_OKAY, NULL, MF_YES);
    }
}

void
mf_disk_scan(void)
{
    mf_comm_task_submit(disk_task_dev_scan, NULL);
}

MF_BOOL
mf_disk_is_can_record()
{
    return g_disk.bCanRecord;
}

void
mf_disk_usb_get_mntpath(MF_U8 port, MF_S8 *mntpath)
{
    MF_S32 i;

    if (!mntpath) {
        return ;
    }
    for (i = 0; i < USB_MAX_PORT; i++) {
        if (usb_table[i].port == port) {
            MFinfo("get port[%d]", port);
            snprintf(mntpath, DEV_NAME_LEN, "%s", usb_table[i].idname);
            break;
        }
    }
}

static MF_BOOL
client_call_result(struct rpc_client *client)
{
    switch (client->result) {
        case RPC_PROCUNAVAIL: {
            MFinfo("[rpc client:%d]RPC_PROCUNAVAIL", client->port);
            return MF_NO;
        }
        case RPC_TIMEDOUT: {
            MFinfo("[rpc client:%d]RPC_TIMEDOUT", client->port);
            return MF_NO;
        }
        case RPC_AUTHERROR: {
            MFinfo("[rpc client:%d]RPC_AUTHERROR", client->port);
            return MF_NO;
        }
        case RPC_FAILED: {
            MFinfo("[rpc client:%d]RPC_FAILED", client->port);
            return MF_NO;
        }
        case RPC_UNKNOWNPROTO: {
            MFinfo("[rpc client:%d]RPC_UNKNOWNPROTO", client->port);
            return MF_NO;
        }
        case RPC_UNKNOWNADDR: {
            MFinfo("[rpc client:%d]RPC_UNKNOWNADDR", client->port);
            return MF_NO;
        }
        case RPC_CANTENCODEARGS: {
            MFinfo("[rpc client:%d]RPC_CANTENCODEARGS", client->port);
            return MF_NO;
        }
        case RPC_PROGUNAVAIL: {
            MFinfo("[rpc client:%d]RPC_PROGUNAVAIL", client->port);
            return MF_NO;
        }
        case RPC_INTR: {
            MFinfo("[rpc client:%d]RPC_INTR", client->port);
            return MF_NO;
        }
        case RPC_SUCCESS: //MFinfo("[Solin][rpc client:%d]RPC_SUCCESS", client->port); break;
        default : //MFinfo("[Solin][rpc client %d]default", client->port);
            return MF_YES;
    }

    return MF_NO;
}

static MF_S32
client_dir_check(MF_S8 *host)
{
    struct exportslist *exports;
    struct exportslist *exp;
    struct exportslist *expTmp;
    struct grouplist *grp;
    struct grouplist *grpTmp;
    MF_S32 estat;
    MF_U32  find = 0;
    MF_S8 ipaddr[DEV_NAME_LEN] = {0};
    MF_S8 path[MAX_LEN_128] = {0};

    if (!host || nas_split_ip_path(host, ipaddr, path) != MF_SUCCESS) {
        MFerr("nas host err! (%s)", host);
        return MF_FAILURE;
    }

    if ((estat = callrpc(ipaddr, RPCPROG_MNT, 1,
                         RPCMNT_EXPORT, (xdrproc_t)xdr_void, (MF_S8 *)0,
                         (xdrproc_t)nas_exports, (MF_S8 *)&exports)) != 0) {
        clnt_perrno(estat);
        return MF_FAILURE;
    }
    exp = exports;
    while (exp) {
        if (find == 0 && strncmp(path, exp->ex_dirp, strlen(exp->ex_dirp)) == 0) {
            find = 1;
        }
        grp = exp->ex_groups;
        if (grp != NULL) {
            while (grp) {
                grpTmp = grp;
                grp = grp->gr_next;
                disk_mem_free(grpTmp);
            }
        }
        expTmp = exp;
        exp = exp->ex_next;
        disk_mem_free(expTmp);
    }

    return find == 0 ? MF_FAILURE : MF_SUCCESS;
}

static void
client_state_notify(struct rpc_client *client, MF_BOOL stat)
{
    struct diskObj *pstDev = NULL;
    NasInfoStr params;

    if (stat == MF_TRUE && !client->type) {
        /* if rpc is online, but dir not exist delay, should check all dir.*/
        switch (client->mntstat) {
            case MF_FAILURE:
                if (client_dir_check(client->host) == MF_SUCCESS) {
                    client->onState = MF_FALSE;
                } else {
                    stat = MF_FALSE;
                }
                break;
            case MF_SUCCESS:
                if (client_dir_check(client->host) == MF_FAILURE) {
                    stat = MF_FALSE;
                }
                break;
            default :
                break;
        }
    }

    if (client->onState == stat) {
        usleep(500000);
        return ;
    }

    client->onState = stat;
    if (stat == MF_TRUE) {
        ms_mutex_lock(&client->mutex);
        DEV_FIND_BY_PORT(client->port, pstDev);
        if (!pstDev) {
            if (client->bRunClients == MF_YES) {
                params.mounttype = client->type;
                snprintf(params.user, sizeof(params.user), "%s", client->user);
                snprintf(params.password, sizeof(params.password), "%s", client->password);
                dev_add_by_name(client->name, client->host, &client->mntstat, &params);
            }
        } else {
            client->mntstat = dev_mount(pstDev);
        }

        ms_mutex_unlock(&client->mutex);
    } else {
        //exit loading
        dev_loading_exit(client->port);
        dev_remove_by_port(client->port);
        client->mntstat = MF_FAILURE;
    }

    MFinfo("------->notify event:[%d:%s]\n", client->port, stat ? "NORMAL" : "OFFLINE");
}

static void
do_nfs_rpc_keepalive(struct rpc_client *client)
{
    static MF_S32 count = 0;
    static MF_S32 errnum = 0;
    struct timeval timeout = {RPC_TIMEOUT}; // must be great 0

    if (client->clnt == NULL) {
        // stay here, to create rpc client handle
        client->clnt = clnt_create(client->ipaddr, RPCPROG_MNT, 1, "tcp");
        if (client->clnt == NULL) {
            MFwarm("create %d %s clnt fail", client->port, client->ipaddr);
            usleep(1000 * 1000);
            return;
        }
        clnt_control(client->clnt, CLSET_TIMEOUT, (char *)&timeout);
        clnt_control(client->clnt, CLSET_RETRY_TIMEOUT, (char *)&timeout);// for udp
        count = 0;
        errnum = 0;
    }

    client->result = clnt_call(client->clnt, NULLPROC, (xdrproc_t)xdr_void, (caddr_t)NULL,
                               (xdrproc_t)xdr_void, (caddr_t)NULL, timeout); // time must greast 0s 0us  > 0

    if (client->result < 0) {
        usleep(1000 * 1000);
        return;
    }

    /* comfirm read/write event about disk error, reconnect after RPC_NAS_ERR_TOTAL times.*/
    ms_mutex_lock(&client->mutex);
    if (client->event == MSFS_EVENT_DISK_ERR && client->result == RPC_SUCCESS) {
        client->result = RPC_FAILED;
        if ((++errnum) >= RPC_NAS_ERR_TOTAL) {
            errnum = 0;
            client->event = MSFS_EVENT_NONE;
        }
    }
    ms_mutex_unlock(&client->mutex);

    if (client_call_result(client)) { //client->result == RPC_SUCCESS)
        count = 0;
        client_state_notify(client, MF_TRUE);
    } else {
        if (count >= RPC_RECONNE_TOTAL) {
            client_state_notify(client, MF_FALSE);
            count = 0;
        } else {
            count++;
        }
    }

    return;
}

static void
do_cifs_keepalive(struct rpc_client *client)
{
    MF_S32 i = 0;
    FILE *fp = NULL;
    MF_S8 cmd[MAX_LEN_256] = {0};
    MF_S8 buf[MAX_LEN_128] = {0};
    static MF_S32 count = 0;
    static MF_S32 errnum = 0;
    MF_S32 oldResult = client->result;

    client->result = RPC_FAILED;
    snprintf(cmd, sizeof(cmd), "ping -c 1 -w 1 %s", client->ipaddr);
    fp = ms_vpopen(cmd, "r");
    if (fp) {
        for (i = 0; i < 6; i++) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                MFshow("i:%d buf:%s", i, buf);
                if (strstr(buf, "ttl=") && strstr(buf, "time=")) {
                    client->result = RPC_SUCCESS;
                    break;
                } else if (strstr(buf, "0 packets received")) {
                    client->result = RPC_FAILED;
                    break;
                } else if (strstr(buf, "bad address")) {
                    client->result = RPC_FAILED;
                    break;
                } else if (strstr(buf, "unknown host")) {
                    client->result = RPC_FAILED;
                    break;
                }
            } else {
                client->result = RPC_FAILED;
                break;
            }
        }
        ms_vpclose(fp);
    }

    if (oldResult != client->result) {
        MFinfo("cifs keepalive result changed from %d --> %d", oldResult, client->result);
    }
    ms_mutex_lock(&client->mutex);
    if (client->event == MSFS_EVENT_DISK_ERR && client->result == RPC_SUCCESS) {
        client->result = RPC_FAILED;
        if ((++errnum) >= RPC_NAS_ERR_TOTAL) {
            errnum = 0;
            client->event = MSFS_EVENT_NONE;
        }
    }
    ms_mutex_unlock(&client->mutex);
    if (client->result == RPC_SUCCESS) {
        count = 0;
        client_state_notify(client, MF_TRUE);
    } else {
        if (count >= RPC_RECONNE_TOTAL) {
            client_state_notify(client, MF_FALSE);
            count = 0;
        } else {
            count++;
        }
    }

    return;
}


static void *
client_task_simple_run(void *argv)
{
    struct rpc_client *client = (struct rpc_client *)argv;

    while (client->bRunClients && g_disk.bRunClients) {
        if (!client->type) {
            do_nfs_rpc_keepalive(client);
        } else {
            do_cifs_keepalive(client);
        }
        usleep(1000 * 1000);
    }

    ms_mutex_lock(&g_disk.cmutex);
    /* remove disk when client on the main list */
    if (client->cremov_bk == MF_NO) {
        dev_loading_exit(client->port);
        // exit format
        dev_remove_by_port(client->port);
    }
    if (client->clnt) {
        clnt_destroy(client->clnt);
    }

    client_list_del(client);
    disk_mem_free(client);
    ms_mutex_unlock(&g_disk.cmutex);
    return MF_SUCCESS;
}

static MF_S32
client_task_simple(struct rpc_client *client)
{
    client->bRunClients = MF_YES;
    mf_comm_task_submit(client_task_simple_run, client);

    return MF_SUCCESS;
}

/*
* wait for releasing all clients with mutex. solin
*/
static void
client_list_release(struct disk *pstDisk)
{
    while (MF_TRUE) {
        usleep(500000);
        ms_mutex_lock(&g_disk.cmutex);
        if (!list_empty(&g_disk.clist)) {
            ms_mutex_unlock(&g_disk.cmutex);
            continue;
        }
        ms_mutex_unlock(&g_disk.cmutex);
        break;
    }
    while (MF_TRUE) {
        usleep(500000);
        ms_mutex_lock(&g_disk.cmutex);
        if (!list_empty(&g_disk.clist_bk)) {
            ms_mutex_unlock(&g_disk.cmutex);
            continue;
        }
        ms_mutex_unlock(&g_disk.cmutex);
        break;
    }
    /*list_for_each_entry_safe(client, n, &g_disk.clist, node)
    {

        if (client->pClients)
        {
            ms_task_join(&client->pClients);
        }
    }*/
    MFinfo("all client quit !!!");
    return ;
}

/*static void *
client_task_online(void *argv)
{
    struct rpc_client *client = NULL, *n = NULL;

    ms_task_set_name("client_task_online");
    // test rpc contact
    while(g_disk.bRunClients)
    {
        ms_mutex_lock(&g_disk.cmutex);
        list_for_each_entry_safe(client, n, &g_disk.clist, node)
        {
            if(client->clnt == NULL && client->bRunClients == MF_NO &&
                client->port > 0 && client->event == MSFS_EVENT_DISK_ADD)
            {
                client_task_simple(client);
                continue;
            }
        }
        ms_mutex_unlock(&g_disk.cmutex);
        usleep(500000);
    }

    client_list_release();
    MFinfo("[Solin] main client quit !!!");

    return MF_SUCCESS;
}*/

#ifdef SELECT_MODE

static void *
disk_task_hotplug(void *argv)
{
    struct list_head list;
    struct disk *pstDisk = (struct disk *)argv;
    struct sockaddr_nl client;
    struct timeval tv;
    MF_S32 skfd, rcvlen, ret;
    fd_set fds;
    MF_S32 buffersize = 1024;
    MF_S8 buf[1024] = { 0 };

    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    memset(&client, 0, sizeof(client));
    client.nl_family = AF_NETLINK;
    client.nl_pid = getpid();
    client.nl_groups = 1; /* receive broadcast message*/
    setsockopt(skfd, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
    bind(skfd, (struct sockaddr *)&client, sizeof(client));

    ms_task_set_name("disk_task_hotplug");
    while (pstDisk->bRunHotplug) {
        FD_ZERO(&fds);
        FD_SET(skfd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;
        ret = select(skfd + 1, &fds, NULL, NULL, &tv);
        if (ret < 0) {
            continue;
        }
        if (!(ret > 0 && FD_ISSET(skfd, &fds))) {
            continue;
        }

        /* receive data */
        rcvlen = recv(skfd, &buf, sizeof(buf), 0);
        if (rcvlen > 0 && strstr(buf, "block")) {
            MFinfo("[hotplug] %s", buf);
            dev_scan_begin(&list);
            if (strstr(buf, "add@")) {
                dev_scan_add(&list);
            } else if (strstr(buf, "remove@")) {
//                dev_scan_remove(&list);
            }
            dev_scan_end(&list);
            DEV_FIND_AND_PRINT();
        }
    }

    close(skfd);
    return MF_SUCCESS;
}
#else

static MF_S32
disk_scan_linkname(char *dname, struct list_head *dlist)
{
    MF_S32 dcount = 0;
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    stscan *scan = NULL;
    char *dir_dev = "/dev";

    if (!dname || !dlist) {
        return dcount;
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
        if ((ptr->d_type == DT_LNK) && strstr(ptr->d_name, dname)) {
            dcount++;
            scan = disk_mem_calloc(1, sizeof(stscan));
            sprintf(scan->name, "%s/%s", dir_dev, ptr->d_name);
            MFinfo("dcount:%d %s", dcount, scan->name);
            list_add_tail(&scan->node, dlist);
        }
    }
    closedir(dir);

    return dcount;
}

static void
disk_scan_symlink(struct sdinfo *si, MF_S8 *prename)
{
    MF_S8 oldname[DISK_NAME_LEN * 2] = {0};
    MF_S8 newname[DISK_NAME_LEN * 2] = {0};
    struct list_head dlist;
    stscan *pos = NULL, *n = NULL;

    if (!si || !prename) {
        return;
    }
    INIT_LIST_HEAD(&dlist);
    switch (si->stat & (SD_S_ADD_DEV | SD_S_REMOVE_DEV)) {
        case SD_S_REMOVE_DEV: {
            sprintf(newname, "/dev/%s%s", prename, si->sdname);
            if (disk_scan_linkname(prename, &dlist)) {
                list_for_each_entry_safe(pos, n, &dlist, node) {
                    MFinfo("[Solin] unlink %s !!!!!", pos->name);
                    msprintf("[wcm] unlink pos->name = %s", pos->name);
                    unlink(pos->name);
                    list_del(&pos->node);
                    disk_mem_free(pos);
                }
            }
            break;
        }
        case SD_S_ADD_DEV: {
            sprintf(newname, "/dev/%s%s", prename, si->sdname);
            sprintf(oldname, "/dev/%s", si->sdname);
            // should delete pre-symlink
            msprintf("[wcm] unlink newname = %s", newname);
            unlink(newname);
            MFinfo("[Solin] symlink %s %s !!!!!", oldname, newname);
            msprintf("[wcm] symlink oldname = %s, newname = %s", oldname, newname);
            symlink(oldname, newname);
            break;
        }
        default:
            break;
    }
}

static void
disk_scan_udisk_linkname(struct sdinfo *si, MF_S32  *usbnum)
{
    MF_S32 i;
    MF_S8 prename[DISK_NAME_LEN] = {0};
    MF_S8 sdname[DISK_NAME_LEN] = {0};
    MF_S8 delname[DISK_NAME_LEN * 2] = {0};
    char *blk_dev = "MF";
    struct diskObj *pstDev = NULL;

    if (!si) {
        return ;
    }

    switch (si->stat & (SD_S_ADD_DEV | SD_S_REMOVE_DEV)) {
        case SD_S_REMOVE_DEV: {
            DIR *dir = opendir("/dev");
            struct dirent *ptr = NULL;
            while ((ptr = readdir(dir)) != NULL) {
                if ((ptr->d_type == DT_LNK) && (strncmp(ptr->d_name, blk_dev, strlen(blk_dev)) == 0)) {
                    sscanf(ptr->d_name, "%*[^_]_%*[^_]_%*d_%s", sdname);
                    if (strncmp(si->sdname, sdname, strlen(si->sdname)) == 0) {
                        sprintf(delname, "/dev/%s", ptr->d_name);
                        MFinfo("DEL: [%s]\n", delname);
                        unlink(delname);
                        break;
                    }
                }
            }
            closedir(dir);
            break;
        }
        case SD_S_ADD_DEV: {
            if (*usbnum >= USB_MAX_PORT) {
                MFinfo("USB: list is fulled, return");
                return;
            }

            for (i = 0; i < USB_MAX_PORT; i++) {
                g_disk.usbindex++;
                if (g_disk.usbindex > USB_DISK_PORT || g_disk.usbindex < USB_START_PORT) {
                    g_disk.usbindex = USB_START_PORT;
                }
                pstDev = NULL;
                DEV_FIND_BY_PORT(g_disk.usbindex, pstDev);
                if (!pstDev) {
                    snprintf(prename, DISK_NAME_LEN, "MF_usb_%d_", g_disk.usbindex);
                    MFinfo("ADD: [%s] ", prename);
                    disk_scan_symlink(si, prename);
                    (*usbnum)++;
                    break;
                } else {
                    (*usbnum)++;
                }
            }

            if (i == USB_MAX_PORT) {
                MFinfo("USB: list is fulled");
            }
            break;
        }
        default:
            break;
    }
    return ;
}

static void
disk_scan_all(sdinfoall *sdia)
{
    MF_S32 i = 0;
    struct disk_scan_ *pos = NULL;
    MF_S32  usbnum = 0;

    if (!sdia || sdia->num <= 0) {
        return;
    }

    for (; i < sdia->num; i++) {
        if (sdia->siall[i].isusb) {
            disk_scan_udisk_linkname(&sdia->siall[i], &usbnum);
        } else {
            list_for_each_entry(pos, &g_dscan_.node, node) {
                if (sdia->siall[i].port == pos->port) {
                    disk_scan_symlink(&sdia->siall[i], pos->name);
                    break;
                }
            }
        }
    }
}

static MF_S32
disk_sdinfo_ioctl(void)
{
    MF_S32 ret = 0;
    sdinfoall sdia;

    ret = ioctl(g_disk_drv_fd, IOSTHOTPLUG, NULL);
    if (ret <= 0) { //first scan will not do with add or remove operate.
        return MF_FAILURE;
    }

    memset(&sdia, 0, sizeof(sdinfoall));
    ret = ioctl(g_disk_drv_fd, IOSTALLSTAT, &sdia);
    if (ret <= 0 || sdia.num <= 0) {
        MFinfo("IOSTALLSTAT fail !");
        return MF_FAILURE;
    }
    disk_scan_all(&sdia);

    return ret;
}

static void
disk_send_state_to_user(struct disk *pstDisk)
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    MF_S32 count = 0;
    static MF_PTS s_timeFull = 0;
    static EVENT_E enDiskEvt = MSFS_EVENT_NONE;
    MF_BOOL canRecord = 0;
    static MF_PTS s_timeStates[MAX_DISK_PORT_NUM] = {0};

    MF_PTS now = time(NULL);
    ms_rwlock_rdlock(&pstDisk->rwlock);

    list_for_each_entry_safe(pos, n, &pstDisk->list, node) {
        if (pos->bRec == MF_NO) {
            continue;
        }

        if (!canRecord && pos->enRw
            && (pos->enState == DISK_STATE_NORMAL || pos->enState == DISK_STATE_FORMATING)
            && (pos->bLoop || pos->free)) {
            canRecord = 1;
        }

        count++;
        if (pos->bLoop == MF_NO && pos->free == 0) {
            if ((pos->bLastFull == MF_NO || now - s_timeFull >= NOTY_INTERVAL) &&
                pos->enState == DISK_STATE_NORMAL) {
                disk_user_notify(MSFS_EVENT_DISK_FULL, pos);
                s_timeFull = now;
            }
            pos->bLastFull = MF_YES;
        } else {
            if (pos->bLastFull == MF_YES && pos->enState == DISK_STATE_NORMAL) {
                disk_user_notify(MSFS_EVENT_DISK_NORMAL, pos);
                pos->bLastFull = MF_NO;
            }
        }

        if (pos->enLastState == pos->enState && (now - s_timeStates[pos->port] < NOTY_INTERVAL)) {
            continue;
        }

        s_timeStates[pos->port] = now;
        pos->enLastState = pos->enState;
        if (pos->enState == DISK_STATE_BAD) {
            // disk_user_notify(MSFS_EVENT_DISK_ERR, pos); // notify by @disk_check_state
        } else if (pos->enState == DISK_STATE_UNFORMATTED) {
            disk_user_notify(MSFS_EVENT_DISK_UNFORMAT, pos);
        } else if (pos->enState == DISK_STATE_OFFLINE) {
            // disk_user_notify(MSFS_EVENT_DISK_OFFLINE, pos); // notify by @disk_check_state
        } else if (pos->bLastFull == MF_NO) {
            disk_user_notify(MSFS_EVENT_DISK_NORMAL, pos);
        }
    }
    ms_rwlock_unlock(&pstDisk->rwlock);

    if (count) {
        if (enDiskEvt != MSFS_EVENT_HAVE_DISK) {
            disk_user_notify(MSFS_EVENT_HAVE_DISK, NULL);
            enDiskEvt = MSFS_EVENT_HAVE_DISK;
        }
    } else {
        if (enDiskEvt != MSFS_EVENT_NO_DISK || now - s_timeStates[pos->port] >= NOTY_INTERVAL) {
            disk_user_notify(MSFS_EVENT_NO_DISK, NULL);
            s_timeStates[pos->port] = now;
        }

        enDiskEvt = MSFS_EVENT_NO_DISK;
    }
    g_disk.bCanRecord = canRecord;
}

static void *
disk_task_hotplug(void *argv)
{
    struct list_head list;
    struct disk *pstDisk = (struct disk *)argv;
    MF_S32 ret;

    int count = 0;
    struct list_head head;
    INIT_LIST_HEAD(&head);
    int badPortTimes = 0;

    if (g_disk_drv_fd < 0) {
        MFerr("--open %s error--\n", DISK_DRV_PATH);
        return 0;
    }

    ms_task_set_name("disk_task_hotplug");
    while (pstDisk->bRunHotplug) {
        usleep(100000);

        if (g_disk.bReady == MF_YES) {
            ret = disk_sdinfo_ioctl();
            if (ret > 0) {
                if (ret & SD_S_ADD_DEV) {
                    MFinfo("SD_S_ADD_DEV. \n");
                    dev_scan_begin(&list);
                    dev_scan_add(&list);
                    dev_scan_end(&list);
                    //check add disk is global disk or not  for raid recovery
                }

                if (ret & SD_S_REMOVE_DEV) {
                    MFinfo("SD_S_REMOVE_DEV. \n");
                    dev_scan_begin(&list);
                    dev_scan_remove(&list);  //send notify to other modules
                    dev_scan_end(&list);
                    mf_check_raid_status(); //check raid  degrade, then find global disk for recovery
                }
            }
        }
        //DEV_FIND_AND_PRINT();

        if ((count + 1) % 3000 == 0 || badPortTimes) {
            if (badPortTimes) {
                msprintf("[wcm] badPortTimes = %d", badPortTimes);
            }
            msfs_disk_get_bad_port(&head);
            if (!list_empty(&head)) {
                if (badPortTimes < 30) { // 3s
                    ++badPortTimes;
                } else {
                    msprintf("[wcm] msfs_disk_bad_reload begin");
                    msfs_disk_bad_reload(&head);
                    msprintf("[wcm] msfs_disk_bad_reload end");
                    badPortTimes = 0;
                }
                msfs_disk_put_bad_port(&head);
            } else {
                badPortTimes = 0;
            }
        }
    }

    return MF_SUCCESS;
}
#endif

static void *
disk_task_state(void *argv)
{
    struct disk *pstDisk = (struct disk *)argv;
    MF_S32 count = 0;

    ms_task_set_name("disk_task_state");
    while (pstDisk->bRunState) {
        usleep(100000);

        if (pstDisk->bReady == MF_NO) {
            continue;
        }
        if (pstDisk->bNotify == MF_NO) {
            continue;
        }

        if (count % 30 == 0) {
            disk_send_state_to_user(pstDisk);
        }
//      if (count % 50 == 0)
//          mf_record_update();
        count++;
    }

    return MF_SUCCESS;
}

static void *
disk_task_io_process(void *argv)
{
    struct disk *pstDisk = argv;
    struct ioReq *pstIOReq = NULL;
    struct ioReq *n = NULL;
    MF_S32 find = 0;
    MF_S32 i = 0;
    ms_task_set_name("task_io_process");

    cpu_set_t mask;
    CPU_ZERO(&mask);
#ifdef HI_3536
    CPU_SET(1, &mask);
#else
    CPU_SET(2, &mask);
#endif
    MFinfo("task_drop_caches run at cpu:1.");
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        MFerr("set thread affinity failed.");
    }

    //1004 脚本卡住问题
    mf_set_task_policy(pstDisk->pIOTask, THTEAD_SCHED_RR_PRI);

    while (pstDisk->bRunIOTask) {
        for (i = 0; i <= MAX_DISK_PORT_NUM; i++) {
            find = 0;
            ms_mutex_lock(&pstDisk->IOMutex);
            list_for_each_entry_safe(pstIOReq, n, &pstDisk->IOList[i], node) {
                if (pstDisk->IONowOffset[i] <= pstIOReq->fileOffset) {
                    list_del(&pstIOReq->node);
                    do {
                        ms_mutex_lock(pstIOReq->pMutex);
                        mf_cond_signal(pstIOReq->pCond);
                        ms_mutex_unlock(pstIOReq->pMutex);
                    } while (pstIOReq->bDone == MF_NO);
                    pstDisk->IONowOffset[i] = pstIOReq->fileOffset;
//                    printf("=====doooooooo======port[%d]====offset[%lld]====\n", pstIOReq->port, pstIOReq->fileOffset);
                    ms_free(pstIOReq);
                    find = 1;
                }
            }
            if (!find) {
                pstDisk->IONowOffset[i] = 0;
            }
            ms_mutex_unlock(&pstDisk->IOMutex);
        }
        usleep(20000);
    }

    ms_mutex_lock(&pstDisk->IOMutex);
    for (i = 0; i <= MAX_DISK_PORT_NUM; i++) {
        list_for_each_entry_safe(pstIOReq, n, &pstDisk->IOList[i], node) {
            list_del(&pstIOReq->node);
            ms_free(pstIOReq);
        }
    }
    ms_mutex_unlock(&pstDisk->IOMutex);
    return NULL;

}

static void *
disk_task_health_temperature_check(void *argv)
{
    struct diskObj *pstDev = argv;
    MF_S8 buf[MAX_LEN_2048] = {0};
    MF_S8 cmd[128] = {0};
    MF_S8 type[24], update[16], failed[16], raw_val[24];
    MF_S8 *startStr = NULL;
    MF_S32 flag, val, worst, thresh;

    //1. get  Temperature
    snprintf(cmd, sizeof(cmd), "smartctl -A %s | grep -v ^$", pstDev->alias);
    output_execute_Info(cmd, buf, sizeof(buf));
    startStr = strstr(buf, "194 Temperature_Celsius");
    if (startStr != NULL) {
        startStr += strlen("194 Temperature_Celsius");
        if (sscanf(startStr, "%x %d %d %d %23s %15s %15s %23[^\n]",
                   &flag, &val, &worst, &thresh, type, update, failed, raw_val) != 8) {
            return MF_SUCCESS;
        }
        time_t     currentTime = time(NULL);
        struct tm *timeInfo    = localtime(&currentTime);
        timeInfo->tm_sec       = 0;
        time_t minTimestamp   = mktime(timeInfo);
        pstDev->logTemperature[pstDev->lastTemperatureIndex].timestamp = minTimestamp;
        if (diskHealthTemperature[pstDev->port] != 0) {
            pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature = diskHealthTemperature[pstDev->port];
            pstDev->avgTemperature = pstDev->avgTemperature != 0 ? (pstDev->avgTemperature + diskHealthTemperature[pstDev->port]) / 2:diskHealthTemperature[pstDev->port];
        } else {
            pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature = strtol(raw_val, NULL, 10);
            pstDev->avgTemperature = pstDev->avgTemperature != 0 ? (pstDev->avgTemperature + pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature) / 2:pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature;
        }
    }
    msprintf("gsjt disk_task_health_check port:%d temperature:%d lastTemperatureIndex%d %d", pstDev->port, pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature, pstDev->lastTemperatureIndex,pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature > 60);

    if (pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature <= 0) {
        disk_user_notify(MSFS_EVENT_DISK_MICROTHERM, pstDev);
    } else if (pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature > 60) {
        msprintf("gsjt disk_user_notify MSFS_EVENT_DISK_HEAT");
        disk_user_notify(MSFS_EVENT_DISK_HEAT, pstDev);
    } else {
        disk_user_notify(MSFS_EVENT_DISK_HEALTH_TEMPERATURE_NORMAL, pstDev);
    }

    if (pstDev->HMStatus == 101) {
        disk_user_notify(MSFS_EVENT_DISK_CONNECTION_EXCEPTION, pstDev);
    } else if (pstDev->HMStatus == 102 || pstDev->HMStatus == 105) {
            disk_user_notify(MSFS_EVENT_DISK_STRIKE, pstDev);
    } else {
            disk_user_notify(MSFS_EVENT_DISK_HEALTH_STATUS_NORMAL, pstDev);
    }
    return MF_SUCCESS;
}

static void enqueueHelathTask(void *argv) {
    struct diskObj *pstDisk = argv;
    sem_wait(&healthTaskSemaphore);
    if (healthTaskCount < MAX_HEALTH_TASK -1) {
        msprintf("gsjt enqueueHelathTask port check :%d",pstDisk->port);
        helathTaskQueue[healthTaskCount] = pstDisk;
        healthTaskCount++;
    }
    sem_post(&healthTaskSemaphore);
}

diskObj *dequeueHelathTask() {
    sem_wait(&healthTaskSemaphore);
    diskObj *disk = NULL;
    if (healthTaskCount > 0) {
        disk = helathTaskQueue[0];
        msprintf("gsjt dequeueHelathTask port check :%d",disk->port);
    }
    int i = 0;
    for (i = 0; i < healthTaskCount - 1; i++) {
        helathTaskQueue[i] = helathTaskQueue[i + 1];
    }
    healthTaskCount--;
    if (healthTaskCount <= 0) {
        healthTaskCount = 0;
    }
    sem_post(&healthTaskSemaphore);
    return disk;
}

static void *
disk_task_health_status_check(void *argv)
{
    struct disk *pstDisk = argv;
    struct diskObj *disk = NULL;;
    MF_S8 buf[MAX_LEN_2048] = {0};
    MF_S8 cmd[128] = {0};
    MF_S8 *startStr = NULL;
    ms_task_set_name("disk_task_health_status_check");
    sem_init(&healthTaskSemaphore, 0, 1);
    msprintf("gsjt disk_task_health_status_check statr");
    while (pstDisk->bRunHMTask) {
        disk = dequeueHelathTask();
        if (disk) {
            msprintf("gsjt port:%d status start check", disk->port);
            // 默认不支持，等检测出可以有导出结果的csv文件才支持。第一次进入时HMStatus为0
            if (disk->supportHM || disk->HMStatus == 0) {
                if (diskHealthstatus[disk->port] != 0) {
                    disk->HMStatus  = diskHealthstatus[disk->port];
                    disk->supportHM = MF_TRUE;
                } else {
                    snprintf(cmd, sizeof(cmd), "DHMTool -d %s --dhm --delayCMDSegment 1000 --outputPath /mnt/nand3 --outputLog HMdiskPort%d_%s.csv", disk->alias, disk->port, disk->wwn);
                    output_execute_Info(cmd, buf, sizeof(buf));
                    if (disk == NULL) {
                        continue;
                    }
                    startStr = strstr(buf, "!!!");
                    if (startStr != NULL) {
                        sscanf(startStr, "%*[^0-9]%d", &disk->HMStatus);
                    } else {
                        disk->HMStatus = -1;
                    }
                    if (disk->HMStatus != 0 && disk->HMStatus < 100) {
                        disk->supportHM = MF_FALSE;
                    } else {
                        disk->supportHM = MF_TRUE;
                    }
                    msprintf("gsjt port:%d status:%d", disk->port, disk->HMStatus);
                }
            }
            msprintf("gsjt port:%d status end check", disk->port);
        }
        usleep(100* 1000);
    }
    sem_destroy(&healthTaskSemaphore);
    return NULL;
}

static void *
disk_task_health_write(void *argv)
{
    struct diskObj *pstDev   = argv;
    MF_S8           path[64] = {0};
    MF_S32          i        = 0,j =0;
    snprintf(path, sizeof(path), "/mnt/nand3/diskPort%d_%s", pstDev->port, pstDev->wwn);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        return MF_SUCCESS;
    }
    //数组没满放到末尾，数组满了重新排序
    time_t     currentTime = time(NULL);
    struct tm *timeInfo    = localtime(&currentTime);
    timeInfo->tm_min       = 0;
    timeInfo->tm_sec       = 0;
    time_t hourTimestamp   = mktime(timeInfo);
    if (pstDev->lastTemperatureIndex == MAX_DISK_HM_TEMPERATURE - 1) {
        for (i = 0; i < MAX_DISK_HM_TEMPERATURE - 2; i++) {
            pstDev->logTemperature[i].timestamp = pstDev->logTemperature[i + 1].timestamp;
            pstDev->logTemperature[i].temperature = pstDev->logTemperature[i + 1].temperature;
        }
        pstDev->logTemperature[i].timestamp = hourTimestamp;
        pstDev->logTemperature[i].temperature = pstDev->avgTemperature;
    } else {
        pstDev->logTemperature[pstDev->lastTemperatureIndex].timestamp = hourTimestamp;
        pstDev->logTemperature[pstDev->lastTemperatureIndex].temperature = pstDev->avgTemperature;
        pstDev->lastTemperatureIndex++;
    }
    pstDev->avgTemperature = 0;

    i = 0;
    time_t earlier_time = hourTimestamp - (720 * 3600);//过滤超过30天的数据
    //最多显示720小时数据，第720条一定是当前时间，因此日志最多719条
    while (i < pstDev->lastTemperatureIndex) {
        if (pstDev->logTemperature[i].timestamp < earlier_time) {
            i++;
            continue;
        }
        fprintf(file, "%ld;%d\n", pstDev->logTemperature[i].timestamp, pstDev->logTemperature[i].temperature);
        i++;
    }

    fclose(file);
}

static void *
disk_task_health_management(void *argv)
{
    struct disk *pstDisk = argv;
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    MF_S32 cnt = 0, checkCnt = 0;
    ms_task_set_name("disk_task_health_management");
    msprintf("gsjt disk_task_health_management create");
    while (pstDisk->bRunHMTask) {
        if (cnt >= 120) {//1 min
            ms_rwlock_rdlock(&pstDisk->rwlock);
            list_for_each_entry_safe(pos, n, &pstDisk->list, node)
            {
                if ((pos->enType != DISK_TYPE_LOCAL && pos->enType != DISK_TYPE_ESATA && pos->enType != DISK_TYPE_IN_RAID) || !pos->bReadyHM) {
                    continue;
                }
                mf_comm_task_submit(disk_task_health_temperature_check, pos);
            }
            
            if (healthCheckedCount >= 5) { // 5min
                healthCheckedCount = 0;
                msprintf("gsjt port check :%d", pos->port);
                list_for_each_entry_safe(pos, n, &pstDisk->list, node)
                {
                    if ((pos->enType != DISK_TYPE_LOCAL && pos->enType != DISK_TYPE_ESATA && pos->enType != DISK_TYPE_IN_RAID) || !pos->bReadyHM) {
                        continue;
                    }
                    if ((pstDisk->HMEnableMask & ((UInt64)1 << pos->port)) == 0) {
                        continue;
                    }
                    enqueueHelathTask(pos);
                }
            }
            if (checkCnt >= 60) {//1h
                list_for_each_entry_safe(pos, n, &pstDisk->list, node)
                {
                    if ((pos->enType != DISK_TYPE_LOCAL && pos->enType != DISK_TYPE_ESATA && pos->enType != DISK_TYPE_IN_RAID) || !pos->bReadyHM) {
                        continue;
                    }
                    disk_task_health_write(pos);
                }
                checkCnt = 0;
            }
            cnt = 0;
            checkCnt ++;
            healthCheckedCount ++;
            ms_rwlock_unlock(&pstDisk->rwlock);
        }
        cnt++;
        usleep(500 * 1000);
    }
    return NULL;
}

static MF_S32
disk_task_create(struct disk *pstDisk)
{
    pstDisk->bRunHotplug = MF_YES;
    ms_task_create_join_stack_size(&pstDisk->pHotplug, 64 * 1024, disk_task_hotplug, pstDisk);

    pstDisk->bRunState = MF_YES;
    ms_task_create_join_stack_size(&pstDisk->pState, 16 * 1024, disk_task_state, pstDisk);

    pstDisk->bRunIOTask = MF_YES;
    ms_task_create_join_stack_size(&pstDisk->pIOTask, 16 * 1024, disk_task_io_process, pstDisk);

#if defined (_HI3536A_)
    pstDisk->bRunHMTask = MF_YES;
    ms_task_create_join_stack_size(&pstDisk->pHMTask, 16 * 1024, disk_task_health_management, pstDisk);
    ms_task_create_join_stack_size(&pstDisk->pHMSTATUSTask, 16 * 1024, disk_task_health_status_check, pstDisk);
#endif

    return MF_SUCCESS;
}

static void
disk_task_destroy(struct disk *pstDisk)
{
    pstDisk->bRunHotplug = MF_NO;
    if (pstDisk->pHotplug) {
        ms_task_join(&pstDisk->pHotplug);
    }

    pstDisk->bRunState = MF_NO;
    if (pstDisk->pState) {
        ms_task_join(&pstDisk->pState);
    }

    pstDisk->bRunIOTask = MF_NO;
    if (pstDisk->pIOTask) {
        ms_task_join(&pstDisk->pIOTask);
    }
#if defined (_HI3536A_)
    pstDisk->bRunHMTask = MF_NO;
    if (pstDisk->pHMTask) {
        ms_task_join(&pstDisk->pHMTask);
    }
    if (pstDisk->pHMSTATUSTask) {
        ms_task_join(&pstDisk->pHMSTATUSTask);
    }
#endif
}

static MF_S32
client_task_create(struct disk *pstDisk)
{
    pstDisk->bRunClients = MF_YES;
    //ms_task_create_join(&pstDisk->pClients, client_task_online, pstDisk);

    return MF_SUCCESS;
}

static void
client_task_destroy(struct disk *pstDisk)
{
    pstDisk->bRunClients = MF_NO;
    client_list_release(pstDisk);
    //if (pstDisk->pClients)
    //    ms_task_join(&pstDisk->pClients);
}

static unsigned int disk_ata_err(unsigned int err_mask)
{
    if (err_mask & AC_ERR_HOST_BUS) {
        return AC_ERR_HOST_BUS;
    }
    if (err_mask & AC_ERR_ATA_BUS) {
        return AC_ERR_ATA_BUS;
    }
    if (err_mask & AC_ERR_TIMEOUT) {
        return AC_ERR_TIMEOUT;
    }
    if (err_mask & AC_ERR_HSM) {
        return AC_ERR_HSM;
    }
    if (err_mask & AC_ERR_SYSTEM) {
        return AC_ERR_SYSTEM;
    }
    if (err_mask & AC_ERR_MEDIA) {
        return AC_ERR_MEDIA;
    }
    if (err_mask & AC_ERR_INVALID) {
        return AC_ERR_INVALID;
    }
    if (err_mask & AC_ERR_DEV) {
        return AC_ERR_DEV;
    }
    return AC_ERR_OTHER;
}

/*
* off_sector: = offset / sector
* count_sectorf: = count / sector
*/
ERROR_E
mf_disk_error(MF_S32 fd, MF_U64 offset, MF_S32 count, MF_U32 sector)
{
    MF_S32 i = 0;
    sd_rwerr rwerr;
    sd_err_info *info = NULL;
    MF_U64 offsec = offset / MF_MAX(sector, 1);
    MF_S32 countsec = count / MF_MAX(sector, 1);
    MF_U64 inipos = 0;
    MF_U64 endpos = 0;
    int err = DISK_ERR_UNKNOWN;

    memset(&rwerr, 0, sizeof(rwerr));
    rwerr.fd = fd;
    if (!disk_get_rwerr_cmd(&rwerr)) {
        return err;
    }

    for (; i < rwerr.count; i++) {
        info = &rwerr.info[i];
        MFinfo(">> err_mask:%u cmd_cmd:%u res_cmd:%u res_feature:%u pos:%llu",
               info->err_mask, info->cmd_cmd, info->res_cmd,
               info->res_feature, info->pos);
        switch (disk_ata_err(info->err_mask)) {
            case AC_ERR_MEDIA: {
                if (err != DISK_ERR_BADBLOCK && (info->res_cmd & ATA_DRDY) &&
                    (info->res_feature & ATA_UNC) && (info->pos > 0) &&
                    ((info->cmd_cmd == ATA_CMD_FPDMA_READ) ||
                     (info->cmd_cmd == ATA_CMD_FPDMA_WRITE))) {
                    inipos = (offsec / 8) * 8;// 8: read/write logcal sector count once. default 4k
                    countsec = countsec % 8 ? ALIGN_UP(countsec, 8) : countsec;// default 4k: 8 sector
                    endpos = offsec + countsec; // pos :inipos - end
                    if (info->pos == inipos || (info->pos > inipos && info->pos < endpos)) {
                        MFinfo(">>>>[Solin] badblock !");
                        err = DISK_ERR_BADBLOCK;
                    }
                }
            }
            break;
            case AC_ERR_DEV: {
                return DISK_ERR_DEV_BAD;
            }
            break;
            case AC_ERR_TIMEOUT:
            case AC_ERR_ATA_BUS:
            case AC_ERR_OTHER:
            default:
                break;
        }
    }
    return err;
}

MF_S32
mf_disk_format(MF_U8 port, FORMAT_EN enFormat, MF_U64 quota)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_FAILURE;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, port, 0, pstDev->enType);
        if (pstDev->enState == DISK_STATE_OFFLINE) {
            return MF_FAILURE;
        }
        ms_mutex_lock(&pstDev->fmtMutex);
        mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pstDev, MF_YES);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, port, mf_random(0, 10), pstDev->enType);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);
        // enter format list
        dev_format_list_add(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, port, mf_random(15, 20), pstDev->enType);
        res = dev_format(pstDev, enFormat, quota);
        // exit format list
        dev_format_list_del(pstDev);

        switch (pstDev->enType) {
            case DISK_TYPE_LOCAL:
            case DISK_TYPE_ESATA:
                if (dev_is_alive(pstDev) == MF_NO) {
                    dev_clear_bit_port(pstDev);
                    ms_mutex_unlock(&pstDev->fmtMutex);
                    dev_unregister(pstDev);
                    res = MF_FAILURE;
                    MFerr("format ng to del");
                    break;
                }
            default : {
                mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, port, mf_random(80, 85), pstDev->enType);
                dev_attribute(pstDev);
                mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, port, mf_random(86, 89), pstDev->enType);
                mf_retr_disk_info_add(pstDev);
                dev_list_add(pstDev);
                mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, port, mf_random(94, 98), pstDev->enType);
                mf_disk_event_notify(MSFS_EVENT_DISK_ADD, pstDev, MF_YES);
                mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, port, 100, pstDev->enType);
                ms_mutex_unlock(&pstDev->fmtMutex);
                MFinfo("format ok to re-add");
            }
            break;
        }
    }

    return res;
}

MF_S32
mf_disk_restore(MF_U8 port)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_SUCCESS;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, 0, pstDev->enType);
        if (pstDev->enState == DISK_STATE_OFFLINE) {
            return MF_FAILURE;
        }
        ms_mutex_lock(&pstDev->fmtMutex);
        mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pstDev, MF_YES);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(0, 10), pstDev->enType);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(15, 20), pstDev->enType);
        res = dev_restore(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(80, 85), pstDev->enType);
        dev_attribute(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(86, 89), pstDev->enType);
        mf_retr_disk_info_add(pstDev);
        dev_list_add(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(94, 98), pstDev->enType);
        mf_disk_event_notify(MSFS_EVENT_DISK_ADD, pstDev, MF_YES);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, 100, pstDev->enType);
        ms_mutex_unlock(&pstDev->fmtMutex);
    }

    return res;
}

MF_S32
mf_disk_restoreEx(MF_U8 port)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_SUCCESS;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, 0, pstDev->enType);
        if (pstDev->enState == DISK_STATE_OFFLINE) {
            return MF_FAILURE;
        }
        ms_mutex_lock(&pstDev->fmtMutex);
        mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pstDev, MF_YES);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(0, 10), pstDev->enType);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(15, 20), pstDev->enType);
        res = dev_restoreEx(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(80, 85), pstDev->enType);
        dev_attribute(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(86, 89), pstDev->enType);
        mf_retr_disk_info_add(pstDev);
        dev_list_add(pstDev);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, mf_random(94, 98), pstDev->enType);
        mf_disk_event_notify(MSFS_EVENT_DISK_ADD, pstDev, MF_YES);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_RESTORE, port, 100, pstDev->enType);
        ms_mutex_unlock(&pstDev->fmtMutex);
    }

    return res;
}

MF_S32
mf_disk_attribute(struct diskObj *disk)
{
    return dev_attribute(disk);
}

MF_BOOL
mf_disk_is_exist(MF_U8 port)
{
    struct diskObj *pstDev = NULL;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev && pstDev->enState != DISK_STATE_OFFLINE) {
        return MF_YES;
    }
    return MF_NO;
}

MF_BOOL
mf_disk_can_write(struct diskObj *disk, MF_U8 chnId)
{
    if (disk == NULL) {
        return MF_NO;
    }

    if (disk->enState != DISK_STATE_NORMAL) {
        MFdbg("disk->enState[%d] != DISK_STATE_NORMAL", disk->enState);
        return MF_NO;
    }

    if (disk->bRec == MF_NO) {
        MFdbg("disk->bRec == MF_NO");
        return MF_NO;
    }

    if (disk->enRw == DISK_R_ONLY) {
        MFdbg("disk->enRw == DISK_R_ONLY");
        return MF_NO;
    }

    if (disk->bLoop == MF_NO && disk->free == 0) {
        MFdbg("disk->bLoop disable & disk full");
        return MF_NO;
    }

    struct mf_grp *grp = &g_disk.stGroup;

    if (grp->bEnable == MF_YES) {
        MF_U64  chnMask;

        chnMask = grp->gn[disk->group].chnMask;
        if (chnMask & ((MF_U64)1 << chnId)) {
            MFdbg("inside of group");
            return MF_YES;
        } else {
            MFdbg("outside of group");
            return MF_NO;
        }
    }

    return MF_YES;
}

void
mf_disk_port_bind_group(MF_U8 port, MF_U8 group)
{
    struct diskObj *pstDev = NULL;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        pstDev->group = group;
    }
    g_disk.stGroup.pg[port].gId = group;
}

void
mf_disk_chn_bind_group(MF_U64 chnMask, MF_U8 group)
{
    g_disk.stGroup.gn[group].chnMask = chnMask;
}

void
mf_disk_group_set_enable(MF_BOOL bEnable)
{
    g_disk.stGroup.bEnable = bEnable;
    mf_record_update();
}

void
mf_disk_quota_set_enable(MF_BOOL bEnable)
{
    g_disk.stQuota.bEnable = bEnable;
    mf_record_update();
}

MF_BOOL
mf_disk_quota_get_status(void)
{
    return g_disk.stQuota.bEnable;
}

void
mf_disk_qta_grp_info(struct mf_grp **grp, struct mf_qta **qta)
{
    if (grp != NULL) {
        *grp = &g_disk.stGroup;
    }

    if (qta != NULL) {
        *qta = &g_disk.stQuota;
    }
}

MF_S32
mf_disk_set_chn_quota(MF_U8 chnId, MF_U32 vidQta, MF_U32 picQta)
{
    struct chn_qta_t *cq = &g_disk.stQuota.cq[chnId];
    if (chnId > MAX_REC_CHN_NUM) {
        MFdbg("quota cap chnId[%d] err", chnId);
        return MF_FAILURE;
    }

    cq->chnId  = chnId;
    cq->vidQta = vidQta;
    cq->picQta = picQta;

    return MF_SUCCESS;
}

MF_S32
mf_disk_get_chn_quota(MF_U8 chnId, quota_info_t *pstQta)
{
    struct chn_qta_t *cq = &g_disk.stQuota.cq[chnId];
    struct list_head *list = mf_disk_get_wr_list(MF_NO);
    mf_disk_qta_chn_refresh(chnId, list);
    mf_disk_put_rw_list(list);

    pstQta->chnId = chnId;
    pstQta->vidQta = cq->vidQta;
    pstQta->picQta = cq->picQta;
    pstQta->vidUsd = cq->vidUsd;
    pstQta->picUsd = cq->picUsd;

    return MF_SUCCESS;
}

void
mf_disk_qta_chn_refresh(MF_U8 chnId, struct list_head *list)
{
    MF_U32 vidSize = 0, picSize = 0;
    struct diskObj *pstDisk;
    struct ipc_info *pstIpcInfo;
    struct file_info *pstFileInfo;
    struct chn_qta_t *cq = &g_disk.stQuota.cq[chnId];

    if (chnId >= MAX_REC_CHN_NUM || list == NULL) {
        return ;
    }

    list_for_each_entry(pstDisk, list, node) {
        if (list_empty(&pstDisk->ipcHead)) {
            continue;
        }

        list_for_each_entry(pstIpcInfo, &pstDisk->ipcHead, node) {
            if (pstIpcInfo->chnId != chnId || list_empty(&pstIpcInfo->fileHead)) {
                continue;
            }

            list_for_each_entry(pstFileInfo, &pstIpcInfo->fileHead, node) {
                if (pstFileInfo->key.stRecord.type < FILE_TYPE_PIC) {
                    vidSize++;
                } else {
                    picSize++;
                }
            }
        }
    }
    cq->vidUsd  = vidSize;
    cq->picUsd  = picSize;

}

void
mf_disk_port_permission(MF_U8 port, RW_EN enRw)
{
    struct diskObj *pstDev = NULL;

    g_disk.stGroup.pg[port].enRw = enRw;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        pstDev->enRw = enRw;
    }
}

void
mf_disk_set_loop(MF_BOOL bLoop)
{
    struct diskObj *pos = NULL;
    struct list_head *list;

    list = mf_disk_get_wr_list(MF_NO);

    g_disk.bLoop = bLoop;
    list_for_each_entry(pos, list, node) {
        pos->bLoop = bLoop;
    }

    mf_disk_put_rw_list(list);
    mf_record_update();
}

void
mf_disk_go_private(MF_U8 port, MF_BOOL bEnable)
{
    struct diskObj *pstDev = NULL;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        if (pstDev->bPrivate == bEnable) {
            return;
        }

        if (pstDev->ops->disk_go_private) {
            pstDev->ops->disk_go_private(pstDev->pstPrivate, bEnable);
        }

        pstDev->bPrivate = bEnable;
    }
}

void
mf_disk_set_esata_usage(MF_BOOL brec)
{
    struct diskObj *pdisk = NULL;
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *list = NULL;

    if (g_disk.bEsataUse == brec) {
        return ;
    }

    g_disk.bEsataUse = brec;
    list = mf_disk_get_rd_list(MF_NO);
    list_for_each_entry_safe(pos, n, list, node) {
        /* 3536 only one esata, 3798 no esata */
        if (pos->enType == DISK_TYPE_ESATA) {
            pdisk = pos;
            break;
        }
    }
    mf_disk_put_rw_list(list);
    if (pdisk) {
        if (g_disk.bEsataUse == MF_YES) {
            //dev_umount(pos);
            //dev_attribute(pos);
            //dev_update_status(pos, MF_YES, MSFS_EVENT_DISK_ADD);
            //disk_user_notify(MSFS_EVENT_DISK_ADD, pos);
            //dev_retr_del(pos);
            /* use storage */
            dev_update_status(pdisk, MF_YES, MSFS_EVENT_DISK_DEL);
            disk_user_notify(MSFS_EVENT_DISK_DEL, pdisk);
            dev_list_del(pdisk);
            mf_disk_dev_loading(pdisk, MF_YES);
        } else {
            dev_update_status(pos, MF_YES, MSFS_EVENT_DISK_DEL);
            disk_user_notify(MSFS_EVENT_DISK_DEL, pdisk);
            dev_retr_del(pdisk);
            dev_mount(pdisk);
            dev_attribute(pdisk);
        }
    }
    mf_record_update();
}

void
mf_disk_bad_reload(struct list_head *head)
{
    if (!head) {
        return ;
    }
    struct disk_port_t *dpos = NULL;
    struct diskObj *pDev = NULL;

    list_for_each_entry(dpos, head, node) {
        pDev = NULL;
        DEV_FIND_BY_PORT(dpos->port, pDev);
        if (pDev) {
            MFinfo("[%u] [load:%d] [stat:%d]", pDev->port, pDev->loadNum, pDev->enState);
        }
        if (pDev && pDev->loadNum < DISK_LOAD_NUM_MAX) {
            MFinfo("reload disk:%d", pDev->port);
            dev_update_status(pDev, MF_YES, MSFS_EVENT_DISK_DEL);
            disk_user_notify(MSFS_EVENT_DISK_DEL, pDev);
            dev_list_del(pDev);
            mf_disk_dev_loading(pDev, MF_NO);
        }
    }
}


void
mf_disk_set_raid_mode(MF_BOOL bmode)
{
    g_disk.bRaidMode = bmode;
}
MF_BOOL
mf_disk_get_raid_mode()
{
    return g_disk.bRaidMode;
}

MF_S32
mf_disk_nas_add(MF_S8 *name, MF_U8 port, MF_S8 *host, MF_S32 type, MF_S8 *user, MF_S8 *password)
{
    MF_S8 devName[128];
    MF_S32 res = MF_FAILURE;

    if (!type) {
        //nfs
        snprintf(devName, sizeof(devName), "/dev/MF_nfs_%d_%s", port, name);
    } else {
        //cifs
        snprintf(devName, sizeof(devName), "/dev/MF_cifs_%d_%s", port, name);
    }
    client_remove_by_port(port);
    res = client_create_client(type, devName, port, host, user, password);
    return res;
}

static diskObj *
dev_find_format_by_port(MS_U8 port)
{
    struct diskObj *pos = NULL;
    ms_mutex_lock(&g_disk.listfmtMutex);
    list_for_each_entry(pos, &g_disk.listFormat, node) {
        if (port == pos->port) {
            ms_mutex_unlock(&g_disk.listfmtMutex);
            return pos;
        }
    }
    ms_mutex_unlock(&g_disk.listfmtMutex);
    return NULL;
}

static diskObj *
dev_find_loading_by_port(MS_U8 port)
{
    struct diskObj *pos = NULL;
    ms_mutex_lock(&g_disk.loadingMutex);
    list_for_each_entry(pos, &g_disk.loadingList, node) {
        if (port == pos->port) {
            ms_mutex_unlock(&g_disk.loadingMutex);
            return pos;
        }
    }
    ms_mutex_unlock(&g_disk.loadingMutex);
    return NULL;
}

MF_S32
mf_disk_nas_del(MF_U8 port)
{
    struct diskObj *pstDev = NULL;

    // not allowed to delete nas in formating and loading
    if ((pstDev = dev_find_format_by_port(port)) 
        || (pstDev = dev_find_format_by_port(port))) {
        return MS_FAILURE;
    }

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        dev_remove_by_port(port);
    }

    return client_remove_by_port(port);
}

MF_S32
mf_disk_nas_rename(MF_S8 *name, MF_U8 port)
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *list;

    list = mf_disk_get_wr_list(MF_NO);
    //MFinfo("rename nas %s", name);
    list_for_each_entry_safe(pos, n, list, node) {
        if (pos->port == port) {
            strcpy(pos->alias, name);
            if (pos->ops->disk_vendor) {
                pos->ops->disk_vendor(pos->pstPrivate);
            }
        }
    }
    mf_disk_put_rw_list(list);
    return MF_SUCCESS;
}

MF_U32
mf_disk_nas_get_dir(MF_S8 *hostIp, struct list_head *list)
{
    return nas_directory_scan(hostIp, list);
}

void
mf_disk_nas_put_dir(struct list_head *list)
{
    struct nas_dir_t *dir;
    struct nas_dir_t *n;

    list_for_each_entry_safe(dir, n, list, node) {
        list_del(&dir->node);
        disk_mem_free(dir);
    }
}

MF_S32
mf_disk_nand_add(MF_S8 *name, MF_U8 port, MF_S8 *host)
{
    MF_S8 devName[128];
    struct diskObj *disk = NULL;

    sprintf(devName, "/dev/MF_nand_%d_%s", port, name);
    disk = dev_add_by_name(devName, host, NULL, NULL);
    if (disk) {
        mf_disk_event_notify(MSFS_EVENT_DISK_ADD, disk, MF_YES);
        return MF_SUCCESS;
    }

    return MF_FAILURE;
}

MF_S32
mf_disk_nand_del(MF_U8 port)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_FAILURE;

    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev && pstDev->enType != DISK_TYPE_NAND) {
        res = dev_remove_by_port(port);
    }
    return res;
}

////////////////////////////////////////////////////
//                  set raid vendor               //
////////////////////////////////////////////////////
void
mf_raid_info_get(struct diskObj *pstObj)
{
    struct raid_info_temp *pos = NULL, *n = NULL;

    if (!pstObj) {
        return ;
    }
    ms_mutex_lock(&g_disk.raidinfoMutex);
    list_for_each_entry_safe(pos, n, &g_disk.raidInfoList, node) {
        if (pos->port == pstObj->port) {
            MFinfo("find raid [%d:%s]", pos->port, pos->vendor);
            strncpy(pstObj->vendor, pos->vendor, DEV_NAME_LEN);
            break;
        }
    }
    ms_mutex_unlock(&g_disk.raidinfoMutex);
}

void
mf_raid_info_add(MF_U8 port, MF_S8 *vendor)
{
    struct raid_info_temp *rit = NULL;
    struct raid_info_temp *pos = NULL, *n = NULL;

    if (port <= 0 || strlen(vendor) <= 0) {
        return ;
    }

    ms_mutex_lock(&g_disk.raidinfoMutex);
    list_for_each_entry_safe(pos, n, &g_disk.raidInfoList, node) {
        if (pos->port == port) {
            ms_mutex_unlock(&g_disk.raidinfoMutex);
            return ;
        }
    }
    rit = disk_mem_calloc(1, sizeof(raid_info_temp));
    if (rit) {
        rit->port = port;
        strncpy(rit->vendor, vendor, DEV_NAME_LEN);
        //:TODO
        list_add(&rit->node, &g_disk.raidInfoList);
    }
    ms_mutex_unlock(&g_disk.raidinfoMutex);
}

/* run after disk scan right */
void
mf_raid_info_dels()
{
    struct raid_info_temp *pos = NULL, *n = NULL;

    ms_mutex_lock(&g_disk.raidinfoMutex);
    list_for_each_entry_safe(pos, n, &g_disk.raidInfoList, node) {
        list_del(&pos->node);
        disk_mem_free(pos);
    }
    ms_mutex_unlock(&g_disk.raidinfoMutex);
}
////////////////////// end /////////////////////////

MF_S32
mf_raid_rename(MF_S8 *name, MF_U8 port)
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *list;

    list = mf_disk_get_wr_list(MF_NO);
    //MFinfo("rename nas %s", name);
    list_for_each_entry_safe(pos, n, list, node) {
        if (pos->port == port) {
            strcpy(pos->vendor, name);
            if (pos->ops->disk_vendor) {
                pos->ops->disk_vendor(pos->pstPrivate);
            }
        }
    }
    mf_disk_put_rw_list(list);
    return MF_SUCCESS;
}

void
mf_raid_deal_create_failed(struct raid_op_t *raid)
{
    if (!raid) {
        return;
    }

    int num;
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    ms_rwlock_wrlock(&g_disk.rwlock);
    list_for_each_entry_safe(pos, n, &g_disk.list, node) {
        for (num = 0; num < raid->disk_num; num++) {
            if (pos->port == raid->disk_port[num]) {
                pos->enType = DISK_TYPE_LOCAL;
                dev_update_status(pos, MF_YES, MSFS_EVENT_DISK_ADD);
            }
        }
    }
    ms_rwlock_unlock(&g_disk.rwlock);

    return;
}

MF_S32
mf_raid_create(struct raid_op_t *raid)
{
    MF_S32 res = MF_FAILURE;

    mf_raid_info_add(raid->raid_port, raid->raid_vendor);
    res = raid_create(raid);
    mf_raid_info_dels();
    if (res == MF_SUCCESS) {
        mf_disk_notify_bar(PROGRESS_BAR_RAID_CREATE, raid->raid_port,
                           mf_random(75, 85), DISK_TYPE_RAID);

        //mf_disk_format(raid->raid_port, FORMAT_TYPE_MS);
        mf_raid_add_update_notify(raid->raid_port, MSFS_EVENT_DISK_ADD);
    } else {
        mf_raid_deal_create_failed(raid);
    }
    return res;
}

MF_S32
mf_raid_del(int raid_port)
{
    diskObj *pstDev = NULL;
    MF_S32 res = MF_FAILURE;

    DEV_FIND_BY_PORT(raid_port, pstDev);
    if (pstDev) {
        mf_disk_event_notify(MSFS_EVENT_DISK_DEL, (void *)pstDev, MF_YES);

        //firstly, must del Bplus info from raid
        ms_mutex_lock(&pstDev->fmtMutex);
        dev_umount(pstDev);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);
        ms_mutex_unlock(&pstDev->fmtMutex);

        res = raid_del(pstDev);// delete raid disk
        dev_unregister(pstDev);
    }
    if (res != MF_SUCCESS) {
        MFinfo("del fail:res %d", res);
    }

    return res;
}

MF_S32
mf_raid_rebuild(struct raid_op_t *raid)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_FAILURE;

    DEV_FIND_BY_PORT(raid->raid_port, pstDev);
    if (pstDev) {
        if (pstDev->enState == DISK_STATE_OFFLINE) {
            return MF_FAILURE;
        }
        mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pstDev, MF_YES);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);

        res = raid_rebuild(pstDev, raid);
        dev_attribute(pstDev);
        dev_retr_add(pstDev);
        dev_list_add(pstDev);
        mf_disk_event_notify(MSFS_EVENT_DISK_ADD, pstDev, MF_YES);
    }

    return res;
}

void
mf_get_component_size_raid(MF_S32 *raidport, MF_U64 *rsize)
{
    struct diskObj *pstDev = NULL;

    DEV_FIND_BY_PORT(*raidport, pstDev);
    if (pstDev) {
        raid_get_min_disk_size(raidport, rsize);
    }
    return ;
}

MF_S32
mf_raid_update_local_inraid(struct diskObj *pos)
{
    if (!pos) {
        return MF_FAILURE;
    }


    pos->enType  = DISK_TYPE_IN_RAID;
    pos->enState = DISK_STATE_UNFORMATTED;
    pos->bRec    = MF_FALSE;   //when disk is used for raid, it can not be use for recording
    dev_umount(pos);
    dev_retr_del(pos);
    return local_reset_allhead_info(pos);//clear all header info
}

MF_S32
mf_raid_create_update_local(raid_op_t *raid)
{
    int num = 0;
    MF_S32 res = 0;
    MF_S32 eraserDisk = 0;
    struct diskObj *pos = NULL, *n = NULL;
    HDD_ATTR attr;
    int isBad = 0;

    MFinfo(">> mf_disk_raid_update_local %d", raid->raid_port);
    ms_rwlock_wrlock(&g_disk.rwlock);
    list_for_each_entry_safe(pos, n, &g_disk.list, node) {
        for (num = 0; num < raid->disk_num; num++) {
            if (pos->port == raid->disk_port[num]) {
                //printf("\n ========= raid add, port:[%d] ==== \n", pos->port);
                isBad = 0;
                //【中心-RAID：用一块Error但还未格式化（状态为Uninitialized）的硬盘创建RAID，RAID应该创建失败（目前可以创建成功显示Normal，但不可使用）】https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001075049
                mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pos, MF_YES);
                memset(&attr, 0, sizeof(attr));
                if (mf_disk_op_smart(pos->alias, &attr, TEST_RES) == MF_SUCCESS) {
                    if (!strstr(attr.res, TEST_PASSED_STR)) {
                        MFerr("disk smart check before creat raid. port %d is bad %s", pos->port, attr.res);
                        isBad = 1;
                    }
                }

                pos->smartTest = mf_disk_get_disk_attr_status(pos->alias);
                if (!isBad) {
                    res = mf_raid_update_local_inraid(pos);
                    if (res == MF_SUCCESS) {
                        eraserDisk |= 1 << pos->port;
                    }
                }
            }
        }
    }
    ms_rwlock_unlock(&g_disk.rwlock);

    return eraserDisk;
}

void
mf_raid_assemble_update_local(MF_S8 *name)
{
    int num = 0;
    struct raid_info *raidInfo = NULL;
    struct diskObj *pstDev = NULL;
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;

    DEV_FIND_BY_NAME(name, pstDev);
    if (pstDev && pstDev->enType == DISK_TYPE_RAID) {
        raidInfo = mf_disk_get_raid_priv(pstDev->pstPrivate);  // get raid priv info for disk info in raid
        if (!raidInfo) {
            return;
        }
        ms_rwlock_wrlock(&g_disk.rwlock);
        list_for_each_entry_safe(pos, n, &g_disk.list, node) {
            for (num = 0; num < raidInfo->disk.cnt; num++) {
                if (pos->port == raidInfo->disk.dev[num].port) {
                    //printf("\n ========= raid add, port:[%d] ==== \n", pos->port);
                    mf_disk_event_notify(MSFS_EVENT_DISK_DEL, pos, MF_YES);
                    mf_raid_update_local_inraid(pos);
                }
            }
        }
        ms_rwlock_unlock(&g_disk.rwlock);
    }
}

MF_S32
mf_raid_rebuild_progress(MF_S32 raidport)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_FAILURE;

    DEV_FIND_BY_PORT(raidport, pstDev);
    if (pstDev) {
        if (pstDev->enState == DISK_STATE_OFFLINE) {
            return MF_FAILURE;
        }
        res = raid_rebuild_progress(raidport);

        // sync raid status
        if (res >= 100) {
            mf_disk_attribute(pstDev);
        }
    }

    return res;
}

void
mf_raid_add_update_notify(MF_U8 port, EVENT_E event)
{
    struct diskObj *pstDev = NULL;
    DEV_FIND_BY_PORT(port, pstDev);
    if (pstDev) {
        dev_update_status(pstDev, MF_YES, event);
        disk_user_notify(event, pstDev);
    }
}

void
mf_raid_fault_update_disk(MF_S8 *name)// disk is fault in raid should be local.
{
    struct diskObj *pstDev = NULL;
    int flag = 0;
    DEV_FIND_BY_ALIAS(name, pstDev);
    if (pstDev) {
        ms_mutex_lock(&pstDev->mutex);
        if (pstDev->enState != DISK_STATE_BAD) {
            pstDev->enState = DISK_STATE_BAD;
            flag = 1;
        }
        ms_mutex_unlock(&pstDev->mutex);
        if (flag) {
            disk_user_notify(MSFS_EVENT_DISK_ERR, pstDev);
        }
    }
}

void
mf_check_raid_status()
{
    check_raid_status();
}

MF_S32
mf_disk_raid_create(MF_S8 *name)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_FAILURE;
    MF_S8 *host = NULL;

    DEV_FIND_BY_NAME(name, pstDev);
    if (!pstDev) {
        pstDev = dev_register(name, host, NULL);
        if (pstDev) {
            mf_raid_info_get(pstDev);
            dev_attribute(pstDev);

            /* solin 20180604
             * immediately format disk after raid was creating,
             * do not move to mod_disk.
             */
            mf_disk_notify_bar(PROGRESS_BAR_RAID_CREATE, pstDev->port, mf_random(70, 75), DISK_TYPE_RAID);
            mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, pstDev->port, 0, pstDev->enType);
            dev_format(pstDev, FORMAT_TYPE_MS, 0);
            mf_disk_notify_bar(PROGRESS_BAR_DISK_FORMAT, pstDev->port, 100, pstDev->enType);
            dev_attribute(pstDev);
            res = dev_retr_add(pstDev);
            dev_list_add(pstDev);
        }
    } else {
        /* re-update all attribute when mscore was killed.solin */
        dev_attribute(pstDev);
    }

    return res;
}

MF_S32
mf_disk_raid_add(MF_S8 *name)
{
    //int num = 0;
    MF_S8 *host = NULL;
    //struct raid_info *raidInfo = NULL;
    //struct diskObj *pstDev = NULL;
    //struct diskObj *pos = NULL;
    //struct diskObj *n = NULL;

    dev_add_by_name(name, host, NULL, NULL);  //add raid to g_disk list

    /*DEV_FIND_BY_NAME(name, pstDev);
    if (pstDev)
    {
        if(pstDev->enType == DISK_TYPE_RAID)
        {
            raidInfo = mf_disk_get_raid_priv(pstDev->pstPrivate);  // get raid priv info for disk info in raid
        }
    }

    ms_mutex_lock(&g_disk.mutex);
    list_for_each_entry_safe(pos, n, &g_disk.list, node)
    {
        for(num = 0; num < raidInfo->disk.cnt; num++)
        {
            if(pos->port == raidInfo->disk.dev[num].port)
            {
                //printf("\n ========= raid add, port:[%d] ==== \n", pos->port);
                mf_disk_event_notify(EVENT_DISK_DEL, pos, MF_YES);
                pos->enType = DISK_TYPE_IN_RAID;
                pos->bRec = MF_FALSE;   //when disk is used for raid, it can not be use for recording
                local_reset_allhead_info(pos);//clear all header info
            }
        }
    }
    ms_mutex_unlock(&g_disk.mutex);*/

    return MF_SUCCESS;
}

MF_S32
mf_disk_raid_del(struct diskObj *pstDev)
{
    struct raid_info *raidInfo = NULL;
    struct diskObj *pos = NULL, *n = NULL;
    MF_U8   num = 0;

    if (!pstDev) {
        MFerr("raid delete failed, pstDev NULL. \n");
        return MF_FAILURE;
    }

    if (pstDev->enType == DISK_TYPE_RAID) {
        raidInfo = mf_disk_get_raid_priv(pstDev->pstPrivate);    // get raid priv info for disk info in raid
    }

    if (!raidInfo) {
        return MF_FAILURE;
    }
    ms_rwlock_wrlock(&g_disk.rwlock);
    list_for_each_entry_safe(pos, n, &g_disk.list, node) {
        for (num = 0; num < raidInfo->disk.cnt; num++) {
            if (pos->port == raidInfo->disk.dev[num].port) {
                pos->enType = DISK_TYPE_LOCAL;
                dev_update_status(pos, MF_YES, MSFS_EVENT_DISK_ADD);
            }
        }
    }
    ms_rwlock_unlock(&g_disk.rwlock);

    return MF_SUCCESS;
}

////////////////////////////////////////////
//      get device info
///////////////////////////////////////////
MF_S8 *
mf_dev_mmac()
{
    return strlen(g_disk.devMmac) ? g_disk.devMmac : "";
}

MF_S8 *
mf_dev_model()
{
    return strlen(g_disk.devModel) ? g_disk.devModel : "";
}

struct list_head *
mf_disk_get_rd_list(MF_BOOL bTry)
{
    if (bTry) {
        if (ms_rwlock_tryrdlock(&g_disk.rwlock) != 0) {
            return NULL;
        }
    } else {
        ms_rwlock_rdlock(&g_disk.rwlock);
    }
    return &g_disk.list;
}

struct list_head *
mf_disk_get_wr_list(MF_BOOL bTry)
{
    if (bTry) {
        if (ms_rwlock_trywrlock(&g_disk.rwlock) != 0) {
            return NULL;
        }
    } else {
        ms_rwlock_wrlock(&g_disk.rwlock);
    }
    return &g_disk.list;
}

void
mf_disk_put_rw_list(struct list_head *list)
{
    if (&g_disk.list == list) {
        ms_rwlock_unlock(&g_disk.rwlock);
    }
}

struct list_head *
mf_disk_get_loading_list()
{
    ms_mutex_lock(&g_disk.loadingMutex);

    return &g_disk.loadingList;
}

void
mf_bkp_put_format()
{
    ms_mutex_unlock(&g_disk.fmtbkpmutex);
    MFdbg(" ");
}

void
mf_bkp_get_format()
{
    ms_mutex_lock(&g_disk.fmtbkpmutex);
    MFdbg(" ");
}

void
mf_disk_put_loading_list(struct list_head *list)
{
    if (&g_disk.loadingList == list) {
        ms_mutex_unlock(&g_disk.loadingMutex);
    }
}

struct list_head *
mf_disk_get_format_list()
{
    ms_mutex_lock(&g_disk.listfmtMutex);
    return &g_disk.listFormat;
}

void
mf_disk_put_format_list(struct list_head *list)
{
    if (&g_disk.listFormat == list) {
        ms_mutex_unlock(&g_disk.listfmtMutex);
    }
}

void
mf_disk_index_dump(MF_U8 portId)
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *list;

    list = mf_disk_get_rd_list(MF_NO);

    list_for_each_entry_safe(pos, n, list, node) {
        if (pos->port == portId && pos->tree) {
            dev_print_head_info(pos->pstHeader);
            mf_index_dump(pos->tree);
        }
    }
    mf_disk_put_rw_list(list);
}

void
mf_disk_file_dump(MF_U8 portId, MF_U8 chnid)
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *list;

    list = mf_disk_get_rd_list(MF_NO);

    list_for_each_entry_safe(pos, n, list, node) {
        if (pos->port == portId && pos->ipcNum > 0) {
            mf_retr_disk_info_dump(pos, chnid);
        }
    }
    mf_disk_put_rw_list(list);
}

void 
mf_disk_set_health_temperature(MF_U8 portId, MF_S32 temperature)
{
    diskHealthTemperature[portId] = temperature;
}

void
mf_disk_set_health_status(MF_U8 portId, MF_S32 status)
{
    diskHealthstatus[portId] = status;
}

void
mf_disk_set_health_enable(MF_U64 enableMask)
{
    g_disk.HMEnableMask = enableMask;
}

void
mf_disk_event_cb(MODULE_E from, EVENT_E event, void *argv)
{
    MFinfo("from = %d event = %d begin\n", from, event);
    diskObj *pstDev = (diskObj *)argv;

    if (pstDev) {
        switch (event) {
            case MSFS_EVENT_DISK_DEL:
                pstDev->enState = DISK_STATE_OFFLINE;
                break;
            case MSFS_EVENT_NONE:
            case MSFS_EVENT_DISK_FORMAT:
            case MSFS_EVENT_DISK_ADD:
                dev_attribute(pstDev);
                break;
            case MSFS_EVENT_DISK_ERR: {
                if (pstDev->enType == DISK_TYPE_NAS || pstDev->enType == DISK_TYPE_CIFS) {
                    client_update_event(pstDev->port, MSFS_EVENT_DISK_ERR);
                }
            }
            break;
            case MSFS_EVENT_DISK_REC_PERMIT:
            case MSFS_EVENT_DISK_FULL:
            case MSFS_EVENT_REC_START:
            case MSFS_EVENT_REC_STOP:
            default:
                break;
        }
        disk_user_notify(event, pstDev);
    }
    MFinfo("from = %d event = %d end\n", from, event);
}

void
mf_disk_event_get()
{
    g_disk.bNotify = MF_YES;
    mf_disk_event_notify(MSFS_EVENT_BOOT_COMPLETE, NULL, MF_YES);
}

void
mf_disk_event_notify(EVENT_E event, void *argv, MF_BOOL bBlock)
{
    if (bBlock == MF_NO) {
        mf_comm_task_notify(MODULE_DISK, event, argv);
    } else {
        mf_comm_event_notify(MODULE_DISK, event, argv);
    }
}

void
mf_disk_dbg_switch(MF_BOOL bDebug)
{
    g_disk.bDebug = bDebug;
}

void
mf_disk_dbg_show_mem_usage()
{
    if (g_disk.pstMem) {
        MFinfo("disk_dbg_show_mem_usage");
        mf_mem_state(g_disk.pstMem, MF_YES);
    }
}

void
mf_disk_init(DISK_USER_CB user_cb, disk_param *para)
{
    // due to set raidmode before init disk, first save raidmode
    MF_BOOL raidmode = g_disk.bRaidMode;
    MF_S8 *chip_info = para ? para->chip_info : NULL;

    mf_comm_event_register(MODULE_DISK, mf_disk_event_cb);
    memset(&g_disk, 0, sizeof(struct disk));
    //debug
    g_disk.usbindex  = USB_START_PORT; // start with 34
    g_disk.bLoop     = MF_YES;
    g_disk.bReady    = MF_NO;
    g_disk.bNotify   = MF_NO;
    g_disk.bCanRecord = MF_NO;
    g_disk.bRaidMode = raidmode;
    g_disk.user_cb   = user_cb;
    strncpy(g_disk.devModel, para->dev_model, sizeof(g_disk.devModel));
    strncpy(g_disk.devMmac, para->dev_mmac, sizeof(g_disk.devMmac));

    memset(&g_dscan_, 0, sizeof(struct disk_scan_));
    INIT_LIST_HEAD(&g_dscan_.node);

    disk_mem_init(&g_disk);
    disk_rules_init(chip_info);
    disk_driver_open();

    ms_rwlock_init(&g_disk.rwlock);
    ms_mutex_init(&g_disk.loadingMutex);
    ms_mutex_init(&g_disk.loadSyncMutex);
    ms_mutex_init(&g_disk.listfmtMutex);
    ms_mutex_init(&g_disk.raidinfoMutex);
    ms_mutex_init(&g_disk.fmtbkpmutex);
    ms_mutex_init(&g_disk.usbMutex);
    ms_mutex_init(&g_disk.IOMutex);
    disk_list_init(&g_disk);
    disk_task_create(&g_disk);

    ms_mutex_init(&g_disk.cmutex);
    client_task_create(&g_disk);
}

void
mf_disk_deinit()
{
    client_task_destroy(&g_disk);
    disk_task_destroy(&g_disk);
    disk_list_deinit(&g_disk);
    ms_rwlock_uninit(&g_disk.rwlock);
    ms_mutex_uninit(&g_disk.loadingMutex);
    ms_mutex_uninit(&g_disk.loadSyncMutex);
    ms_mutex_uninit(&g_disk.listfmtMutex);
    ms_mutex_uninit(&g_disk.raidinfoMutex);
    ms_mutex_uninit(&g_disk.fmtbkpmutex);
    ms_mutex_uninit(&g_disk.usbMutex);
    ms_mutex_uninit(&g_disk.IOMutex);
    disk_driver_close();

    usleep(500000);//wait for someting
    disk_mem_uninit(&g_disk);

    //client_list_deinit(&g_disk.clist); // del by client task-self
    ms_mutex_uninit(&g_disk.cmutex);
    mf_comm_event_unregister(MODULE_DISK);
}

MF_S32
mf_disk_get_Udisk(MF_S8 *mnt)
{
    struct diskObj *pstDev = NULL;
    if (mnt && strlen(mnt)) {
        DEV_FIND_BY_MNT(mnt, pstDev);
        if (pstDev) {
            ms_mutex_lock(&pstDev->fmtMutex);

            MFinfo("port[%d]", pstDev->port);
            return pstDev->port;
        }
    }
    return MF_FAILURE;
}

void
mf_disk_put_Udisk(MF_S8 *mnt)
{
    struct diskObj *pstDev = NULL;
    if (mnt && strlen(mnt)) {
        DEV_FIND_BY_MNT(mnt, pstDev);
        if (pstDev) {
            ms_mutex_unlock(&pstDev->fmtMutex);
            MFinfo("port[%d]", pstDev->port);
        }
    }
}

MF_BOOL
mf_disk_Udisk_protected(struct diskObj *pdisk)
{
    if (!pdisk || pdisk->enType != DISK_TYPE_USB) {
        return MF_FALSE;
    }
    MF_S32 ret = MF_FALSE;
    char *filename = "/proc/mounts";
    FILE *mntfile = NULL;
    struct mntent *mntent = NULL;

    mntfile = setmntent(filename, "r");
    if (!mntfile) {
        MFerr("Failed to read mtab file, error [%s]\n", strerror(errno));
        return ret;
    }

    while ((mntent = getmntent(mntfile))) {
        if (strstr(mntent->mnt_fsname, pdisk->alias) && !strstr(mntent->mnt_opts, "rw")) {
            ret = MF_TRUE;
            break;
        }
    }
    endmntent(mntfile);
    return ret;
}

struct diskObj *mf_get_disk_by_port(MF_U8 portp)
{
    struct diskObj *pstDev = NULL;

    DEV_FIND_BY_PORT(portp, pstDev);
    return pstDev;
}

MF_S32
mf_find_name_port(MF_S8 *name, MF_U8 *port, MF_U8 cmd_type)
{
    struct diskObj *pstDev = NULL;

    if ((name == NULL) || (port == NULL)) {
        msprintf("name is NULL. \n");
        return MF_FAILURE;
    }

    if (cmd_type == CMD_FIND_NAME_BY_PORT) {
        DEV_FIND_BY_PORT(*port, pstDev);
        if (pstDev) {
            strncpy(name, pstDev->name, sizeof(pstDev->name));
            return MF_SUCCESS;
        }
    } else if (cmd_type == CMD_FIND_ALIAS_BY_PORT) {
        DEV_FIND_BY_PORT(*port, pstDev);
        if (pstDev) {
            strncpy(name, pstDev->alias, sizeof(pstDev->alias));
            return MF_SUCCESS;
        }
    } else if (cmd_type == CMD_FIND_PORT_BY_NAME) {
        DEV_FIND_BY_NAME(name, pstDev);
        if (pstDev) {
            *port = pstDev->port;
            return MF_SUCCESS;
        }
    } else if (cmd_type == CMD_FIND_PORT_BY_ALIAS) {
        DEV_FIND_BY_ALIAS(name, pstDev);
        if (pstDev) {
            *port = pstDev->port;
            return MF_SUCCESS;
        }
    }

    //msprintf("Not supported type: [%d]. \n", cmd_type);
    return MF_FAILURE;
}

struct raid_info *
mf_disk_get_raid_priv(void *pstPriv)
{
    struct raid_info *raidInfo = NULL;
    if (pstPriv != NULL) {
        raidInfo = raid_get_priv_ptr(pstPriv);

        return raidInfo;
    }

    return NULL;
}

MF_S32
mf_write_super_magic(char *devname, MF_U32 magic_num)
{
    return write_super_magic(devname, magic_num);
}

MF_S32
mf_read_super_magic(char *devname, MF_U32 *magic_num)
{
    return read_super_magic(devname, magic_num);
}


MF_S32
mf_create_global_spare(MF_S32 disk_port)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_SUCCESS;

    DEV_FIND_BY_PORT(disk_port, pstDev);
    if (pstDev) {
        dev_update_status(pstDev, MF_YES, MSFS_EVENT_DISK_DEL);
        disk_user_notify(MSFS_EVENT_DISK_DEL, pstDev);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);
        res = create_global_spare(disk_port, pstDev->alias);

        dev_attribute(pstDev);
        dev_retr_add(pstDev);
        dev_list_add(pstDev);

        dev_update_status(pstDev, MF_YES, MSFS_EVENT_DISK_ADD);
        disk_user_notify(MSFS_EVENT_DISK_ADD, pstDev);

        /* do not need loading opration, should keep disk showing. solin 20180615*/
        //mf_disk_dev_loading(pstDev);
    } else {
        MFinfo("Create global spare failed, no found disk_port:[%d] in dev list. \n", disk_port);
        res = MF_FAILURE;
    }

    return res;
}

MF_S32
mf_remove_global_spare(MF_S32 disk_port)
{
    struct diskObj *pstDev = NULL;
    MF_S32 res = MF_FAILURE;

    DEV_FIND_BY_PORT(disk_port, pstDev);
    if (pstDev) {
        dev_update_status(pstDev, MF_YES, MSFS_EVENT_DISK_DEL);
        disk_user_notify(MSFS_EVENT_DISK_DEL, pstDev);
        dev_list_del(pstDev);
        dev_retr_del(pstDev);
        res = remove_global_spare(disk_port, pstDev->alias);
        mf_disk_dev_loading(pstDev, MF_YES);
        return res;
    }

    MFinfo("Remove global spare failed, no found disk_port:[%d] in dev list. \n", disk_port);
    return res;
}

// now no use
MF_S32
mf_disk_io_status(int fd)
{
    MF_S32 status = MF_SUCCESS;

    if (g_disk_drv_fd < 0) {
        MFerr("g_disk_drv_fd not init. \n ");
        return MF_FAILURE;
    }

    status = ioctl(g_disk_drv_fd, IOSTSTAT, &fd);
    if (status < 0) {
        return status;
    }

    if (status == (SD_S_LINK_UP | SD_S_ADD_DEV)) { //disk is link up and add dev
        return MF_SUCCESS;
    }

    // disk is link down or no add dev
    return MF_FAILURE;
}

void
mf_disk_io_request(struct ioReq *pstIOReq)
{
    ms_mutex_lock(pstIOReq->pMutex);
    ms_mutex_lock(&g_disk.IOMutex);
    LIST_INSERT_SORT_ASC(pstIOReq, &g_disk.IOList[pstIOReq->port], fileOffset);
    //    printf("===========port[%d]====offset[%lld]====\n", pstIOReq->port, pstIOReq->fileOffset);
    ms_mutex_unlock(&g_disk.IOMutex);
    mf_cond_timedwait(pstIOReq->pCond, pstIOReq->pMutex, 1000000);
    pstIOReq->bDone = MF_YES;
    ms_mutex_unlock(pstIOReq->pMutex);
}

//>>>>>>>>>>>>>>>>>>>> smartctl opt about disk <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
MF_S32
mf_disk_get_smart_attr(HDD_ATTR *attr, MF_U8 port)
{
    if (!attr) {
        MFerr("attr is NULL\n");
        return MF_FAILURE;
    }

    FILE *fp = NULL;
    char *ptr = NULL;
    SMART_NODE *smnode = NULL;
    MF_S8 buf[256], cmd[64], name[40], type[24], update[16], failed[16], raw_val[24], devpath[64];
    MF_S32 id, flag, val, worst, thresh;
    MF_S8 *p;

    if (mf_find_name_port(devpath, &port, CMD_FIND_ALIAS_BY_PORT) != MF_SUCCESS) {
        return MF_FAILURE;
    }

    snprintf(cmd, sizeof(cmd), "smartctl -A %s | sed -n '8,$p' | grep -v ^$", devpath);
    if ((fp = ms_vpopen(cmd, "r")) == NULL) {
        perror("ms_vpopen");
        return MF_FAILURE;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (sscanf(buf, "%d %39s %x %d %d %d %23s %15s %15s %23[^\n]",
                   &id, name, &flag, &val, &worst, &thresh, type, update, failed, raw_val) != 10) {
            MFerr("sscanf error\n");
            continue;
        }

        smnode = (SMART_NODE *)disk_mem_calloc(1, sizeof(SMART_NODE));
        if (!smnode) {
            MFerr("smartctl calloc error\n");
            return MF_FAILURE;
        }

        // get all values of attribute
        smnode->id = id;
        if (id == 9) {
            attr->uptime = atoi(raw_val);
        }
        if (id == 194) {
            p = strtok_r(raw_val, " ", &ptr);
            attr->temp = atoi(p);
        }
        smnode->flag   = flag;
        smnode->thresh = thresh;
        smnode->val    = val;
        smnode->worst  = worst;
        strncpy(smnode->raw_val, raw_val, sizeof(smnode->raw_val));
        strncpy(smnode->failed, failed, sizeof(smnode->failed));
        strncpy(smnode->type, type, sizeof(smnode->type));
        strncpy(smnode->name, name, sizeof(smnode->name));
        strncpy(smnode->update, update, sizeof(smnode->update));

        /* lzm add attr of sata status */
        smnode->status = STATUS_GOOD;
        if ((id == 5) || (id == 196) || (id == 197) || (id == 198)) {   //05 C4 C5 C6
            if (atoi(raw_val) > 0) {
                smnode->status = STATUS_WARN;
            }
        }

        if (val <= thresh) {
            if (strncmp(type, "Pre-fail", strlen(type))) { //Old_age
                smnode->status = STATUS_WARN;
            } else {                    //Pre-fail
                smnode->status = STATUS_BAD;
            }
        }

        list_add_tail(&smnode->node, &attr->smartlist);
        attr->count++;
    }
    MFinfo("get smart attr done, nodes: %d\n", attr->count);
    ms_vpclose(fp);
    return MF_SUCCESS;
}

MF_S32
mf_disk_op_smart(MF_S8 *pdevpath, HDD_ATTR *attr, int test_type)
{
    MF_S8 cmd[128] = {0};
    MF_S8 buf[128] = {0};
    FILE *fp = NULL;
    MF_S32 flag = MF_SUCCESS;
    MF_S8 process[8] = {0}, *p = NULL, *psave = NULL;

    if (!attr || !pdevpath) {
        MFerr("attr or pdevpath is null\n");
        return MF_FAILURE;
    }
    if (strlen(pdevpath) <= 0) {
        MFerr("pdevpath is none value\n");
        return MF_FAILURE;
    }

    switch (test_type) {
        case TEST_SHORT:
            snprintf(cmd, sizeof(cmd), "smartctl -t short %s", pdevpath);
            ms_system(cmd);
            break;
        case TEST_LONG:
            snprintf(cmd, sizeof(cmd), "smartctl -t long %s", pdevpath);
            ms_system(cmd);
            break;
        case TEST_STOP:
            snprintf(cmd, sizeof(cmd), "smartctl -X %s", pdevpath);
            ms_vsystem(cmd);
            break;
        case TEST_RES:
            snprintf(cmd, sizeof(cmd), "smartctl -H %s | grep 'result' | awk '{print $NF}'", pdevpath);
            if ((fp = ms_vpopen(cmd, "r")) == NULL) {
                MFerr("ms_vpopen %s error\n", cmd);
                flag = MF_FAILURE;
                break;
            }
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                strncpy(attr->res, buf, sizeof(attr->res));
            } else {
                flag = MF_FAILURE;
            }
            ms_vpclose(fp);
            break;
        case TEST_STATUS:
            /*snprintf(cmd, sizeof(cmd), "smartctl -l error %s | sed -n '6p'", dev_path);
              if ((fp = ms_vpopen(cmd, "r")) == NULL) {
              TRACE("ms_vpopen %s error\n", cmd);
              flag = -1;
              break;
              }
              if (fgets(buf, sizeof(buf), fp) == NULL) {
              flag = -1;
              ms_vpclose(fp);
              break;
              }
              if (strstr(buf, "No"))
              attr->status = STATUS_OK;
              else
              attr->status = STATUS_BAD;
              ms_vpclose(fp);*/
            break;
        case TEST_PROCESS:
            snprintf(cmd, sizeof(cmd), "smartctl -c %s | sed -n '/execution/{n;p;};h' | sed s/^[[:space:]]*//g | grep '[1-9][0-9]'",
                     pdevpath);
            if ((fp = ms_vpopen(cmd, "r")) == NULL) {
                MFerr("ms_vpopen %s error\n", cmd);
                flag = MF_FAILURE;
                break;
            }
            if (fgets(buf,  sizeof(buf), fp) == NULL) {
                attr->process = 100;
            } else {
                sscanf(buf, "%7s", process);
                p = strtok_r(buf, "%", &psave);
                attr->process = 100 - atoi(p);
            }
            ms_vpclose(fp);
            break;
        default:
            MFinfo("unknown test type\n");
            flag = MF_FAILURE;
            break;
    }
    return flag;
}

MF_S32
mf_disk_del_smart_attr_list(HDD_ATTR *attr)
{
    if (!attr) {
        return MF_FAILURE;
    }

    SMART_NODE *pos = NULL, *n = NULL;
    list_for_each_entry_safe(pos, n, &attr->smartlist, node) {
        list_del(&pos->node);
        disk_mem_free(pos);
    }
    return MF_SUCCESS;
}

DISK_SMART_RESULT
mf_disk_get_disk_attr_status(MF_S8 *pdevpath)
{
    int ret = SMART_NONE;
    if (!pdevpath) {
        return ret;
    }

    FILE *fp = NULL;
    MF_S8 buf[256], cmd[64], name[40], type[24], update[16], failed[16], raw_val[24];
    MF_S32 id, flag, val, worst, thresh;

    snprintf(cmd, sizeof(cmd), "smartctl -A %s | sed -n '8,$p' | grep -v ^$", pdevpath);
    if ((fp = ms_vpopen(cmd, "r")) == NULL) {
        perror("ms_vpopen");
        return ret;
    }

    ret = SMART_PASS;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (sscanf(buf, "%d %39s %x %d %d %d %23s %15s %15s %23[^\n]",
                   &id, name, &flag, &val, &worst, &thresh, type, update, failed, raw_val) != 10) {
            MFerr("sscanf error\n");
            continue;
        }

        if ((id == 5) || (id == 196) || (id == 197) || (id == 198)) {   //05 C4 C5 C6
            if (atoi(raw_val) > 0) {
                ret = SMART_WARN;
                break;
            }
        }
    }
    MFinfo("get smart attr status dev:%s ret:%d\n", pdevpath, ret);
    ms_vpclose(fp);
    return ret;
}

//>>>>>>>>>>>>>>>>>>>>>> smart attr end <<<<<<<<<<<<<<<<<<<<<


