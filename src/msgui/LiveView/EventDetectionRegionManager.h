#ifndef EVENTDETECTIONREGIONMANAGER_H
#define EVENTDETECTIONREGIONMANAGER_H

#include "MsObject.h"
#include <QMultiMap>
#include <QQueue>

class QTimer;
class LiveVideo;
class MessageReceive;

class EventDetectionRegionManager : public MsObject {
    Q_OBJECT

    struct RequestInfo {
        int channel = -1;
        int message = -1;

        bool operator==(const RequestInfo &other)
        {
            return channel == other.channel && message == other.message;
        }
    };

public:
    explicit EventDetectionRegionManager(QObject *parent);
    ~EventDetectionRegionManager();

    static EventDetectionRegionManager *instance();

    void registerVideo(LiveVideo *video);
    void unregisterVideo(LiveVideo *video);

    void appendMessage(int channel, int message);
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_REGIONENTRANCE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_REGIONEXIT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LOITERING(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(MessageReceive *message);

public slots:

private slots:
    void onRequestTimer();

private:
    static EventDetectionRegionManager *self;

    QTimer *m_requestTimer = nullptr;
    bool m_isTimerActive = false;

    RequestInfo m_currentRequestInfo;
    QQueue<RequestInfo> m_requestQueue;
    QMultiMap<int, LiveVideo *> m_videoMap;
};

#endif // EVENTDETECTIONREGIONMANAGER_H
