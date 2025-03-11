#include "msscheduledraw.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include <QMouseEvent>
#include <QPainter>
#include <qmath.h>

MsScheduleDraw::MsScheduleDraw(QWidget *parent)
    : QWidget(parent)
{
    m_scheduleArray = new uint16_t *[MAX_DAY_NUM];
    for (int i = Sunday; i <= Holiday; ++i) {
        m_scheduleArray[i] = new uint16_t[MAX_SCHEDULE_SEC] { 0 };
    }
}

MsScheduleDraw::~MsScheduleDraw()
{
    for (int i = Sunday; i <= Holiday; ++i) {
        delete[] m_scheduleArray[i];
    }
    delete[] m_scheduleArray;
}

void MsScheduleDraw::setCurrentType(const int &type)
{
    m_currentType = type;
}

int MsScheduleDraw::currentType() const
{
    return m_currentType;
}

void MsScheduleDraw::setTypeColor(int type, QColor color)
{
    m_typeColorMap.insert(type, color);
}

void MsScheduleDraw::clearTypeColor()
{
    m_typeColorMap.clear();
}

void MsScheduleDraw::setScheduleMode(int mode)
{
    m_scheduleMode = mode;
}

void MsScheduleDraw::setCheckDayEnable(bool enable)
{
    m_checkDayEnable = enable;
}

void MsScheduleDraw::clearCurrent()
{
    for (int i = Sunday; i <= Holiday; ++i) {
        for (int j = 0; j < MAX_SCHEDULE_SEC; ++j) {
            if (m_scheduleArray[i][j] == m_currentType) {
                m_scheduleArray[i][j] = 0;
            }
        }
    }

    drawImage();
}

void MsScheduleDraw::clearAll()
{
    for (int i = Sunday; i <= Holiday; ++i) {
        memset(m_scheduleArray[i], 0, sizeof(uint16_t) * MAX_SCHEDULE_SEC);
    }

    drawImage();
}

void MsScheduleDraw::selectAll()
{
    for (int i = Sunday; i <= Holiday; ++i) {
        for (int j = 0; j < MAX_SCHEDULE_SEC; ++j) {
            m_scheduleArray[i][j] = static_cast<uint16_t>(m_currentType);
        }
    }

    drawImage();
}

void MsScheduleDraw::setSchedule(schedule_day *schedule_day_array)
{
    for (int i = Sunday; i <= Holiday; ++i) {
        memset(m_scheduleArray[i], 0, sizeof(uint16_t) * MAX_SCHEDULE_SEC);
    }

    for (int i = 0; i < MAX_DAY_NUM; ++i) {
        const schedule_day &day = schedule_day_array[i];
        if (m_checkDayEnable && !day.wholeday_enable) {
            continue;
        }
        const schdule_item *item_array = day.schedule_item;
        for (int j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; ++j) {
            const schdule_item &item = item_array[j];

            const QString &strStart = QString(item.start_time);
            const QString &strEnd = QString(item.end_time);

            if (strStart.isEmpty() || strEnd.isEmpty()) {
                continue;
            }

            //HH:mm
            QRegExp rx1("(\\d+):(\\d+)");
            QRegExp rx2("(\\d+):(\\d+)");
            if (rx1.exactMatch(strStart) && rx2.exactMatch(strEnd)) {
                int beginMinute = rx1.cap(1).toInt() * 60 + rx1.cap(2).toInt();
                int endMinute = rx2.cap(1).toInt() * 60 + rx2.cap(2).toInt();

                if (beginMinute < 0) {
                    beginMinute = 0;
                }
                if (beginMinute > 1440) {
                    beginMinute = 1440;
                }
                if (endMinute > 1440) {
                    endMinute = 1440;
                }

                if (beginMinute < endMinute) {
                    int beginSec = beginMinute * 60;
                    int endSec = endMinute * 60;
                    for (int m = beginSec; m <= endSec; ++m) {
                        m_scheduleArray[i][m] = static_cast<uint16_t>(item.action_type);
                    }
                }
            }
        }
    }

    //
    drawImage();
}

void MsScheduleDraw::getSchedule(schedule_day *schedule_day_array)
{
    memset(schedule_day_array, 0, sizeof(schedule_day) * MAX_DAY_NUM);
    for (int i = Sunday; i <= Holiday; ++i) {
        schedule_day &s_day = schedule_day_array[i];
        if (m_checkDayEnable) {
            s_day.wholeday_enable = 1;
        }
        schdule_item *item_array = s_day.schedule_item;
        QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(m_scheduleArray[i]);
        int index = 0;
        for (int j = 0; j < times.size(); ++j) {
            const ScheduleTime &time = times.at(j);

            schdule_item &s_item = item_array[index];
            s_item.action_type = time.type();
            QString strBegin = QString("%1:%2").arg(time.beginMinute() / 60, 2, 10, QLatin1Char('0')).arg(time.beginMinute() % 60, 2, 10, QLatin1Char('0'));
            QString strEnd = QString("%1:%2").arg(time.endMinute() / 60, 2, 10, QLatin1Char('0')).arg(time.endMinute() % 60, 2, 10, QLatin1Char('0'));
            snprintf(s_item.start_time, sizeof(s_item.start_time), "%s", strBegin.toStdString().c_str());
            snprintf(s_item.end_time, sizeof(s_item.end_time), "%s", strEnd.toStdString().c_str());

            index++;
        }
    }
}

void MsScheduleDraw::setScheduleArray(uint16_t **dayArray)
{
    for (int i = Sunday; i <= Holiday; ++i) {
        memcpy(m_scheduleArray[i], dayArray[i], sizeof(uint16_t) * MAX_SCHEDULE_SEC);
    }
    drawImage();
}

uint16_t **MsScheduleDraw::scheduleArray() const
{
    return m_scheduleArray;
}

QSize MsScheduleDraw::sizeHint() const
{
    return QSize(900, 300);
}

void MsScheduleDraw::showEvent(QShowEvent *)
{
    drawImage();
}

void MsScheduleDraw::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.drawImage(rect(), m_image);
    drawRubberBand(painter);
}

void MsScheduleDraw::mousePressEvent(QMouseEvent *event)
{
    m_pressPoint = event->pos();
    if (m_drawRect.contains(m_pressPoint)) {
        m_isPressed = true;
    }
}

void MsScheduleDraw::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_isPressed = false;

    const qreal &w = m_drawRect.width() / 24.0;
    const qreal &h = m_drawRect.height() / dayCount();
    int beginHour = qFloor((m_rubberBand.left() - m_drawRect.left()) / w);
    int endHour = qCeil((m_rubberBand.right() - m_drawRect.left()) / w);
    int beginDay = qFloor((m_rubberBand.top() - m_drawRect.top()) / h);
    int endDay = qCeil((m_rubberBand.bottom() - m_drawRect.top()) / h) - 1;
    if (beginHour >= 0 && beginDay >= 0) {
        int beginSecs = beginHour * 3600;
        int endSecs = endHour * 3600;
        //绘制后的结果小于12段，才可绘制
        for (int day = beginDay; day <= endDay; ++day) {
            uint16_t *tempDay = new uint16_t[MAX_SCHEDULE_SEC];
            memcpy(tempDay, m_scheduleArray[day], sizeof(uint16_t) * MAX_SCHEDULE_SEC);
            for (int sec = beginSecs; sec <= endSecs; ++sec) {
                tempDay[sec] = static_cast<uint16_t>(m_currentType);
            }
            QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(tempDay);
            if (times.count() <= 12) {
                memcpy(m_scheduleArray[day], tempDay, sizeof(uint16_t) * MAX_SCHEDULE_SEC);
            }
            delete[] tempDay;
        }
    }

    m_rubberBand = QRectF();

    drawImage();
}

void MsScheduleDraw::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) {
        m_rubberBand = QRect(m_pressPoint, event->pos()).normalized();
        if (m_rubberBand.left() < m_drawRect.left()) {
            m_rubberBand.setLeft(m_drawRect.left());
        }
        if (m_rubberBand.top() < m_drawRect.top()) {
            m_rubberBand.setTop(m_drawRect.top());
        }
        if (m_rubberBand.right() > m_drawRect.right()) {
            m_rubberBand.setRight(m_drawRect.right());
        }
        if (m_rubberBand.bottom() > m_drawRect.bottom()) {
            m_rubberBand.setBottom(m_drawRect.bottom());
        }
        update();
    }
}

int MsScheduleDraw::dayCount() const
{
    switch (m_scheduleMode) {
    case Mode_Normal:
        return 8;
    case Mode_NoHoliday:
        return 7;
    case Mode_OnlySunday:
        return 1;
    default:
        return 8;
    }
}

QColor MsScheduleDraw::scheduleColor(int type)
{
    QColor color;

    if (m_typeColorMap.contains(type)) {
        color = m_typeColorMap.value(type);
        return color;
    }

    switch (type) {
    case TIMING_RECORD:
    case VIDEOLOSS_ACTION:
        color = QColor("#B8D7F4");
        break;
    case MOTION_RECORD:
    case MOTION_ACTION:
        color = QColor("#1BC15B");
        break;
    case ALARM_RECORD:
    case ALARMIN_ACTION:
    case ALARMOUT_ACTION:
        color = QColor("#FF3600");
        break;
    case SMART_EVT_RECORD:
        color = QColor("#FCAEC8");
        break;
    case EVENT_RECORD:
        color = QColor("#D01FAD");
        break;
    case ANPT_EVT_RECORD:
    case PRI_PEOPLECNT_ACTION:
    case POS_RECORD:
    case REGIONAL_RECORD:
    case FACE_RECORD:
        color = QColor("#FFE793");
        break;
    case AUDIO_ALARM_RECORD:
        color = QColor("#A484FF");
        break;
    default:
        color = QColor("#DEDEDE");
        break;
    }
    return color;
}

void MsScheduleDraw::drawImage()
{
    if (!isVisible()) {
        return;
    }
    m_image = QImage(size(), QImage::Format_ARGB32);
    m_image.fill(Qt::transparent);

    QPainter painter(&m_image);
    const int &w = width();
    const int &h = height();

    const int timeHeight = 20;
    const int marginRight = 10;
    const int marginBottom = 2;

    m_daysLabelRect.setLeft(0);
    m_daysLabelRect.setTop(timeHeight);
    m_daysLabelRect.setRight((w - marginRight - 20) / 7);
    m_daysLabelRect.setBottom(h - marginBottom);

    m_timeHeaderRect.setLeft(0);
    m_timeHeaderRect.setTop(0);
    m_timeHeaderRect.setRight(w);
    m_timeHeaderRect.setBottom(timeHeight);

    m_drawRect.setLeft(m_daysLabelRect.right() + 20);
    m_drawRect.setTop(timeHeight);
    m_drawRect.setRight(w - marginRight);
    m_drawRect.setBottom(h - marginBottom);

    m_dayHeight = (qreal)m_drawRect.height() / dayCount();

    drawDaysLabel(painter);
    drawTimeHeader(painter);
    drawSchedule(painter);

    update();
}

void MsScheduleDraw::drawDaysLabel(QPainter &painter)
{
    QRectF rc;
    rc.setLeft(m_daysLabelRect.left());
    rc.setRight(m_daysLabelRect.right());

    const qreal &h = (qreal)m_daysLabelRect.height() / dayCount();

    QColor backgroundColor;
    QString text;

    painter.save();
    for (int i = 0; i < dayCount(); ++i) {
        rc.setTop(m_daysLabelRect.top() + h * i);
        rc.setBottom(rc.top() + h);
        switch (i) {
        case Sunday:
            backgroundColor = QColor("#067FAC");
            if (m_scheduleMode == Mode_OnlySunday) {
                text = GET_TEXT("IMAGE/162003", "Day/Night");
            } else {
                text = GET_TEXT("COMMON/1024", "Sunday");
            }
            break;
        case Monday:
            backgroundColor = QColor("#09A8E2");
            text = GET_TEXT("COMMON/1025", "Monday");
            break;
        case Tuesday:
            backgroundColor = QColor("#09A8E2");
            text = GET_TEXT("COMMON/1026", "Tuesday");
            break;
        case Wednesday:
            backgroundColor = QColor("#09A8E2");
            text = GET_TEXT("COMMON/1027", "Wednesday");
            break;
        case Thursday:
            backgroundColor = QColor("#09A8E2");
            text = GET_TEXT("COMMON/1028", "Thursday");
            break;
        case Friday:
            backgroundColor = QColor("#09A8E2");
            text = GET_TEXT("COMMON/1029", "Friday");
            break;
        case Saturday:
            backgroundColor = QColor("#067FAC");
            text = GET_TEXT("COMMON/1030", "Saturday");
            break;
        case Holiday:
            backgroundColor = QColor("#067FAC");
            text = GET_TEXT("COMMON/1031", "Holiday");
            break;
        }
        painter.setPen(QColor("#BDFFFF"));
        painter.setBrush(backgroundColor);
        painter.drawRect(rc);

        QRectF textRc = rc;
        textRc.setLeft(rc.left() + 15);
        painter.setPen(QColor("#FFFFFF"));
        painter.drawText(textRc, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
    painter.restore();
}

void MsScheduleDraw::drawTimeHeader(QPainter &painter)
{
    painter.save();
    painter.setPen(QColor("#4A4A4A"));
    const qreal &w = m_drawRect.width() / 24.0;
    const qreal &y = m_timeHeaderRect.center().y();
    for (int i = 0; i <= 24; ++i) {
        if (i % 2 == 0) {
            QString text = QString("%1").arg(i);
            QFontMetrics fm(painter.font());
            int fontWidth = fm.width(text);
            qreal x = m_drawRect.left() + w * i - fontWidth / 2.0;
            QPointF point(x, y);
            painter.drawText(point, QString("%1").arg(i));
        }
    }
    painter.restore();
}

void MsScheduleDraw::drawSchedule(QPainter &painter)
{
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#DEDEDE"));
    painter.drawRect(m_drawRect);
    //
    for (int day = Sunday; day <= Holiday; ++day) {
        if (day >= dayCount()) {
            break;
        }
        qreal top = m_drawRect.top() + m_dayHeight * day;
        qreal bottom = m_drawRect.top() + m_dayHeight * (day + 1);
        QList<ScheduleTime> times = ScheduleTime::fromSecondsHash(m_scheduleArray[day]);
        for (int j = 0; j < times.size(); ++j) {
            const ScheduleTime &time = times.at(j);
            qreal left = m_drawRect.left() + m_drawRect.width() * time.beginMinute() / 1440;
            qreal right = m_drawRect.left() + m_drawRect.width() * time.endMinute() / 1440;
            painter.setBrush(scheduleColor(time.type()));
            painter.drawRect(QRectF(QPointF(left, top), QPointF(right, bottom)));
        }
    }
    painter.restore();
    //
    painter.save();
    painter.setPen(QColor("#FFFFFF"));
    const qreal &w = m_drawRect.width() / 24.0;
    for (qreal x = m_drawRect.left(); x <= m_drawRect.right(); x += w) {
        QPointF point1(x, m_drawRect.top());
        QPointF point2(x, m_drawRect.bottom());
        painter.drawLine(point1, point2);
    }
    const qreal &h = (qreal)m_drawRect.height() / dayCount();
    for (qreal y = m_drawRect.top(); y <= m_drawRect.bottom(); y += h) {
        QPointF point1(m_drawRect.left(), y);
        QPointF point2(m_drawRect.right(), y);
        painter.drawLine(point1, point2);
    }
    painter.restore();
}

void MsScheduleDraw::drawRubberBand(QPainter &painter)
{
    if (!m_isPressed) {
        return;
    }
    painter.save();
    painter.setPen(QColor("#4A4A4A"));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_rubberBand);
    painter.restore();
}
