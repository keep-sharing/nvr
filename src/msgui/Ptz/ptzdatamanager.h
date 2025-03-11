#ifndef PTZDATAMANAGER_H
#define PTZDATAMANAGER_H

#include <QEventLoop>
#include <QMap>
#include "MsObject.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

#define gPtzDataManager (PtzDataManager::instance())

class PtzDataManager : public MsObject {
    Q_OBJECT

public:
    enum PatternOperation {
        PatternStartRecord,
        PatternStopRecord,
        PatternRun,
        PatternStop,
        PatternDelete
    };

    explicit PtzDataManager(QObject *parent = nullptr);

    static PtzDataManager *instance();

    void setFishCorrect(int channel, int correct);
    int fishCorrect(int channel);

    void setChannel(int channel);

    //先调用begin做一些初始化
    void beginGetData(int channel);
    int waitForGetCameraModelType();
    int waitForGetFisheyeInfo();
    int waitForGetCameraModelInfo();
    int waitForGetPtzSupport();
    int waitForGetPtzOvfInfo();
    int waitForGetPtzHttpFishInfo(int bundleFisheyeStream);
    int waitForGetPtzZoomPos();
    int waitForGetPtzSpeed();
    int waitForGetAutoScan();
    int waitForGetLedParams();
    int waitForGetLedStatus();
    int waitForGetPtzManualTracking();
    void endGetData();
    void waitForGetSystemInfo();

    CAM_MODEL_INFO cameraModelInfo() const;
    QString cameraModelString() const;
    int modelType() const;
    int ptzSupport() const;
    bool isPtzSupport() const;
    bool hasPtzInfo() const;
    resp_ptz_ovf_info *ptzOvfInfo();
    resp_ptz_ovf_info *ptzHttpFishInfo();
    int zoomPos() const;
    int ptzSpeed() const;
    int autoScan() const;
    ms_ipc_led_params_resp ledParams() const;
    int ledStatus() const;
    int autoTrackingStatus() const;
    int manualTrackingStatus() const;
    static int maxPresetCount(int channel);

    //fisheye
    bool isFisheye() const;
    bool isBundleFisheye();
    bool isShowBundleStream();
    int fisheyeStream();
    int fisheyeDisplayMode();
    bool isFisheyeAutoScan(int stream);
    bool isFisheyeSupportAutoScan();
    bool isFisheyeSupportPreset() const;
    bool isFisheyeSupportPath();

    void writePtzSpeed(int speed);
    void writePtzZoomPos(int zoom);

    void sendPtzControl(int action, int speed);
    void sendPresetControl(int action, int presetid, int speed, const QString &name);
    void sendPatrolControl(int action, int tourid);
    void sendPatrolData(int tourid, const QList<ptz_path> &list);
    void sendPatternControl(const PatternOperation &operation, int id, int speed);
    void sendPresetControlJson(IPC_PTZ_CONTORL_TYPE cmd, int value);

    //
    bool isAutoZoomModel() const;
    //(ABF) 枪机
    bool isAbfProBox() const;
    //PTZ Bullet
    bool isPtzBullet() const;
    //某些不支持preset的机型，PTZ的控制面板中preset ，path，pattern图标需置灰
    bool isPresetEnable() const;
    bool isSupportWiper() const;

    bool modelDetect(const QRegExp &rx);

    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PTZ_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_PTZ_OVF_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_PTZ_HTTP_FISH_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_LED_STATUS(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);

signals:

public slots:

private:
    static PtzDataManager *s_msPtz;

    int m_channel = -1;
    CAM_MODEL_INFO m_cameraModelInfo;
    int m_modeType = -1;

    //fisheye
    resp_fishmode_param m_fishmode_param;
    int m_multiFisheyeStream = 0;
    int m_bundleFisheyeStream = 0;

    int m_ptzSupport = -1;
    bool m_hasPtzInfo = false;
    resp_ptz_ovf_info m_ptzInfo;
    int m_zoompos = 0;
    int m_speed = 0;
    int m_autoScan = 0;

    ms_ipc_led_params_resp m_led_params;
    int m_ledStatus = 0;
    int m_auto_tracking = 0;
    int m_manual_tracking = 0;
    bool m_supportWiper = false;

    QMap<int, int> m_fishCorrectMap;

    QEventLoop m_eventLoop;
};

#endif // PTZDATAMANAGER_H
