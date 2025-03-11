#ifndef CUSTOMLAYOUTRUBBERBAND_H
#define CUSTOMLAYOUTRUBBERBAND_H

#include <QGraphicsRectItem>

class CustomLayoutRubberBand : public QGraphicsRectItem
{
public:
    explicit CustomLayoutRubberBand(QGraphicsItem *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif // CUSTOMLAYOUTRUBBERBAND_H
