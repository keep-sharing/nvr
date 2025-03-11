#ifndef VIDEOCONTAINER_H
#define VIDEOCONTAINER_H

#include <QWidget>

class LiveVideo;

namespace Ui {
class VideoContainer;
}

class VideoContainer : public QWidget {
    Q_OBJECT

public:
    explicit VideoContainer(int index, QWidget *parent = nullptr);
    ~VideoContainer();

    int index() const;

    void setGlobalIndex(int index);
    int globalIndex() const;

    void setScreen(int screen);
    int screen() const;

    void setVideo(LiveVideo *video);
    void clearVideo();
    LiveVideo *video() const;

    QRect globalGeometry() const;

    int vapiWinId() const;

    void showLayoutButton();
    void hideLayoutButton();

protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
    void resizeEvent(QResizeEvent *) override;

    void mousePressEvent(QMouseEvent *) override;

signals:
    void clicked(int channel);

private slots:
    void onDisplayColorChanged(const QColor &color);
    void on_toolButtonLayout_clicked();

private:
    Ui::VideoContainer *ui;

    int m_index = -1;
    int m_globalIndex = -1;
    int m_screen = -1;
    LiveVideo *m_video = nullptr;
};

#endif // VIDEOCONTAINER_H
