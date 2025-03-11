#ifndef GRAPHICSTRACKRECTITEM_H
#define GRAPHICSTRACKRECTITEM_H

#include "TrackInfo.h"
#include <QGraphicsItem>

class GraphicsTrackRectItem : public QGraphicsItem {
public:
    explicit GraphicsTrackRectItem(QGraphicsItem *parent = nullptr);

    void setTrackInfo(const TrackInfo &info);

    bool valid() const;
    void setValid(bool valid);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    bool m_valid = false;
    TrackInfo m_info;
};

#endif // GRAPHICSTRACKRECTITEM_H
