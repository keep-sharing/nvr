#include "PlaybackTimeLine.h"
#include "MyDebug.h"
#include "PlaybackCut.h"
#include "PlaybackEventData.h"
#include "PlaybackSplit.h"
#include "PlaybackTagData.h"
#include "SmartSearchControl.h"
#include "ThumbWidget.h"
#include "centralmessage.h"
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

PlaybackCut *PlaybackTimeLine::s_playbackCut = nullptr;

PlaybackTimeLine::PlaybackTimeLine(QWidget *parent)
    : BasePlayback(parent)
{
    setMouseTracking(true);

    PlaybackCut::s_playbackTimeLine = this;
    s_playbackTimeLine = this;

    m_timerHideCurrentTip = new QTimer(this);
    m_timerHideCurrentTip->setSingleShot(true);
    m_timerHideCurrentTip->setInterval(5000);
    connect(m_timerHideCurrentTip, SIGNAL(timeout()), this, SLOT(onHideCurrentTip()));

    m_dragTimer = new QTimer(this);
    connect(m_dragTimer, SIGNAL(timeout()), this, SLOT(onDragTimerTimeout()));
    m_dragTimer->setSingleShot(true);
    m_dragTimer->setInterval(3000);

    m_timerThumbReady = new QTimer(this);
    m_timerThumbReady->setSingleShot(true);
    m_timerThumbReady->setInterval(500);
    connect(m_timerThumbReady, SIGNAL(timeout()), this, SLOT(onTimerThumbReady()));

    m_timerThumbGet = new QTimer(this);
    m_timerThumbGet->setSingleShot(true);
    m_timerThumbGet->setInterval(200);
    connect(m_timerThumbGet, SIGNAL(timeout()), this, SLOT(onTimerThumbGet()));

    m_labelTimeTip = new QLabel(this);
    m_labelTimeTip->setStyleSheet("color: #FFFFFF");
    m_labelTimeTip->hide();

    setTimeLineBeginDateTime(QDateTime::currentDateTime());

    //
    m_timelineThread = new TimeLineThread();
    connect(m_timelineThread, SIGNAL(imageFinished(int, QImage)), this, SLOT(onTimeLineImageFinished(int, QImage)));
}

PlaybackTimeLine::~PlaybackTimeLine()
{
    m_timelineThread->stopThread();
    delete m_timelineThread;
    m_timelineThread = nullptr;
}

void PlaybackTimeLine::setTimeLineBeginDateTime(const QDateTime &dateTime)
{
    m_beginDateTime = dateTime;
    m_endDateTime = m_beginDateTime.addDays(1);

    s_timelineBeginDateTime = m_beginDateTime;
    s_timelineEndDateTime = m_endDateTime;

    qMsDebug() << "\n----m_beginDateTime:" << m_beginDateTime.toString("yyyy-MM-dd HH:mm:ss")
               << "\n----m_endDateTime:" << m_endDateTime.toString("yyyy-MM-dd HH:mm:ss");
}

QDateTime PlaybackTimeLine::timeLineBeginDateTime() const
{
    return m_beginDateTime;
}

QDateTime PlaybackTimeLine::timeLineEndDateTime() const
{
    return m_endDateTime;
}

void PlaybackTimeLine::zoomIn()
{
    if (m_zoomLevel < TimeLineZoomLevel_6) {
        m_zoomLevel = TimeLineZoomLevel(m_zoomLevel + 1);
        reCalculateRange();
        update();
    }
}

void PlaybackTimeLine::zoomOut()
{
    if (m_zoomLevel > TimeLineZoomLevel_1) {
        m_zoomLevel = TimeLineZoomLevel(m_zoomLevel - 1);
        reCalculateRange();
        update();
    }
}

void PlaybackTimeLine::setZoomLevel(const TimeLineZoomLevel &level)
{
    m_zoomLevel = TimeLineZoomLevel(level);
    reCalculateRange();
    update();
}

void PlaybackTimeLine::setCurrentTime(const QDateTime &dateTime)
{
    if (isJumped()) {
        clearJumped();
        return;
    }

    if (m_dragMode != DragNone) {
        return;
    }

    m_currentDateTime = dateTime;

    if (m_isDrag || m_dragTimer->isActive()) {
        //避免和手动多拽冲突，拖拽后3秒才可以自动纠正
    } else {
        //超出显示范围时自动纠正
        setPlayCursorCentre();
    }

    update();
}

void PlaybackTimeLine::forwardSec(int sec)
{
    m_currentDateTime = m_currentDateTime.addSecs(sec);
    if (m_currentDateTime > m_endDateTime) {
        m_currentDateTime = m_endDateTime;
    }
    m_showCurrentTip = true;
    m_timerHideCurrentTip->start();
    update();

    emit timeClicked(m_currentDateTime);
    setJumped();
}

void PlaybackTimeLine::backwardSec(int sec)
{
    m_currentDateTime = m_currentDateTime.addSecs(-sec);
    if (m_currentDateTime < m_beginDateTime) {
        m_currentDateTime = m_beginDateTime;
    }
    m_showCurrentTip = true;
    m_timerHideCurrentTip->start();
    update();

    emit timeClicked(m_currentDateTime);
    setJumped();
}

void PlaybackTimeLine::updateTimeLine()
{
    drawTimeImage();
    update();
}

void PlaybackTimeLine::setJumped()
{
    m_isJumped = true;
}

void PlaybackTimeLine::clearJumped()
{
    m_isJumped = false;
}

bool PlaybackTimeLine::isJumped()
{
    return m_isJumped;
}

void PlaybackTimeLine::openCut()
{
    m_cutEnable = true;

    switch (m_zoomLevel) {
    case TimeLineZoomLevel_1:
        m_cutBeginDateTime = m_currentDateTime.addSecs(-1800);
        m_cutEndDateTime = m_currentDateTime.addSecs(1800);
        break;
    case TimeLineZoomLevel_2:
        m_cutBeginDateTime = m_currentDateTime.addSecs(-900);
        m_cutEndDateTime = m_currentDateTime.addSecs(900);
        break;
    default:
        m_cutBeginDateTime = m_currentDateTime.addSecs(-600);
        m_cutEndDateTime = m_currentDateTime.addSecs(600);
        break;
    }

    if (m_cutBeginDateTime < m_beginDateTime) {
        m_cutBeginDateTime = m_beginDateTime;
    }
    if (m_cutEndDateTime > m_endDateTime) {
        m_cutEndDateTime = m_endDateTime;
    }

    s_playbackCut->setRange(m_cutBeginDateTime, m_cutEndDateTime);

    update();
}

void PlaybackTimeLine::closeCut()
{
    m_cutEnable = false;
    update();
}

bool PlaybackTimeLine::isCutEnable()
{
    return m_cutEnable;
}

void PlaybackTimeLine::setCutBeginDateTime(const QDateTime &dateTime)
{
    //qDebug() << "begin:" << dateTime.toString("yyyy-MM-dd HH:mm:ss");
    m_cutBeginDateTime = dateTime;
    update();
}

void PlaybackTimeLine::setCutEndDateTime(const QDateTime &dateTime)
{
    //qDebug() << "end:" << dateTime.toString("yyyy-MM-dd HH:mm:ss");
    m_cutEndDateTime = dateTime;
    update();
}

void PlaybackTimeLine::drawTimeImage()
{
    TimeLineThread::ImageInfo info;
    info.imageSize = size();
    info.beginDateTime = m_beginDateTime;
    info.endDateTime = m_endDateTime;
    info.secWidth = perSecondWidth();
    info.playbackDate = playbackDate();
    info.playbackMode = playbackType();
    info.isEventOnly = getIsEventOnly();
    if (playbackType() == SplitPlayback) {
        PlaybackSplit *splitPlayback = PlaybackSplit::instance();
        Q_ASSERT(splitPlayback);
        info.channel = splitPlayback->channel();
        info.commonList = splitPlayback->splitCommonBackupList();
    } else {
        info.channel = currentChannel();
        info.commonList = commonBackupList(currentChannel());
        if (isSmartSearchMode()) {
            info.isSmartSearch = true;
            info.eventList = SmartSearchControl::instance()->backupList();
        } else {
            info.eventList = gPlaybackEventData.backupList(info.channel, static_cast<uint>(getFilterEvent()));
        }
    }
    info.tagMap = gPlaybackTagData.tagMap(info.channel);
    m_timelineThread->drawImage(info);
}

void PlaybackTimeLine::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    m_padding = width() / 50.0;

    //背景
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush("#2E2E2E"));
    painter.drawRect(rect());
    painter.restore();

    //
    drawTimeHeader(&painter);
    drawTimeRecord(&painter);
    drawCut(&painter);
}

void PlaybackTimeLine::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    m_pressed = true;
    m_pressedPoint = event->pos();
    if (m_pressedPoint.y() < height() / 2) {
        m_dragMode = DragTime;
        //setCursor(Qt::ClosedHandCursor);
        m_tempBeginDateTime = m_beginDateTime;
    } else {
        if (isCutEnable()) {
            int beginX = xCoordinate(m_cutBeginDateTime);
            int endX = xCoordinate(m_cutEndDateTime);
            int x = event->pos().x();
            if (qAbs(x - beginX) < 5) {
                m_dragMode = DragCutBegin;
                setCursor(Qt::SizeHorCursor);
            } else if (qAbs(x - endX) < 5) {
                m_dragMode = DragCutEnd;
                setCursor(Qt::SizeHorCursor);
            }
        }
        switch (m_dragMode) {
        case DragCutBegin:
        case DragCutEnd:
            break;
        default:
            m_dragMode = DragRecord;
            m_tempBeginDateTime = m_beginDateTime;
            break;
        }
    }
}

void PlaybackTimeLine::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed) {
        switch (m_dragMode) {
        case DragTime:
        case DragRecord: {
            /**时间部分拖拽**/
            /**录像部分拖拽**/
            m_isDrag = true;
            setCursor(Qt::ClosedHandCursor);

            qreal secWidth = perSecondWidth();
            int moveSec = (event->pos().x() - m_pressedPoint.x()) / secWidth;

            //左边缘检测
            QDateTime tempBegin = m_tempBeginDateTime.addSecs(-moveSec);
            if (tempBegin < s_timelineBeginDateTime) {
                tempBegin = s_timelineBeginDateTime;
            }
            //右边缘检测
            int sec = (width() - 2 * m_padding) / secWidth;
            QDateTime tempEnd = tempBegin.addSecs(sec);
            if (tempEnd > s_timelineEndDateTime) {
                tempEnd = s_timelineEndDateTime;
                tempBegin = tempEnd.addSecs(-sec);
            }

            m_beginDateTime = tempBegin;
            m_endDateTime = tempEnd;

            break;
        }
        case DragCutBegin: {
            m_cutBeginDateTime = currentDateTime(event->posF().x());
            if (m_cutBeginDateTime > m_cutEndDateTime) {
                qSwap(m_cutBeginDateTime, m_cutEndDateTime);
                m_dragMode = DragCutEnd;
            }
            s_playbackCut->setBeginTime(m_cutBeginDateTime);
            break;
        }
        case DragCutEnd: {
            m_cutEndDateTime = currentDateTime(event->posF().x());
            if (m_cutEndDateTime < m_cutBeginDateTime) {
                qSwap(m_cutEndDateTime, m_cutBeginDateTime);
                m_dragMode = DragCutBegin;
            }
            s_playbackCut->setEndTime(m_cutEndDateTime);
            break;
        }
        default:
            break;
        }

        drawTimeImage();
        update();
    } else {
        if (event->pos().y() < height() / 2) {
            /**时间部分**/
            //setCursor(Qt::OpenHandCursor);
            //
            if (m_timerThumbReady->isActive()) {
                m_timerThumbReady->stop();
            }
            if (m_timerThumbGet->isActive()) {
                m_timerThumbGet->stop();
            }
            m_isThumbing = false;
            s_thumbWidget->setEnabled(false);
            s_thumbWidget->hide();
        } else {
            /**录像部分**/
            if (isCutEnable()) {
                int beginX = xCoordinate(m_cutBeginDateTime);
                int endX = xCoordinate(m_cutEndDateTime);
                int x = event->pos().x();
                if (qAbs(x - beginX) < 5 || qAbs(x - endX) < 5) {
                    setCursor(Qt::SizeHorCursor);
                } else {
                    unsetCursor();
                }
            } else {
                unsetCursor();
            }
            //
            if (m_isThumbing) {
                if (!m_timerThumbGet->isActive()) {
                    m_timerThumbGet->start();
                }
            } else if (!m_timerThumbReady->isActive()) {
                m_timerThumbReady->start();
            }
            qreal mouseX = event->posF().x();
            if (mouseX < m_padding || mouseX > width() - m_padding) {
                m_isThumbing = false;
                m_dateTimeUnderMouse = QDateTime();
                s_thumbWidget->setEnabled(false);
                s_thumbWidget->hide();
            } else {
                m_dateTimeUnderMouse = currentDateTime(mouseX);
            }
        }
    }
}

void PlaybackTimeLine::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    m_pressed = false;
    if (m_isDrag) {
        m_isDrag = false;
        m_dragTimer->start();
    }
    //
    if (m_pressedPoint == event->pos() && event->pos().y() > height() / 2) {
        m_currentDateTime = currentDateTime(event->posF().x());
        m_showCurrentTip = true;
        m_timerHideCurrentTip->start();
        update();

        qDebug() << "TimeLine clicked:" << m_currentDateTime.toString("yyyy-MM-dd HH:mm:ss");
        emit timeClicked(m_currentDateTime);
        setJumped();
    }
    m_dragMode = DragNone;
    //
    if (event->pos().y() < height() / 2) {
        unsetCursor();
    } else {
        unsetCursor();
        m_timerHideCurrentTip->start();
    }
}

void PlaybackTimeLine::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta();
    if (numDegrees < 0) {
        zoomOut();
    } else if (numDegrees > 0) {
        zoomIn();
    }
}

void PlaybackTimeLine::leaveEvent(QEvent *event)
{
    m_isThumbing = false;
    m_dateTimeUnderMouse = QDateTime();
    s_thumbWidget->setEnabled(false);
    s_thumbWidget->hide();

    QWidget::leaveEvent(event);
}

void PlaybackTimeLine::drawTimeHeader(QPainter *painter)
{
    switch (m_zoomLevel) {
    case TimeLineZoomLevel_1: {
        drawTimeScale(painter, 60 * 60, 120 * 60);
        break;
    }
    case TimeLineZoomLevel_2: {
        drawTimeScale(painter, 30 * 60, 60 * 60);
        break;
    }
    case TimeLineZoomLevel_3: {
        drawTimeScale(painter, 10 * 60, 60 * 60);
        break;
    }
    case TimeLineZoomLevel_4: {
        drawTimeScale(painter, 5 * 60, 30 * 60);
        break;
    }
    case TimeLineZoomLevel_5: {
        drawTimeScale(painter, 2 * 60, 10 * 60);
        break;
    }
    case TimeLineZoomLevel_6: {
        drawTimeScale(painter, 1 * 60, 5 * 60);
        break;
    }
    default:
        break;
    }
}

void PlaybackTimeLine::drawTimeRecord(QPainter *painter)
{
    QRectF rc = rect();
    rc.setTop(rc.top() + rc.height() / 2);
    rc.setLeft(m_padding);
    rc.setRight(width() - m_padding);
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush("#5F5F5F"));
    painter->drawRect(rc);
    painter->restore();

    //
    drawRecordInfo(painter);

    //
    drawCurrentLine(painter);
}

void PlaybackTimeLine::drawRecordInfo(QPainter *painter)
{
    if (playbackType() == SplitPlayback) {
        if (!PlaybackSplit::instance()->hasCommonBackup()) {
            return;
        }
    } else {
        const int &channel = currentChannel();
        if (!isChannelChecked(channel)) {
            return;
        }
    }

    painter->save();

    //
    QRect imageRc = rect();
    //imageRc.moveBottom(height() / 2);
    painter->drawImage(imageRc, m_timeImage);

#if 0
    painter->setPen(Qt::NoPen);

    qreal secWidth = perSecondWidth();

    QRectF rcf;
    rcf.setTop(height() / 2 + 1);
    rcf.setBottom(height());

    //int lastBeginSec = 0;
    int lastEndSec = 0;
    QList<resp_search_common_backup> commonList = commonBackupList(channel);
    for (int i = 0; i < commonList.size(); ++i)
    {
        const resp_search_common_backup &common_backup = commonList.at(i);
        QDateTime beginDateTime = QDateTime::fromString(common_backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime endDateTime = QDateTime::fromString(common_backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        int beginSec = QTime(0, 0).secsTo(beginDateTime.time());
        int endSec = QTime(0, 0).secsTo(endDateTime.time());

        //过滤跨天情况2015-05-29 23:59:59 - 2015-05-30 00:01:01
        if (beginDateTime.date() < endDateTime.date())
        {
            beginSec = 0;
        }

        switch ((REC_EVENT_EN)common_backup.enEvent) {
        case REC_EVENT_TIME:
            painter->setBrush(QBrush(QColor("#09A8E0")));
            break;
        case REC_EVENT_ANR:
            painter->setBrush(QBrush(QColor("#FF7F24")));//ANR暂用颜色
            break;
        default:
            painter->setBrush(QBrush(QColor("#E70000")));
            break;
        }

        //显示范围过滤
        bool isOverLeft = false;
        bool isOverRight = false;
        if (endSec < m_beginSec)
        {
            continue;
        }
        if (beginSec < m_beginSec && endSec > m_beginSec)
        {
            isOverLeft = true;
            beginSec = m_beginSec;
        }
        if (beginSec > m_endSec)
        {
            continue;
        }
        if (beginSec < m_endSec && endSec > m_endSec)
        {
            isOverRight = true;
            endSec = m_endSec;
        }

        //正常情况录像
        //2019-05-15 00:52:05 - 2019-05-16 01:25:33
        //2019-05-15 01:25:33 - 2019-05-16 01:59:02
        //有可能也会有一秒的间隔，也是正常的，如：
        //2019-05-15 00:52:05 - 2019-05-16 01:25:33
        //2019-05-15 01:25:34 - 2019-05-16 01:59:02
        //避免界面显示录像间断，这里修正一下
        if (beginSec - lastEndSec == 1)
        {
            beginSec = lastEndSec;
        }
        //lastBeginSec = beginSec;
        lastEndSec = endSec;

        //
        rcf.setLeft(m_padding + (beginSec - m_beginSec) * secWidth);
        rcf.setRight(m_padding + (endSec - m_beginSec) * secWidth);
        painter->drawRect(rcf);

        //绘制lock
        if (playbackMode() == GeneralPlayback)
        {
            if (common_backup.isLock)
            {
                painter->save();

                painter->setPen(QPen(QColor("#FF9D00")));
                painter->drawLine(QPointF(rcf.left(), rcf.top() - 3), QPointF(rcf.right(), rcf.top() - 3));
                if (!isOverLeft)
                {
                    painter->drawLine(QPointF(rcf.left(), rcf.top() - 3), QPointF(rcf.left(), rcf.top()));
                }
                if (!isOverRight)
                {
                    painter->drawLine(QPointF(rcf.right(), rcf.top() - 3), QPointF(rcf.right(), rcf.top()));
                }

                QRect lockRect;
                lockRect.setLeft(rcf.left());
                lockRect.setRight(rcf.right());
                lockRect.setTop(height() / 2.0 / 8.0 * 5.0 + 1);
                lockRect.setBottom(rcf.top() - 3);

                QPixmap pixmap(":/playback/playback/lock2.png");
                QRect pixmapRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(lockRect.height() - 4, lockRect.height() - 4), lockRect);
                painter->drawPixmap(pixmapRect, pixmap);

                painter->restore();
            }
        }
    }

    //绘制event
    QList<resp_search_event_backup> eventList = eventBackupList(channel);
    for (int i = 0; i < eventList.size(); ++i)
    {
        const resp_search_event_backup &event_backup = eventList.at(i);
        QDateTime beginDateTime = QDateTime::fromString(event_backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime endDateTime = QDateTime::fromString(event_backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        int beginSec = QTime(0, 0).secsTo(beginDateTime.time());
        int endSec = QTime(0, 0).secsTo(endDateTime.time());

        //过滤跨天情况2015-05-29 23:59:59 - 2015-05-30 00:01:01
        if (beginDateTime.date() < endDateTime.date())
        {
            beginSec = 0;
        }

        switch ((REC_EVENT_EN)event_backup.enEvent) {
        case REC_EVENT_TIME:
            painter->setBrush(QBrush(QColor("#09A8E0")));
            break;
        case REC_EVENT_ANR:
            painter->setBrush(QBrush(QColor("#FF7F24")));//ANR暂用颜色
            break;
        default:
            painter->setBrush(QBrush(QColor("#E70000")));
            break;
        }

        //显示范围过滤
        if (endSec < m_beginSec)
        {
            continue;
        }
        if (beginSec < m_beginSec && endSec > m_beginSec)
        {
            beginSec = m_beginSec;
        }
        if (beginSec > m_endSec)
        {
            continue;
        }
        if (beginSec < m_endSec && endSec > m_endSec)
        {
            endSec = m_endSec;
        }

        rcf.setLeft(m_padding + (beginSec - m_beginSec) * secWidth);
        rcf.setRight(m_padding + (endSec - m_beginSec) * secWidth);
        painter->drawRect(rcf);
    }

    //绘制tag
    if (playbackMode() == GeneralPlayback)
    {
        auto tagMap = drawTagMap(channel);
        if (!tagMap.isEmpty())
        {
            painter->save();
            painter->setPen(QPen(QColor("#D54CE3"), 2));

            QRect tagRect;
            tagRect.setTop(height() / 2.0 / 8.0 * 5.0 + 1);
            tagRect.setBottom(rcf.top() - 3);

            for (auto iter = tagMap.constBegin(); iter != tagMap.constEnd(); ++iter)
            {
                const QDateTime &dateTime = iter.key();
                if (dateTime.date() != playbackDate())
                {
                    continue;
                }
                int beginSec = QTime(0, 0).secsTo(dateTime.time());
                //qDebug() << beginSec << dateTime;
                if (beginSec < m_beginSec)
                {
                    continue;
                }
                if (beginSec > m_endSec)
                {
                    continue;
                }

                int x = m_padding + (beginSec - m_beginSec) * secWidth;
                tagRect.setLeft(x - tagRect.height() / 2 - 2);
                tagRect.setRight(x + tagRect.height() / 2 + 2);

                painter->drawLine(QPointF(x, rcf.top() - 5), QPointF(x, rcf.top()));

                QPixmap pixmap(":/playback/playback/tag2.png");
                painter->drawPixmap(tagRect, pixmap);
            }

            painter->restore();
        }
    }
#endif
    painter->restore();
}

void PlaybackTimeLine::drawCurrentLine(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);

    int top = height() / 2;
    int bottom = height();
    //竖线
    qreal x = currentX();
    if (x < m_padding || x > width() - m_padding) {
        painter->restore();
        return;
    }
    painter->drawLine(QLineF(x, top, x, bottom));

    //上三角
    qreal triangleHeight = (bottom - top) / 10;
    QPointF topTriangle[3] = {
        QPointF(x - triangleHeight, top),
        QPointF(x + triangleHeight, top),
        QPointF(x, top + triangleHeight)
    };
    painter->drawPolygon(topTriangle, 3);

    //下三角
    QPointF bottomTriangle[3] = {
        QPointF(x - triangleHeight, bottom - 1),
        QPointF(x + triangleHeight, bottom - 1),
        QPointF(x, bottom - triangleHeight - 1)
    };
    painter->drawPolygon(bottomTriangle, 3);

    //时间tip
    if (m_showCurrentTip) {
        QFont font = painter->font();
        font.setPixelSize(14);
        painter->setFont(font);
        QString strTime = m_currentDateTime.time().toString("HH:mm:ss");
        int textWidth = painter->fontMetrics().width(strTime);
        int textHeight = painter->fontMetrics().height();
        QPoint textPoint;
        textPoint.setY(height() / 4 * 3 + textHeight / 2);
        if (x + textWidth + 5 < width() - m_padding) {
            textPoint.setX(x + 5);
        } else {
            textPoint.setX(x - textWidth - 5);
        }
        painter->drawText(textPoint, strTime);
    }

    painter->restore();
}

void PlaybackTimeLine::drawCut(QPainter *painter)
{
    if (!isCutEnable()) {
        return;
    }

    bool drawLeftHandle = true;
    bool drawRightHandle = true;
    //显示范围过滤
    QDateTime beginDateTime = m_cutBeginDateTime;
    QDateTime endDateTime = m_cutEndDateTime;
    if (endDateTime < m_beginDateTime) {
        return;
    }
    if (beginDateTime < m_beginDateTime && endDateTime > m_beginDateTime) {
        beginDateTime = m_beginDateTime;
        drawLeftHandle = false;
    }
    if (beginDateTime > m_endDateTime) {
        return;
    }
    if (beginDateTime < m_endDateTime && endDateTime > m_endDateTime) {
        endDateTime = m_endDateTime;
        drawRightHandle = false;
    }

    QRectF rcf;
    rcf.setTop(height() / 2 + 1);
    rcf.setBottom(height());
    rcf.setLeft(xCoordinate(beginDateTime));
    rcf.setRight(xCoordinate(endDateTime));

    //
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 255, 150));
    painter->drawRect(rcf);
    painter->restore();

    //两边的小三角
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#FFFFFF"));
    if (drawLeftHandle) {
        QPointF leftPoint[3] = {
            QPointF(rcf.left(), rcf.top()),
            QPointF(rcf.left() + 7, rcf.top()),
            QPointF(rcf.left(), rcf.top() + 10),
        };
        painter->drawPolygon(leftPoint, 3);
    }
    if (drawRightHandle) {
        QPointF rightPoint[3] = {
            QPointF(rcf.right(), rcf.top()),
            QPointF(rcf.right() - 7, rcf.top()),
            QPointF(rcf.right(), rcf.top() + 10),
        };
        painter->drawPolygon(rightPoint, 3);
    }
    painter->restore();
}

void PlaybackTimeLine::reCalculateRange()
{
    if (m_zoomLevel == TimeLineZoomLevel_1) {
        m_beginDateTime = s_timelineBeginDateTime;
        m_endDateTime = s_timelineEndDateTime;
    } else {
        if (m_currentDateTime < m_beginDateTime || m_currentDateTime > m_endDateTime) {
            m_currentDateTime = m_beginDateTime;
        }

        qreal ratio = (qreal)(m_beginDateTime.secsTo(m_currentDateTime)) / (m_beginDateTime.secsTo(m_endDateTime));
        int range = 0;
        switch (m_zoomLevel) {
        case TimeLineZoomLevel_2:
            range = 720 * 60;
            break;
        case TimeLineZoomLevel_3:
            range = 360 * 60;
            break;
        case TimeLineZoomLevel_4:
            range = 180 * 60;
            break;
        case TimeLineZoomLevel_5:
            range = 60 * 60;
            break;
        case TimeLineZoomLevel_6: //30 minute
            range = 30 * 60;
            break;
        default:
            break;
        }

        QDateTime beginDateTime = m_currentDateTime.addSecs(-range * ratio);
        if (beginDateTime < s_timelineBeginDateTime) {
            beginDateTime = s_timelineBeginDateTime;
        }
        QDateTime endDateTime = beginDateTime.addSecs(range);
        if (endDateTime > s_timelineEndDateTime) {
            endDateTime = s_timelineEndDateTime;
            beginDateTime = endDateTime.addSecs(-range);
        }
        m_beginDateTime = beginDateTime;
        m_endDateTime = endDateTime;
    }
    //    MsDebug() << "\n----zoom level:" << m_zoomLevel
    //              << "\n----begin:" << m_beginDateTime.toString("yyyy-MM-dd HH:mm:ss")
    //              << "\n----end:" << m_endDateTime.toString("yyyy-MM-dd HH:mm:ss");

    //
    drawTimeImage();
}

void PlaybackTimeLine::setPlayCursorCentre()
{
    if (m_currentDateTime < s_timelineBeginDateTime || m_currentDateTime > s_timelineEndDateTime) {
        m_currentDateTime = s_timelineBeginDateTime;
    }

    if (m_currentDateTime < m_beginDateTime || m_currentDateTime > m_endDateTime) {
        int totalSec = 0;
        switch (m_zoomLevel) {
        case TimeLineZoomLevel_1:
            break;
        case TimeLineZoomLevel_2:
            totalSec = 720 * 60;
            break;
        case TimeLineZoomLevel_3:
            totalSec = 360 * 60;
            break;
        case TimeLineZoomLevel_4:
            totalSec = 180 * 60;
            break;
        case TimeLineZoomLevel_5:
            totalSec = 60 * 60;
            break;
        case TimeLineZoomLevel_6: //30 minute
            totalSec = 30 * 60;
            break;
        }
        QDateTime beginDateTime = m_currentDateTime.addSecs(-totalSec / 2);
        if (beginDateTime < s_timelineBeginDateTime) {
            beginDateTime = s_timelineBeginDateTime;
        }
        QDateTime endDateTime = beginDateTime.addSecs(totalSec);
        if (endDateTime > s_timelineEndDateTime) {
            endDateTime = s_timelineEndDateTime;
            beginDateTime = endDateTime.addSecs(-totalSec);
        }
        m_beginDateTime = beginDateTime;
        m_endDateTime = endDateTime;

        //        MsDebug() << "\n----zoom level:" << m_zoomLevel
        //                  << "\n----begin:" << m_beginDateTime.toString("yyyy-MM-dd HH:mm:ss")
        //                  << "\n----end:" << m_endDateTime.toString("yyyy-MM-dd HH:mm:ss");
        //
        drawTimeImage();
    }
}

bool PlaybackTimeLine::timeHasEventPlayBack(const QDateTime &dateTime)
{
    QList<resp_search_event_backup> list = gPlaybackEventData.backupList(currentChannel(), static_cast<uint>(getFilterEvent()));
    for (int i = 0; i < list.size(); ++i) {
        const resp_search_event_backup &event_backup = list.at(i);
        QDateTime beginDateTime = QDateTime::fromString(event_backup.pStartTime, "yyyy-MM-dd HH:mm:ss");
        QDateTime endDateTime = QDateTime::fromString(event_backup.pEndTime, "yyyy-MM-dd HH:mm:ss");
        if (dateTime >= beginDateTime && dateTime <= endDateTime) {
            return true;
        }
    }
    return false;
}

void PlaybackTimeLine::onHideCurrentTip()
{
    m_showCurrentTip = false;
    update();
}

/**
 * @brief PlaybackTimeLine::drawTimeScale
 * 绘制时间刻度
 * @param painter
 * @param span      //刻度间隔(秒)
 * @param mainSpan  //主刻度间隔(秒)
 */
void PlaybackTimeLine::drawTimeScale(QPainter *painter, int span, int mainSpan)
{
    /*******************************
     * 分为三部分：
     * 1.顶部数字，占1/2
     * 2.中间刻度，占1/8
     * 3.底部空白，占3/8
     *******************************/

    qreal yStep = height() / 2.0 / 8.0;
    int y1 = 0;
    int y2 = y1 + 4 * yStep;
    int y3 = y2 + yStep;

    //横线
    painter->save();
    painter->setPen(Qt::white);
    painter->drawLine(QLine(m_padding, y3, width() - m_padding, y3));
    painter->restore();

    //
    painter->save();
    QFont font = painter->font();
    font.setPixelSize(14);
    painter->setFont(font);
    painter->setPen(Qt::white);

    qreal secWidth = perSecondWidth();
    uint beginSec = m_beginDateTime.toTime_t();
    uint endSec = m_endDateTime.toTime_t();
    uint offsetSec = QDateTime(m_beginDateTime.date(), QTime(0, 0)).secsTo(m_beginDateTime);
    for (uint i = beginSec; i <= endSec; ++i) {
        uint sec = i - beginSec + offsetSec;
        //刻度间隔
        if (sec % span == 0) {
            int x = m_padding + secWidth * (i - beginSec);
            if (x > width() - m_padding) {
                break;
            }

            int hour = sec / 3600;
            int minute = sec % 3600 / 60;
            if (hour > 24) {
                hour -= 24;
            }
            //主刻度间隔
            if (sec % mainSpan == 0) {
                QLine line(x, y3, x, y3 - yStep);
                painter->drawLine(line);

                QString strText = QString("%1:%2").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0'));
                int textWidth = painter->fontMetrics().width(strText);
                QPoint point(x - textWidth / 2, y1 + yStep * 4 - 4);
                painter->drawText(point, strText);
            } else {
                QLine line(x, y3, x, y3 - yStep / 2);
                painter->drawLine(line);
            }
        }
    }
    painter->restore();
}

/**
 * @brief PlaybackTimeLine::perSecondWidth
 * 根据当前显示范围计算出1秒实际占用屏幕多少像素
 * @return
 */
qreal PlaybackTimeLine::perSecondWidth()
{
    qreal secWidth = 0;
    switch (m_zoomLevel) {
    case TimeLineZoomLevel_1:
        secWidth = (width() - m_padding * 2) / 86400.0; //24 hour
        break;
    case TimeLineZoomLevel_2:
        secWidth = (width() - m_padding * 2) / 43200.0; //12 hour
        break;
    case TimeLineZoomLevel_3:
        secWidth = (width() - m_padding * 2) / 21600.0; //6 hour
        break;
    case TimeLineZoomLevel_4:
        secWidth = (width() - m_padding * 2) / 10800.0; //3 hour
        break;
    case TimeLineZoomLevel_5:
        secWidth = (width() - m_padding * 2) / 3600.0; //1 hour
        break;
    case TimeLineZoomLevel_6:
        secWidth = (width() - m_padding * 2) / 1800.0; //0.5 hour
        break;
    default:
        break;
    }
    return secWidth;
}

qreal PlaybackTimeLine::currentX()
{
    return m_padding + perSecondWidth() * m_beginDateTime.secsTo(m_currentDateTime);
}

qreal PlaybackTimeLine::xCoordinate(const QDateTime &dateTime)
{
    return m_padding + perSecondWidth() * (m_beginDateTime.secsTo(dateTime));
}

QDateTime PlaybackTimeLine::currentDateTime(qreal xpos)
{
    QDateTime dateTime;
    if (xpos < m_padding) {
        dateTime = m_beginDateTime;
    } else if (xpos > width() - m_padding) {
        dateTime = m_endDateTime;
    } else {
        dateTime = m_beginDateTime.addSecs((xpos - m_padding) / perSecondWidth());
    }
    return dateTime;
}

void PlaybackTimeLine::onDragTimerTimeout()
{
}

void PlaybackTimeLine::onTimerThumbReady()
{
    m_isThumbing = true;
    m_timerThumbGet->start();
}

void PlaybackTimeLine::onTimerThumbGet()
{
    if (m_dateTimeUnderMouse.isNull()) {
        return;
    }
    if (getIsEventOnly() && !timeHasEventPlayBack(m_dateTimeUnderMouse)) {
        return;
    }

    PB_IN_DIAGRAM thumb;
    memset(&thumb, 0, sizeof(PB_IN_DIAGRAM));
    if (playbackType() == SplitPlayback) {
        thumb.chnid = PlaybackSplit::instance()->channel();
        thumb.sid = PlaybackSplit::instance()->selectedSid();
    } else {
        thumb.chnid = currentChannel();
        thumb.sid = currentSid();
    }
    snprintf(thumb.playtime, sizeof(thumb.playtime), "%s", m_dateTimeUnderMouse.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

    if (thumb.chnid < 0) {
        return;
    }

    qDebug() << QString("REQUEST_FLAG_PB_INDENTATION_DIAGRAM, channel: %1, sid: %2, time: %3").arg(thumb.chnid).arg(thumb.sid).arg(thumb.playtime);
    MsSendMessage(SOCKET_TYPE_CORE, REQUEST_FLAG_PB_INDENTATION_DIAGRAM, &thumb, sizeof(PB_IN_DIAGRAM));

    if (!s_thumbWidget->isEnabled()) {
        s_thumbWidget->setEnabled(true);
    }
}

void PlaybackTimeLine::onTimeLineImageFinished(int channel, QImage image)
{
    //qDebug() << "PlaybackTimeLine::onTimeLineImageFinished";
    if (playbackType() == SplitPlayback) {
        if (channel == PlaybackSplit::instance()->playingChannel()) {
            m_timeImage = image;
            update();
        }
    } else {
        if (channel == currentChannel()) {
            m_timeImage = image;
            update();
        }
    }
}
