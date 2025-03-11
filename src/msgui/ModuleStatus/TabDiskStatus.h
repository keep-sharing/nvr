#ifndef TABDISKSTATUS_H
#define TABDISKSTATUS_H

#include "AbstractSettingTab.h"

class MessageReceive;
class MsWaitting;

namespace Ui {
class PageDiskStatus;
}

class TabDiskStatus : public AbstractSettingTab {
    Q_OBJECT

public:
    enum DiskColumn {
        ColumnPort,
        ColumnVendor,
        ColumnStatus,
        ColumnTotal,
        ColumnFree,
        ColumnProperty,
        ColumnType,
        ColumnGroup
    };

    explicit TabDiskStatus(QWidget *parent = 0);
    ~TabDiskStatus();

    void initializeData() override;

    QList<int> localDiskList();

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;
    void refreshData();

protected:
    void ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLanguageChanged() override;

private:
    Ui::PageDiskStatus *ui;
    MsWaitting *m_waitting;

    QList<int> m_localDiskPortList;
};

#endif // TABDISKSTATUS_H
