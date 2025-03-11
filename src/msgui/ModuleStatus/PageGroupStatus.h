#ifndef PAGEGROUPSTATUS_H
#define PAGEGROUPSTATUS_H

#include "AbstractSettingPage.h"
#include <QMap>

extern "C" {
#include "msdb.h"
}

namespace Ui {
class GroupStatus;
}

class PageGroupStatus : public AbstractSettingPage {
    Q_OBJECT

public:
    enum GroupSort {
        GroupSortColumnGroup,
        GroupSortColumnDisk,
        GroupSortColumnChannel
    };
    enum ChannelSort {
        ChannelSortColumnChannel,
        ChannelSortColumnGroup
    };

    explicit PageGroupStatus(QWidget *parent = 0);
    ~PageGroupStatus();

    void initializeData() override;

    bool canAutoLogout() override;

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void onTabClicked(int index);

    void on_pushButton_back_clicked();

private:
    void sortByGroup();
    void sortByChannel();

private:
    Ui::GroupStatus *ui;

    bool m_is_group_enable = false;
    bool m_is_sata_storage = false;
    QMap<int, groupInfo> m_groupMap;
};

#endif // PAGEGROUPSTATUS_H
