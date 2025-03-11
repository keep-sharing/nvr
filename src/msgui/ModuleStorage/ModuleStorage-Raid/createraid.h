#ifndef CREATERAID_H
#define CREATERAID_H

#include "basediskinfo.h"
#include "BaseShadowDialog.h"
#include <QCheckBox>

class MsWaitting;

namespace Ui {
class CreateRaid;
}

class CreateRaid : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit CreateRaid(QWidget *parent = nullptr);
    ~CreateRaid();

    void setDiskInfoMap(const QMap<int, BaseDiskInfo> &diskInfoMap, const QList<int> &checkedList);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_CREATE_RAID(MessageReceive *message);

    quint64 checkRaid(int level, const QMap<int, int> &portMap) const;

private slots:
    void onLanguageChanged() override;
    void onCheckBoxClicked();

    void on_comboBox_type_activated(int index);
    void on_pushButton_create_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::CreateRaid *ui;
    MsWaitting *m_waitting;

    QList<QCheckBox *> m_checkBoxList;

    QMap<int, BaseDiskInfo> m_diskInfoMap;
    quint64 m_bytes = 0;
};

#endif // CREATERAID_H
