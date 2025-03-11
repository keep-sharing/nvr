#ifndef MSDISK_H
#define MSDISK_H

#include "MsObject.h"
#include "basediskinfo.h"
#include <QMap>

class MessageReceive;

#define gMsDisk MsDisk::instance()

class MsDisk : public MsObject {
    Q_OBJECT
public:
    explicit MsDisk(QObject *parent = nullptr);
    ~MsDisk();

    static MsDisk &instance();

    QMap<int, BaseDiskInfo> diskInfoMap() const;

    void getDiskInfo();
    //nas
    void removeNas(int port);
    //disk
    int validNetworkDiskPort();
    void removeDisk(int port);
    //raid
    int validRaidPort();
    void removeRaid(int port);
    void removeAllRaid();
    //
    bool addFormatPort(int port);
    void clearFormatPort();
    void startFormat();
    bool hasUnformatDisk();

    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_RAID_REBUILD(MessageReceive *message);

    void ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_REMOVE_RAID(MessageReceive *message);

private:
    void dealNewDisk(const resp_get_msfs_diskinfo &diskinfo);
    void removeDatabaseDisk(int port);
    void addDatabaseDisk(const resp_get_msfs_diskinfo &diskinfo);

signals:
    void detectedUninitializedDisk();

public slots:

private:
    QMap<int, BaseDiskInfo> m_dbDiskInfoMap;
    //<vendor, port>
    QMap<QString, int> m_dbDiskVendorMap;
    //
    QMap<int, BaseDiskInfo> m_diskInfoMap;
    //
    bool m_isFormatting = false;
    QList<int> m_formatList;
};

#endif // MSDISK_H
