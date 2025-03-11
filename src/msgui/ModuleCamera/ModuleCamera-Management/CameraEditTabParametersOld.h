#ifndef CAMERAEDITTABPARAMETERSOLD_H
#define CAMERAEDITTABPARAMETERSOLD_H

#include "AbstractSettingTab.h"
#include <QEventLoop>
#include "MsCameraModel.h"

extern "C"
{
#include "msdb.h"
#include "msg.h"
}

class CameraEdit;

namespace Ui {
class CameraEditTabParametersOld;
}

class CameraEditTabParametersOld : public AbstractSettingTab
{
    Q_OBJECT

public:
    explicit CameraEditTabParametersOld(QWidget *parent = 0);
    ~CameraEditTabParametersOld();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCPARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_EVENT_STREAM_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_EVENT_STREAM_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_COMMON_PARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCPARAM_EX(MessageReceive *message);

    void showGeneralParameters();
    void showEventParameters();

    void updateFrameRateMain(int maxFrameRate, int currentFrameRate);
    void setSubStreamEnableForFisheye(bool enable);

    void showBitrateCtl();
    void saveBitrateCtl();

    int saveParameters();
    void clearSetting();

private slots:
    void onLanguageChanged() override;

    void on_comboBox_recordStreamType_activated(int index);
    void on_comboBox_secondaryStream_activated(int index);

    void on_comboBox_frameRate_main_activated(int index);
    void on_comboBox_frameRate_sub_activated(int index);

    void onCheckBoxEventStreamCheckStateSet(int state);

    void onComboBoxMainBitRateSet(int bitRate);
    void onComboBoxMainBitRateEditingFinished(int bitRate);

    void onComboBoxSubBitRateEditingFinished(int bitRate);

    void on_comboBox_frameInterval_main_currentIndexChanged(int index);

    void on_comboBox_frameSize_main_indexSet(int index);
    void on_comboBox_frameSize_main_activated(int index);
    void on_comboBox_frameSize_sub_activated(int index);

    void on_comboBox_smartStream_main_activated(int index);
    void on_comboBox_smartStream_sub_activated(int index);

    void on_comboBox_codec_sub_activated(int index);

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton_applly_clicked();

private:
    Ui::CameraEditTabParametersOld *ui;

    CameraEdit *m_edit = nullptr;

    QEventLoop m_eventLoop;

    MsCameraModel m_cameraModel;
    int m_channel_previous = -1;
    struct camera m_camera_previous;
    int m_channel_substream = -1;
    CAM_MODEL_INFO m_model_info;

    quint64 m_ipcSetParamResult;
    resp_get_ipc_param m_ipc_param;
    req_set_ipc_param m_set_ipc_param;
    int m_mainMaxFrameRate;

    bool m_showEventStream = false;
    event_stream_info m_event_info;

    QMap<int, int> m_readybitctlMap;
    QMap<int, int> m_readygetbitctlMap;

    //fisheye
    resp_fishmode_param m_fishmode_param;

    //保存次码流状态，次码流由关闭变成开启时，分辨率要设置为0，不然设置失败
    int m_lastSecondaryStreamEnable = false;
};

#endif // CAMERAEDITTABPARAMETERSOLD_H
