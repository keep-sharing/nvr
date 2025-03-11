#ifndef PUSHEVENTEDIT_H
#define PUSHEVENTEDIT_H

#include "BaseShadowDialog.h"
#include <QMap>

extern "C" {
#include "msdb.h"
}

class QCheckBox;

namespace Ui {
class PushEventEdit;
}

class PushEventEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit PushEventEdit(QWidget *parent = nullptr);
    ~PushEventEdit();

    QMap<int, int> getIPCPushType();
    int getNVRAlarmInType();

    void setType(QMap<int, int> ipcType, int nvrType);
    void setNvrEventPos(PushMsgNvrEvent *event);

private:
    void showPushType(int event);
    int pushType();
    //    void savePushEvent(int channel, int type);

public slots:
    void onLanguageChanged();

private slots:
    void on_comboBox_channel_activated(int index);

    void onCheckBoxTypeClicked(bool checked);
    void on_checkBox_all_clicked(bool checked);
    void on_checkBox_alarmIn_clicked(bool checked);
    void on_checkBox_anpr_clicked(bool checked);

    void on_pushButton_copy_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void onTabBarClicked(int index);

private:
    Ui::PushEventEdit *ui;

    int m_channel = 0;

    QMap<int, QCheckBox *> m_checkBoxMap;
    QMap<int, int> IPCPushType;
    int NVRAlarmInType = 0;

    PushMsgNvrEvent *m_nvrEventPos = nullptr;
};

#endif // PUSHEVENTEDIT_H
