#ifndef BASEDISKINFO_H
#define BASEDISKINFO_H

#include <QMetaType>

extern "C" {
#include "msdb.h"
#include "msg.h"
}

enum HotSpareStatus {
    HotSpareDisable,
    HotSpareOn,
    HotSpareOff
};

class BaseDiskInfo {
public:
    BaseDiskInfo();
    BaseDiskInfo(const resp_get_msfs_diskinfo &diskinfo);
    BaseDiskInfo(const diskInfo &diskinfo);

    void updateDiskInfo(const resp_get_msfs_diskinfo &diskinfo);
    void updateDiskInfo(const diskInfo &diskinfo);

    void setReadyUpdate();
    void setUpdated();
    bool isUpdated() const;

    void saveToDatebase();

    bool isShowInDiskWidget() const;
    bool isCheckable() const;

    //raid
    bool isShowInRaidWidget() const;
    bool isEnableInRaidWidget() const;
    bool isCheckableInRaidWidget() const;
    HotSpareStatus hotSpareStatus() const;

    int port() const;
    QString vendorString() const;
    QString address() const;
    QString directory() const;
    QString statusString() const;
    STATE_EN statusValue() const;
    quint64 totalBytes() const;
    QString totalBytesString() const;
    quint64 freeBytes() const;
    void setPropertyValue(int property);
    int propertyValue() const;
    QString propertyString() const;
    int privateValue() const;
    bool isReadOnly() const;
    QString typeString() const;
    TYPE_EN typeValue() const;
    void setGroupValue(int group);
    int groupValue() const;
    QString groupString() const;
    bool isEditEnable() const;
    bool isDeleteEnable() const;
    QString userName() const;
    QString password() const;

    QString raidListString() const;
    QString raidStatusString() const;
    MD_STATE_EN raidStatusValue() const;
    int raidLevel() const;
    QString raidLevelString() const;

    void setFormatProgress(int value);
    int formatProgress() const;
    void setLoadProgress(int value);
    int loadProgress() const;
    void setRebulidProgress(int value);
    int rebulidProgress() const;

    static QString bytesString(quint64 bytes);

    QString SmartStatusString() const;

    //eSATA是否作为存储盘（当做普通硬盘）
    static bool is_sata_storage;
    //是否开启了Group
    static bool is_group_enable;

protected:
    QString typeToString(int type) const;

protected:
    int m_formatProgress = -1;
    int m_loadProgress = -1;
    int m_rebulidProgress = -1;

    int m_diskPort = 0;
    QString m_diskVendor;
    QString m_diskAddress;
    QString m_diskDirectory;
    STATE_EN m_diskStaus = DISK_STATE_OFFLINE;
    TYPE_EN m_diskType = DISK_TYPE_NONE;
    int m_diskGroup = 0;
    int m_diskProperty;
    int m_diskPrivate = 0;
    int m_busable = 0;
    QString m_userName;
    QString m_password;
    qint64 m_totalBytes = 0;
    qint64 m_usedBytes = 0;
    qint64 m_freeBytes = 0;

    int m_raidLevel = 0;
    MD_STATE_EN m_raidStatus = RAID_STATE_NONE;
    int m_raidTask = 0;
    QList<int> m_raidList;

    int m_enable = 0;

    bool m_isUpdated = true;
    DISK_SMART_RESULT m_smartTest = SMART_NONE;
};
Q_DECLARE_METATYPE(BaseDiskInfo)

#endif // BASEDISKINFO_H
