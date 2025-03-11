#include "VideoScene.h"
#include "DisplaySetting.h"
#include "DrawItemObjectBox.h"
#include "DrawVideoGrid.h"
#include "DrawVideoLine.h"
#include "DynamicDisplayData.h"
#include "EventDetectionRegionManager.h"
#include "GraphicsMultiRegionItem.h"
#include "GraphicsTrackPolylineItem.h"
#include "GraphicsTrackRectItem.h"
#include "LiveVideo.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "centralmessage.h"

#ifdef MS_FISHEYE_SOFT_DEWARP
#include "FisheyeDewarpControl.h"
#endif

extern "C" {
#include "msg.h"
}

const int MaxDynamicRectCount = 60;

VideoScene::VideoScene(QObject *parent)
    : MsGraphicsScene(parent)
{
    m_video = static_cast<LiveVideo *>(parent);

    //track
    m_trackLineColorList.append(QColor(238, 235, 18));
    m_trackLineColorList.append(QColor(23, 18, 214));
    m_trackLineColorList.append(Qt::cyan);
    m_trackLineColorList.append(Qt::magenta);
    m_trackLineColorList.append(Qt::gray);
    m_trackLineColorList.append(Qt::darkYellow);
    m_trackLineColorList.append(Qt::darkBlue);
    m_trackLineColorList.append(Qt::darkCyan);
    m_trackLineColorList.append(Qt::darkMagenta);
    m_trackLineColorList.append(Qt::darkGray);

    m_timerCheckEnable = new QTimer(this);
    connect(m_timerCheckEnable, SIGNAL(timeout()), this, SLOT(onTimerCheckEnable()));

    m_timerGetPoint = new QTimer(this);
    connect(m_timerGetPoint, SIGNAL(timeout()), this, SLOT(onTimerGetPoint()));
    m_timerGetPoint->setInterval(200);

    //
    m_itemGrid = new DrawVideoGrid();
    addItem(m_itemGrid);
    m_itemGrid->hide();

    m_drawMultiControl = new DrawMultiControlPolygon();
    addItem(m_drawMultiControl);
    m_drawMultiControl->setAlarmable(true);
    m_drawMultiControl->hide();

    m_drawLine = new GraphicsDrawLineCrossing();
    addItem(m_drawLine);
    m_drawLine->hide();

    m_itemLine = new DrawVideoLine();
    addItem(m_itemLine);
    m_itemLine->hide();

    for (int i = 0; i < 4; ++i) {
        GraphicsMultiRegionItem *item = new GraphicsMultiRegionItem();
        addItem(item);
        item->hide();
        m_itemsRegionalPeopleCounting.append(item);
    }

    for (int i = 0; i < MaxDynamicRectCount; ++i) {
        DrawItemObjectBox *itemBox = new DrawItemObjectBox();
        itemBox->setZValue(1);
        addItem(itemBox);
        itemBox->hide();
        m_itemBoxs.append(itemBox);
    }

    connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));

    //
    m_eventFlags = qMsNvr->displayInfo().event_region;
    connect(qMsNvr, SIGNAL(displayEventDetectionChanged(int)), this, SLOT(onLiveViewEventDetectionChanged(int)));

    connect(&gDynamicDisplayData, SIGNAL(dynamicDataChanged(int, int)), this, SLOT(updateDynamicDisplay(int, int)));
}

void VideoScene::setChannel(int channel)
{
    m_channel = channel;
}

void VideoScene::setTrackMode(TrackMode mode, bool enable)
{
    m_trackModeMap.insert(mode, enable);

    bool hasEnable = false;
    for (auto iter = m_trackModeMap.constBegin(); iter != m_trackModeMap.constEnd(); ++iter) {
        hasEnable = iter.value();
        if (hasEnable) {
            m_trackMode = iter.key();
            break;
        }
    }
    if (hasEnable) {
        m_timerGetPoint->start();
    } else {
        m_timerGetPoint->stop();
    }
}

bool VideoScene::isTrackEnable() const
{
    bool hasEnable = false;
    for (auto iter = m_trackModeMap.constBegin(); iter != m_trackModeMap.constEnd(); ++iter) {
        hasEnable = iter.value();
        if (hasEnable) {
            break;
        }
    }
    return hasEnable;
}

void VideoScene::ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(MessageReceive *message)
{
    ms_personpoint_info *info = static_cast<ms_personpoint_info *>(message->data);
    if (info == nullptr) {
        qWarning() << QString("DrawTrackItem::ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT, data is null.");
        return;
    }

    qMsCDebug("qt_track_vca") << QString("----channel: %1, point count: %2").arg(info->chanid).arg(info->person_point_cnt);
    dealTrackPointData(info);
}

void VideoScene::ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT(MessageReceive *message)
{
    ms_personpoint_info *info = static_cast<ms_personpoint_info *>(message->data);
    if (info == nullptr) {
        qWarning() << QString("DrawTrackItem::ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT, data is null.");
        return;
    }

    qMsCDebug("qt_track_vca") << QString("----channel: %1, point count: %2").arg(info->chanid).arg(info->person_point_cnt);
    dealTrackPointData(info);
}

void VideoScene::dealTrackPointData(ms_personpoint_info *info)
{
#ifdef MS_FISHEYE_SOFT_DEWARP
    if (FisheyeDewarpControl::fisheyeChannel(FisheyeDewarpControl::ModeLiveview).channel == info->chanid) {
        clearAllTrackItem();
        return;
    }
#endif
    //
    if (info->person_point_cnt < 1) {
        clearAllTrackItem();
        return;
    }
    m_trackRealWidth = info->width;
    m_trackRealHeight = info->height;
    //
    for (QMap<int, TrackInfo>::iterator iter = m_trackInfoMap.begin(); iter != m_trackInfoMap.end(); ++iter) {
        TrackInfo &trackInfo = iter.value();
        trackInfo.vaild = false;
    }
    for (auto iter = m_rectMap.constBegin(); iter != m_rectMap.constEnd(); ++iter) {
        GraphicsTrackRectItem *item = iter.value();
        item->setValid(false);
        item->hide();
    }
    for (auto iter = m_polylineMap.constBegin(); iter != m_polylineMap.constEnd(); ++iter) {
        GraphicsTrackPolylineItem *item = iter.value();
        item->setValid(false);
        item->hide();
    }
    //生成数据
    for (int i = 0; i < info->person_point_cnt; ++i) {
        const personpoint_info &personPoint = info->point[i];
        if (personPoint.startX == 0 && personPoint.startY == 0 && personPoint.stopX == 0 && personPoint.stopY == 0) {
            continue;
        }

        TrackInfo &trackInfo = m_trackInfoMap[personPoint.id];
        trackInfo.vaild = true;
        trackInfo.id = personPoint.id;
        trackInfo.mode = m_trackMode;
        if (!trackInfo.lineColor.isValid()) {
            trackInfo.lineColor = m_trackLineColorList.at(i % 10);
        }
        QPointF point1 = trackPhysicalPos(personPoint.startX, personPoint.startY);
        QPointF point2 = trackPhysicalPos(personPoint.stopX, personPoint.stopY);
        trackInfo.rc = QRectF(point1, point2);
        //
        QPointF tempPoint = trackInfo.rc.center();
        if (trackInfo.trackPoints.isEmpty()) {
            trackInfo.trackPoints.append(tempPoint);
        } else {
            const QPointF &lastPoint = trackInfo.trackPoints.last();
            QLineF line(lastPoint, tempPoint);
            if (line.length() > sceneRect().width() / 20) {
                trackInfo.trackPoints.append(tempPoint);
            }
        }
    }

    if (m_trackInfoMap.isEmpty()) {
        clearAllTrackItem();
    }

    //绘制item
    for (auto iter = m_trackInfoMap.constBegin(); iter != m_trackInfoMap.constEnd(); ++iter) {
        const TrackInfo &trackInfo = iter.value();
        if (!trackInfo.vaild) {
            continue;
        }

        const int id = trackInfo.id;

        GraphicsTrackRectItem *rectItem = m_rectMap.value(id);
        if (!rectItem) {
            if (!m_rectPool.isEmpty()) {
                rectItem = m_rectPool.takeFirst();
            } else {
                rectItem = new GraphicsTrackRectItem();
                addItem(rectItem);
            }
            m_rectMap.insert(id, rectItem);
        }
        rectItem->setTrackInfo(trackInfo);
        rectItem->setValid(true);
        rectItem->show();

        GraphicsTrackPolylineItem *polylineItem = m_polylineMap.value(id);
        if (!polylineItem) {
            if (!m_polylinePool.isEmpty()) {
                polylineItem = m_polylinePool.takeFirst();
            } else {
                polylineItem = new GraphicsTrackPolylineItem();
                addItem(polylineItem);
            }
            m_polylineMap.insert(id, polylineItem);
        }
        polylineItem->setTrackInfo(trackInfo);
        polylineItem->setValid(true);
        polylineItem->show();
    }

    //清除无效item
    for (auto iter = m_rectMap.begin(); iter != m_rectMap.end();) {
        GraphicsTrackRectItem *item = iter.value();
        if (!item->valid()) {
            iter = m_rectMap.erase(iter);
            item->hide();
            m_rectPool.append(item);
        } else {
            ++iter;
        }
    }
    for (auto iter = m_polylineMap.begin(); iter != m_polylineMap.end();) {
        GraphicsTrackPolylineItem *item = iter.value();
        if (!item->valid()) {
            iter = m_polylineMap.erase(iter);
            item->hide();
            m_polylinePool.append(item);
        } else {
            ++iter;
        }
    }
}

QPointF VideoScene::trackPhysicalPos(qreal x, qreal y)
{
    return QPointF(x / m_trackRealWidth * sceneRect().width(), y / m_trackRealHeight * sceneRect().height());
}

void VideoScene::clearAllTrackItem()
{
    for (auto iter = m_rectMap.constBegin(); iter != m_rectMap.constEnd(); ++iter) {
        GraphicsTrackRectItem *item = iter.value();
        item->hide();
        m_rectPool.append(item);
    }
    m_rectMap.clear();

    for (auto iter = m_polylineMap.constBegin(); iter != m_polylineMap.constEnd(); ++iter) {
        GraphicsTrackPolylineItem *item = iter.value();
        item->hide();
        m_polylinePool.append(item);
    }
    m_polylineMap.clear();

    m_trackInfoMap.clear();
}

void VideoScene::setVcaAlarm(const MS_VCA_ALARM &alarm)
{
    switch (alarm.event) {
    case VCA_REGIONIN:
        if (m_eventFlags & DisplaySetting::EventRegionEntrance) {
            if (alarm.region == 0) {
                m_itemGrid->setState(alarm.alarm);
            } else {
                m_drawMultiControl->setAlarmState(alarm.alarm, alarm.region - 1);
            }
        }
        break;
    case VCA_REGIONOUT:
        if (m_eventFlags & DisplaySetting::EventRegionExiting) {
            if (alarm.region == 0) {
                m_itemGrid->setState(alarm.alarm);
            } else {
                m_drawMultiControl->setAlarmState(alarm.alarm, alarm.region - 1);
            }
        }
        break;
    case VCA_ADVANCED_MOTION:
        if (m_eventFlags & DisplaySetting::EventMotionDetection) {
            if (alarm.region == 0) {
                m_itemGrid->setState(alarm.alarm);
            } else {
                m_drawMultiControl->setAlarmState(alarm.alarm, alarm.region - 1);
            }
        }
        break;
    case VCA_LOITERING:
        if (m_eventFlags & DisplaySetting::EventLoitering) {
            if (alarm.region == 0) {
                m_itemGrid->setState(alarm.alarm);
            } else {
                m_drawMultiControl->setAlarmState(alarm.alarm, alarm.region - 1);
            }
        }
        break;
    case VCA_OBJECT_LEFT:
    case VCA_OBJECT_REMOVE:
        if (m_eventFlags & DisplaySetting::EventObject) {
            if (alarm.region == 0) {
                m_itemGrid->setState(alarm.alarm);
            } else {
                m_drawMultiControl->setAlarmState(alarm.alarm, alarm.region - 1);
            }
        }
        break;
    case VCA_LINECROSS:
        if (m_eventFlags & DisplaySetting::EventLineCrossing) {
            if (alarm.region == 0) {
                m_itemLine->setLineCrossState(alarm.alarm, alarm.linenumber);
            } else {
                m_drawLine->setLineCrossState(alarm.alarm, alarm.linenumber);
            }
        }
        break;
    case VCA_PEOPLECNT:
        if (m_eventFlags & DisplaySetting::EventPeopleCounting) {
            if (alarm.palarm.line == 0) { //total
                m_drawLine->setLineCrossState(alarm.alarm, 0);
                m_drawLine->setLineCrossState(alarm.alarm, 1);
                m_drawLine->setLineCrossState(alarm.alarm, 2);
                m_drawLine->setLineCrossState(alarm.alarm, 3);
            } else {
                m_drawLine->setLineCrossState(alarm.alarm, alarm.palarm.line - 1);
            }
        }
        break;
    default:
        break;
    }
}

void VideoScene::showVcaRects(VacDynamicBoxALL *info)
{
    if (m_eventFlags & DisplaySetting::EventRegionalPeopleCounting) {
        return;
    }
    qint64 tempTimestamp = m_timestamp;
    m_timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if (m_timestamp - tempTimestamp <= 100 && info->typeId != 0) {
        return;
    }
    //update();
    for (int i = 0; i < MaxDynamicRectCount; ++i) {
        DrawItemObjectBox *item = m_itemBoxs.at(i);
        if (i >= info->typeId) {
            item->hide();
            continue;
        }
        const VacDynamicBoxS &box = info->dBox[i];
        if (gDebug.checkDebugCategory("qt_dynamic_box")) {
            qDebug() << QString("index: %1, id: %2, event: %3, nClass: %4, alarm: %5").arg(i).arg(box.id).arg(box.event).arg(box.nClass).arg(box.alarm);
        }
        //
        if (!checkVcaBoxEvent(box.event)) {
            item->hide();
            continue;
        }
        //
        if (info->resWidth <= 0 || info->resHeight <= 0) {
            qMsWarning() << QString("invalid resWidth(%1) or resHeight(%2)").arg(info->resWidth).arg(info->resHeight);
            continue;
        }
        qreal left = static_cast<qreal>(box.left) / info->resWidth * sceneRect().width();
        qreal top = static_cast<qreal>(box.top) / info->resHeight * sceneRect().height();
        qreal right = static_cast<qreal>(box.right) / info->resWidth * sceneRect().width();
        qreal bottom = static_cast<qreal>(box.bottom) / info->resHeight * sceneRect().height();
        QRectF rcf(QPointF(0, 0), QSize(static_cast<int>(right - left), static_cast<int>(bottom - top)));
        //【ID1050639】QT-人车框显示：IPC图像配置了翻转，导致NVR接收的坐标位置有误【NVR端可以兼容下，正确坐标IPC79版本修改】
        rcf = rcf.normalized();
        //
        if (rcf.isEmpty()) {
            item->hide();
            continue;
        }
        item->showBox(box.id, box.nClass, checkVcaBoxAlarm(box.alarm));
        item->setRect(rcf);
        item->setPos(left, top);
        item->show();
    }
}

void VideoScene::hideVcaRects()
{
    for (int i = 0; i < MaxDynamicRectCount; ++i) {
        DrawItemObjectBox *item = m_itemBoxs.at(i);
        item->hide();
    }
}

void VideoScene::showRegionRects(RegionalRectInfo *info)
{
    if (!(m_eventFlags & DisplaySetting::EventRegionalPeopleCounting)) {
        return;
    }
    for (int i = 0; i < MaxDynamicRectCount; ++i) {
        DrawItemObjectBox *item = m_itemBoxs.at(i);
        if (i >= info->regionNum) {
            item->hide();
            continue;
        }
        const RegionalRectBox &box = info->rect[i];
        if (gDebug.checkDebugCategory("qt_region_box")) {
            qDebug() << QString("index: %1, id: %2").arg(i).arg(box.id);
        }
        //
        if (info->resolutionWidth <= 0 || info->resolutionHeight <= 0) {
            qMsWarning() << QString("invalid resWidth(%1) or resHeight(%2)").arg(info->resolutionWidth).arg(info->resolutionHeight);
            continue;
        }
        qreal left = (qreal)box.left / info->resolutionWidth * sceneRect().width();
        qreal top = (qreal)box.top / info->resolutionHeight * sceneRect().height();
        qreal right = (qreal)box.right / info->resolutionWidth * sceneRect().width();
        qreal bottom = (qreal)box.bottom / info->resolutionHeight * sceneRect().height();
        QRectF rcf(QPointF(0, 0), QSize(right - left, bottom - top));
        //【ID1050639】QT-人车框显示：IPC图像配置了翻转，导致NVR接收的坐标位置有误【NVR端可以兼容下，正确坐标IPC79版本修改】
        rcf = rcf.normalized();
        //
        if (rcf.isEmpty()) {
            item->hide();
            continue;
        }
        item->showBox(box);
        item->setRect(rcf);
        item->setPos(left, top);
        item->show();
    }
}

void VideoScene::hideRegionRects()
{
    for (int i = 0; i < MaxDynamicRectCount; ++i) {
        DrawItemObjectBox *item = m_itemBoxs.at(i);
        item->hide();
    }
}

void VideoScene::showRegionAlarm(const RegionalAlarmInfo &info)
{
    for (int i = 0; i < m_itemsRegionalPeopleCounting.size(); ++i) {
        GraphicsMultiRegionItem *item = m_itemsRegionalPeopleCounting.at(i);
        item->updateRegionAlarm(info);
    }
}

void VideoScene::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void VideoScene::dealEventDetectionRegionMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_VCA_REGIONENTRANCE:
        ON_RESPONSE_FLAG_GET_VCA_REGIONENTRANCE(message);
        break;
    case RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION:
        ON_RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION(message);
        break;
    case RESPONSE_FLAG_GET_VCA_REGIONEXIT:
        ON_RESPONSE_FLAG_GET_VCA_REGIONEXIT(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LOITERING:
        ON_RESPONSE_FLAG_GET_VCA_LOITERING(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LINECROSSING:
        ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(message);
        break;
    case RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2:
        ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(message);
        break;
    case RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT:
        ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LEFTREMOVE:
        ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(message);
        break;
    case RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE:
        ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(message);
        break;
    }
}

void VideoScene::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VAC_PERSONPOINT:
        ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACK_POINT:
        ON_RESPONSE_FLAG_GET_AUTO_TRACK_POINT(message);
        break;
    default:
        break;
    }
}

void VideoScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);

#if 0
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::red, 2));
    painter->drawRect(sceneRect());
#endif
}

void VideoScene::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void VideoScene::ON_RESPONSE_FLAG_GET_VCA_REGIONENTRANCE(MessageReceive *message)
{
    dealVcaGridEvent(message, DisplaySetting::EventRegionEntrance);
}

void VideoScene::ON_RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION(MessageReceive *message)
{
    dealVcaGridEvent(message, DisplaySetting::EventMotionDetection);
}

void VideoScene::ON_RESPONSE_FLAG_GET_VCA_REGIONEXIT(MessageReceive *message)
{
    dealVcaGridEvent(message, DisplaySetting::EventRegionExiting);
}

void VideoScene::ON_RESPONSE_FLAG_GET_VCA_LOITERING(MessageReceive *message)
{
    dealVcaGridEvent(message, DisplaySetting::EventLoitering);
}

void VideoScene::ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(MessageReceive *message)
{
    hideAllItems();

    if (m_eventFlags & DisplaySetting::EventLineCrossing) {

        ms_linecrossing_info *info = (ms_linecrossing_info *)message->data;
        if (info) {
            if (info->chanid != m_video->channel()) {
                return;
            }
            if (info->enable) {
                m_itemLine->setLineCrossInfo(info);
                m_itemLine->show();
            }
        }
    }
}

void VideoScene::ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(MessageReceive *message)
{
    hideAllItems();

    if (m_eventFlags & DisplaySetting::EventLineCrossing) {
        ms_linecrossing_info2 *info = (ms_linecrossing_info2 *)message->data;
        if (info) {
            if (info->chanid != m_video->channel()) {
                return;
            }
            m_itemLine->setLineCrossInfo2(info);
            m_itemLine->show();
        }
    }
}

void VideoScene::ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(MessageReceive *message)
{
    hideAllItems();

    if (m_eventFlags & DisplaySetting::EventPeopleCounting) {
        ms_smart_event_people_cnt *info = (ms_smart_event_people_cnt *)message->data;
        if (info) {
            if (info->chanid != m_video->channel()) {
                return;
            }
            m_drawLine->setPeopleCountInfo(info);
            if (info->lineNum == 4) {
                MS_PEOPLECNT_DATA peopleCntInfo = gDynamicDisplayData.peopleCntData(info->chanid);
                m_drawLine->setLineCntData(&peopleCntInfo);
                m_drawLine->setLineCntOsdType(info->osdType);
            }
            m_drawLine->show();
        }
    }
}

void VideoScene::ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(MessageReceive *message)
{
    hideAllItems();

    if (m_eventFlags & DisplaySetting::EventObject) {
        ms_smart_leftremove_info *info = (ms_smart_leftremove_info *)message->data;
        if (info) {
            if (info->chanid != m_video->channel()) {
                return;
            }
            if (info->regionType != REGION_SINGLE) {
                m_drawMultiControl->setRegions(info->regionInfo, info->regionScene, MAX_LEN_4);
                m_drawMultiControl->clearAllEditState();
                m_drawMultiControl->show();
            } else {
                if (info->left_enable || info->remove_enable) {
                    m_itemGrid->setRegion(info->area, info->polygonX, info->polygonY);
                    m_itemGrid->show();
                }
            }
        }
    }
}

void VideoScene::ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(MessageReceive *message)
{
    hideAllItems();

    if (m_eventFlags & DisplaySetting::EventRegionalPeopleCounting) {
        MsIpcRegionalPeople *info = static_cast<MsIpcRegionalPeople *>(message->data);
        if (info) {
            for (int i = 0; i < m_itemsRegionalPeopleCounting.size(); ++i) {
                auto item = m_itemsRegionalPeopleCounting.at(i);
                item->setIndex(i);
                item->showItem(info, i);
            }
        }
    }
}

void VideoScene::dealVcaGridEvent(MessageReceive *message, int flag)
{
    hideAllItems();

    if (m_eventFlags & flag) {
        ms_smart_event_info *info = (ms_smart_event_info *)message->data;
        if (info) {
            if (info->chanid != m_video->channel()) {
                return;
            }
            if (info->regionType != REGION_SINGLE) {
                m_drawMultiControl->setRegions(info->regionInfo, info->regionScene, MAX_LEN_4);
                m_drawMultiControl->clearAllEditState();
                m_drawMultiControl->clearAlarmState();
                m_drawMultiControl->show();
            } else {
                if (info->enable) {
                    m_itemGrid->setRegion(info->area, info->polygonX, info->polygonY);
                    m_itemGrid->show();
                }
            }
        }
    }
}

void VideoScene::hideRegionItems()
{
    for (int i = 0; i < m_itemsRegionalPeopleCounting.size(); ++i) {
        m_itemsRegionalPeopleCounting.at(i)->hide();
    }
}

void VideoScene::hideAllItems()
{
    m_itemLine->hide();
    m_itemGrid->hide();
    m_drawMultiControl->hide();
    m_drawLine->clearAllLine();
    m_drawLine->hide();
    hideVcaRects();
    hideRegionItems();
    hideRegionRects();
}

bool VideoScene::checkVcaBoxEvent(int event) const
{
    return m_eventFlags & event;
}

bool VideoScene::checkVcaBoxAlarm(int alarm) const
{

    return m_eventFlags & alarm;
}

void VideoScene::onVideoShow()
{
    //重新获取区域
    onLiveViewEventDetectionChanged(m_eventFlags);
}

void VideoScene::onVideoHide()
{
    clearAllTrackItem();
    hideAllItems();
}

void VideoScene::onConnectedStateChanged(bool connected)
{
    if (!connected) {
        m_itemGrid->clearState();
        m_itemLine->clearState();
        m_drawMultiControl->clearAlarmState();
    }
}

void VideoScene::onSceneRectChanged(const QRectF &rect)
{
    Q_UNUSED(rect)

    m_itemGrid->setRect(sceneRect());
    m_itemLine->setRect(sceneRect());
    m_drawMultiControl->setRect(sceneRect());
    m_drawLine->setRect(sceneRect());

    hideVcaRects();
}

void VideoScene::onTimerCheckEnable()
{
}

void VideoScene::onTimerGetPoint()
{
    switch (m_trackMode) {
    case TrackModeVca:
        sendMessage(REQUEST_FLAG_GET_VAC_PERSONPOINT, (void *)&m_channel, sizeof(int));
        break;
    case TrackModePtz:
    case TrackModeFisheye:
        sendMessage(REQUEST_FLAG_GET_AUTO_TRACK_POINT, (void *)&m_channel, sizeof(int));
        break;
    default:
        break;
    }
}

void VideoScene::onLiveViewEventDetectionChanged(int event)
{
    if (m_eventFlags != event) {
        m_eventFlags = event;
        m_itemGrid->hide();
        m_itemLine->hide();
        m_drawLine->hide();
        m_drawMultiControl->hide();
    }
    if (!m_video->isVisible()) {
        return;
    }
    if (m_channel < 0) {
        return;
    }
    //
    if (m_eventFlags == 0) {
        m_itemGrid->hide();
        m_itemLine->hide();
        m_drawLine->hide();
        m_drawMultiControl->hide();
        hideRegionItems();
    } else if (m_eventFlags & DisplaySetting::EventRegionEntrance) {
        EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_VCA_REGIONENTRANCE);
    } else if (m_eventFlags & DisplaySetting::EventRegionExiting) {
        EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_VCA_REGIONEXIT);
    } else if (m_eventFlags & DisplaySetting::EventMotionDetection) {
        EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_VCA_ADVANCEDMOTION);
    } else if (m_eventFlags & DisplaySetting::EventLoitering) {
        EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_VCA_LOITERING);
    } else if (m_eventFlags & DisplaySetting::EventLineCrossing) {
        MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(m_channel);
        if (cameraVersion >= MsCameraVersion(7, 75)) {
            EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_IPC_VCA_LINECROSSING2);
        } else {
            EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_VCA_LINECROSSING);
        }
    } else if (m_eventFlags & DisplaySetting::EventPeopleCounting) {
        EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_VCA_PEOPLE_COUNT);
    } else if (m_eventFlags & DisplaySetting::EventObject) {
        EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_VCA_LEFTREMOVE);
    } else if (m_eventFlags & DisplaySetting::EventRegionalPeopleCounting) {
        EventDetectionRegionManager::instance()->appendMessage(m_channel, REQUEST_FLAG_GET_IPC_REGIONAL_PEOPLE);
    }
}

void VideoScene::updateDynamicDisplay(int type, int channel)
{
    Q_UNUSED(type)
    Q_UNUSED(channel)
}
