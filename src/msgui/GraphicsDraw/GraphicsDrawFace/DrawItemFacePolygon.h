#ifndef DRAWITEMFACEPOLYGON_H
#define DRAWITEMFACEPOLYGON_H

#include "DrawItemPolygon.h"
class DrawItemFacePolygon : public DrawItem_Polygon {
public:
    enum {
        Type = UserType + 2
    };
    explicit DrawItemFacePolygon(QGraphicsItem *parent = nullptr);
    void setIsFinished(bool value);
    int type() const override;

    void setIsEnable(bool isEnable);
    void setPoints(const QVector<QPointF> &pointVect);

protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    QColor lineColor() override;

    void showText() override;
    void clearText() override;

    void addPointItem(const QPointF &p);
private:
    void showStayValue();
    void hideStayValue();

private:

    QGraphicsTextItem *m_textItem;
};

#endif // DRAWITEMFACEPOLYGON_H
