#include "TimeLineThread.h"
#include "MyDebug.h"
#include <QPainter>
#include <QStyle>
#include <QtDebug>

TimeLineThread::TimeLineThread(QObject *parent)
    : QObject(parent)
{
    m_thread.setObjectName("Qt-TimeLineThread");
    moveToThread(&m_thread);
    m_thread.start();
}

void TimeLineThread::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void TimeLineThread::drawImage(const ImageInfo &info)
{
    m_mutex.lock();

    if (m_imageInfoList.isEmpty()) {
        QMetaObject::invokeMethod(this, "onDrawImage", Qt::QueuedConnection);
    } else {
        m_imageInfoList.clear();
    }
    m_imageInfoList.append(info);

    m_mutex.unlock();
}

void TimeLineThread::onDrawImage()
{
    m_mutex.lock();
    if (m_imageInfoList.isEmpty()) {
        m_mutex.unlock();
        return;
    }

    ImageInfo info = m_imageInfoList.takeFirst();
    m_mutex.unlock();

    qDebug() << "Draw Time Line Imgae"
             << ", channel:" << info.channel
             << ", beginDateTime:" << info.beginDateTime.toString("yyyy-MM-dd HH:mm:ss")
             << ", endDateTime:" << info.endDateTime.toString("yyyy-MM-dd HH:mm:ss");

    QImage image(info.imageSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    //image.fill(QColor(255, 0, 0, 80));

    QPainter painter(&image);

    painter.save();
    painter.setPen(Qt::NoPen);

    QRectF rcf;
    rcf.setTop(info.imageSize.height() / 2 + 1);
    rcf.setBottom(info.imageSize.height());

    int padding = info.imageSize.width() / 50.0;

    uint timeLineBeginSec = info.beginDateTime.toTime_t();

    uint lastEndSec = 0;
    for (int i = 0; i < info.commonList.size(); ++i) {
        const resp_search_common_backup &common_backup = info.commonList.at(i);
        QDateTime beginDateTime = QDateTime::fromString(common_backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime endDateTime = QDateTime::fromString(common_backup.pEndTime, "yyyy-MM-dd HH:mm:ss");

        if (common_backup.chnid != info.channel) {
            qMsWarning() << QString("error common_backup, info.channel: %1, channel: %2, pStartTime: %3, pEndTime: %4").arg(info.channel).arg(common_backup.chnid).arg(common_backup.pStartTime).arg(common_backup.pEndTime);
            continue;
        }

        //显示范围过滤
        bool isOverLeft = false;
        bool isOverRight = false;
        if (endDateTime < info.beginDateTime) {
            continue;
        }
        if (beginDateTime < info.beginDateTime && endDateTime >= info.beginDateTime) {
            isOverLeft = true;
            beginDateTime = info.beginDateTime;
        }
        if (beginDateTime > info.endDateTime) {
            continue;
        }
        if (beginDateTime <= info.endDateTime && endDateTime > info.endDateTime) {
            isOverRight = true;
            endDateTime = info.endDateTime;
        }

        uint beginSec = beginDateTime.toTime_t();
        uint endSec = endDateTime.toTime_t();

        //正常情况录像
        //2019-05-15 00:52:05 - 2019-05-16 01:25:33
        //2019-05-15 01:25:33 - 2019-05-16 01:59:02
        //有可能也会有一秒的间隔，也是正常的，如：
        //2019-05-15 00:52:05 - 2019-05-16 01:25:33
        //2019-05-15 01:25:34 - 2019-05-16 01:59:02
        //避免界面显示录像间断，这里修正一下
        if (beginSec - lastEndSec == 1) {
            beginSec = lastEndSec;
        }
        lastEndSec = endSec;

        //
        if (info.isSmartSearch) {
            painter.setBrush(QBrush(QColor("#09A8E0")));
        } else {
            switch ((REC_EVENT_EN)common_backup.enEvent) {
            case REC_EVENT_TIME:
                painter.setBrush(QBrush(QColor("#09A8E0")));
                break;
            case REC_EVENT_ANR:
                painter.setBrush(QBrush(QColor("#FF7F24"))); //ANR暂用颜色
                break;
            default:
                painter.setBrush(QBrush(QColor("#E70000")));
                break;
            }
        }

        //
        rcf.setLeft(padding + (beginSec - timeLineBeginSec) * info.secWidth);
        rcf.setRight(padding + (endSec - timeLineBeginSec) * info.secWidth);
        if (!info.isEventOnly) {
            painter.drawRect(rcf);
        }

        //绘制lock
        if (info.playbackMode == GeneralPlayback) {
            if (common_backup.isLock) {
                painter.save();

                painter.setPen(QPen(QColor("#FF9D00")));
                painter.drawLine(QPointF(rcf.left(), rcf.top() - 3), QPointF(rcf.right(), rcf.top() - 3));
                if (!isOverLeft) {
                    painter.drawLine(QPointF(rcf.left(), rcf.top() - 3), QPointF(rcf.left(), rcf.top()));
                }
                if (!isOverRight) {
                    painter.drawLine(QPointF(rcf.right(), rcf.top() - 3), QPointF(rcf.right(), rcf.top()));
                }

                QRect lockRect;
                lockRect.setLeft(rcf.left());
                lockRect.setRight(rcf.right());
                lockRect.setTop(info.imageSize.height() / 2.0 / 8.0 * 5.0 + 1);
                lockRect.setBottom(rcf.top() - 3);

                QImage lockImage(":/playback/playback/lock2.png");
                QRect pixmapRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(lockRect.height() - 4, lockRect.height() - 4), lockRect);
                painter.drawImage(pixmapRect, lockImage);

                painter.restore();
            }
        }
    }

    //绘制event
    for (int i = 0; i < info.eventList.size(); ++i) {
        const resp_search_event_backup &event_backup = info.eventList.at(i);
        QDateTime beginDateTime = QDateTime::fromString(event_backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime endDateTime = QDateTime::fromString(event_backup.pEndTime, "yyyy-MM-dd HH:mm:ss");

        if (event_backup.chnid != info.channel) {
            qMsWarning() << QString("error event_backup, info.channel: %1, channel: %2, pStartTime: %3, pEndTime: %4").arg(info.channel).arg(event_backup.chnid).arg(event_backup.pStartTime).arg(event_backup.pEndTime);
            continue;
        }

        //显示范围过滤
        if (endDateTime < info.beginDateTime) {
            continue;
        }
        if (beginDateTime < info.beginDateTime && endDateTime >= info.beginDateTime) {
            beginDateTime = info.beginDateTime;
        }
        if (beginDateTime > info.endDateTime) {
            continue;
        }
        if (beginDateTime <= info.endDateTime && endDateTime > info.endDateTime) {
            endDateTime = info.endDateTime;
        }

        uint beginSec = beginDateTime.toTime_t();
        uint endSec = endDateTime.toTime_t();
        if (beginSec == endSec) {
            endSec++;
        }

        //
        switch ((REC_EVENT_EN)event_backup.enEvent) {
        case REC_EVENT_TIME:
            painter.setBrush(QBrush(QColor("#09A8E0")));
            break;
        case REC_EVENT_ANR:
            painter.setBrush(QBrush(QColor("#FF7F24"))); //ANR暂用颜色
            break;
        default:
            if (BasePlayback::isSmartSearchMode()) {
                painter.setBrush(QBrush(QColor("#E70000")));
            } else {
                painter.setBrush(QBrush(QColor("#2741B1")));
            }
            break;
        }

        //
        rcf.setLeft(padding + (beginSec - timeLineBeginSec) * info.secWidth);
        rcf.setRight(padding + (endSec - timeLineBeginSec) * info.secWidth);
        painter.drawRect(rcf);
    }

    //绘制tag
    if (info.playbackMode == GeneralPlayback) {
        if (!info.tagMap.isEmpty()) {
            painter.save();
            painter.setPen(QPen(QColor("#D54CE3"), 2));

            QRect tagRect;
            tagRect.setTop(info.imageSize.height() / 2.0 / 8.0 * 5.0 + 1);
            tagRect.setBottom(rcf.top() - 3);

            for (auto iter = info.tagMap.constBegin(); iter != info.tagMap.constEnd(); ++iter) {
                QDateTime dateTime = iter.key();
                if (dateTime.date() != info.playbackDate) {
                    continue;
                }
                if (dateTime < info.beginDateTime) {
                    continue;
                }
                if (dateTime > info.endDateTime) {
                    continue;
                }

                uint beginSec = dateTime.toTime_t();

                int x = padding + (beginSec - timeLineBeginSec) * info.secWidth;
                tagRect.setLeft(x - tagRect.height() / 2 - 2);
                tagRect.setRight(x + tagRect.height() / 2 + 2);

                painter.drawLine(QPointF(x, rcf.top() - 5), QPointF(x, rcf.top()));

                QImage tagImage(":/playback/playback/tag2.png");
                painter.drawImage(tagRect, tagImage);
            }
            painter.restore();
        }
    }
    painter.restore();

    //
    emit imageFinished(info.channel, image);

    //
    if (!m_imageInfoList.isEmpty()) {
        QMetaObject::invokeMethod(this, "onDrawImage", Qt::QueuedConnection);
    }
}
