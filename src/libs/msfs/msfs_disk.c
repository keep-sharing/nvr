/* 
 * ***************************************************************
 * Filename:      	msfs_disk.c
 * Created at:    	2017.10.10
 * Description:   	msfs disk API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include "MFdisk.h"
#include "MFcommon.h"
#include "msfs_disk.h"


MF_S8 *
msfs_disk_get_strtype(TYPE_EN enType)
{
	switch(enType)
	{
		case DISK_TYPE_LOCAL: return "LOCAL";
	    case DISK_TYPE_USB:	return "USB";
	    case DISK_TYPE_NAS: return "NFS";
        case DISK_TYPE_CIFS: return "CIFS";
	    case DISK_TYPE_RAID: return "RAID";
	    case DISK_TYPE_ESATA: return "eSATA";
	    case DISK_TYPE_IPSAN: return "IPSAN";
	    case DISK_TYPE_GLOBAL_SPARE: return "Spare";
	    case DISK_TYPE_IN_RAID: return "Array";
	    default :	return "unknown";
	}
	return " ";
}

MF_S8 *
msfs_disk_get_strstatus(STATE_EN enStat)
{
	switch(enStat)
	{
		case DISK_STATE_UNFORMATTED: return "Uninitialized";
		case DISK_STATE_NORMAL: return "Normal";
		case DISK_STATE_OFFLINE: return "Offline";
		case DISK_STATE_FORMATING: return "Formating";
		case DISK_STATE_BAD: return "Bad";
		case DISK_STATE_LOADING: return "Loading";
	    default :	return "unknown";
	}
	return " ";
}

MF_S32
msfs_disk_set_port_info(struct port_info_t *pstPort)
{
    if (!pstPort)
    {
        MFerr("pstPort is NULL");
        return MF_FAILURE;
    }
    MFinfo("id[%d] group[%d] enRw[%d]", pstPort->id, pstPort->group, pstPort->enRw);
    
    mf_disk_port_bind_group(pstPort->id, pstPort->group);
    mf_disk_port_permission(pstPort->id, pstPort->enRw);
    
    if (mf_disk_is_exist(pstPort->id))
        mf_record_update();
    
    return MF_SUCCESS;
}

MF_S32
msfs_disk_set_group_info(struct group_info_t *pstGrp)
{
    if (!pstGrp)
    {
        MFerr("pstGrp is NULL");
        return MF_FAILURE;
    }

    MFinfo("id[%d] mask[%#llx]", pstGrp->id, pstGrp->chnMaskl);    
    mf_disk_chn_bind_group(pstGrp->chnMaskl, pstGrp->id);
    
    return MF_SUCCESS;
}

void
msfs_disk_get_pv(struct list_head *head)
{
	if (!head) return;

	struct diskObj *pos = NULL;
	struct list_head *list = NULL;
	struct disk_port_vendor_t *pdport = NULL;

	list = mf_disk_get_rd_list(MF_NO);
	list_for_each_entry(pos, list, node)
	{
		if (pos->enType == DISK_TYPE_LOCAL ||
			pos->enType == DISK_TYPE_ESATA ||
			pos->enType == DISK_TYPE_IN_RAID ||
			pos->enType == DISK_TYPE_GLOBAL_SPARE)
		{
			pdport = ms_calloc(1, sizeof(struct disk_port_vendor_t));
			if (pdport)
			{
				pdport->port = pos->port;
				strncpy(pdport->vendor, pos->vendor, strlen(pos->vendor));
				list_add(&pdport->node, head);
			}
		}
	}
	mf_disk_put_rw_list(list);
}

void
msfs_disk_put_pv(struct list_head *head)
{
	if (!head) return;
	struct disk_port_vendor_t *pos = NULL, *n = NULL;
	list_for_each_entry_safe(pos, n, head, node)
	{
		list_del(&pos->node);
		ms_free(pos);
	}
}

void
msfs_disk_get_bad_port(struct list_head *head)
{
	if (!head) return;

	struct diskObj *pos = NULL;
	struct list_head *list = NULL;
	struct disk_port_t *pdport = NULL;

	list = mf_disk_get_rd_list(MF_NO);
	list_for_each_entry(pos, list, node)
	{
		/* because of nas owns a keep-alive process, do not contain */
		if (pos->enState == DISK_STATE_BAD &&
			(pos->enType == DISK_TYPE_LOCAL ||
			pos->enType == DISK_TYPE_RAID ||
			(pos->enType == DISK_TYPE_ESATA && pos->bRec == MF_YES)))
		{
			pdport = ms_calloc(1, sizeof(struct disk_port_t));
			if (pdport)
			{
				pdport->port = pos->port;
				list_add(&pdport->node, head);
			}
		}
	}
	mf_disk_put_rw_list(list);
}

void
msfs_disk_put_bad_port(struct list_head *head)
{
	if (!head) return;
	struct disk_port_t *pos = NULL, *n = NULL;
	list_for_each_entry_safe(pos, n, head, node)
	{
		list_del(&pos->node);
		ms_free(pos);
	}
}


MF_S32
msfs_disk_get_disk_info(struct disk_attr_t *pstAttr, MF_U8 port)
{
    if (!pstAttr)
    {
        MFerr("pstAttr is NULL");
        return MF_FAILURE;
    }
    MFinfo("port[%d]", port);
    
    struct diskObj *disk = NULL;
    struct list_head *list;
    struct disk_info_t *diskInfo;
    struct raid_info *raidInfo = NULL;
    MF_U8 diskNum = 0;
	MF_BOOL b1info = port < MAX_DISK_PORT_NUM ? MF_TRUE : MF_FALSE;
    struct list_head *loadingList;
    struct load_s *load;
	MF_U32 magic_num = 0;
	MF_U64 surplus = 0;

    INIT_LIST_HEAD(&pstAttr->list);

    list = mf_disk_get_rd_list(MF_NO);
    
    list_for_each_entry_reverse(disk, list, node)
    {
        if (b1info && disk->port != port)
			continue;
        if ((disk->enType != DISK_TYPE_RAID && disk->enState == DISK_STATE_OFFLINE) ||
			disk->enType == DISK_TYPE_NAND)
            continue;

    	diskInfo = ms_calloc(1, sizeof(struct disk_info_t));

        if (disk->enType == DISK_TYPE_RAID)
		{
			mf_check_raid_status();// find degrade raid to recovery
			//mf_disk_attribute(disk);// should update raid state anytime
			raidInfo = mf_disk_get_raid_priv(disk->pstPrivate);
			if (raidInfo != NULL)
			{
				memcpy(&diskInfo->astDisk.raid, raidInfo, sizeof(struct raid_info));
			}
        }

		/* sync free just for normal show the value */
		if (disk->enState == DISK_STATE_NORMAL)
		{
            if (disk->enType == DISK_TYPE_NAS || disk->enType == DISK_TYPE_CIFS)
            {
				if (disk->ops->disk_size) {
					disk->ops->disk_size(disk->pstPrivate);
				}

                surplus = disk->capacity - disk->fileNum*disk->pstHeader->perFileSize;
                MFinfo("surplus:%lld number:%d persize:%lld disk->free:%lld", surplus,
                    disk->fileNum,disk->pstHeader->perFileSize, disk->free);
                if (surplus < disk->free)
                    diskInfo->astDisk.free = surplus;
                else
                    diskInfo->astDisk.free = disk->free;
            }
            else
                diskInfo->astDisk.free = disk->free;
		}
		else
			diskInfo->astDisk.free = 0;

		MFinfo("disk[%d] free %lluG", disk->port, disk->free/1024/1024/1024);
        diskInfo->astDisk.bLoop 	= disk->bLoop;
        diskInfo->astDisk.bPrivate  = disk->bPrivate;
        diskInfo->astDisk.capacity  = disk->capacity;
        diskInfo->astDisk.port      = disk->port;
        diskInfo->astDisk.group     = disk->group;
        diskInfo->astDisk.enRw      = disk->enRw;
        diskInfo->astDisk.enType    = disk->enType;
        diskInfo->astDisk.smartTest = disk->smartTest;
		/* wait the raid disk to update state .solin 180528 */
		if (mf_disk_is_ready() == MF_FALSE &&
			mf_disk_get_raid_mode() == MF_YES &&
			disk->enType == DISK_TYPE_LOCAL &&
			disk->enState == DISK_STATE_UNFORMATTED)
		{
			if (mf_read_super_magic(disk->alias, &magic_num) == MF_SUCCESS &&
				(magic_num == RAID_MAGIC))
				diskInfo->astDisk.enType    = DISK_TYPE_IN_RAID;
				MFinfo("local port[%d] -> in raid temporary", disk->port);
        		//continue;
		}
		diskInfo->astDisk.enState   = disk->enState;

		if (disk->enType == DISK_TYPE_LOCAL ||
			disk->enType == DISK_TYPE_GLOBAL_SPARE ||
			disk->enType == DISK_TYPE_IN_RAID)
		{
			if(disk->enState == DISK_STATE_BAD || disk->enState == DISK_STATE_OFFLINE)
				diskInfo->astDisk.busable = MF_FALSE;
			else
				diskInfo->astDisk.busable = MF_TRUE;
		}

        strcpy(diskInfo->astDisk.name, disk->name);
        strcpy(diskInfo->astDisk.vendor, disk->vendor);
        strcpy(diskInfo->astDisk.host, disk->host);
        LIST_INSERT_SORT_ASC(diskInfo, &pstAttr->list, astDisk.port);
        diskNum++;

		if (b1info)
			break;
    }
	mf_disk_put_rw_list(list);

    loadingList = mf_disk_get_loading_list();
    list_for_each_entry(load, loadingList, node)
    {
        diskInfo = ms_calloc(1, sizeof(struct disk_info_t));

		if (load->enType == DISK_TYPE_RAID && load->pstObj)
		{
			raidInfo = mf_disk_get_raid_priv(load->pstObj->pstPrivate);
			if (raidInfo != NULL)
				memcpy(&diskInfo->astDisk.raid, raidInfo, sizeof(struct raid_info));
        }

        diskInfo->astDisk.capacity  = load->capacity;
        diskInfo->astDisk.free      = load->free;
        diskInfo->astDisk.port      = load->port;
        diskInfo->astDisk.group     = load->group;
        diskInfo->astDisk.enRw      = load->enRw;
        diskInfo->astDisk.enType    = load->enType;
        diskInfo->astDisk.enState   = load->enState;
        strcpy(diskInfo->astDisk.vendor, load->vendor);
        strcpy(diskInfo->astDisk.host, load->host);
        LIST_INSERT_SORT_ASC(diskInfo, &pstAttr->list, astDisk.port);
        diskNum++;
        MFinfo("loading [%d]\n", load->port);
    }
    mf_disk_put_loading_list(loadingList);

    list = mf_disk_get_format_list();
    list_for_each_entry(disk, list, node)
    {
        diskInfo = ms_calloc(1, sizeof(struct disk_info_t));

        diskInfo->astDisk.capacity  = disk->capacity;
        diskInfo->astDisk.free      = disk->free;
        diskInfo->astDisk.port      = disk->port;
        diskInfo->astDisk.group     = disk->group;
        diskInfo->astDisk.enRw      = disk->enRw;
        diskInfo->astDisk.enType    = disk->enType;
        diskInfo->astDisk.enState   = disk->enState;
        strcpy(diskInfo->astDisk.vendor, disk->vendor);
        strcpy(diskInfo->astDisk.host, disk->host);
        LIST_INSERT_SORT_ASC(diskInfo, &pstAttr->list, astDisk.port);
        diskNum++;
    }
    mf_disk_put_format_list(list);

    pstAttr->diskNum = diskNum;

    return MF_SUCCESS;
}

MF_S32
msfs_disk_put_disk_info(struct disk_attr_t *pstAttr)
{
    if (!pstAttr)
    {
        MFerr("pstAttr is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_disk_put_disk_info");
    
    struct disk_info_t *disk = NULL;
    struct disk_info_t *n = NULL;

    list_for_each_entry_safe(disk, n, &pstAttr->list, node)
    {
        list_del(&disk->node);
        ms_free(disk);
    }

    return MF_SUCCESS;
}

void
msfs_disk_set_mode(MODE_EN enMode)
{
    MFinfo("enMode[%d]", enMode); 

	if (enMode == DISK_MODE_QUOTA_ON)
        mf_disk_quota_set_enable(MF_YES);
	else if(enMode == DISK_MODE_QUOTA_OFF)
		mf_disk_quota_set_enable(MF_NO);

	if (enMode == DISK_MODE_GROUP_ON)
		mf_disk_group_set_enable(MF_YES);
	else if(enMode == DISK_MODE_GROUP_OFF)
		mf_disk_group_set_enable(MF_NO);
}	

MF_BOOL
msfs_disk_get_quota_enable(void)
{
	return mf_disk_quota_get_status();
}

MF_S32
msfs_disk_set_quota_info(quota_info_t *pstQta, MF_U32 chnNum)
{
	MF_U32 chnId, ret = MF_SUCCESS;
	
	for (chnId = 0; chnId<chnNum; chnId++)
	{
		ret |= mf_disk_set_chn_quota(pstQta[chnId].chnId, pstQta[chnId].vidQta, pstQta[chnId].picQta);
	}
	if (mf_disk_quota_get_status() == MF_YES)
		mf_record_update();
	return ret;
}

MF_S32
msfs_disk_get_quota_info(MF_U8 chnId, quota_info_t *pstQta)
{
	if (pstQta == NULL)
	{
		MFerr("pstQta pointer is NULL");
		return MF_FAILURE;
	}
	
	return mf_disk_get_chn_quota(chnId, pstQta);
}

MF_S32
msfs_disk_format(MF_U8 port, FORMAT_EN enFormat, MF_U64 quota)
{
    MFinfo("port[%d], enFormat[%d] quota[%llu]", port, enFormat, quota);    
    return  mf_disk_format(port, enFormat, quota);
}

MF_S32
msfs_disk_restore(MF_U8 port)
{
    MFinfo("port[%d]", port);    
    return  mf_disk_restore(port);
}

MF_S32
msfs_disk_restoreEx(MF_U8 port)
{
    MFinfo("port[%d]", port);    
    return  mf_disk_restoreEx(port);
}

void
msfs_disk_index_dump(MF_U8 portId)
{
    MFinfo("portId[%d]", portId);    
    mf_disk_index_dump(portId);
}

MF_S32
msfs_disk_get_Udisk(MF_S8 *mnt)
{
	MFinfo("mnt[%s]", mnt);
	return mf_disk_get_Udisk(mnt);
}

void
msfs_disk_put_Udisk(MF_S8 *mnt)
{
	MFinfo("mnt[%s]", mnt);
	mf_disk_put_Udisk(mnt);
}

void
msfs_disk_file_dump(MF_U8 portId, MF_U8 chnId)
{
    MFinfo("portId[%d] chnid[%d]", portId, chnId);    
    mf_disk_file_dump(portId, chnId);
}

void
msfs_disk_set_health_temperature(MF_U8 portId, MF_S32 temperature)
{
    MFinfo("portId[%d] temperature[%d]", portId, temperature);    
    mf_disk_set_health_temperature(portId, temperature);
}

void
msfs_disk_set_health_status(MF_U8 portId, MF_S32 status)
{
    MFinfo("portId[%d] status[%d]", portId, status);    
    mf_disk_set_health_status(portId, status);
}

void
msfs_disk_set_health_enable(MF_U64 enableMask)
{
    mf_disk_set_health_enable(enableMask);
}

MF_U32
msfs_disk_nas_get_dir(MF_S8 *hostIp, struct disk_nas_t *pstNas)
{
    MF_U32 dirNum = 0;
    
    if (!hostIp || !strlen(hostIp))
    {
        MFerr("hostIp[%p] is NULL", hostIp);
        return dirNum;
    }    
    MFinfo("hostIp[%s]", hostIp);
    
    INIT_LIST_HEAD(&pstNas->list);
    dirNum = mf_disk_nas_get_dir(hostIp, &pstNas->list);
    pstNas->dirNum = dirNum;
    return dirNum;
}

void
msfs_disk_nas_put_dir(struct disk_nas_t *pstNas)
{
    if (!pstNas)
    {
        MFerr("pstNas is NULL");
        return;
    }

    MFinfo("msfs_disk_nas_put_dir");    
    mf_disk_nas_put_dir(&pstNas->list);
}

MF_S32
msfs_disk_nas_add(MF_S8 * name, MF_U8 port, MF_S8 * host, MF_S32 type, MF_S8 *user, MF_S8 *password)
{
    if (!name || !strlen(name)) {
        return MF_FAILURE;
    }

    if (type == 0) {
        if (!host || !strlen(host) || !strchr(host, ':')) {
            return MF_FAILURE;
        }
    	if (strstr(host, "//") || (*(strchr(host, ':') + 1) != '/')) {
            MFerr("path[%s] is illegal", host);
            return MF_FAILURE;
        }
    }

    MF_S32 ret = mf_disk_nas_add(name, port, host, type, user, password);
    MFinfo("name[%s], type[%s] port[%d], host[%s] user[%s] pwd[%s] ret:%d", name,
        type==0?"nfs":"cifs", port, host, user, password, ret);
    return ret;
}

MF_S32
msfs_disk_nas_del(MF_U8 port)
{
    MFinfo("port[%d]", port);
    return mf_disk_nas_del(port);
}

MF_S32
msfs_disk_nas_rename(MF_S8 *name, MF_U8 port)
{
    if (!name || !strlen(name))
    {
        MFerr("name[%p] is NULL", name);
        return MF_FAILURE;
    }
    MFdbg("port[%d]rename %s", port, name);
    
    return mf_disk_nas_rename(name, port);
}

MF_S32
msfs_raid_info_add(MF_U8 port, MF_S8 * vendor)
{
    if (!vendor || !strlen(vendor))
    {
        MFerr("name[%p] is NULL", vendor);
        return MF_FAILURE;
    }

    MFinfo("port[%d], vendor[%s]", port, vendor);
	mf_raid_info_add(port, vendor);
	
    return MF_SUCCESS;
}

MF_S32
msfs_raid_rename(MF_S8 *vendor, MF_U8 port)
{
    if (!vendor || !strlen(vendor))
    {
        MFerr("name[%p] is NULL", vendor);
        return MF_FAILURE;
    }
	return	mf_raid_rename(vendor, port);
}

void 
msfs_raid_deal_create_failed(struct raid_op_t *raid)
{
    if (!raid) {
        MFerr("raid is NULL");
        return;
    }
    MFinfo("msfs_raid_deal_create_failed");

    return mf_raid_deal_create_failed(raid);
}

MF_S32 
msfs_raid_create(struct raid_op_t *raid)
{
    if (!raid)
    {
        MFerr("raid is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_raid_create");
    
	return mf_raid_create(raid);
}

MF_S32
msfs_raid_del(int raid_port)
{
    MFinfo("raid_port[%d]", raid_port);
    
	return mf_raid_del(raid_port);
}

MF_S32
msfs_raid_rebuild(struct raid_op_t *raid)
{
    if (!raid)
    {
        MFerr("raid is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_raid_rebuild");
    
	return mf_raid_rebuild(raid);
}

void
msfs_get_component_size_raid(MF_S32 *raidport, MF_U64 *rsize)
{    
    if (!raidport || !rsize)
    {
        MFerr("raidport[%p] rsize[%p] is NULL", raidport, rsize);
        return;
    }
    MFinfo("msfs_get_component_size_raid");
    
	mf_get_component_size_raid(raidport, rsize);
}

MF_S32
msfs_raid_rebuild_progress(MF_S32 raid_port)
{   
    MFinfo("raid_port[%d]", raid_port);
	return mf_raid_rebuild_progress(raid_port);
}

void
msfs_disk_set_loop(MF_BOOL bLoop)
{
    MFinfo("bLoop[%d]", bLoop);
    mf_disk_set_loop(bLoop);
}

void
msfs_disk_set_private(MF_U8 port, MF_BOOL bEnable)
{
    MFinfo("port[%d] private[%d]", port, bEnable);
    mf_disk_go_private(port, bEnable);
}

void
msfs_disk_set_esata_usage(MF_BOOL brec)
{
	MF_BOOL type = brec == 0 ? MF_YES : MF_NO;
	
    MFinfo("brec[%d]", brec);
    mf_disk_set_esata_usage(type);
}

void
msfs_disk_set_raid_mode(MF_BOOL bmode)
{
    MFinfo("bmode[%d]", bmode);
    mf_disk_set_raid_mode(bmode);
}

MF_S32
msfs_raid_create_update_local(raid_op_t *raid)
{
    if (!raid) {
        MFerr("raid is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_raid_create_update_local");
    
    return mf_raid_create_update_local(raid);
}


void 
msfs_disk_scan(void)
{
    MFinfo("msfs_disk_scan");    
	mf_disk_scan();
}

MF_S32
msfs_disk_init(DISK_USER_CB user_cb, disk_param *para)
{
    MFinfo("user_cb[%p] para[%p]", user_cb, para);
    mf_common_init();
    mf_retr_init();
    mf_disk_init(user_cb, para);
    mf_index_init();
    //mf_scan_raid_init();
    return MF_SUCCESS;
}

void
msfs_disk_deinit()
{
    MFinfo("msfs_disk_deinit");
    mf_index_deinit();
    mf_disk_deinit();
    mf_retr_deinit();
    mf_common_deinit();
}

MF_S32 
msfs_create_global_spare(MF_S32 disk_port)
{
    MFinfo("disk_port[%d]", disk_port);
	MF_S32 res = mf_create_global_spare(disk_port);
	mf_check_raid_status();// find degrade raid to recovery
	
	return res;
}

MF_S32 
msfs_remove_global_spare(MF_S32 disk_port)
{
    MFinfo("disk_port[%d]", disk_port);
	return mf_remove_global_spare(disk_port);
}

void
msfs_disk_notify_bar(PROGRESS_BAR_E enBar, MF_U8 port, MF_S32 percent, TYPE_EN type)
{
    MFinfo("port[%d] percent[%d] type[%d]", port, percent, type);
	mf_disk_notify_bar(enBar, port, percent, type);
}

void
msfs_disk_bad_reload(struct list_head *head)
{
    MFinfo(" ");
	mf_disk_bad_reload(head);
}

MF_S32
msfs_disk_op_smart(int port, HDD_ATTR *attr, int test_type)
{
    if (!attr)
    {
        MFerr("attr is NULL");
        return MF_FAILURE;
    }
	MF_S32 ret = MF_FAILURE;
	MF_S8 dev_path[DEV_NAME_LEN] = {0};
	
    MFinfo("port[%d] test_type[%d]", port, test_type);

	if(mf_find_name_port(dev_path, (MF_U8 *)&port, CMD_FIND_ALIAS_BY_PORT) == MF_SUCCESS)
		ret = mf_disk_op_smart(dev_path, attr, test_type);

	return ret;
}

MF_S32
msfs_disk_get_smart_attr(HDD_ATTR *attr, int port)
{
    MFinfo("port[%d]", port);
	return mf_disk_get_smart_attr(attr, port);
}

MF_S32
msfs_disk_del_smart_attr_list(HDD_ATTR *attr)
{
    MFinfo(" ");
	return mf_disk_del_smart_attr_list(attr);
}

void
msfs_disk_event_get()
{
    mf_disk_event_get();
}

void
msfs_disk_dbg_switch(MF_BOOL bDebug)
{
    MFinfo("bDebug[%d]", bDebug);
    mf_disk_dbg_switch(bDebug);
}

void
msfs_disk_dbg_mem_usage()
{
    MFinfo("msfs_disk_dbg_mem_usage");
    mf_disk_dbg_show_mem_usage();
}

void
msfs_disk_dbg_level(DBG_E enLevel)
{
    MFinfo("enLevel[%d]", enLevel);
    g_msfs_debug = enLevel;
}

void
msfs_disk_dbg_print(void *print)
{
    g_debug_print = print;
}

MF_BOOL
msfs_disk_is_can_record()
{
    return mf_disk_is_can_record();
}

void
msfs_check_raid_status()
{
    mf_check_raid_status();
}

MF_S32
msfs_disk_get_health_data(disk_health_data_t *health, int port)
{
    if (!health) {
        return MF_FAILURE;
    }
    msprintf("port[%d]", port);
    msprintf("msfs_disk_get_health_data start");
    
    struct diskObj *disk = NULL;
    struct list_head *list;
    int i = 0,j = 0;
    list = mf_disk_get_rd_list(MF_NO);
    health->port = port;
    list_for_each_entry_reverse(disk, list, node)
    {
        if (disk->port != port) {
            continue;
        }
        if ((disk->enType != DISK_TYPE_LOCAL && disk->enType != DISK_TYPE_ESATA && disk->enType != DISK_TYPE_IN_RAID)) {
            continue;
        }

        health->HMStatus = disk->HMStatus;
        health->supportHM = disk->supportHM;
        health->hasDisk = 1;
        strcpy(health->vendor, disk->vendor);
        time_t     currentTime = time(NULL);
        struct tm *timeInfo    = localtime(&currentTime);
        timeInfo->tm_min       = 0;
        timeInfo->tm_sec       = 0;
        time_t hourTimestamp   = mktime(timeInfo);
        time_t earlier_time = hourTimestamp - (719 * 3600);
        for (i = 0; i < MAX_DISK_HM_TEMPERATURE - 1; i++) {
            health->temperatureList[i].timestamp= earlier_time;
            while (disk->logTemperature[j].timestamp < earlier_time) {
                j++;
            }
            if (disk->logTemperature[j].timestamp == earlier_time) { 
                health->temperatureList[i].temperature = disk->logTemperature[j].temperature;
                if (health->temperatureList[i].temperature < -40) {
                    health->temperatureList[i].temperature = -40;
                }
                if (health->temperatureList[i].temperature > 100) {
                    health->temperatureList[i].temperature = 100;
                }
                j++;
            } else {
                health->temperatureList[i].temperature = -300;
            }
            earlier_time += 3600;
        }
        health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].timestamp = disk->logTemperature[disk->lastTemperatureIndex].timestamp;
        health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature = disk->logTemperature[disk->lastTemperatureIndex].temperature;
        if (health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature < -40) {
            health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature = -40;
        }
        if (health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature > 100) {
            health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature = 100;
        }
        if (health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].timestamp == 0) {
            timeInfo    = localtime(&currentTime);
            timeInfo->tm_sec       = 0;
            time_t minTimestamp   = mktime(timeInfo);
            health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].timestamp = minTimestamp;
            health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature = 1;
        }
        if (health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature <= 0) {
            health->temperatureStatus = 2;
        }
        if (health->temperatureList[MAX_DISK_HM_TEMPERATURE - 1].temperature > 60) {
            health->temperatureStatus = 1;
        }
        break;
    }

    msprintf("Gsjt :port :%d", health->port);
    msprintf("msfs_disk_get_health_data end");
	mf_disk_put_rw_list(list);
    return MF_SUCCESS;
}

MF_S32
msfs_disk_get_wwn(char *wwn, int port)
{
    msprintf("port[%d]", port);
    
    struct diskObj *disk = NULL;
    struct list_head *list;
    int i = 0,j = 0;
    list = mf_disk_get_rd_list(MF_NO);
    
    list_for_each_entry_reverse(disk, list, node)
    {
        msprintf("gsjt %d",disk->port);
        if (disk->port != port) {
            continue;
        }
        if ((disk->enType != DISK_TYPE_LOCAL && disk->enType != DISK_TYPE_ESATA)) {
            continue;
        }
        strcpy(wwn, disk->wwn);
        break;

    }

    msprintf("Gsjt :port :%d wwn:%s", port,wwn);
	mf_disk_put_rw_list(list);
    return MF_SUCCESS;
}