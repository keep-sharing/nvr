#include "MsDisk.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "mswizard.h"

MsDisk::MsDisk(QObject *parent)
    : MsObject(parent)
{
    qMsDebug();
}

MsDisk::~MsDisk()
{
    qMsDebug();
}

MsDisk &MsDisk::instance()
{
    static MsDisk self;
    return self;
}

QMap<int, BaseDiskInfo> MsDisk::diskInfoMap() const
{
    return m_diskInfoMap;
}

void MsDisk::getDiskInfo()
{
    sendMessage(REQUEST_FLAG_GET_MSFS_DISKINFO, nullptr, 0);
}

void MsDisk::removeNas(int port)
{
    sendMessage(REQUEST_FLAG_DEL_MSFS_NAS, (void *)&port, sizeof(int));

    removeDatabaseDisk(port);
}

int MsDisk::validNetworkDiskPort()
{
    QMap<int, int> usedMap;
    for (auto iter = m_diskInfoMap.constBegin(); iter != m_diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        switch (disk.typeValue()) {
        case DISK_TYPE_NAS:
        case DISK_TYPE_CIFS:
            usedMap.insert(disk.port(), 0);
            break;
        default:
            break;
        }
    }
    for (int port = 17; port <= 24; ++port) {
        if (!usedMap.contains(port)) {
            return port;
        }
    }
    return -1;
}

void MsDisk::removeDisk(int port)
{
    sendMessage(REQUEST_FLAG_DEL_MSFS_LOCAL, (void *)&port, sizeof(int));

    removeDatabaseDisk(port);
}

int MsDisk::validRaidPort()
{
    int port = 26;
    for (auto iter = m_diskInfoMap.constBegin(); iter != m_diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        if (disk.typeValue() == DISK_TYPE_RAID && disk.port() == port) {
            port++;
        }
    }
    return port;
}

void MsDisk::removeRaid(int port)
{
    sendMessage(REQUEST_FLAG_REMOVE_RAID, (void *)&port, sizeof(int));
    removeDatabaseDisk(port);
}

void MsDisk::removeAllRaid()
{
    QList<int> portList;
    for (auto iter = m_diskInfoMap.constBegin(); iter != m_diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &info = iter.value();
        if (info.typeValue() == DISK_TYPE_RAID) {
            portList.append(info.port());
        }
    }
    for (int i = 0; i < portList.size(); ++i) {
        int port = portList.at(i);
        removeRaid(port);
    }
}

bool MsDisk::addFormatPort(int port)
{
    if (m_formatList.contains(port)) {
        return false;
    } else {
        m_formatList.append(port);
        return true;
    }
}

void MsDisk::clearFormatPort()
{
    m_formatList.clear();
}

void MsDisk::startFormat()
{
    if (m_isFormatting) {
        return;
    }
    m_isFormatting = true;
}

bool MsDisk::hasUnformatDisk()
{
    bool unformat = false;
    for (auto iter = m_diskInfoMap.constBegin(); iter != m_diskInfoMap.constEnd(); ++iter) {
        const BaseDiskInfo &disk = iter.value();
        switch (disk.typeValue()) {
        case DISK_TYPE_GLOBAL_SPARE:
        case DISK_TYPE_IN_RAID:
        case DISK_TYPE_USB:
            continue;
        case DISK_TYPE_ESATA:
            if (!disk.is_sata_storage) {
                continue;
            }
            break;
        default:
            break;
        }
        if (disk.statusValue() == DISK_STATE_UNFORMATTED) {
            unformat = true;
            break;
        }
    }
    return unformat;
}

void MsDisk::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    switch (message->type()) {
    case RESPONSE_FLAG_PROGRESS_DISK_INIT:
        ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(message);
        break;
    case RESPONSE_FLAG_PROGRESS_DISK_LOAD:
        ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(message);
        break;
    case RESPONSE_FLAG_PROGRESS_RAID_REBUILD:
        ON_RESPONSE_FLAG_PROGRESS_RAID_REBUILD(message);
        break;
    }
}

void MsDisk::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_MSFS_DISKINFO:
        ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(message);
        break;
    case RESPONSE_FLAG_FORMAT_MSFS_DISK:
        ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(message);
        break;
    case RESPONSE_FLAG_REMOVE_RAID:
        ON_RESPONSE_FLAG_REMOVE_RAID(message);
        break;
    }
}

void MsDisk::ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message)
{
    //读数据库中记录的硬盘信息
    m_dbDiskInfoMap.clear();
    m_dbDiskVendorMap.clear();
    for (auto iter = m_diskInfoMap.begin(); iter != m_diskInfoMap.end(); ++iter) {
        BaseDiskInfo &info = iter.value();
        info.setReadyUpdate();
    }
    //
    qDebug() << QString("----begin read_disks----");
    int dbCount = 0;
    diskInfo db_disk_array[MAX_DISK_NUM];
    memset(db_disk_array, 0, sizeof(db_disk_array));
    read_disks(SQLITE_FILE_NAME, db_disk_array, &dbCount);
    for (int i = 0; i < dbCount; ++i) {
        const diskInfo &disk = db_disk_array[i];
        if (!QString(disk.disk_vendor).isEmpty()) {
            //直接insert会导致进度信息丢失
            BaseDiskInfo &info = m_diskInfoMap[disk.disk_port];
            info.updateDiskInfo(disk);
            //
            m_dbDiskInfoMap.insert(disk.disk_port, info);
            m_dbDiskVendorMap.insert(QString(disk.disk_vendor), disk.disk_port);
            qDebug() << QString("index: %1, port: %2, vendor: %3, type: %4, address: %5, directory: %6")
                        .arg(i)
                        .arg(disk.disk_port)
                        .arg(QString(disk.disk_vendor).trimmed())
                        .arg(disk.disk_type)
                        .arg(disk.disk_address)
                        .arg(disk.disk_directory);
        }
    }
    qDebug() << QString("----end read_disks----");
    //eSATA Mode
    BaseDiskInfo::is_sata_storage = (get_param_int(SQLITE_FILE_NAME, PARAM_ESATA_TYPE, 0) == 0);
    //Group
    BaseDiskInfo::is_group_enable = (get_param_int(SQLITE_FILE_NAME, PARAM_DISK_MODE, 0) == 1);
    //中心传回来的硬盘信息
    struct resp_get_msfs_diskinfo *msfs_diskinfo_array = (struct resp_get_msfs_diskinfo *)message->data;
    int disk_count = message->header.size / sizeof(struct resp_get_msfs_diskinfo);
    qDebug() << QString("----begin RESPONSE_FLAG_GET_MSFS_DISKINFO, count: %1----").arg(disk_count);
    for (int i = 0; i < disk_count; ++i) {
        const resp_get_msfs_diskinfo &msfs_diskinfo = msfs_diskinfo_array[i];
        BaseDiskInfo &info = m_diskInfoMap[msfs_diskinfo.disk_port];
        info.updateDiskInfo(msfs_diskinfo);
        qDebug() << QString("index: %1, port: %2, vendor: %3, status: %4, type: %5, private: %6, address: %7, directory: %8")
                    .arg(i)
                    .arg(msfs_diskinfo.disk_port)
                    .arg(QString(msfs_diskinfo.disk_vendor).trimmed())
                    .arg(msfs_diskinfo.status)
                    .arg(msfs_diskinfo.disk_type)
                    .arg(msfs_diskinfo.disk_private)
                    .arg(msfs_diskinfo.disk_address)
                    .arg(msfs_diskinfo.disk_directory);
        //
        dealNewDisk(msfs_diskinfo);
    }
    qDebug() << QString("----end RESPONSE_FLAG_GET_MSFS_DISKINFO----");
    //
    for (auto iter = m_diskInfoMap.begin(); iter != m_diskInfoMap.end();) {
        BaseDiskInfo &info = iter.value();
        if (info.isUpdated()) {
            iter++;
        } else {
            iter = m_diskInfoMap.erase(iter);
        }
    }
    //
    static bool isFirst = true;
    if (isFirst && hasUnformatDisk()) {
        //启动向导也会获取硬盘信息，这里不提示
        if (!MsWizard::instance()->isVisible()) {
            isFirst = false;
            emit detectedUninitializedDisk();
        }
    }
}

void MsDisk::ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message)
{
    int result = 0;
    if (message->data) {
        result = (*(int *)(message->data));
    }
    switch (result) {
    case -1:
        break;
    case -3:
        break;
    default:
        break;
    }
    if (!m_formatList.isEmpty()) {

    } else {
        m_isFormatting = false;
    }
}

void MsDisk::ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "MsDisk::ON_RESPONSE_FLAG_PROGRESS_DISK_INIT, data is null.";
        return;
    }
    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    qDebug() << QString("RESPONSE_FLAG_PROGRESS_DISK_INIT, port: %1, percent: %2%").arg(progressinfo->port).arg(progressinfo->percent);
    if (m_diskInfoMap.contains(progressinfo->port)) {
        m_diskInfoMap[progressinfo->port].setFormatProgress(progressinfo->percent);
    }
    //
    if (progressinfo->percent >= 100) {
        getDiskInfo();
    }
}

void MsDisk::ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "MsDisk::ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD, data is null.";
        return;
    }
    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    qDebug() << QString("RESPONSE_FLAG_PROGRESS_DISK_LOAD, port: %1, percent: %2%").arg(progressinfo->port).arg(progressinfo->percent);
    if (m_diskInfoMap.contains(progressinfo->port)) {
        m_diskInfoMap[progressinfo->port].setLoadProgress(progressinfo->percent);
    }
    //
    if (progressinfo->percent == 0) {
        getDiskInfo();
    }
    if (progressinfo->percent >= 100) {
        getDiskInfo();
    }
}

void MsDisk::ON_RESPONSE_FLAG_REMOVE_RAID(MessageReceive *message)
{
    int result = 0;
    if (message->data) {
        result = *(int *)message->data;
    }
    switch (result) {
    case -1:
        break;
    default:
        break;
    }
}

void MsDisk::ON_RESPONSE_FLAG_PROGRESS_RAID_REBUILD(MessageReceive *message)
{
    PROGRESS_BAR_T *progressinfo = (PROGRESS_BAR_T *)message->data;
    if (!progressinfo) {
        return;
    }
    qDebug() << QString("RESPONSE_FLAG_PROGRESS_RAID_REBUILD, port: %1, percent: %2%").arg(progressinfo->port).arg(progressinfo->percent);
    if (m_diskInfoMap.contains(progressinfo->port)) {
        m_diskInfoMap[progressinfo->port].setRebulidProgress(progressinfo->percent);
    }
    //
    if (progressinfo->percent == 0) {
        getDiskInfo();
    }
    if (progressinfo->percent >= 100) {
        getDiskInfo();
    }
}

void MsDisk::dealNewDisk(const resp_get_msfs_diskinfo &diskinfo)
{
    bool isValidType = false;
    switch (diskinfo.disk_type) {
    case DISK_TYPE_LOCAL:
    case DISK_TYPE_RAID:
    case DISK_TYPE_IN_RAID:
    case DISK_TYPE_GLOBAL_SPARE:
        isValidType = true;
        break;
    case DISK_TYPE_ESATA:
        if (BaseDiskInfo::is_sata_storage) {
            isValidType = true;
        }
        break;
    default:
        break;
    }
    //
    if (isValidType) {
        if (m_dbDiskVendorMap.contains(QString(diskinfo.disk_vendor))) {
            int port = m_dbDiskVendorMap.value(diskinfo.disk_vendor);
            if (port != diskinfo.disk_port) {
                addDatabaseDisk(diskinfo);
            }
        } else {
            addDatabaseDisk(diskinfo);
        }
    }
}

void MsDisk::removeDatabaseDisk(int port)
{
    struct diskInfo disk;
    memset(&disk, 0, sizeof(struct diskInfo));
    disk.disk_port = port;
    disk.disk_property = 1;
    write_disk(SQLITE_FILE_NAME, &disk);

    m_diskInfoMap.remove(port);
}

void MsDisk::addDatabaseDisk(const resp_get_msfs_diskinfo &diskinfo)
{
    diskInfo new_disk;
    memset(&new_disk, 0, sizeof(new_disk));
    new_disk.disk_port = diskinfo.disk_port;
    new_disk.enable = 1;
    snprintf(new_disk.disk_vendor, sizeof(new_disk.disk_vendor), "%s", diskinfo.disk_vendor);
    new_disk.disk_type = diskinfo.disk_type;
    new_disk.disk_group = diskinfo.disk_group;
    new_disk.disk_property = diskinfo.disk_property;
    new_disk.raid_level = diskinfo.raid_level;
    write_disk(SQLITE_FILE_NAME, &new_disk);
}
