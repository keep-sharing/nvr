#ifndef ADDPTZACTION_H
#define ADDPTZACTION_H

#include "BaseShadowDialog.h"
#include <QMap>

extern "C" {
#include "msdb.h"
#include "msg.h"
}

class MsWaitting;

namespace Ui {
class AddPtzAction;
}

class AddPtzAction : public BaseShadowDialog {
    Q_OBJECT

public:
    enum Mode {
        ModeNone,
        ModeAdd,
        ModeEdit
    };

    explicit AddPtzAction(QWidget *parent = 0);
    ~AddPtzAction();

    void showAdd(int channel, int eventType, QMap<int, ptz_action_params> *ptzActionMap);
    void showEdit(int channel, int eventType, const ptz_action_params &action, QMap<int, ptz_action_params> *ptzActionMap);

    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_PTZ_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_PTZ_OVF_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message);

    bool modelDetect(const QRegExp &rx);
    void showPtzActionList();
    void setAll();

signals:

private slots:
    void onLanguageChanged();

    void on_comboBox_ptzChannel_indexSet(int index);
    void on_comboBox_actionType_indexSet(int index);

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_comboBox_ptzChannel_activated(int index);

private:
    Ui::AddPtzAction *ui;
    MsWaitting *m_waitting = nullptr;

    QMap<int, ptz_action_params> *m_ptzActionMap;

    Mode m_mode = ModeNone;
    int m_actionChannel;
    int m_eventType;

    int m_itemChannel;
    int m_itemNumber = -1;
    ptz_action_params m_currentAction;

    resp_ptz_ovf_info *m_info = nullptr;
    ptz_action_params m_ptzActionParams[MAX_CAMERA];

    CAM_MODEL_INFO m_model_info;
    bool m_isPtzSupport = false;
    bool m_isEdit = false;
};

#endif // ADDPTZACTION_H
