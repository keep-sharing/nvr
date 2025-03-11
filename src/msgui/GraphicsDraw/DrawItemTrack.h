#ifndef DRAWITEMTRACK_H
#define DRAWITEMTRACK_H

#include "TrackInfo.h"
#include <QGraphicsRectItem>

class MessageReceive;
class GraphicsTrackRectItem;
class GraphicsTrackPolylineItem;

struct ms_personpoint_info;

class DrawItemTrack : public QGraphicsRectItem {
public:
    explicit DrawItemTrack(QGraphicsItem *parent = nullptr);

    void setTrackMode(TrackMode mode);
    void dealMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT(MessageReceive *message);

    void dealPointData(ms_personpoint_info *info);
    QPointF physicalPos(qreal x, qreal y);

    void clearAllItem();

private:
    TrackMode m_mode = TrackModeVca;

    QList<QColor> m_lineColorList;

    int m_realWidth = 0;
    int m_realHeight = 0;

    QMap<int, TrackInfo> m_trackInfoMap;
    QMap<int, GraphicsTrackRectItem *> m_rectMap;
    QMap<int, GraphicsTrackPolylineItem *> m_polylineMap;
};

#endif // DRAWITEMTRACK_H
