#ifndef BLCWIDGET_H
#define BLCWIDGET_H

#include "maskwidget.h"

enum BlcType
{
    BlcCustomize,
    BlcCentre
};

class BlcWidget : public MaskWidget
{
    Q_OBJECT
public:
    explicit BlcWidget(QWidget *parent = nullptr);

    void setBlcType(BlcType type);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:

public slots:

private:
    BlcType m_blcType;
};

#endif // BLCWIDGET_H
