#ifndef CUSTOMLAYOUTBUTTON_H
#define CUSTOMLAYOUTBUTTON_H

#include "mytoolbutton.h"

class CustomLayoutButton : public MyToolButton {
    Q_OBJECT

public:
    enum TrianglePosition {
        Right,
        BottomRight
    };

    explicit CustomLayoutButton(QWidget *parent = nullptr);

    void setTrianglePosition(CustomLayoutButton::TrianglePosition position);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void drawRightTriangle(QPainter *painter);
    void drawBottomRightTriangle(QPainter *painter);

signals:

private:
    TrianglePosition m_position = Right;
};

#endif // CUSTOMLAYOUTBUTTON_H
