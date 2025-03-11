#ifndef TIMELINETHREAD_H
#define TIMELINETHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QSize>
#include <QImage>
#include <QDateTime>
#include <QMap>
#include "BasePlayback.h"

class TimeLineThread : public QObject
{
    Q_OBJECT

public:
    struct ImageInfo
    {
        int channel;
        //
        QSize imageSize;
        QDateTime beginDateTime;
        QDateTime endDateTime;
        qreal secWidth;
        //
        QDate playbackDate;
        MsPlaybackType playbackMode;
        //
        QList<resp_search_common_backup> commonList;
        QList<resp_search_event_backup> eventList;
        QMap<QDateTime, int> tagMap;
        //
        bool isSmartSearch = false;
        bool isEventOnly = false;
    };

public:
    explicit TimeLineThread(QObject *parent = nullptr);

    void stopThread();
    //
    void drawImage(const ImageInfo &info);

signals:
    void imageFinished(int channel, QImage image);

public slots:
    void onDrawImage();

private:
    QThread m_thread;
    QMutex m_mutex;

    QList<ImageInfo> m_imageInfoList;
};

#endif // TIMELINETHREAD_H
