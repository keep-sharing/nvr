#ifndef VIDEOLAYOUT_H
#define VIDEOLAYOUT_H

#include "VideoWidget.h"
#include <QMap>
#include <QWidget>

namespace Ui {
class VideoLayout;
}

class VideoLayout : public QWidget {
    Q_OBJECT

public:
    explicit VideoLayout(QWidget *parent = nullptr);
    ~VideoLayout();

public slots:
    void setVideoLayout(int row, int column);

private:
    Ui::VideoLayout *ui;

    QMap<int, VideoWidget *> m_mapVideoWidget;
};

#endif // VIDEOLAYOUT_H
