#ifndef GRAPHICSTRACKPOLYLINEITEM_H
#define GRAPHICSTRACKPOLYLINEITEM_H

#include "TrackInfo.h"
#include <QGraphicsItem>

class GraphicsTrackPolylineItem : public QGraphicsItem {
public:
    explicit GraphicsTrackPolylineItem(QGraphicsItem *item = nullptr);

    void setTrackInfo(const TrackInfo &info);

    bool valid() const;
    void setValid(bool valid);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    bool m_valid = false;
    TrackInfo m_info;
    QPainterPath m_path;
};

#endif // GRAPHICSTRACKPOLYLINEITEM_H
