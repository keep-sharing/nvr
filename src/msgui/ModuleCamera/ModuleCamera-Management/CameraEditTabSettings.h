#ifndef CAMERAEDITTABSETTINGS_H
#define CAMERAEDITTABSETTINGS_H

#include "AbstractSettingTab.h"
#include <QEventLoop>

class CameraEdit;

extern "C"
{
#include "msdb.h"
#include "msg.h"
}

namespace Ui {
class CameraEditTabSettings;
}

class CameraEditTabSettings : public AbstractSettingTab
{
    Q_OBJECT

public:
    explicit CameraEditTabSettings(QWidget *parent = 0);
    ~CameraEditTabSettings();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_FISHEYE_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_REMOVE_IPC(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message);

    void testIpcConnect();
    bool isInputValid();
    void copyCameraSchedule(int channel_previous, int channel_result);
    bool isForbidSameChannelExist();
    void updateFisheyeInstallMode();
    void updateFisheyeDisplayMode();
    void updateFisheyeChannelId();
    int saveSettings();
    void lineEditIPShow(bool enable);

private slots:
    void onLanguageChanged() override;

    void on_comboBox_protocol_activated(int index);
    void on_comboBox_channel_activated(int index);

    void on_comboBox_transferMode_indexSet(int index);
    void on_comboBox_fisheyeInstallMode_indexSet(int index);
    void on_comboBox_fisheyeDisplayMode_indexSet(int index);

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton_applly_clicked();

    void on_lineEdit_ip_textEdited(const QString &arg1);
    void on_lineEdit_primary_textEdited(const QString &arg1);
    void on_lineEdit_secondary_textEdited(const QString &arg1);
    void on_comboBox_transportProtocol_activated(int index);

private:
    Ui::CameraEditTabSettings *ui;

    CameraEdit *m_edit = nullptr;

    QEventLoop m_eventLoop;

    int m_channel_previous = -1;
    struct camera m_camera_previous;
    int m_channel_substream = -1;
    CAM_MODEL_INFO m_model_info;

    IPC_CONN_RES m_ipcTestResult;
    quint64 m_ipcSetParamResult;

    //fisheye
    resp_fishmode_param m_fishmode_param;
    ms_ipc_fisheye_info m_fisheye_info;
    bool m_isFisheye_previous = false;

    //protocol
    QString m_msddns;
    QString m_ipAddress;
    QString m_primary;
    QString m_secondary;
    IPC_PROTOCOL  m_perProtocol;
};

#endif // CAMERAEDITTABSETTINGS_H
