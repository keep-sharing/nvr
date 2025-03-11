#ifndef VIDEOSCENE_H
#define VIDEOSCENE_H

#include "MsGraphicsScene.h"
#include "DrawItemTrack.h"
#include <QEventLoop>
#include <QTimer>
#include "DrawMultiControlPolygon.h"
#include "GraphicsDrawLineCrossing.h"

extern "C" {
#include "msg.h"
}

class LiveVideo;
class DrawVideoGrid;
class DrawVideoLine;
class DrawItemObjectBox;
class DrawVideoPolygon;
class GraphicsMultiRegionItem;

class VideoScene : public MsGraphicsScene {
    Q_OBJECT

public:
    explicit VideoScene(QObject *parent = nullptr);

    void setChannel(int channel);

    void setVcaAlarm(const MS_VCA_ALARM &alarm);

    //
    void showVcaRects(VacDynamicBoxALL *info);
    void hideVcaRects();

    //
    void showRegionRects(RegionalRectInfo *info);
    void hideRegionRects();

    //
    void showRegionAlarm(const RegionalAlarmInfo &info);

    //
    void dealMessage(MessageReceive *message);
    void dealEventDetectionRegionMessage(MessageReceive *message);

    void processMessage(MessageReceive *message) override;

    /**track begin**/
public:
    void setTrackMode(TrackMode mode, bool enable);
    bool isTrackEnable() const;

private:
    void ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT(MessageReceive *message);

    void dealTrackPointData(ms_personpoint_info *info);
    QPointF trackPhysicalPos(qreal x, qreal y);
    void clearAllTrackItem();

private slots:
    void onTimerCheckEnable();
    void onTimerGetPoint();

private:
    TrackMode m_trackMode = TrackModeNone;
    QMap<TrackMode, bool> m_trackModeMap;
    QTimer *m_timerCheckEnable = nullptr;
    QTimer *m_timerGetPoint = nullptr;
    DrawItemTrack *m_trackItem = nullptr;

    QList<QColor> m_trackLineColorList;

    int m_trackRealWidth = 0;
    int m_trackRealHeight = 0;

    QMap<int, TrackInfo> m_trackInfoMap;
    QMap<int, GraphicsTrackRectItem *> m_rectMap;
    QList<GraphicsTrackRectItem *> m_rectPool;
    QMap<int, GraphicsTrackPolylineItem *> m_polylineMap;
    QList<GraphicsTrackPolylineItem *> m_polylinePool;
    /**track end**/

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_REGIONENTRANCE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_REGIONEXIT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LOITERING(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(MessageReceive *message);

    void dealVcaGridEvent(MessageReceive *message, int flag);

    void hideRegionItems();
    void hideAllItems();

    //vca box
    bool checkVcaBoxEvent(int event) const;
    bool checkVcaBoxAlarm(int alarm) const;

signals:

public slots:
    void onVideoShow();
    void onVideoHide();

    void onConnectedStateChanged(bool connected);

private slots:
    void onSceneRectChanged(const QRectF &rect);

    void onLiveViewEventDetectionChanged(int event);

    void updateDynamicDisplay(int type, int channel);

private:
    LiveVideo *m_video = nullptr;

    QEventLoop m_eventLoop;
    int m_channel = -1;

    DrawVideoGrid *m_itemGrid = nullptr;
    DrawVideoLine *m_itemLine = nullptr;
    GraphicsDrawLineCrossing *m_drawLine = nullptr;
    DrawMultiControlPolygon *m_drawMultiControl = nullptr;
    QList<GraphicsMultiRegionItem *> m_itemsRegionalPeopleCounting;

    //vca box
    QList<DrawItemObjectBox *> m_itemBoxs;
    bool m_hasVcaBoxShow = false;

    int m_eventFlags = 0;

    qint64 m_timestamp = 0;
};

#endif // VIDEOSCENE_H
