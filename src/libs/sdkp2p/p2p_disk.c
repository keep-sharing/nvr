#include "msg.h"
#include "sdk_util.h"


SDK_ERROR_CODE get_format_export_disk_str(const char *buff, struct req_usb_info *info)
{
    char sValue[256] = {0};

    if (!buff || !info) {
        return PARAM_ERROR;
    }
    
    memset(info, 0, sizeof(struct req_usb_info));
    if (!sdkp2p_get_section_info(buff, "devName=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info->dev_name, sizeof(info->dev_name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "devPath=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info->dev_path, sizeof(info->dev_path), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "formatType=", sValue, sizeof(sValue))) {
        info->formatType = atoi(sValue);
    } else {
        info->formatType = FORMAT_FAT32_TYPE;
    }

    return REQUEST_SUCCESS;
}

int get_storage_info_json(void *data, int cnt, char *resp, int respLen)
{
    if (!resp || respLen <= 0) {
        return -1;
    }

    char *sResp = NULL;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    RESP_GET_MSFS_DISKINFO *disks = (RESP_GET_MSFS_DISKINFO *)data;
    int count = 0;
    int i = 0, iNum = 0, sata_port = 0;
    char encode[128] = {0};
    int total_size = 0, free_size = 0, used_size = 0;
    int port = 0;
    struct diskInfo tmp[MAX_MSDK_NUM];
    int diskCnt = 0;
    int index;
    int esataType;
    RESP_GET_MSFS_DISKINFO info[MAX_MSDK_NUM];

    json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    
    memset(&info[0], 0, sizeof(RESP_GET_MSFS_DISKINFO) * MAX_MSDK_NUM);
    //1.from mscore
    for (i = 0 ; i < cnt; i++) {
        port = disks[i].disk_port;
        if (port >= 1 && port <= MAX_MSDK_NUM) {
            memcpy(&info[port - 1], &disks[i], sizeof(RESP_GET_MSFS_DISKINFO));
        }
    }
    //2.from table disk for offline
    memset(&tmp[0], 0, sizeof(struct diskInfo) * MAX_MSDK_NUM);
    read_disks(SQLITE_FILE_NAME, tmp, &diskCnt);
    for (i = 0; i < diskCnt; i++) {
        port = tmp[i].disk_port;
        if (tmp[i].enable != 1 || port <= 0 || port > MAX_MSDK_NUM) {
            continue;
        }
        index = port - 1;
        if (info[index].disk_port) {
            //从数据库获取nfs smb的相关信息
            if (info[index].disk_type == DISK_TYPE_NAS || info[index].disk_type == DISK_TYPE_CIFS) {
                snprintf(info[index].disk_vendor, sizeof(info[index].disk_vendor), "%s", tmp[i].disk_vendor);
                snprintf(info[index].disk_address, sizeof(info[index].disk_address), "%s", tmp[i].disk_address);
                snprintf(info[index].disk_directory, sizeof(info[index].disk_address), "%s", tmp[i].disk_directory);
                snprintf(info[index].user, sizeof(info[index].user), "%s", tmp[i].user);
                snprintf(info[index].password, sizeof(info[index].password), "%s", tmp[i].password);
            }
        } else {
            //离线的盘从数据库获取
            info[index].disk_port = tmp[i].disk_port;
            info[index].disk_type = tmp[i].disk_type;
            info[index].disk_property = tmp[i].disk_property;
            info[index].disk_group = tmp[i].disk_group;
            info[index].raid_level = tmp[i].raid_level;
            snprintf(info[index].disk_vendor, sizeof(info[index].disk_vendor), "%s", tmp[i].disk_vendor);
            snprintf(info[index].disk_address, sizeof(info[index].disk_address), "%s", tmp[i].disk_address);
            snprintf(info[index].disk_directory, sizeof(info[index].disk_address), "%s", tmp[i].disk_directory);
            info[index].status = DISK_STATE_OFFLINE;
            snprintf(info[index].user, sizeof(info[index].user), "%s", tmp[i].user);
            snprintf(info[index].password, sizeof(info[index].password), "%s", tmp[i].password);
        }
    }

    array = cJSON_CreateArray();
    for (i = 0; i < MAX_MSDK_NUM; i++) {
        if (info[i].disk_port == 0) {
            continue;
        }
        obj = cJSON_CreateObject();
        if (!obj) {
            continue;
        }
        
        total_size = info[i].total / (1024 * 1024);
        used_size = info[i].used / (1024 * 1024);
        free_size = info[i].free / (1024 * 1024);
        cJSON_AddNumberToObject(obj, "portId", info[i].disk_port);
        cJSON_AddStringToObject(obj, "vendorName", info[i].disk_vendor);
        cJSON_AddStringToObject(obj, "address", info[i].disk_address);
        cJSON_AddStringToObject(obj, "directory", info[i].disk_directory);
        cJSON_AddNumberToObject(obj, "totalSize", total_size);
        cJSON_AddNumberToObject(obj, "usedSize", used_size);
        cJSON_AddNumberToObject(obj, "freeSize", free_size);
        cJSON_AddNumberToObject(obj, "hddType", info[i].disk_type);
        cJSON_AddNumberToObject(obj, "state", info[i].status);
        cJSON_AddNumberToObject(obj, "group", info[i].disk_group);
        cJSON_AddNumberToObject(obj, "property", info[i].disk_property);
        cJSON_AddNumberToObject(obj, "private", info[i].disk_private);
        cJSON_AddNumberToObject(obj, "raid_id", info[i].disk_port);
        cJSON_AddNumberToObject(obj, "raid_level", info[i].raid_level);
        cJSON_AddNumberToObject(obj, "raid_state", info[i].raid_status);
        cJSON_AddNumberToObject(obj, "raid_progress", info[i].raid_task);
        cJSON_AddNumberToObject(obj, "raid_sata_cnt", info[i].raid_diskcnt);
        cJSON_AddNumberToObject(obj, "raid_task", info[i].raid_task);
        cJSON_AddNumberToObject(obj, "bUsable", info[i].busable);
        get_url_encode(info[i].user, strlen(info[i].user), encode, sizeof(encode));
        cJSON_AddStringToObject(obj, "userName", encode);
        get_url_encode(info[i].password, strlen(info[i].password), encode, sizeof(encode));
        cJSON_AddStringToObject(obj, "password", encode);
        sata_port = 0;
        for (iNum = 0; iNum < info[i].raid_diskcnt; iNum++) {
            if (info[i].raid_disk[iNum] != 0) {
                sata_port |= (1 << (info[i].raid_disk[iNum]));
            }
        }
        cJSON_AddNumberToObject(obj, "raid_sata_port", sata_port);
        cJSON_AddNumberToObject(obj, "smartTest", info[i].smartTest);
        cJSON_AddItemToArray(array, obj);
        count++;
    }

    cJSON_AddNumberToObject(json, "maxDiskNum", MAX_MSDK_NUM);
    if (cnt == 0 && count == 0) {
        cJSON_AddStringToObject(json, "resultString", "NO");
    } else {
        cJSON_AddStringToObject(json, "resultString", "OK");
    }

    esataType = get_param_int(SQLITE_FILE_NAME, PARAM_ESATA_TYPE, 0);
    cJSON_AddNumberToObject(json, "esataType", esataType);
    int MaxHMDiskCnt = 0;
#if defined(_HI3536A_)
    struct device_info device = {0};
    db_get_device(SQLITE_FILE_NAME, &device);
    if (device.prefix[0] == '8') {
        MaxHMDiskCnt = 9;
    } else {
        MaxHMDiskCnt = 4;
    }
#endif
    cJSON_AddNumberToObject(json, "maxHMDiskNum", MaxHMDiskCnt);
    cJSON_AddItemToObject(json, "HDD", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}

static void req_disk_msfs_update_diskmode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_recycle_mode *res = (struct req_recycle_mode *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.port=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->port));
    }
    if (!sdkp2p_get_section_info(buf, "r.mode=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->mode));
    }

    *datalen = sizeof(struct req_recycle_mode);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}



static void req_disk_add_nas(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_batch_del_nas_info res;
    char sValue[256] = {0};
    char temp[256] = {0};
    int cnt = 0;
    int i = 0;
    struct diskInfo disk;
    if (!sdkp2p_get_section_info(buf, "r.cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }
    memset(&disk, 0x0, sizeof(struct diskInfo));
    memset(&res, 0x0, sizeof(struct req_batch_del_nas_info));


    struct op_lr_add_del_network_disk oland;
    memset(&oland, 0, sizeof(oland));
    for (i = 0; i < cnt; i++) {
        disk.enable = 1;
        snprintf(temp, sizeof(temp), "r[%d].port=", i);
        if (sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            continue;
        }
        disk.disk_port = atoi(sValue);
        disk.disk_type = DISK_TYPE_NAS;
        snprintf(temp, sizeof(temp), "r[%d].type=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            disk.disk_type = atoi(sValue);
        }
        disk.disk_property = 1;
        snprintf(temp, sizeof(temp), "r[%d].property=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            disk.disk_property = atoi(sValue);
        }
        snprintf(temp, sizeof(temp), "r[%d].vendor=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            snprintf(disk.disk_vendor, sizeof(disk.disk_vendor), "%s", sValue);
        }
        snprintf(temp, sizeof(temp), "r[%d].address=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            snprintf(disk.disk_address, sizeof(disk.disk_address), "%s", sValue);
        }
        snprintf(temp, sizeof(temp), "r[%d].directory=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            snprintf(disk.disk_directory, sizeof(disk.disk_directory), "%s", sValue);
        }
        res.disk_port[res.nCnt] = disk.disk_port;
        res.nCnt++;
        write_disk(SQLITE_FILE_NAME, &disk);

        if (DISK_TYPE_NAS == disk.disk_type || DISK_TYPE_CIFS == disk.disk_type) {
            snprintf(oland.type, sizeof(oland.type), "%s", "NFS");
        } else if (DISK_TYPE_CIFS == disk.disk_type) {
            snprintf(oland.type, sizeof(oland.type), "%s", "SMB/CIFS");
        } else {
            snprintf(oland.type, sizeof(oland.type), "%d", disk.disk_type);
        }
        snprintf(oland.ip, sizeof(oland.ip), "%s", disk.disk_address);
        snprintf(oland.path, sizeof(oland.path), "%s", disk.disk_directory);
        sdkp2p_common_write_log(conf, MAIN_OP, SUP_OP_ADD_NETWORK_DISK_REMOTE, SUB_PARAM_NONE, disk.disk_port,
                                OP_ADD_NETWORK_DISK, &oland, sizeof(struct op_lr_add_del_network_disk));
    }

    sdkp2p_send_msg(conf, conf->req, &res, sizeof(struct req_batch_del_nas_info), SDKP2P_NEED_CALLBACK);
}

static void resp_disk_remove_nas(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_disk_remove_nas(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_batch_del_nas_info *res = (struct req_batch_del_nas_info *)param;
    char sValue[256] = {0};
    char temp[256] = {0};
    int cnt = 1;
    int i = 0;
    if (!sdkp2p_get_section_info(buf, "r.cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }
    memset(res, 0x0, sizeof(struct req_batch_del_nas_info));
    for (i = 0; i < cnt; i++) {
        snprintf(temp, sizeof(temp), "r[%d].port=", i);
        if (sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            continue;
        }
        res->disk_port[res->nCnt] = atoi(sValue);
        res->nCnt++;
    }
    *datalen = sizeof(struct req_batch_del_nas_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void req_disk_edit_nas(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int res = 0;
    char sValue[256] = {0};
    struct diskInfo disk;

    struct op_lr_add_del_network_disk oland;
    memset(&oland, 0, sizeof(oland));
    memset(&disk, 0x0, sizeof(struct diskInfo));
    disk.disk_port = -1;
    disk.enable = 1;
    if (!sdkp2p_get_section_info(buf, "r.port=", sValue, sizeof(sValue))) {
        disk.disk_port = atoi(sValue);
    }
    disk.disk_type = DISK_TYPE_NAS;
    if (!sdkp2p_get_section_info(buf, "r.type=", sValue, sizeof(sValue))) {
        disk.disk_type = atoi(sValue);
    }
    disk.disk_property = 1;
    if (!sdkp2p_get_section_info(buf, "r.property=", sValue, sizeof(sValue))) {
        disk.disk_property = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.vendor=", sValue, sizeof(sValue))) {
        snprintf(disk.disk_vendor, sizeof(disk.disk_vendor), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.address=", sValue, sizeof(sValue))) {
        snprintf(disk.disk_address, sizeof(disk.disk_address), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.directory=", sValue, sizeof(sValue))) {
        snprintf(disk.disk_directory, sizeof(disk.disk_directory), "%s", sValue);
    }
    if (disk.disk_port >= 0) {
        write_disk(SQLITE_FILE_NAME, &disk);
    }
    res = disk.disk_port;

    sdkp2p_send_msg(conf, conf->req, &res, sizeof(int), SDKP2P_NEED_CALLBACK);

    if (DISK_TYPE_NAS == disk.disk_type || DISK_TYPE_CIFS == disk.disk_type) {
        snprintf(oland.type, sizeof(oland.type), "%s", "NFS");
    } else if (DISK_TYPE_CIFS == disk.disk_type) {
        snprintf(oland.type, sizeof(oland.type), "%s", "SMB/CIFS");
    } else {
        snprintf(oland.type, sizeof(oland.type), "%d", disk.disk_type);
    }
    snprintf(oland.ip, sizeof(oland.ip), "%s", disk.disk_address);
    snprintf(oland.path, sizeof(oland.path), "%s", disk.disk_directory);
    sdkp2p_common_write_log(conf, MAIN_OP, SUP_OP_CONFIG_NETWORK_DISK_REMOTE, SUB_PARAM_NONE, disk.disk_port,
                            OP_ADD_NETWORK_DISK, &oland, sizeof(struct op_lr_add_del_network_disk));
}

static void req_disk_get_nas_search(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_search_nas_info res;
    char sValue[256] = {0};

    memset(&res, 0x0, sizeof(struct req_search_nas_info));
    if (!sdkp2p_get_section_info(buf, "r.port=", sValue, sizeof(sValue))) {
        res.disk_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.address=", sValue, sizeof(sValue))) {
        snprintf(res.disk_address, sizeof(res.disk_address), "%s", sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &res, sizeof(struct req_search_nas_info), SDKP2P_NEED_CALLBACK);
}

static void resp_disk_get_nas_search(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct resp_search_nas_info *resp = (struct resp_search_nas_info *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.cnt=%d&", resp->nCnt);
    if (resp->nCnt > 0) {
        for (i = 0; i < resp->nCnt; i++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].directory=%s&", i, resp->directory[i]);
        }
    }
    *datalen = strlen(buf) + 1;
}

static void get_smart_attr_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_smart_attr *
    struct resp_get_smart_attr *res = (struct resp_get_smart_attr *)param;
    int num = size / sizeof(struct resp_get_smart_attr);
    int i;
    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", num);
    for (i = 0; i < num; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status.uptime=%d&", i, res[i].status.uptime);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status.temperature=%d&", i, res[i].status.temperature);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status.status=%d&", i, res[i].status.status);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status.result=%s&", i, res[i].status.result);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].attrsize=%d&", i, res[i].attrsize);
    }

    *datalen = strlen(buf) + 1;
}

static void get_smart_process_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void get_diskinfo_msfs_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_diskinfo*
    struct resp_get_msfs_diskinfo *disks = (struct resp_get_msfs_diskinfo *)param;
    int num = size / sizeof(struct resp_get_msfs_diskinfo);
    int i = 0, j = 0, total_size = 0, free_size = 0, used_size = 0;
    int realCnt = 0;

    //need sort
    int port = 0;
    struct resp_get_msfs_diskinfo info[MAX_MSDK_NUM];
    memset(info, 0x0, sizeof(struct resp_get_msfs_diskinfo)*MAX_MSDK_NUM);

    //from mscore
    for (i = 0 ; i < num; i++) {
        port = disks[i].disk_port;
        if (port >= 1 && port <= MAX_MSDK_NUM) {
            memcpy(&info[port - 1], &disks[i], sizeof(struct resp_get_msfs_diskinfo));
        }
    }

    //from table disk
    struct diskInfo tmpDisk;
    for (i = 0; i < MAX_MSDK_NUM; i++) {
        if (read_disk(SQLITE_FILE_NAME, &tmpDisk, i + 1) == 0 && tmpDisk.enable == 1) {
            port = tmpDisk.disk_port;
            for (j = 0; j < num; j++) {
                if (port == disks[j].disk_port) {
                    //��ͬ�Ķ˿�
                    if (disks[j].disk_type == DISK_TYPE_NAS) {
                        if (disks[j].disk_address[0] == '\0') {
                            snprintf(info[port - 1].disk_address, sizeof(info[port - 1].disk_address), "%s", tmpDisk.disk_address);
                        }

                        if (disks[j].disk_directory[0] == '\0') {
                            snprintf(info[port - 1].disk_directory, sizeof(info[port - 1].disk_directory), "%s", tmpDisk.disk_directory);
                        }
                    }

                    break;
                }
            }

            if (j >= num && port >= 1 && port <= MAX_MSDK_NUM) {
                memset(&info[port - 1], 0x0, sizeof(struct resp_get_msfs_diskinfo));
                info[port - 1].disk_port = port;
                snprintf(info[port - 1].disk_vendor, sizeof(info[port - 1].disk_vendor), "%s", tmpDisk.disk_vendor);
                snprintf(info[port - 1].disk_address, sizeof(info[port - 1].disk_address), "%s", tmpDisk.disk_address);
                snprintf(info[port - 1].disk_directory, sizeof(info[port - 1].disk_directory), "%s", tmpDisk.disk_directory);
                info[port - 1].status = DISK_STATE_OFFLINE;
                info[port - 1].disk_property = tmpDisk.disk_property;
                info[port - 1].raid_level = tmpDisk.raid_level;
                info[port - 1].disk_type = tmpDisk.disk_type;
                info[port - 1].raid_level = tmpDisk.raid_level;
            }
        }

    }


    for (realCnt = 0, i = 0; i < MAX_MSDK_NUM; i++) {
        if (info[i].disk_port == 0) {
            continue;
        }
        realCnt++;
    }


    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", realCnt);
    for (i = 0, j = 0; i < MAX_MSDK_NUM; i++) {
        if (info[i].disk_port == 0) {
            continue;
        }
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].port=%d&", j, info[i].disk_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].vendor=%s&", j, info[i].disk_vendor);
        total_size = info[i].total / (1024 * 1024 * 1024);
        used_size = info[i].used / (1024 * 1024 * 1024);
        free_size = info[i].free / (1024 * 1024 * 1024);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].total=%u&", j, total_size);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].used=%u&", j, used_size);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].free=%u&", j, free_size);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].state=%d&", j, info[i].status);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].hddtype=%d&", j, info[i].disk_type);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].disk_property=%d&", j, info[i].disk_property);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].disk_group=%d&", j, info[i].disk_group);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].disk_private=%d&", j, info[i].disk_private);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].raid.state=%d&", j, info[i].raid_status);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].raid.level=%d&", j, info[i].raid_level);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].raid.task=%d&", j, info[i].raid_task);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].raid.diskcnt=%d&", j, info[i].raid_diskcnt);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].raid.smartTest=%d&", j, info[i].smartTest);
        j++;
    }

    *datalen = strlen(buf) + 1;
}

static void req_disk_msfs_format_disk(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int diskid = 0;

    if (!sdkp2p_get_section_info(buf, "diskid=", sValue, sizeof(sValue))) {
        diskid = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &diskid, sizeof(int), SDKP2P_NEED_CALLBACK);
    struct op_lr_init_disk olid;
    memset(&olid, 0x0, sizeof(struct op_lr_init_disk));
    olid.port = diskid;
    olid.state = 1;
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_DISK_INIT_REMOTE, SUB_PARAM_NONE, diskid, OP_INIT_DISK, &olid,
                            sizeof(struct op_lr_init_disk));
}


static void resp_disk_msfs_format_disk(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_disk_remove_raid(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int diskid = 0;

    if (!sdkp2p_get_section_info(buf, "portId=", sValue, sizeof(sValue))) {
        diskid = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &diskid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_disk_remove_raid(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}


static void req_disk_remove_local(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int diskid = 0;

    if (!sdkp2p_get_section_info(buf, "portId=", sValue, sizeof(sValue))) {
        diskid = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &diskid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_disk_remove_local(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_disk_set_port(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct diskInfo disk;
    struct req_port_info info;

    memset(&disk, 0x0, sizeof(struct diskInfo));
    memset(&info, 0, sizeof(struct req_port_info));

    if (!sdkp2p_get_section_info(buf, "portId=", sValue, sizeof(sValue))) {
        disk.disk_port = atoi(sValue);
    }
    read_disk(SQLITE_FILE_NAME, &disk, disk.disk_port);

    if (!sdkp2p_get_section_info(buf, "property=", sValue, sizeof(sValue))) {
        disk.disk_property  = atoi(sValue);
    }
    if (disk.disk_property != 1) {
        disk.disk_property = 0;
    }

    if (!sdkp2p_get_section_info(buf, "group=", sValue, sizeof(sValue))) {
        disk.disk_group = atoi(sValue);
    }
    write_disk(SQLITE_FILE_NAME, &disk);

    info.id = disk.disk_port;
    info.enRw = disk.disk_property;
    info.group = disk.disk_group;
    info.iPrivate = (!sdkp2p_get_section_info(buf, "private=", sValue, sizeof(sValue)) ? atoi(sValue) : MS_INVALID_VALUE);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_port_info), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_DISK_RECYCLE_MODE, disk.disk_port, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void resp_disk_get_rec_advanced(void *param, int size, char *buf, int len, int *datalen)
{
    RECORD_ADVANCED *res = (RECORD_ADVANCED *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "mode=%d&", res->recycle_mode);
    snprintf(buf + strlen(buf), len - strlen(buf), "esata=%d&", res->esata_type);

    *datalen = strlen(buf) + 1;
}

static void req_disk_set_rec_advanced(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_record_conf info;

    memset(&info, 0, sizeof(struct req_record_conf));
    if (!sdkp2p_get_section_info(buf, "mode=", sValue, sizeof(sValue))) {
        info.mode = atoi(sValue);
    }

    if (info.mode > 1) {
        info.mode = 1;
    } else if (info.mode < 0) {
        info.mode = 0;
    }

    if (sdkp2p_get_section_info(buf, "esata=", sValue, sizeof(sValue))) {
        info.esata = get_param_int(SQLITE_FILE_NAME, PARAM_ESATA_TYPE, 0);
    } else {
        info.esata = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_record_conf), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}


static void resp_disk_add_nas(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_disk_edit_nas(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_disk_get_quota(void *param, int size, char *buf, int len, int *datalen)
{
    if (size <= 0) {
        return ;
    }
    struct req_quota *quota = (struct req_quota *)param;
    if (!quota) {
        return;
    }

    int i = 0, chnid = -1;
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", quota->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", quota->cnt);
    for (i = 0; i < quota->cnt && i < MAX_REAL_CAMERA; i++) {
        chnid = quota->quota_conf[i].chnId;
        snprintf(buf + strlen(buf), len - strlen(buf), "video_quota[%d]=%d&", chnid, quota->quota_conf[chnid].vidQta);
        snprintf(buf + strlen(buf), len - strlen(buf), "video_used[%d]=%d&", chnid, quota->quota_conf[chnid].vidUsd);
        snprintf(buf + strlen(buf), len - strlen(buf), "picture_quota[%d]=%d&", chnid, quota->quota_conf[chnid].picQta);
        snprintf(buf + strlen(buf), len - strlen(buf), "picture_used[%d]=%d&", chnid, quota->quota_conf[chnid].picUsd);
    }

    return ;
}

static void req_disk_set_quota(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char tmp[128] = {0};
    char sValue[256] = {0};
    int cnt = 0;
    struct req_quota req_info;
    memset(&req_info, 0, sizeof(struct req_quota));

    quota_info_t quota_conf[MAX_REAL_CAMERA];
    memset(&quota_conf, 0, sizeof(quota_conf));
    read_quota(SQLITE_FILE_NAME, quota_conf, &cnt);

    if (sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    req_info.enable = atoi(sValue) == 0 ? 0 : 1;

    int i;
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        cnt = 0;
        req_info.quota_conf[i].chnId = i;
        snprintf(tmp, sizeof(tmp), "video_quota[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            cnt++;
            req_info.quota_conf[i].vidQta = atoi(sValue);
        } else {
            req_info.quota_conf[i].vidQta = quota_conf[i].vidQta;
        }

        snprintf(tmp, sizeof(tmp), "picture_quota[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            cnt++;
            req_info.quota_conf[i].picQta = atoi(sValue);
        } else {
            req_info.quota_conf[i].picQta = quota_conf[i].picQta;
        }

        if (cnt > 0) {
            req_info.cnt = i + 1;
        }
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&req_info, sizeof(req_info), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_disk_set_quota(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_disk_get_group(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int disk_cnt, group_cnt;
    struct groupInfo groups[MAX_MSDK_GROUP_NUM];
    struct diskInfo disks[MAX_MSDK_NUM];
    memset(&disks, 0, sizeof(struct diskInfo)*MAX_MSDK_NUM);
    memset(&groups, 0, sizeof(struct groupInfo)*MAX_MSDK_GROUP_NUM);
    read_disks(SQLITE_FILE_NAME, disks, &disk_cnt);
    read_groups(SQLITE_FILE_NAME, groups, &group_cnt);
//    int max_chan = get_param_int(SQLITE_FILE_NAME, PARAM_MAX_CAM, DEF_MAX_CAMERA_NUM);
    int group_enable = get_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, 0);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "group_enable=%d&", group_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "group_cnt=%d&", group_cnt);

    int i = 0, j = 0;
    char group_disk[MAX_MSDK_NUM + 1] = {0};
    for (i = 0; i < MAX_MSDK_GROUP_NUM; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "group_chnMaskl[%d]=%s&", groups[i].groupid,
                 groups[i].chnMaskl);//每个盘组拥有的通道
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "group_chnMaskh[%d]=%s&", groups[i].groupid,
                 groups[i].chnMaskh);

        memset(group_disk, 0, sizeof(group_disk));
        for (j = 0; j < MAX_MSDK_NUM; j++) {
            group_disk[j] = '0';
            if (disks[j].enable && disks[j].disk_group == groups[i].groupid) {
                group_disk[j] = '1';
            }
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "group_diskMask[%d]=%s&", groups[i].groupid,
                 group_disk);//每个盘组拥有的硬盘
    }

    *(conf->len) = strlen(conf->resp) + 1;
    return ;
}

static void req_disk_set_group(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i, cnt;
    int group_enable;
    char tmp[128] = {0};
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "group_enable=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    group_enable = atoi(sValue);
    group_enable = group_enable == 0 ? 0 : 1;

    if (group_enable) {
        if (!get_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, 0)) {
            set_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, group_enable);
            sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MSFS_MODE, (void *)&group_enable, sizeof(group_enable), SDKP2P_NOT_CALLBACK);
        }

        struct req_group_info group_info;
        struct groupInfo groups[MAX_MSDK_GROUP_NUM];
        read_groups(SQLITE_FILE_NAME, groups, &cnt);

        for (i = 0; i < MAX_MSDK_GROUP_NUM; ++i) {
            cnt = 0;
            group_info.id = groups[i].groupid;
            snprintf(tmp, sizeof(tmp), "group_chnMaskl[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(group_info.chnMaskl, sizeof(group_info.chnMaskl), "%s", sValue);
            } else {
                cnt++;
                snprintf(group_info.chnMaskl, sizeof(group_info.chnMaskl), "%s", groups[i].chnMaskl);
            }

            snprintf(tmp, sizeof(tmp), "group_chnMaskh[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(group_info.chnMaskh, sizeof(group_info.chnMaskh), "%s", sValue);
            } else {
                cnt++;
                snprintf(group_info.chnMaskh, sizeof(group_info.chnMaskh), "%s", groups[i].chnMaskh);
            }

            if (cnt < 2) {
                sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MSFS_GROUP, (void *)&group_info, sizeof(group_info), SDKP2P_NOT_CALLBACK);
            }
        }
        sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RECORD_UPDATE, NULL, 0, SDKP2P_NOT_CALLBACK);
    } else {
        set_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, group_enable);
        sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MSFS_MODE, (void *)&group_enable, sizeof(group_enable), SDKP2P_NOT_CALLBACK);
    }

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

int get_raid_mode_json(void *data, int dataLen, char *resp, int respLen)
{
#if 0
    return get_common_num_json("mode", data, dataLen, resp, respLen);
#else    
    if (!data || dataLen < sizeof(int) || !resp || respLen <= 0) {
        return -1;
    }
    
    char *jsonStr = NULL;
    cJSON *json = NULL;
    int res = *(int *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
#if defined(_HI3536_) || defined(_HI3536A_)
    cJSON_AddNumberToObject(json, "support", 1);
    cJSON_AddNumberToObject(json, "mode", res);
#else
    cJSON_AddNumberToObject(json, "support", 0);
#endif

    jsonStr = cJSON_Print(json);
    cJSON_Delete(json);
    if (!jsonStr) {
        return -1;
    }
    snprintf(resp, respLen, "%s", jsonStr);
    cJSON_free(jsonStr);

    return 0;
#endif
}

int get_raid_component_size_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(unsigned long long) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    unsigned long long res = *(unsigned long long *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "size", res);

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

SDK_ERROR_CODE get_raid_req_info(const char *buff, REQ_RAID_T *info)
{
    if (!buff || !info) {
        return PARAM_ERROR;
    }
    int i = 0;
    char sValue[256] = {0};

    memset(info, 0, sizeof(REQ_RAID_T));
    info->raid_level = -1;
    if (!sdkp2p_get_section_info(buff, "level=", sValue, sizeof(sValue))) {
        info->raid_level = atoi(sValue);
    }
    if (info->raid_level != RAID_0
        && info->raid_level != RAID_1
        && info->raid_level != RAID_5
        && info->raid_level != RAID_6
        && info->raid_level != RAID_10) {
        return PARAM_ERROR;
    }

    if (!sdkp2p_get_section_info(buff, "raidPort=", sValue, sizeof(sValue))) {
        info->raid_port = atoi(sValue);
    }

    snprintf(info->raid_vendor, sizeof(info->raid_vendor), "%s", "RAID");
    if (!sdkp2p_get_section_info(buff, "name=", sValue, sizeof(sValue))) {
        snprintf(info->raid_vendor, sizeof(info->raid_vendor), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buff, "diskPort=", sValue, sizeof(sValue))) {
        for (i = 0; i < SATA_MAX && i < strlen(sValue); i++) {
            if (sValue[i] == '1') {
                info->disk_port[info->disk_num] = i + 1;
                info->disk_num++;
            }
        }
    }
    if (info->disk_num <= 0) {
        return PARAM_ERROR;
    }

    return REQUEST_SUCCESS;
}

static void req_disk_get_raid_mode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, REQUEST_FLAG_GET_RAID_MODE, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_disk_get_raid_mode(void *param, int size, char *buf, int len, int *datalen)
{
    if (get_raid_mode_json(param, size, buf, len) == 0) {
        *datalen = strlen(buf) + 1;
    } else {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
    }

    return;
}

static void req_disk_set_raid_mode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret = 0;
    int mode = 0;
    char sValue[256] = {0};
    int port = MAX_MSDK_NUM + 1;

    if (sdkp2p_get_section_info(buf, "mode=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    mode = (atoi(sValue) == 0 ? 0 : 1);

    
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RAID_MODE, &mode, sizeof(mode), SDKP2P_NOT_CALLBACK);
    if (mode == 0) {
        sdkp2p_send_msg(conf, REQUEST_FLAG_REMOVE_RAID, &port, sizeof(port), SDKP2P_NOT_CALLBACK);
    }

    *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, REQUEST_SUCCESS, &ret);

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_NETWORK_RESTART, NULL, 0, SDKP2P_NOT_CALLBACK);
}

static void req_disk_create_raid(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret;
    REQ_RAID_T info;
    memset(&info, 0, sizeof(info));

    ret = get_raid_req_info(buf, &info);
    if (ret != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_CREATE_RAID, &info, sizeof(info), SDKP2P_NEED_CALLBACK);
}

static void req_disk_rebuild_raid(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret;
    REQ_RAID_T info;
    memset(&info, 0, sizeof(info));

    ret = get_raid_req_info(buf, &info);
    if (ret != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_REBUILD_RAID, &info, sizeof(info), SDKP2P_NEED_CALLBACK);
}

static void req_disk_get_raid_component_size(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int raidPort = 0;
    char sValue[256] = {0};
    
    if (sdkp2p_get_section_info(buf, "raidPort=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    raidPort = atoi(sValue);
    
    sdkp2p_send_msg(conf, REQUEST_FLAG_GET_COMPONENT_SIZE_RAID, &raidPort, sizeof(raidPort), SDKP2P_NEED_CALLBACK);
}

static void resp_disk_get_raid_component_size(void *param, int size, char *buf, int len, int *datalen)
{
    if (get_raid_component_size_json(param, size, buf, len) == 0) {
        *datalen = strlen(buf) + 1;
    } else {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
    }
}

static void req_disk_get_health_management_data(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sResp[512] = {0};
    int port;
    if (ware_camera_get_disk_health_management_data_in(conf->reqUrl, buf, sResp, sizeof(sResp), &port)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, REQUEST_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA, &port, sizeof(port), SDKP2P_NEED_CALLBACK);
}

static void resp_disk_get_health_management_data(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA);
    ware_camera_get_disk_health_management_data_out(reqUrl, param, size/sizeof(disk_health_data_t), buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_disk_get_health_management_log(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sResp[512] = {0};
    REQ_DISKHM_LOG_S info;
    if (ware_camera_get_disk_health_management_data_in(conf->reqUrl, buf, sResp, sizeof(sResp), &info.port)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, REQUEST_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG, &info, sizeof(info), SDKP2P_NEED_CALLBACK);
}

static void resp_disk_get_health_management_log(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG);
    ware_camera_get_disk_health_management_log_out(reqUrl, param, size/sizeof(FILE_DISKHM_Info_S), buf, len);
    *datalen = strlen(buf) + 1;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_FORMAT_MSFS_DISK, req_disk_msfs_format_disk, RESPONSE_FLAG_FORMAT_MSFS_DISK, resp_disk_msfs_format_disk},//set.disk.format
    {REQUEST_FLAG_GET_MSFS_DISKINFO, NULL, RESPONSE_FLAG_GET_MSFS_DISKINFO, get_diskinfo_msfs_to_str},//get.disk.storage_info
    {REQUEST_FLAG_UPDATE_DISKMODE, req_disk_msfs_update_diskmode, -1, NULL},//set.disk.storage_info
    {REQUEST_FLAG_REMOVE_RAID, req_disk_remove_raid, RESPONSE_FLAG_REMOVE_RAID, resp_disk_remove_raid},//set.disk.delete
    {REQUEST_FLAG_DEL_MSFS_NAS_REMOTE, req_disk_remove_nas, RESPONSE_FLAG_DEL_MSFS_NAS_REMOTE, resp_disk_remove_nas},//set.disk.delete & set.disk.del_nas
    {REQUEST_FLAG_DEL_MSFS_LOCAL, req_disk_remove_local, RESPONSE_FLAG_DEL_MSFS_LOCAL, resp_disk_remove_local},//set.disk.delete
    {REQUEST_FLAG_SET_MSFS_PORT, req_disk_set_port, -1, NULL},//set.disk.property
    {REQUEST_FLAG_GET_REC_ADVANCED, NULL, RESPONSE_FLAG_GET_REC_ADVANCED, resp_disk_get_rec_advanced},//get.disk.recycle_mode
    {REQUEST_FLAG_SET_REC_RECYCLEMODE, req_disk_set_rec_advanced, -1, NULL},//set.disk.recycle_mode
    {REQUEST_FLAG_SEARCH_MSFS_NAS, req_disk_get_nas_search, RESPONSE_FLAG_SEARCH_MSFS_NAS, resp_disk_get_nas_search},//set.disk.search_nas
    {REQUEST_FLAG_ADD_MSFS_NAS_REMOTE, req_disk_add_nas, RESPONSE_FLAG_ADD_MSFS_NAS_REMOTE, resp_disk_add_nas},//set.disk.add_nas
    {REQUEST_FLAG_UPDATE_MSFS_NAS, req_disk_edit_nas, RESPONSE_FLAG_UPDATE_MSFS_NAS, resp_disk_edit_nas},//set.disk.edit_nas

    {REQUEST_FLAG_GET_SMART_ATTR, NULL, RESPONSE_FLAG_GET_SMART_ATTR, get_smart_attr_to_str},
    {REQUEST_FLAG_GET_SMART_PROCESS, NULL, RESPONSE_FLAG_GET_SMART_PROCESS, get_smart_process_to_str},
    {REQUEST_FLAG_GET_MSFS_QUOTA, NULL, RESPONSE_FLAG_GET_MSFS_QUOTA, resp_disk_get_quota},//get.disk.quota
    {REQUEST_FLAG_SET_MSFS_QUOTA, req_disk_set_quota, RESPONSE_FLAG_SET_MSFS_QUOTA, resp_disk_set_quota},//set.disk.quota
    {REQUEST_FLAG_GET_MSFS_GROUP, req_disk_get_group, -1, NULL},//get.disk.group
    {REQUEST_FLAG_SET_MSFS_GROUP, req_disk_set_group, -1, NULL},//set.disk.group
    {REQUEST_FLAG_GET_RAID_MODE, req_disk_get_raid_mode, RESPONSE_FLAG_GET_RAID_MODE, resp_disk_get_raid_mode}, // get.disk.raid.mode
    {REQUEST_FLAG_SET_RAID_MODE, req_disk_set_raid_mode, -1, NULL}, // set.disk.raid.mode
    {REQUEST_FLAG_CREATE_RAID, req_disk_create_raid, RESPONSE_FLAG_CREATE_RAID, sdkp2p_resp_common_set}, // create.disk.raid
    {REQUEST_FLAG_REBUILD_RAID, req_disk_rebuild_raid, RESPONSE_FLAG_REBUILD_RAID, sdkp2p_resp_common_set}, // rebuild.disk.raid
    {REQUEST_FLAG_GET_COMPONENT_SIZE_RAID, req_disk_get_raid_component_size, RESPONSE_FLAG_GET_COMPONENT_SIZE_RAID, resp_disk_get_raid_component_size}, // get.disk.raid.component.size
    {REQUEST_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA, req_disk_get_health_management_data, RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_DATA, resp_disk_get_health_management_data},
    {REQUEST_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG, req_disk_get_health_management_log, RESPONSE_FLAG_GET_DISK_HEALTH_MANAGEMENT_LOG, resp_disk_get_health_management_log},
    // request register array end
};

void p2p_disk_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_disk_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

