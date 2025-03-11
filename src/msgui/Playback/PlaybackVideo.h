#ifndef PLAYBACKVIDEO_H
#define PLAYBACKVIDEO_H

#include <QWidget>
#include "GraphicsItemPosText.h"

class DrawView;
class GraphicsScene;

namespace Ui {
class PlaybackVideo;
}

class PlaybackVideo : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackVideo(QWidget *parent = 0);
    ~PlaybackVideo();

    void setChannel(int channel);
    int channel() const;
    void setSplitChannel(int channel, int sid);
    void showNoResource(bool show);

    //每个page内唯一的，主要用于和底层通信
    void setIndexInPage(int index);
    int indexInPage() const;
    int vapiWinId();

    void setSid(int sid);
    int sid();

    QRect globalGeomery();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setChannelText(const QString &text);

signals:
    void mouseClicked(int channel);
    void mouseDoubleClicked(int channel);

private slots:
    void onNoResourceChanged(int winid, int bNoResource);
    //pos
    void onPosClicked(bool show);
    void onPauseClicked(bool pause);
    void onPosDataReceived(PosData data);

private:
    Ui::PlaybackVideo *ui;

    int m_channel = -1;
    int m_indexInPage = -1;
    int m_sid = -1;

    //
    DrawView *m_viewPos = nullptr;
    GraphicsScene *m_scenePos = nullptr;
    QMap<int, GraphicsItemPosText *> m_itemPosMap;
};

#endif // PLAYBACKVIDEO_H
