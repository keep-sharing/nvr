#ifndef COMMONVIDEO_H
#define COMMONVIDEO_H

#include "MsWidget.h"
#include "PosTextEdit.h"
#include "normallabel.h"
#include <QMap>
#include "GraphicsScene.h"

extern "C" {
#include "msg.h"
}

class DrawMask;
class DrawMotion;
class DrawView;
class GraphicsItemPosText;

namespace Ui {
class CommonVideo;
}

class CommonVideo : public MsWidget {
    Q_OBJECT

    enum MESSAGE_TYPE {
        MSG_PERMISSION, //无权限
        MSG_RESOURCE,   //无资源
        MSG_FACEPRIVACY //人脸隐私模式，无法预览
    };

public:
    explicit CommonVideo(QWidget *parent = 0);
    ~CommonVideo();

    static CommonVideo *instance();

    static QRect qtVideoGeometry();
    static void hideVideo();

    void showCurrentChannel(int channel);

    void showPixmap(const QPixmap &pixmap);
    void showPixmap();
    void hidePixmap();

    void playVideo(int channel);
    void stopVideo();
    void playbackVideo(int channel);

    static int stopAllVideo();
    static bool isPlaying();

    void setDrawWidget(QWidget *widget);
    void showDrawWidget(QWidget *widget);
    void showDrawWidget();
    void hideDrawWidget();
    void setDrawWidgetVisible(bool visible);
    void setDrawWidgetEnable(bool enable);

    void showDrawView();
    void hideDrawView();
    void setDrawScene(QGraphicsScene *scene);
    void showDrawScene(QGraphicsScene *scene);
    void removeDrawScene();
    void setDrawViewVisible(bool visible);
    void setDrawViewEnable(bool enable);

    void adjustVideoRegion();

    QRect videoFrameGeometry() const;

    //pos
    void setPosVisible(bool visible);
    void setPosPaused(bool pause);
    void clearPos();

    //
    void addGraphicsItem(QGraphicsItem *item);
    void removeGraphicsItem(QGraphicsItem *item);

    static void setBanOnBack(bool value);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void showMessage();
    void hideMessage();

    void setCurrentChannel(int channel);
    void setChannelName(const QString &name);

private slots:
    void onNoResourceChanged(int winid, int bNoResource);
    void onCameraFacePrivacyState(int channel, int state);

    //pos
    void onPosDataReceived(PosData data);

private:
    static CommonVideo *s_commonVideo;

    Ui::CommonVideo *ui;

    static req_pipmode2_s s_pipmode;
    static QRect s_nvrVideoGeometry;
    static QRect s_qtVideoGeometry;

    static int s_channel;
    static bool s_isPlaying;
    static bool s_banOnBack;

    //
    QLabel *m_pixmap = nullptr;

    //
    QLabel *m_labelMessage = nullptr;
    QMap<MESSAGE_TYPE, int> m_messageMap;

    //后面会移除draw widget，全部用drawView
    QWidget *m_drawWidget = nullptr;
    //
    DrawView *m_drawView = nullptr;
    GraphicsScene *m_drawScene = nullptr;

    //
    DrawView *m_commonView = nullptr;
    GraphicsScene *m_commonScene = nullptr;
    //pos
    QMap<int, GraphicsItemPosText *> m_itemPosMap;
};

#endif // COMMONVIDEO_H
