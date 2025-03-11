#include "PlaybackRealTimeThread.h"

#include "MsDevice.h"
#include "MyDebug.h"

extern "C" {

}

PlaybackRealTimeThread *PlaybackRealTimeThread::s_self = nullptr;

PlaybackRealTimeThread::PlaybackRealTimeThread()
    : QObject(nullptr)
{
    s_self = this;

    qRegisterMetaType<QMap<int, QDateTime>>("QMap<int, QDateTime>");

    moveToThread(&m_thread);
    m_thread.setObjectName("Qt-PlaybackRealTimeThread");
    m_thread.start();
}

PlaybackRealTimeThread::~PlaybackRealTimeThread()
{
    s_self = nullptr;
}

PlaybackRealTimeThread *PlaybackRealTimeThread::instance()
{
    if (!s_self) {
        s_self = new PlaybackRealTimeThread();
    }
    return s_self;
}

void PlaybackRealTimeThread::startThread()
{
    if (!m_thread.isRunning()) {
        m_thread.start();
    }
}

void PlaybackRealTimeThread::stopThread()
{
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait();
    }
}

void PlaybackRealTimeThread::getPlaybackRealTime()
{
    if (m_isGetPlaybackRealTimeEnd) {
        QMetaObject::invokeMethod(this, "onGetPlaybackRealTime");
    }
}

void PlaybackRealTimeThread::getSplitPlaybackRealTime(int channel, QString sidMaskl)
{
    if (m_isGetSplitPlaybackRealTimeEnd) {
        QMetaObject::invokeMethod(this, "onGetSplitPlaybackRealTime", Q_ARG(int, channel), Q_ARG(QString, sidMaskl));
    }
}

void PlaybackRealTimeThread::onGetPlaybackRealTime()
{
    m_isGetPlaybackRealTimeEnd = false;

    m_isGetPlaybackRealTimeEnd = true;
}

void PlaybackRealTimeThread::onGetSplitPlaybackRealTime(int channel, QString sidMaskl)
{
    Q_UNUSED(channel)
    Q_UNUSED(sidMaskl)
}
