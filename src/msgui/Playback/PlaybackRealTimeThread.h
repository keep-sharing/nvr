#ifndef PLAYBACKREALTIMETHREAD_H
#define PLAYBACKREALTIMETHREAD_H

#include <QThread>
#include <QMap>
#include <QDateTime>

class PlaybackRealTimeThread : public QObject
{
    Q_OBJECT
public:
    explicit PlaybackRealTimeThread();
    ~PlaybackRealTimeThread();

    static PlaybackRealTimeThread *instance();

    void startThread();
    void stopThread();

    //
    void getPlaybackRealTime();
    void getSplitPlaybackRealTime(int channel, QString sidMaskl);

signals:
    void playbackRealTime(QDateTime dateTime);
    void splitPlaybackRealTime(int channel, QMap<int, QDateTime> dateTimeMap);

private slots:
    void onGetPlaybackRealTime();
    void onGetSplitPlaybackRealTime(int channel, QString sidMaskl);

private:
    static PlaybackRealTimeThread *s_self;
    QThread m_thread;

    bool m_isGetPlaybackRealTimeEnd = true;
    bool m_isGetSplitPlaybackRealTimeEnd = true;
};

#endif // PLAYBACKREALTIMETHREAD_H
