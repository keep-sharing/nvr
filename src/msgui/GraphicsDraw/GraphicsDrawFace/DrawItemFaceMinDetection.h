#ifndef DRAWITEMFACEMINDETECTION_H
#define DRAWITEMFACEMINDETECTION_H

#include "DrawItemMotion.h"

class DrawItemFaceMinDetection : public QGraphicsRectItem
{

public:
    enum {
        Type = UserType + 3
    };
    explicit DrawItemFaceMinDetection(QGraphicsItem *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setObjectSize(int value);
    int type() const override;
private:
    void drawObjectSize(QPainter *painter);
    int m_objectSize = -1;

};

#endif // DRAWITEMFACEMINDETECTION_H
