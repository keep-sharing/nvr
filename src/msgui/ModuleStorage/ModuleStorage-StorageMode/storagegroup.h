#ifndef GROUPSETTING_H
#define GROUPSETTING_H

#include "AbstractSettingTab.h"

namespace Ui {
class StorageGroup;
}

class StorageGroup : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit StorageGroup(QWidget *parent = 0);
    ~StorageGroup();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_MSFS_DISKINFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_RECORD_UPDATE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_MSFS_MODE(MessageReceive *message);

private slots:
    void onLanguageChanged() override;

    void onChannelCheckBoxClicked();

    void on_comboBoxEnable_indexSet(int index);
    void on_comboBox_group_activated(int index);

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::StorageGroup *ui;

    QMap<int, struct groupInfo> m_groupMap;
};

#endif // GROUPSETTING_H
