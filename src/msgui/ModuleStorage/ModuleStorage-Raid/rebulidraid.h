#ifndef REBULIDRAID_H
#define REBULIDRAID_H

#include "basediskinfo.h"
#include "BaseShadowDialog.h"
#include "MsWaitting.h"
#include <QCheckBox>

class MessageReceive;

namespace Ui {
class RebulidRaid;
}

class RebulidRaid : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit RebulidRaid(QWidget *parent = nullptr);
    ~RebulidRaid();

    void setDiskInfoMap(const QMap<int, BaseDiskInfo> &diskInfoMap);
    int execRaidRebulid(const BaseDiskInfo &raidInfo);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_COMPONENT_SIZE_RAID(MessageReceive *message);

private slots:
    void onLanguageChanged() override;
    void onCheckBoxClicked();

    void on_pushButton_rebulid_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::RebulidRaid *ui;

    MsWaitting *m_waitting;

    QList<QCheckBox *> m_checkBoxList;

    QMap<int, BaseDiskInfo> m_diskInfoMap;
    BaseDiskInfo m_rebulidDiskInfo;
    quint64 m_rebulidMinSize = 0;
};

#endif // REBULIDRAID_H
