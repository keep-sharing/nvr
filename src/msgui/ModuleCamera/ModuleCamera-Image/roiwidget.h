#ifndef ROIWIDGET_H
#define ROIWIDGET_H

#include "maskwidget.h"

class RoiWidget : public MaskWidget
{
    Q_OBJECT
public:
    explicit RoiWidget(QWidget *parent = nullptr);

    void setIndex(int index);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:

private:
    int m_index = -1;
};

#endif // ROIWIDGET_H
