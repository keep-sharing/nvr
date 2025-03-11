#ifndef CAMERADATA_H
#define CAMERADATA_H

#include "MsObject.h"
#include "vapi.h"
#include <QMap>
extern "C" {
#include "recortsp.h"
}

class QTimer;

#define gCameraData CameraData::instance()

class CameraData : public MsObject {
    Q_OBJECT

public:
    struct State {
        int chanid;
        int mainsub; //0:sub, 1:main
        int state;
        int livepb;

        bool isMainStream() const
        {
            return mainsub == 1;
        }
        bool isLiveView() const
        {
            return livepb == DSP_MODE_LIVE;
        }
        bool isConnected() const
        {
            return state == RTSP_CLIENT_CONNECT;
        }
        bool isDisconnected() const
        {
            return state == RTSP_CLIENT_DISCONNECT;
        }
        bool operator==(const State &other) const
        {
            if (chanid == other.chanid && mainsub == other.mainsub && state == other.state && livepb == other.livepb) {
                return true;
            }
            return false;
        }
    };

    explicit CameraData();
    ~CameraData();

    static CameraData &instance();

    void readyToQuit();

    int maxCameraCount();

    QString cameraIPv4Address(int channel);
    QString cameraName(int channel);
    QString cameraDDNS(int channel);

    bool isCameraEnable(int channel);
    bool isCameraConnected(int channel);
    bool isCameraMainStreamConnected(int channel);
    bool isCameraSubStreamConnected(int channel);

    bool isCameraFacePrivacyMode(int channel);

    void getAllIpcData();
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_ALL_IPCTYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message);

signals:
    void updateStreamInfo();
    void cameraStateChanged(CameraData::State state);
    void cameraConnectionStateChanged(int channel, int state);
    void cameraFacePrivacyState(int channel, int state);

public slots:
    void updateCameraState(CameraData::State state);

private slots:
    void onTimerGetCameraList();

private:
    QTimer *m_timerGetCameraList = nullptr;

    QMap<int, int> m_cameraFacePrivacyMap;
    //
    QMap<int, int> m_cameraConnectionStateMap;
};

#endif // CAMERADATA_H
