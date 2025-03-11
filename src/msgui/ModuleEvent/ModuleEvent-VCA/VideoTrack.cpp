#include "VideoTrack.h"
#include "MsDevice.h"
#include "centralmessage.h"
#include <QPainter>
#include <QTimer>
#include <QtDebug>

VideoTrack::VideoTrack(QWidget *parent)
    : MsWidget(parent)
{
    m_rcColor = QColor(24, 190, 18);
    m_pointColor = QColor(255, 0, 0);
    m_lineColorList.append(QColor(238, 235, 18));
    m_lineColorList.append(QColor(23, 18, 214));
    m_lineColorList.append(Qt::cyan);
    m_lineColorList.append(Qt::magenta);
    m_lineColorList.append(Qt::gray);
    m_lineColorList.append(Qt::darkYellow);
    m_lineColorList.append(Qt::darkBlue);
    m_lineColorList.append(Qt::darkCyan);
    m_lineColorList.append(Qt::darkMagenta);
    m_lineColorList.append(Qt::darkGray);

    m_trackTimer = new QTimer(this);
    connect(m_trackTimer, SIGNAL(timeout()), this, SLOT(onTrackTimer()));
}

void VideoTrack::setCurrentChannel(int channel)
{
    m_channel = channel;
}

bool VideoTrack::isTrackEnable()
{
    return m_isTrackEnable;
}

void VideoTrack::setTrackEnable(bool enable)
{
    m_isTrackEnable = enable;
    if (m_isTrackEnable) {
        m_trackTimer->start(300);
    } else {
        m_trackTimer->stop();
        clear();
    }
}

void VideoTrack::clear()
{
    for (QMap<int, TrackInfo>::iterator iter = m_trackInfoMap.begin(); iter != m_trackInfoMap.end(); ++iter) {
        TrackInfo &trackInfo = iter.value();
        trackInfo.vaild = false;
    }
    update();
}

void VideoTrack::setMode(VideoTrack::Mode mode)
{
    m_trackMode = mode;
}

void VideoTrack::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VAC_PERSONPOINT:
        ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(message);
        break;
    }
}

void VideoTrack::ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(MessageReceive *message)
{
    if (!isVisible()) {
        return;
    }
    ms_personpoint_info *info = (ms_personpoint_info *)message->data;
    if (info == nullptr) {
        qWarning() << QString("VideoTrack::ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT, data is null.");
        return;
    }
    if (info->chanid != m_channel) {
        return;
    }
    if (info->person_point_cnt < 1) {
        m_trackInfoMap.clear();
        update();
        return;
    }
    m_realWidth = info->width;
    m_realHeight = info->height;
    //
    for (QMap<int, TrackInfo>::iterator iter = m_trackInfoMap.begin(); iter != m_trackInfoMap.end(); ++iter) {
        TrackInfo &trackInfo = iter.value();
        trackInfo.vaild = false;
    }
    //
    for (int i = 0; i < info->person_point_cnt; ++i) {
        const personpoint_info &personPoint = info->point[i];
        if (personPoint.startX == 0 && personPoint.startY == 0 && personPoint.stopX == 0 && personPoint.stopY == 0) {
            continue;
        }

        TrackInfo &trackInfo = m_trackInfoMap[personPoint.id];
        trackInfo.vaild = true;
        trackInfo.id = personPoint.id;
        if (!trackInfo.lineColor.isValid()) {
            trackInfo.lineColor = m_lineColorList.at(i);
        }
        QPoint point1 = physicalPos(personPoint.startX, personPoint.startY);
        QPoint point2 = physicalPos(personPoint.stopX, personPoint.stopY);
        trackInfo.rc = QRect(point1, point2);
        //
        QPoint tempPoint = trackInfo.rc.center();
        if (trackInfo.trackPoints.isEmpty()) {
            trackInfo.trackPoints.append(tempPoint);
        } else {
            const QPoint &lastPoint = trackInfo.trackPoints.last();
            QLineF line(lastPoint, tempPoint);
            if (line.length() > width() / 20) {
                trackInfo.trackPoints.append(tempPoint);
            }
        }
    }

    if (!m_trackInfoMap.isEmpty()) {
        update();
    }
}

void VideoTrack::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);

    QMap<int, TrackInfo>::const_iterator iter = m_trackInfoMap.constBegin();
    for (; iter != m_trackInfoMap.constEnd(); ++iter) {
        const TrackInfo &trackInfo = iter.value();
        if (!trackInfo.vaild) {
            continue;
        }
        //rect
        painter.save();
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(m_rcColor, 2));
        painter.drawRect(trackInfo.rc);
        painter.restore();
        //ptz track只画框
        if (m_trackMode != ModePtz) {
            //id
            painter.save();
            QPoint textPoint = trackInfo.rc.normalized().topLeft();
            textPoint.setX(textPoint.x() + 3);
            textPoint.setY(textPoint.y() + 15);
            painter.setPen(QColor("#EFEC11"));
            painter.drawText(textPoint, QString("ID: %1").arg(trackInfo.id));
            painter.restore();
            //line
            painter.save();
            painter.setPen(QPen(trackInfo.lineColor, 2));
            painter.drawPolyline(QPolygon(trackInfo.trackPoints));
            painter.restore();
            //point
            painter.save();
            painter.setPen(QPen(m_pointColor, 4));
            painter.drawPoints(QPolygon(trackInfo.trackPoints));
            painter.restore();
        }
    }
}

void VideoTrack::showEvent(QShowEvent *)
{
    if (m_isTrackEnable) {
        m_trackTimer->start();
    }
}

void VideoTrack::hideEvent(QHideEvent *)
{
    m_trackTimer->stop();
    clear();
}

QPoint VideoTrack::physicalPos(int x, int y)
{
    return QPoint((qreal)x / m_realWidth * width(), (qreal)y / m_realHeight * height());
}

void VideoTrack::onTrackTimer()
{
    switch (m_trackMode) {
    case ModeVca:
        sendMessage(REQUEST_FLAG_GET_VAC_PERSONPOINT, (void *)&m_channel, sizeof(int));
        break;
    case ModePtz:
        sendMessage(REQUEST_FLAG_GET_AUTO_TRACK_POINT, (void *)&m_channel, sizeof(int));
        break;
    }
}
