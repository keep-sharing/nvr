#ifndef DISKSETTING_H
#define DISKSETTING_H

#include "AbstractSettingTab.h"

class AddNetworkDisk;
class HardDiskEdit;
class NetworkDiskEdit;

namespace Ui {
class DiskSetting;
}

class DiskSetting : public AbstractSettingTab {
    Q_OBJECT

public:
    enum DiskColumn {
        ColumnCheck,
        ColumnPort,
        ColumnVendor,
        ColumnStatus,
        ColumnTotal,
        ColumnFree,
        ColumnProperty,
        ColumnType,
        ColumnGroup,
        ColumnEdit,
        ColumnDelete
    };

    explicit DiskSetting(QWidget *parent = 0);
    ~DiskSetting();

    void initializeData() override;

    void dealMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_FORMAT_MSFS_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_DISK_INIT(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_DISK_LOAD(MessageReceive *message);
    void ON_RESPONSE_FLAG_DEL_MSFS_NAS(MessageReceive *message);
    void ON_RESPONSE_FLAG_REMOVE_RAID(MessageReceive *message);

    void refreshTable();

private slots:
    void onLanguageChanged() override;

    void onTableHeaderChecked(bool checked);
    void onTableClicked(int row, int column);

    void on_pushButton_refresh_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_initialize_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::DiskSetting *ui;

    HardDiskEdit *m_hardDiskEdit = nullptr;
    QMap<int, bool> m_selectDiskMap;
};

#endif // DISKSETTING_H
