#ifndef STORAGEQUOTA_H
#define STORAGEQUOTA_H

#include "AbstractSettingTab.h"
#include <QEventLoop>

extern "C"
{
#include "msfs_disk.h"
}

namespace Ui {
class StorageQuota;
}

class StorageQuota : public AbstractSettingTab
{
    Q_OBJECT

    enum TableColumn
    {
        ColumnCheck,
        ColumnChannel,
        ColumnChannelName,
        ColumnUsedRecord,
        ColumnUsedSnapshot,
        ColumnRecordQuota,
        ColumnSnapshotQuota
    };

    struct GroupInfo
    {
        QMap<int, int> mapChannelQuota;
        //key: disk port, value: gb size
        QMap<int, int> mapDiskSize;
    };

public:
    explicit StorageQuota(QWidget *parent = 0);
    ~StorageQuota();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

protected:
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;

private:
    void ON_RESPONSE_FLAG_GET_MSFS_QUOTA(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_MSFS_QUOTA(MessageReceive *message);

    void resizeTableView();
    void showQuotaInfo();
    void setQuotaEnable(bool enable);

private slots:
    void onLanguageChanged() override;

    void on_comboBoxEnable_indexSet(int index);
    void on_comboBoxChannel_activated(int index);

    void on_lineEditRecordQuota_editingFinished();
    void on_lineEditSnapshotQuota_editingFinished();

    void on_pushButtonCopy_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();


private:
    Ui::StorageQuota *ui;

    QEventLoop m_eventLoop;

    int m_currentChannel = 0;
    req_quota m_quota_info;
};

#endif // STORAGEQUOTA_H
