#include "basediskinfo.h"

#include "MsLanguage.h"

#include "MyDebug.h"
#include <QLocale>
#include <QtDebug>

bool BaseDiskInfo::is_sata_storage = false;
bool BaseDiskInfo::is_group_enable = false;

BaseDiskInfo::BaseDiskInfo()
{
}

BaseDiskInfo::BaseDiskInfo(const resp_get_msfs_diskinfo &diskinfo)
{
    m_diskPort = diskinfo.disk_port;
    m_diskVendor = QString(diskinfo.disk_vendor).trimmed();
    m_diskAddress = QString(diskinfo.disk_address).trimmed();
    m_diskDirectory = QString(diskinfo.disk_directory).trimmed();
    m_diskStaus = static_cast<STATE_EN>(diskinfo.status);
    m_busable = diskinfo.busable;
    m_totalBytes = diskinfo.total;
    m_usedBytes = diskinfo.used;
    m_freeBytes = diskinfo.free;
    m_diskType = static_cast<TYPE_EN>(diskinfo.disk_type);
    m_diskGroup = diskinfo.disk_group;
    m_diskProperty = diskinfo.disk_property;
    m_diskPrivate = diskinfo.disk_private;
    m_raidLevel = diskinfo.raid_level;
    m_raidStatus = static_cast<MD_STATE_EN>(diskinfo.raid_status);
    m_raidTask = diskinfo.raid_task;
    m_smartTest = diskinfo.smartTest;
    m_raidList.clear();
    for (int i = 0; i < diskinfo.raid_diskcnt; ++i) {
        m_raidList.append(diskinfo.raid_disk[i]);
    }
    qSort(m_raidList.begin(), m_raidList.end());
}

BaseDiskInfo::BaseDiskInfo(const diskInfo &diskinfo)
{
    m_diskPort = diskinfo.disk_port;
    m_diskVendor = QString(diskinfo.disk_vendor).trimmed();
    m_diskAddress = QString(diskinfo.disk_address).trimmed();
    m_diskDirectory = QString(diskinfo.disk_directory).trimmed();
    m_diskType = static_cast<TYPE_EN>(diskinfo.disk_type);
    m_diskGroup = diskinfo.disk_group;
    m_diskProperty = diskinfo.disk_property;
    m_raidLevel = diskinfo.raid_level;
    m_enable = diskinfo.enable;
    m_userName = QString(diskinfo.user);
    m_password = QString(diskinfo.password);
}

void BaseDiskInfo::updateDiskInfo(const resp_get_msfs_diskinfo &diskinfo)
{
    resp_get_msfs_diskinfo &test = const_cast<resp_get_msfs_diskinfo &>(diskinfo);
    STRUCT(resp_get_msfs_diskinfo, &test,
           FIELD(int, disk_port);
           FIELD(int, disk_type);
           FIELD(int, status);
           FIELD(int, smartTest));
    m_diskPort = diskinfo.disk_port;
    m_diskVendor = QString(diskinfo.disk_vendor).trimmed();
    m_diskAddress = QString(diskinfo.disk_address).trimmed();
    m_diskDirectory = QString(diskinfo.disk_directory).trimmed();
    m_diskStaus = static_cast<STATE_EN>(diskinfo.status);
    m_busable = diskinfo.busable;
    m_totalBytes = diskinfo.total;
    m_usedBytes = diskinfo.used;
    m_freeBytes = diskinfo.free;
    m_diskType = static_cast<TYPE_EN>(diskinfo.disk_type);
    m_diskGroup = diskinfo.disk_group;
    m_diskProperty = diskinfo.disk_property;
    m_diskPrivate = diskinfo.disk_private;
    m_raidLevel = diskinfo.raid_level;
    m_raidStatus = static_cast<MD_STATE_EN>(diskinfo.raid_status);
    m_raidTask = diskinfo.raid_task;
    m_smartTest = diskinfo.smartTest;
    m_raidList.clear();
    for (int i = 0; i < diskinfo.raid_diskcnt; ++i) {
        m_raidList.append(diskinfo.raid_disk[i]);
    }
    qSort(m_raidList.begin(), m_raidList.end());

    setUpdated();
}

void BaseDiskInfo::updateDiskInfo(const diskInfo &diskinfo)
{
    m_diskPort = diskinfo.disk_port;
    m_diskVendor = QString(diskinfo.disk_vendor).trimmed();
    m_diskAddress = QString(diskinfo.disk_address).trimmed();
    m_diskDirectory = QString(diskinfo.disk_directory).trimmed();
    m_diskStaus = DISK_STATE_OFFLINE;
    m_busable = 0;
    m_totalBytes = 0;
    m_usedBytes = 0;
    m_freeBytes = 0;
    m_diskType = static_cast<TYPE_EN>(diskinfo.disk_type);
    m_diskGroup = diskinfo.disk_group;
    m_diskProperty = diskinfo.disk_property;
    m_raidLevel = diskinfo.raid_level;
    m_enable = diskinfo.enable;
    m_raidStatus = RAID_STATE_NONE;

    setUpdated();
}

void BaseDiskInfo::setReadyUpdate()
{
    m_isUpdated = false;
}

void BaseDiskInfo::setUpdated()
{
    m_isUpdated = true;
}

bool BaseDiskInfo::isUpdated() const
{
    return m_isUpdated;
}

void BaseDiskInfo::saveToDatebase()
{
    diskInfo disk;
    memset(&disk, 0, sizeof(diskInfo));
    disk.disk_port = m_diskPort;
    disk.enable = 1;
    snprintf(disk.disk_address, sizeof(disk.disk_address), "%s", m_diskAddress.toStdString().c_str());
    snprintf(disk.disk_directory, sizeof(disk.disk_directory), "%s", m_diskAddress.toStdString().c_str());
    snprintf(disk.disk_vendor, sizeof(disk.disk_vendor), "%s", m_diskAddress.toStdString().c_str());
    disk.disk_type = m_diskType;
    disk.disk_group = m_diskGroup;
    disk.disk_property = m_diskProperty;
    disk.raid_level = m_raidLevel;
    write_disk(SQLITE_FILE_NAME, &disk);
}

bool BaseDiskInfo::isShowInDiskWidget() const
{
    bool isShow = false;
    switch (m_diskType) {
    case DISK_TYPE_LOCAL:
    case DISK_TYPE_NAS:
    case DISK_TYPE_CIFS:
    case DISK_TYPE_RAID:
    case DISK_TYPE_IPSAN:
        isShow = true;
        break;
    case DISK_TYPE_ESATA:
        if (is_sata_storage) {
            isShow = true;
        }
        break;
    default:
        break;
    }

    return isShow;
}

bool BaseDiskInfo::isCheckable() const
{
    bool checkable = false;
    switch (m_diskStaus) {
    case DISK_STATE_UNFORMATTED:
    case DISK_STATE_NORMAL:
        checkable = true;
        break;
    default:
        break;
    }
    return checkable;
}

bool BaseDiskInfo::isShowInRaidWidget() const
{
    bool isShow = true;

    if (isReadOnly()) {
        isShow = false;
    }

    switch (m_diskType) {
    case DISK_TYPE_LOCAL:
    case DISK_TYPE_IN_RAID:
    case DISK_TYPE_GLOBAL_SPARE:
        break;
    default:
        isShow = false;
        break;
    }

    return isShow;
}

bool BaseDiskInfo::isEnableInRaidWidget() const
{
    bool enable = true;
    if (m_diskType == DISK_TYPE_LOCAL) {
        switch (m_diskStaus) {
        case DISK_STATE_NORMAL:
        case DISK_STATE_UNFORMATTED:
            break;
        default:
            enable = false;
            break;
        }
    } else {
        enable = false;
    }
    //
    if (!m_busable) {
        enable = false;
    }
    //
    return enable;
}

bool BaseDiskInfo::isCheckableInRaidWidget() const
{
    bool checkable = false;
    if (hotSpareStatus() != HotSpareOn) {
        checkable = true;
    }
    if (m_loadProgress >= 0) {
        checkable = false;
    }
    if (m_diskType == DISK_TYPE_IN_RAID) {
        checkable = false;
    }
    if (statusValue() == DISK_STATE_OFFLINE || statusValue() == DISK_STATE_BAD) {
        checkable = false;
    }
    return checkable;
}

HotSpareStatus BaseDiskInfo::hotSpareStatus() const
{
    HotSpareStatus status = HotSpareDisable;
    switch (m_diskType) {
    case DISK_TYPE_LOCAL:
        status = HotSpareOff;
        break;
    case DISK_TYPE_GLOBAL_SPARE:
        status = HotSpareOn;
        break;
    default:
        break;
    }
    //
    switch (m_diskStaus) {
    case DISK_STATE_BAD:
    case DISK_STATE_OFFLINE:
        status = HotSpareDisable;
        break;
    default:
        break;
    }

    return status;
}

int BaseDiskInfo::port() const
{
    return m_diskPort;
}

QString BaseDiskInfo::vendorString() const
{
    return m_diskVendor;
}

QString BaseDiskInfo::address() const
{
    return m_diskAddress;
}

QString BaseDiskInfo::directory() const
{
    return m_diskDirectory;
}

QString BaseDiskInfo::statusString() const
{
    QString strState;
    if (m_diskType == DISK_TYPE_RAID) {
        if ((m_diskStaus == DISK_STATE_OFFLINE) || (m_raidStatus == RAID_STATE_OFFLINE)) {
            strState = GET_TEXT("DISKMANAGE/72091", "Offline");
        } else if (m_diskStaus == DISK_STATE_UNFORMATTED) {
            strState = GET_TEXT("DISKMANAGE/72100", "Uninitialized");
        } else if ((m_diskStaus == DISK_STATE_NORMAL) && (m_raidStatus == RAID_STATE_ACTIVE)) {
            strState = SmartStatusString();
        } else if (m_raidStatus == RAID_STATE_DEGRADE) {
            strState = GET_TEXT("DISKMANAGE/72093", "Degraded");
        } else if (m_raidStatus == RAID_STATE_RECOVERY) {
            strState = GET_TEXT("DISKMANAGE/156002", "Recovery");
        }
    } else if (m_diskType == DISK_TYPE_IN_RAID) {
        if (m_busable) {
            strState = SmartStatusString();
        } else {
            strState = GET_TEXT("DISKMANAGE/72098", "Error");
        }
    } else {
        switch (m_diskStaus) {
        case DISK_STATE_UNFORMATTED:
            strState = GET_TEXT("DISKMANAGE/72100", "Uninitialized");
            break;
        case DISK_STATE_NORMAL:
            strState = SmartStatusString();
            break;
        case DISK_STATE_OFFLINE:
            strState = GET_TEXT("DISKMANAGE/72091", "Offline");
            break;
        case DISK_STATE_FORMATING:
            strState = "Formating";
            break;
        case DISK_STATE_BAD:
            strState = GET_TEXT("DISKMANAGE/72098", "Error");
            break;
        case DISK_STATE_NONE:
            strState = GET_TEXT("DISKMANAGE/156001", "Unknown");
            break;
        default:
            break;
        }
    }
    //
    if (m_formatProgress >= 0) {
        strState = QString("%1: %2%").arg(GET_TEXT("DISK/92036", "Initializing")).arg(m_formatProgress);
    }
    if (m_loadProgress >= 0) {
        strState = QString("%1: %2%").arg(GET_TEXT("DISKMANAGE/72112", "Loading")).arg(m_loadProgress);
    }
    //
    if (strState.isEmpty()) {
        strState = GET_TEXT("DISKMANAGE/156001", "Unknown");
    }
    return strState;
}

STATE_EN BaseDiskInfo::statusValue() const
{
    return m_diskStaus;
}

quint64 BaseDiskInfo::totalBytes() const
{
    return m_totalBytes;
}

QString BaseDiskInfo::totalBytesString() const
{
    return bytesString(m_totalBytes);
}

quint64 BaseDiskInfo::freeBytes() const
{
    return m_freeBytes;
}

void BaseDiskInfo::setPropertyValue(int property)
{
    m_diskProperty = property;
}

int BaseDiskInfo::propertyValue() const
{
    return m_diskProperty;
}

QString BaseDiskInfo::propertyString() const
{
    QString text;
    switch (m_diskProperty) {
    case 0:
        text = GET_TEXT("DISKMANAGE/72108", "Read-only");
        break;
    case 1:
        text = GET_TEXT("DISKMANAGE/72107", "R/W");
        break;
    default:
        break;
    }
    return text;
}

int BaseDiskInfo::privateValue() const
{
    return m_diskPrivate;
}

bool BaseDiskInfo::isReadOnly() const
{
    return (m_diskProperty == 0);
}

QString BaseDiskInfo::typeString() const
{
    QString strType = typeToString(m_diskType);
    if (m_diskType == DISK_TYPE_RAID) {
        strType.append(QString::number(m_raidLevel));
    }
    return strType;
}

TYPE_EN BaseDiskInfo::typeValue() const
{
    return static_cast<TYPE_EN>(m_diskType);
}

void BaseDiskInfo::setGroupValue(int group)
{
    m_diskGroup = group;
}

int BaseDiskInfo::groupValue() const
{
    return m_diskGroup;
}

QString BaseDiskInfo::groupString() const
{
    return QString("%1").arg(m_diskGroup + 1);
}

bool BaseDiskInfo::isEditEnable() const
{
    bool enable = false;
    switch (m_diskType) {
    case DISK_TYPE_RAID:
        if (m_raidStatus == RAID_STATE_DEGRADE || m_raidStatus == RAID_STATE_RECOVERY) {
            enable = true;
        } else if (m_diskStaus == DISK_STATE_NORMAL) {
            enable = true;
        }
        break;
    case DISK_TYPE_NAS:
    case DISK_TYPE_CIFS:
        if (m_diskStaus == DISK_STATE_NORMAL || m_diskStaus == DISK_STATE_UNFORMATTED) {
            enable = true;
        }
        break;
    case DISK_TYPE_LOCAL:
    case DISK_TYPE_ESATA:
        if (m_diskStaus == DISK_STATE_NORMAL) {
            enable = true;
        }
        break;
    default:
        break;
    }
    return enable;
}

bool BaseDiskInfo::isDeleteEnable() const
{
    bool enable = false;
    switch (m_diskType) {
    case DISK_TYPE_RAID:
        if (m_raidStatus == RAID_STATE_DEGRADE || m_raidStatus == RAID_STATE_RECOVERY) {
            enable = true;
        } else {
            if (m_diskStaus == DISK_STATE_OFFLINE) {
                enable = true;
            }
        }
        break;
    case DISK_TYPE_NAS:
    case DISK_TYPE_CIFS:
        if (m_diskStaus == DISK_STATE_OFFLINE || m_diskStaus == DISK_STATE_NORMAL || m_diskStaus == DISK_STATE_UNFORMATTED) {
            enable = true;
        }
        break;
    case DISK_TYPE_ESATA:
    case DISK_TYPE_LOCAL:
        if (m_diskStaus == DISK_STATE_OFFLINE) {
            enable = true;
        }
        break;
    default:
        break;
    }
    return enable;
}

QString BaseDiskInfo::userName() const
{
    return m_userName;
}

QString BaseDiskInfo::password() const
{
    return m_password;
}

QString BaseDiskInfo::raidListString() const
{
    QString str;
    for (int i = 0; i < m_raidList.size(); ++i) {
        if (m_raidList.at(i) > 0) {
            str.append(QString("%1,").arg(m_raidList.at(i)));
        }
    }
    if (!str.isEmpty()) {
        str.chop(1);
    }
    return str;
}

QString BaseDiskInfo::raidStatusString() const
{
    QString str;
    switch (m_raidStatus) {
    case RAID_STATE_NOEXIST:
        str = QString("No Exist");
        break;
    case RAID_STATE_OFFLINE:
        str = GET_TEXT("DISKMANAGE/72091", "Offline");
        break;
    case RAID_STATE_ACTIVE:
        str = SmartStatusString();
        break;
    case RAID_STATE_DEGRADE:
        str = GET_TEXT("DISKMANAGE/72093", "Degraded");
        break;
    case RAID_STATE_RESYNC:
        str = QString("Resync");
        break;
    case RAID_STATE_RECOVERY:
        str = GET_TEXT("DISKMANAGE/156002", "Recovery");
        break;
    case RAID_STATE_NOINIT:
        str = GET_TEXT("DISKMANAGE/72100", "Uninitialized");
        break;
    case RAID_STATE_BUSY:
        str = QString("Busy");
        break;
    case RAID_STATE_FAIL:
        break;
    default:
        str = GET_TEXT("DISKMANAGE/72091", "Offline");
        break;
    }
    return str;
}

MD_STATE_EN BaseDiskInfo::raidStatusValue() const
{
    return m_raidStatus;
}

int BaseDiskInfo::raidLevel() const
{
    return m_raidLevel;
}

QString BaseDiskInfo::raidLevelString() const
{
    QString str = QString("RAID%1").arg(m_raidLevel);
    return str;
}

void BaseDiskInfo::setFormatProgress(int value)
{
    m_formatProgress = value;
    if (value >= 100) {
        m_formatProgress = -1;
    }
}

int BaseDiskInfo::formatProgress() const
{
    return m_formatProgress;
}

void BaseDiskInfo::setLoadProgress(int value)
{
    m_loadProgress = value;
    if (value >= 100) {
        m_loadProgress = -1;
    }
}

int BaseDiskInfo::loadProgress() const
{
    return m_loadProgress;
}

void BaseDiskInfo::setRebulidProgress(int value)
{
    m_rebulidProgress = value;
    if (value >= 100) {
        m_rebulidProgress = -1;
    }
}

int BaseDiskInfo::rebulidProgress() const
{
    return m_rebulidProgress;
}

QString BaseDiskInfo::bytesString(quint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;
    if (bytes == 0) {
        return QString("0");
    }
    if (bytes >= tb) {
        return QString("%1 TB").arg(QLocale().toString(qreal(bytes) / tb - 0.0004, 'f', 3));
    } else {
        return QString("%1 GB").arg(QLocale().toString(qreal(bytes) / gb - 0.004, 'f', 2));
    }
}

QString BaseDiskInfo::SmartStatusString() const
{
    if (m_smartTest == SMART_WARN) {
        return GET_TEXT("DISKMANAGE/156000", "Warning");
    } else {
        return GET_TEXT("DISKMANAGE/72099", "Normal");
    }
}

QString BaseDiskInfo::typeToString(int type) const
{
    QString text;
    switch (type) {
    case DISK_TYPE_UNKNOWN:
        text = QString("Unknown");
        break;
    case DISK_TYPE_LOCAL:
        text = GET_TEXT("DISKMANAGE/72055", "Local");
        break;
    case DISK_TYPE_USB:
        text = QString("USB");
        break;
    case DISK_TYPE_NAS:
        text = QString("NFS");
        break;
    case DISK_TYPE_CIFS:
        text = QString("SMB/CIFS");
        break;
    case DISK_TYPE_RAID:
        text = QString("RAID");
        break;
    case DISK_TYPE_ESATA:
        text = QString("eSATA");
        break;
    case DISK_TYPE_IPSAN:
        text = QString("IPSAN");
        break;
    case DISK_TYPE_GLOBAL_SPARE:
        text = QString("Hot Spare");
        break;
    case DISK_TYPE_IN_RAID:
        text = QString("Array");
        break;
    case DISK_TYPE_NONE:
        text = QString("None");
        break;
    default:
        break;
    }
    return text;
}
