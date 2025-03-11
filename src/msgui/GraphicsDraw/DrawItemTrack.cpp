#include "DrawItemTrack.h"
#include "GraphicsTrackRectItem.h"
#include "GraphicsTrackPolylineItem.h"
#include "MyDebug.h"
#include "TrackInfo.h"
#include "centralmessage.h"
#include <QPainter>

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpControl.h"
#endif

extern "C" {
#include "msg.h"
}

DrawItemTrack::DrawItemTrack(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
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
}

void DrawItemTrack::setTrackMode(TrackMode mode)
{
    m_mode = mode;
}

void DrawItemTrack::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VAC_PERSONPOINT:
        ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACK_POINT:
        ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT(message);
        break;
    default:
        break;
    }
}

void DrawItemTrack::ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(MessageReceive *message)
{
    ms_personpoint_info *info = static_cast<ms_personpoint_info *>(message->data);
    if (info == nullptr) {
        qWarning() << QString("DrawTrackItem::ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT, data is null.");
        return;
    }

    qMsCDebug("qt_track_vca") << QString("----channel: %1, point count: %2").arg(info->chanid).arg(info->person_point_cnt);
    dealPointData(info);
}

void DrawItemTrack::ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT(MessageReceive *message)
{
    ms_personpoint_info *info = static_cast<ms_personpoint_info *>(message->data);
    if (info == nullptr) {
        qWarning() << QString("DrawTrackItem::ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT, data is null.");
        return;
    }

    qMsCDebug("qt_track_vca") << QString("----channel: %1, point count: %2").arg(info->chanid).arg(info->person_point_cnt);
    dealPointData(info);
}

void DrawItemTrack::dealPointData(ms_personpoint_info *info)
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    if (FisheyeDewarpControl::fisheyeChannel(FisheyeDewarpControl::ModeLiveview).channel == info->chanid) {
        clearAllItem();
        return;
    }
#endif
    //
    if (info->person_point_cnt < 1) {
        clearAllItem();
        return;
    }
    m_realWidth = info->width;
    m_realHeight = info->height;
    //
    for (QMap<int, TrackInfo>::iterator iter = m_trackInfoMap.begin(); iter != m_trackInfoMap.end(); ++iter) {
        TrackInfo &trackInfo = iter.value();
        trackInfo.vaild = false;
    }
    for (auto iter = m_rectMap.constBegin(); iter != m_rectMap.constEnd(); ++iter) {
        GraphicsTrackRectItem *item = iter.value();
        item->setValid(false);
    }
    for (auto iter = m_polylineMap.constBegin(); iter != m_polylineMap.constEnd(); ++iter) {
        GraphicsTrackPolylineItem *item = iter.value();
        item->setValid(false);
    }
    //生成数据
    for (int i = 0; i < info->person_point_cnt; ++i) {
        const personpoint_info &personPoint = info->point[i];
        if (personPoint.startX == 0 && personPoint.startY == 0 && personPoint.stopX == 0 && personPoint.stopY == 0) {
            continue;
        }

        TrackInfo &trackInfo = m_trackInfoMap[personPoint.id];
        trackInfo.vaild = true;
        trackInfo.id = personPoint.id;
        trackInfo.mode = m_mode;
        if (!trackInfo.lineColor.isValid()) {
            trackInfo.lineColor = m_lineColorList.at(i % 10);
        }
        QPointF point1 = physicalPos(personPoint.startX, personPoint.startY);
        QPointF point2 = physicalPos(personPoint.stopX, personPoint.stopY);
        trackInfo.rc = QRectF(point1, point2);
        //
        QPointF tempPoint = trackInfo.rc.center();
        if (trackInfo.trackPoints.isEmpty()) {
            trackInfo.trackPoints.append(tempPoint);
        } else {
            const QPointF &lastPoint = trackInfo.trackPoints.last();
            QLineF line(lastPoint, tempPoint);
            if (line.length() > rect().width() / 20) {
                trackInfo.trackPoints.append(tempPoint);
            }
        }
    }

    if (m_trackInfoMap.isEmpty()) {
        clearAllItem();
    }

    //绘制item
    for (auto iter = m_trackInfoMap.constBegin(); iter != m_trackInfoMap.constEnd(); ++iter) {
        const TrackInfo &trackInfo = iter.value();
        if (!trackInfo.vaild) {
            continue;
        }

        const int id = trackInfo.id;

        GraphicsTrackRectItem *rectItem = m_rectMap.value(id);
        if (!rectItem) {
            rectItem = new GraphicsTrackRectItem(this);
            m_rectMap.insert(id, rectItem);
        }
        rectItem->setTrackInfo(trackInfo);
        rectItem->setValid(true);
        rectItem->show();

        GraphicsTrackPolylineItem *polylineItem = m_polylineMap.value(id);
        if (!polylineItem) {
            polylineItem = new GraphicsTrackPolylineItem(this);
            m_polylineMap.insert(id, polylineItem);
        }
        polylineItem->setTrackInfo(trackInfo);
        polylineItem->setValid(true);
        polylineItem->show();
    }

    //清除无效item
    for (auto iter = m_rectMap.begin(); iter != m_rectMap.end();) {
        GraphicsTrackRectItem *item = iter.value();
        if (!item->valid()) {
            iter = m_rectMap.erase(iter);
            item->hide();
        } else {
            ++iter;
        }
    }
    for (auto iter = m_polylineMap.begin(); iter != m_polylineMap.end();) {
        GraphicsTrackPolylineItem *item = iter.value();
        if (!item->valid()) {
            iter = m_polylineMap.erase(iter);
            item->hide();
        } else {
            ++iter;
        }
    }
}

QPointF DrawItemTrack::physicalPos(qreal x, qreal y)
{
    return QPointF(x / m_realWidth * rect().width(), y / m_realHeight * rect().height());
}

void DrawItemTrack::clearAllItem()
{
    for (auto iter = m_rectMap.constBegin(); iter != m_rectMap.constEnd(); ++iter) {
        GraphicsTrackRectItem *item = iter.value();
        item->hide();
    }
    m_rectMap.clear();

    for (auto iter = m_polylineMap.constBegin(); iter != m_polylineMap.constEnd(); ++iter) {
        GraphicsTrackPolylineItem *item = iter.value();
        item->hide();
    }
    m_polylineMap.clear();

    m_trackInfoMap.clear();
}
