#ifndef PAGERAIDSETTING_H
#define PAGERAIDSETTING_H

#include "AbstractSettingPage.h"

class CreateRaid;
class RebulidRaid;

namespace Ui {
class RaidSetting;
}

class PageRaidSetting : public AbstractSettingPage {
    Q_OBJECT

public:
    enum DiskColumn {
        DiskColumnCheck,
        DiskColumnPort,
        DiskColumnVendor,
        DiskColumnStatus,
        DiskColumnCapacity,
        DiskColumnType,
        DiskColumnHotSpare
    };
    enum ArrayColumn {
        ArrayColumnCheck,
        ArrayColumnNumber,
        ArrayColumnName,
        ArrayColumnDisk,
        ArrayColumnCapacity,
        ArrayColumnStatus,
        ArrayColumnLevel,
        ArrayColumnHotSpare,
        ArrayColumnRebulid,
        ArrayColumnDelete,
        ArrayColumnTask
    };

    explicit PageRaidSetting(QWidget *parent = 0);
    ~PageRaidSetting();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_RAID_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(MessageReceive *message);
    void ON_RESPONSE_FLAG_CREATE_RAID(MessageReceive *message);
    void ON_RESPONSE_FLAG_CREATE_SPACE(MessageReceive *message);
    void ON_RESPONSE_FLAG_REMOVE_SPACE(MessageReceive *message);

    void ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message);
    void ON_RESPONSE_FLAG_REMOVE_RAID(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_RAID_REBUILD(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;

private:
    void initializeDiskTable();
    void initializeArrayTable();

    void refreshDiskTable();

private slots:
    void onLanguageChanged() override;

    void onTableDiskClicked(int row, int column);
    void onTableArrayClicked(int row, int column);

    void on_pushButton_quickCreator_clicked();
    void on_pushButton_creator_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::RaidSetting *ui;

    bool m_isRaidEnable = false;

    CreateRaid *m_createRaid = nullptr;
    RebulidRaid *m_rebulidRaid = nullptr;
};

#endif // PAGERAIDSETTING_H
