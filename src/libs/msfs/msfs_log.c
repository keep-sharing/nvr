#include "MFdisk.h"
#include "MFindex.h"
#include "MFlog.h"
#include "MFcommon.h"
#include "msfs_log.h"
#include "log.h"

static MF_BOOL g_uninit = MF_TRUE;


MF_S32
msfs_log_write(struct log_data *data)
{
	if (g_uninit) return MF_SUCCESS;

	struct log_data_node *dnode = NULL;

    dnode = mf_log_data_dup(data);
    if (dnode)
        mf_log_cache_push(dnode);
    
    return MF_SUCCESS;
}

void
msfs_log_search_dup(struct log_data_info *dst, struct log_data_node *dnode)
{
	mf_log_search_dup(dst, dnode);
	return ;
}


void
msfs_log_search_cancel(MF_BOOL flag)
{
	mf_log_search_cancel(flag);
	return ;
}

void
msfs_log_notify_bar(PROGRESS_BAR_E enBar, MF_S32 percent)
{
	mf_log_notify_bar(enBar, percent);
	return ;
}

MF_S32
msfs_log_deinit()
{
	mf_log_deinit();
	return MF_SUCCESS;
}

MF_S32
msfs_log_init(log_search_mac_type *lmac, LOG_USER_CB user_cb)
{
	g_uninit = MF_FALSE;
	mf_log_init(lmac, user_cb);
	return MF_SUCCESS;
}


void
msfs_log_show_disks( )
{
	if (g_uninit) return ;
	mf_log_show_disks();
}

void
msfs_log_show_seg(MF_U8 port)
{
	if (g_uninit) return ;
	mf_log_show_seg(port);
}

void
msfs_log_search_all(struct search_criteria *cond,struct log_search_result_n *result,
	MF_BOOL bnotify)
{
	if (g_uninit) return ;
	mf_log_search_all(cond, result, bnotify);
}

void
msfs_log_search_release(struct log_search_result_n *result)
{
	mf_log_search_release(result);
}


MF_S32
msfs_log_unpack_detail(struct log_detail_pkg *pkg, struct log_data *l_data)
{
	return mf_log_unpack_detail(pkg, l_data);
}

/* package detail to log_data->detail */
int
msfs_log_pack_detail(struct log_data *l_data, MF_U32 type, void *body, MF_S32 size)
{
	if(!l_data || !body || size > LOG_PKG_DATA_MAX || size <= 0)
	{
		printf("---------[Solin] detail size over !!!\n");
		return MF_FAILURE;
	}
	return mf_log_pack_detail(l_data, type, body, size);
}

void
msfs_log_mem_usage()
{
    MFinfo("msfs_log_dbg_mem_usage");
    mf_log_show_mem_usage();
}

int log_channo_is_disk(int mainType, long long subType)
{
    int ret = 0;

    switch (mainType) {
    case MAIN_OP:
        switch (subType) {
        case SUB_OP_DISK_INIT_LOCAL:
        case SUP_OP_ADD_NETWORK_DISK_LOCK:
        case SUP_OP_DELETE_NETWORK_DISK_LOCK:
        case SUP_OP_CONFIG_NETWORK_DISK_LOCK:
        case SUP_OP_PHYSICAL_DISK_SELF_CHECK_LOCK:
        case SUP_OP_MOUNT_DISK_LOCK:
        case SUP_OP_UNMOUNT_DISK_LOCK:
        case SUP_OP_DELETE_ABNORMAL_LOCK:
        case SUB_OP_DISK_INIT_REMOTE:
        case SUP_OP_ADD_NETWORK_DISK_REMOTE:
        case SUP_OP_DELETE_NETWORK_DISK_REMOTE:
        case SUP_OP_CONFIG_NETWORK_DISK_REMOTE:
        case SUP_OP_MOUNT_DISK_REMOTE:
        case SUP_OP_UNMOUNT_DISK_REMOTE:
        case SUP_OP_DELETE_ABNORMAL_DISK_REMOTE:
            ret = 1;
            break;

        default:
            break;
        }
        break;

    case MAIN_INFO:
        switch (subType) {
        case SUB_INFO_DISK_INFO:
        case SUP_INFO_NETWORK_DISK_INFORMATION:
        case SUB_INFO_DISK_SMART:
            ret = 1;
            break;

        default:
            break;
        }
        break;

    case MAIN_EXCEPT:
        switch (subType) {
        case SUB_EXCEPT_DISK_FULL:
        case SUB_EXCEPT_DISK_FAILURE:
        case SUB_EXCEPT_DISK_UNINITIALIZED:
        case SUB_EXCEPT_DISK_OFFLINE:
        case SUB_EXCEPT_DISK_HEAT:
        case SUB_EXCEPT_DISK_MICROTHERM:
        case SUB_EXCEPT_DISK_CONNECTION_EXCEPTION:
        case SUB_EXCEPT_DISK_DISK_STRIKE:
            ret = 1;
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return ret;
}
