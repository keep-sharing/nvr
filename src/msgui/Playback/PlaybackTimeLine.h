#ifndef PLAYBACKTIMELINE_H
#define PLAYBACKTIMELINE_H

#include "TimeLineThread.h"
#include <QDateTime>
#include <QLabel>
#include <QTimer>

class PlaybackCut;

enum TimeLineZoomLevel {
    TimeLineZoomLevel_1,
    TimeLineZoomLevel_2,
    TimeLineZoomLevel_3,
    TimeLineZoomLevel_4,
    TimeLineZoomLevel_5,
    TimeLineZoomLevel_6
};

class PlaybackTimeLine : public BasePlayback {
    Q_OBJECT

    enum DragMode {
        DragNone,
        DragTime,
        DragRecord,
        DragCutBegin,
        DragCutEnd
    };

public:
    explicit PlaybackTimeLine(QWidget *parent = 0);
    ~PlaybackTimeLine();

    static PlaybackCut *s_playbackCut;

    void setTimeLineBeginDateTime(const QDateTime &dateTime);
    QDateTime timeLineBeginDateTime() const;
    QDateTime timeLineEndDateTime() const;

    void zoomIn();
    void zoomOut();
    void setZoomLevel(const TimeLineZoomLevel &level);

    void setCurrentTime(const QDateTime &dateTime);

    void forwardSec(int sec);
    void backwardSec(int sec);

    void updateTimeLine();

    //
    void setJumped();
    void clearJumped();
    bool isJumped();

    //cut
    void openCut();
    void closeCut();
    bool isCutEnable();
    void setCutBeginDateTime(const QDateTime &dateTime);
    void setCutEndDateTime(const QDateTime &dateTime);

    //
    void drawTimeImage();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    qreal perSecondWidth();
    //当前时间对应的x坐标
    qreal currentX();
    //秒对应的x坐标
    qreal xCoordinate(const QDateTime &dateTime);
    //当前x坐标对应的时间
    QDateTime currentDateTime(qreal xpos);

    void drawTimeHeader(QPainter *painter);
    void drawTimeScale(QPainter *painter, int span, int mainSpan);

    void drawTimeRecord(QPainter *painter);
    void drawRecordInfo(QPainter *painter);
    void drawCurrentLine(QPainter *painter);
    //cut
    void drawCut(QPainter *painter);

    //zoom改变后，根据光标位置进行缩放
    void reCalculateRange();

    //光标超出显示范围时移动到中间
    void setPlayCursorCentre();

    //只显示事件下，只找存在事件录像时的缩略图
    bool timeHasEventPlayBack(const QDateTime &dateTime);

signals:
    void timeClicked(const QDateTime &dateTime);

private slots:
    void onHideCurrentTip();
    void onDragTimerTimeout();
    void onTimerThumbReady();
    void onTimerThumbGet();
    //
    void onTimeLineImageFinished(int channel, QImage image);

private:
    TimeLineZoomLevel m_zoomLevel = TimeLineZoomLevel_1;

    QDateTime m_beginDateTime;
    QDateTime m_endDateTime;
    QDateTime m_currentDateTime;
    QDateTime m_tempBeginDateTime;
    QDateTime m_tempEndDateTime;

    //左右空出来一小段，避免时间显示不完整
    int m_padding;

    bool m_pressed = false;
    QPoint m_pressedPoint;
    DragMode m_dragMode = DragNone;

    bool m_showCurrentTip = false;
    QTimer *m_timerHideCurrentTip = nullptr;

    bool m_isDrag = false;
    QTimer *m_dragTimer = nullptr;

    QLabel *m_labelTimeTip;

    //避免时间显示跳动，手动跳转完后第二个RESPONSE_FLAG_GET_PLAYBACK_REALTIME才更新
    bool m_isJumped = false;

    //cut
    bool m_cutEnable = false;
    QDateTime m_cutBeginDateTime;
    QDateTime m_cutEndDateTime;

    //缩略图
    QTimer *m_timerThumbReady = nullptr;
    QTimer *m_timerThumbGet = nullptr;
    QDateTime m_dateTimeUnderMouse;
    bool m_isThumbing = false;

    //
    TimeLineThread *m_timelineThread = nullptr;
    QImage m_timeImage;
};

#endif // PLAYBACKTIMELINE_H
