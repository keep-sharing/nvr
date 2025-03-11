#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QTimer>

extern "C" {
#include "recortsp.h"
#include "vapi.h"
}

class CameraInfo : public QObject {
    Q_OBJECT

public:
    explicit CameraInfo(QObject *parent = nullptr);
    explicit CameraInfo(const QString &ip, const QString &user, const QString &password, QObject *parent = nullptr);

    static CameraInfo *fromChannel(int channel);
    static void cleanup();

    static int count();
    static int validCount();

    void initialize();

    bool isValid() const;
    void clear();

    const QString &ip() const;
    void setIp(const QString &newIp);

    const QString &user() const;
    void setUser(const QString &newUser);

    const QString &password() const;
    void setPassword(const QString &newPassword);

    int channel() const;
    void setChannel(int newChannel);
    QString channelString() const;

    int protocol() const;
    void setProtocol(int newProtocol);
    QString protocolString() const;

    int codec() const;
    void setCodec(int newCodec);
    QString codecString() const;

    QString frameSize() const;
    void setFrameSize(const QString &newFrameSize);

    QString stateString() const;

    QString rtspUrl() const;

    bool isDecodeEnable();
    void setDecodeEnable(bool enable);

    void *rtspServerHandle() const;
    void setRtspServerHandle(void *newRtspServerHandle);

    int rtspServerState() const;
    void setRtspServerState(int newRtspServerState);

    int rtspClientState() const;
    void setRtspClientState(int newRtspClientState);

    void updateFrameInfo(reco_frame *frame);

    //
    int createRtspClient();
    int deleteRtspClient();
    int createRtspServer();
    int deleteRtspServer();

private:
    static int callbackRtspClientReceive(void *arg, struct reco_frame *frame);
    static int callbackRtspClientState(int state, void *arg, struct rtsp_client_state_info *info);

    static int callbackRtspServerState(int state, void *arg);

private slots:
    void onTimerReconnect();

signals:
    void stateChanged(int channel, int state);

private:
    static QMap<int, CameraInfo *> s_mapCameraInfo;

    QMutex m_mutex;

    int m_channel = -1;
    QString m_ip;
    QString m_user;
    QString m_password;
    int m_protocol = TRANSPROTOCOL_UDP;
    int m_codec = ENC_TYPE_H264;
    QString m_frameSize;

    int m_width = 0;
    int m_height = 0;


    void *m_rtspClientHandle = nullptr;
    int m_rtspClientState = 0;

    void *m_rtspServerHandle = nullptr;
    int m_rtspServerState = 0;

    bool m_bDecode = false;

    QTimer *m_timerReconnect = nullptr;
};

#endif // CAMERAINFO_H
