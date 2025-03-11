#ifndef LiveViewPccupancyDataWidget_H
#define LiveViewPccupancyDataWidget_H

#include <QWidget>

namespace Ui {
class LiveViewPccupancyDataWidget;
}

class LiveViewPccupancyDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LiveViewPccupancyDataWidget(QWidget *parent = nullptr);
    ~LiveViewPccupancyDataWidget();
    void setIcon(QString text);
    void setItemText(QString text);
    void setValue(QString text);
    void setIconVisible(bool isVisible);
    void setTextSizeSmall();
    void setTextSizeGreat();

private:
    Ui::LiveViewPccupancyDataWidget *ui;
};

#endif // LiveViewPccupancyDataWidget_H
