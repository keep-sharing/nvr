#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>

namespace Ui {
class VideoWidget;
}

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

    void setChannel(int channel);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    Ui::VideoWidget *ui;

    int m_channel = -1;
};

#endif // VIDEOWIDGET_H
