#include "EventDetectionRegionManager.h"
#include "LiveVideo.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "mainwindow.h"

extern "C" {
#include "msg.h"
}

EventDetectionRegionManager *EventDetectionRegionManager::self = nullptr;

EventDetectionRegionManager::EventDetectionRegionManager(QObject *parent)
    : MsObject(parent)
{
    m_requestTimer = new QTimer(this);
    connect(m_requestTimer, SIGNAL(timeout()), this, SLOT(onRequestTimer()));
    m_requestTimer->setSingleShot(true);
}

EventDetectionRegionManager::~EventDetectionRegionManager()
{
}

EventDetectionRegionManager *EventDetectionRegionManager::instance()
{
    if (!self) {
        QMS_ASSERT(MainWindow::instance());
        self = new EventDetectionRegionManager(MainWindow::instance());
    }
    return self;
}

void EventDetectionRegionManager::registerVideo(LiveVideo *video)
{
    int channel = video->channel();
    if (channel < 0) {
        //MsWarning() << video << ", channel:" << channel;
    } else {
        m_videoMap.insert(channel, video);
    }
}

void EventDetectionRegionManager::unregisterVideo(LiveVideo *video)
{
    for (auto iter = m_videoMap.begin(); iter != m_videoMap.end();) {
        if (video == iter.value()) {
            iter = m_videoMap.erase(iter);
        } else {
            iter++;
        }
    }
}

void EventDetectionRegionManager::appendMessage(int channel, int message)
{
    RequestInfo info;
    info.channel = channel;
    info.message = message;
    if (m_requestQueue.contains(info)) {
        if (!m_isTimerActive) {
            m_requestTimer->start(2000);
            m_isTimerActive = true;
        }
    } else {
        m_requestQueue.enqueue(info);
        if (!m_isTimerActive) {
            //3.2s 避免在ON_RESPONSE_FLAG_GET_ALL_IPCTYPE前触发
            m_requestTimer->start(3200);
            m_isTimerActive = true;
        }
    }
}

void EventDetectionRegionManager::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_VCA_REGIONENTRANCE:
        ON_RESPONSE_FLAG_GET_VCA_REGIONENTRANCE(message);
        break;
    case RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION:
        ON_RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION(message);
        break;
    case RESPONSE_FLAG_GET_VCA_REGIONEXIT:
        ON_RESPONSE_FLAG_GET_VCA_REGIONEXIT(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LOITERING:
        ON_RESPONSE_FLAG_GET_VCA_LOITERING(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LINECROSSING:
        ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(message);
        break;
    case RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2:
        ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(message);
        break;
    case RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT:
        ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LEFTREMOVE:
        ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(message);
        break;
    case RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE:
        ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(message);
        break;
    }
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_VCA_REGIONENTRANCE(MessageReceive *message)
{
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION(MessageReceive *message)
{
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_VCA_REGIONEXIT(MessageReceive *message)
{
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_VCA_LOITERING(MessageReceive *message)
{
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(MessageReceive *message)
{
    ms_linecrossing_info *info = (ms_linecrossing_info *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(MessageReceive *message)
{
    ms_linecrossing_info2 *info = (ms_linecrossing_info2 *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(MessageReceive *message)
{
    ms_smart_event_people_cnt *info = (ms_smart_event_people_cnt *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(MessageReceive *message)
{
    ms_smart_leftremove_info *info = (ms_smart_leftremove_info *)message->data;
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chanid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(MessageReceive *message)
{
    MsIpcRegionalPeople *info = static_cast<MsIpcRegionalPeople *>(message->data);
    if (info) {
        QList<LiveVideo *> videos = m_videoMap.values(info->chnid);
        for (int i = 0; i < videos.size(); ++i) {
            LiveVideo *video = videos.at(i);
            video->dealEventDetectionRegionMessage(message);
        }
    } else {
        qMsWarning() << message;
    }
    m_requestTimer->start(0);
}

void EventDetectionRegionManager::onRequestTimer()
{
    if (m_requestQueue.isEmpty()) {
        m_isTimerActive = false;
        return;
    }
    m_currentRequestInfo = m_requestQueue.dequeue();
    if (!m_videoMap.contains(m_currentRequestInfo.channel)) {
        m_requestTimer->start(0);
        return;
    }
    if (qMsNvr->isFisheye(m_currentRequestInfo.channel)) {
        sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, &m_currentRequestInfo.channel, sizeof(int));
    } else {
        sendMessage(m_currentRequestInfo.message, &m_currentRequestInfo.channel, sizeof(int));
    }
}
