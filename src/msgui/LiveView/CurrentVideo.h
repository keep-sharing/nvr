#ifndef CURRENTVIDEO_H
#define CURRENTVIDEO_H

#include <QWidget>

namespace Ui {
class CurrentVideo;
}

class CurrentVideo : public QWidget
{
    Q_OBJECT

public:
    explicit CurrentVideo(QWidget *parent = 0);
    ~CurrentVideo();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *) override;

private:
    Ui::CurrentVideo *ui;
    int m_borderWidth = 4;
};

#endif // CURRENTVIDEO_H
