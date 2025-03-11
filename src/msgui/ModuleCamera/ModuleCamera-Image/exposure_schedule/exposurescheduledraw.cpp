#include "exposurescheduledraw.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include <QMouseEvent>
#include <QPainter>
#include <qmath.h>

ExposureScheduleDraw::ExposureScheduleDraw(QWidget *parent)
    : QWidget(parent)
{
    m_scheduleArray = new ExposureManualValue *[MAX_DAY_NUM_IPC];
    for (int i = Sunday; i <= Saturday; ++i) {
        m_scheduleArray[i] = new ExposureManualValue[MAX_SCHEDULE_SEC] {};
    }
    for (int day = Sunday; day <= Saturday; ++day) {
        insertTime(day, ExposureScheduleTime(ExposureManualValue(ExposureActionAuto, 0, 0), 0 * 60, 24 * 60));
    }
}

ExposureScheduleDraw::~ExposureScheduleDraw()
{
    for (int i = Sunday; i <= Saturday; ++i) {
        delete[] m_scheduleArray[i];
    }
    delete[] m_scheduleArray;
}

void ExposureScheduleDraw::setCurrentType(const ExposureManualValue &type)
{
    m_currentType = type;
}

ExposureManualValue ExposureScheduleDraw::currentType() const
{
    return m_currentType;
}

void ExposureScheduleDraw::clearAll()
{
    for (int i = Sunday; i <= Saturday; ++i) {
        for (int j = 0; j < MAX_SCHEDULE_SEC; j++) {
            m_scheduleArray[i][j] = ExposureManualValue(0, 0, 0);
        }
    }
    drawImage();
}

void ExposureScheduleDraw::selectAll()
{
    for (int i = Sunday; i <= Saturday; ++i) {
        for (int j = 0; j < MAX_SCHEDULE_SEC; j++) {
            m_scheduleArray[i][j] = ExposureManualValue(0, 0, 0);
        }
    }
    for (int day = Sunday; day <= Saturday; ++day) {
        insertTime(day, ExposureScheduleTime(m_currentType, 0 * 60, 24 * 60));
    }
    drawImage();
}

void ExposureScheduleDraw::setSchedule(exposure_schedule_day *schedule_day_array)
{
    for (int i = Sunday; i <= Saturday; ++i) {
        for (int j = 0; j < MAX_SCHEDULE_SEC; j++) {
            m_scheduleArray[i][j] = ExposureManualValue(0, 0, 0);
        }
    }
    for (int i = 0; i < 7; ++i) {
        const exposure_schedule_day &day = schedule_day_array[i];
        const exposure_schedule_item *item_array = day.schedule_item;
        for (int j = 0; j < 48; ++j) {
            const exposure_schedule_item &item = item_array[j];

            const QString &strStart = QString(item.start_time);
            const QString &strEnd = QString(item.end_time);

            if (strStart.isEmpty() || strEnd.isEmpty()) {
                continue;
            }

            ExposureScheduleTime exposureScheduleTime(item);
            if (exposureScheduleTime.isValid()) {
                insertTime(i, exposureScheduleTime);
            }
        }
    }
    drawImage();
}

void ExposureScheduleDraw::getSchedule(exposure_schedule_day *schedule_day_array)
{
    memset(schedule_day_array, 0, sizeof(exposure_schedule_day) * MAX_DAY_NUM);
    for (int day = Sunday; day <= Saturday; ++day) {
        exposure_schedule_day &s_day = schedule_day_array[day];
        exposure_schedule_item *item_array = s_day.schedule_item;
        QList<ExposureScheduleTime> times = ExposureScheduleTime::fromSecondsHash(m_scheduleArray[day]);
        int index = 0;
        for (int i = 0; i < times.size(); ++i) {
            const ExposureScheduleTime &time = times.at(i);

            exposure_schedule_item &s_item = item_array[index];
            time.getExposureItem(&s_item);

            index++;
        }
    }
}

void ExposureScheduleDraw::setScheduleArray(ExposureManualValue **dayArray)
{
    for (int i = Sunday; i <= Saturday; ++i) {
        memcpy(m_scheduleArray[i], dayArray[i], sizeof(ExposureManualValue) * MAX_SCHEDULE_SEC);
    }
    drawImage();
}

ExposureManualValue **ExposureScheduleDraw::scheduleArray() const
{
    return m_scheduleArray;
}

QSize ExposureScheduleDraw::sizeHint() const
{
    return QSize(900, 300);
}

void ExposureScheduleDraw::showEvent(QShowEvent *)
{
    drawImage();
}

void ExposureScheduleDraw::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.drawImage(rect(), m_image);
    drawRubberBand(painter);
}

void ExposureScheduleDraw::mousePressEvent(QMouseEvent *event)
{
    m_pressPoint = event->pos();
    if (m_drawRect.contains(m_pressPoint)) {
        m_isPressed = true;
    }
}

void ExposureScheduleDraw::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_isPressed = false;

    const qreal &w = m_drawRect.width() / 24.0;
    const qreal &h = m_drawRect.height() / 7.0;
    int beginHour = qFloor((m_rubberBand.left() - m_drawRect.left()) / w);
    int endHour = qCeil((m_rubberBand.right() - m_drawRect.left()) / w);
    int beginDay = qFloor((m_rubberBand.top() - m_drawRect.top()) / h);
    int endDay = qCeil((m_rubberBand.bottom() - m_drawRect.top()) / h) - 1;
    if (beginHour >= 0 && beginDay >= 0) {
        for (int day = beginDay; day <= endDay; ++day) {
            insertTime(day, ExposureScheduleTime(m_currentType, beginHour * 60, endHour * 60));
        }
    }

    m_rubberBand = QRectF();
    drawImage();
}

void ExposureScheduleDraw::mouseMoveEvent(QMouseEvent *event)
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

int ExposureScheduleDraw::dayCount() const
{
    return 7;
}

QColor ExposureScheduleDraw::scheduleColor(const ExposureManualValue &value)
{
    QColor color;
    switch (value.actionType) {
    case ExposureActionAuto:
        color = QColor("#DEDEDE");
        break;
    case ExposureActionManual:
        color = QColor("#1BC15B");
        break;
    default:
        break;
    }

    return color;
}

void ExposureScheduleDraw::drawImage()
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

void ExposureScheduleDraw::drawDaysLabel(QPainter &painter)
{
    QRectF rc;
    rc.setLeft(m_daysLabelRect.left());
    rc.setRight(m_daysLabelRect.right());

    const qreal &h = m_daysLabelRect.height() / 7.0;

    QColor backgroundColor;
    QString text;

    painter.save();
    for (int i = 0; i < 7; ++i) {
        rc.setTop(m_daysLabelRect.top() + h * i);
        rc.setBottom(rc.top() + h);
        switch (i) {
        case Sunday:
            backgroundColor = QColor("#067FAC");
            text = GET_TEXT("COMMON/1024", "Sunday");
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

void ExposureScheduleDraw::drawTimeHeader(QPainter &painter)
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

void ExposureScheduleDraw::drawSchedule(QPainter &painter)
{
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#DEDEDE"));
    painter.drawRect(m_drawRect);
    //
    for (int day = Sunday; day <= Saturday; ++day) {
        qreal top = m_drawRect.top() + m_dayHeight * day;
        qreal bottom = m_drawRect.top() + m_dayHeight * (day + 1);
        QList<ExposureScheduleTime> times = ExposureScheduleTime::fromSecondsHash(m_scheduleArray[day]);
        for (int i = 0; i < times.size(); ++i) {
            const ExposureScheduleTime &time = times.at(i);
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
    const qreal &h = m_drawRect.height() / 7.0;
    for (qreal y = m_drawRect.top(); y <= m_drawRect.bottom(); y += h) {
        QPointF point1(m_drawRect.left(), y);
        QPointF point2(m_drawRect.right(), y);
        painter.drawLine(point1, point2);
    }
    painter.restore();
}

void ExposureScheduleDraw::drawRubberBand(QPainter &painter)
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

void ExposureScheduleDraw::insertTime(int day, const ExposureScheduleTime &time)
{
    //Exposure例外，因为IPC逻辑中空的也算一种类型。所以空段落+绘制段落一共不超过24
    ExposureManualValue *tempDay = new ExposureManualValue[MAX_SCHEDULE_SEC];
    memcpy(tempDay, m_scheduleArray[day], sizeof(ExposureManualValue) * MAX_SCHEDULE_SEC);
    int beginSec = time.beginMinute() * 60;
    int endSec = time.endMinute() * 60;
    for (int i = beginSec; i <= endSec; ++i) {
        tempDay[i] = time.type();
    }
    QList<ExposureScheduleTime> times = ExposureScheduleTime::fromSecondsHash(tempDay);
    if (times.size() <= 24) {
        memcpy(m_scheduleArray[day], tempDay, sizeof(ExposureManualValue) * MAX_SCHEDULE_SEC);
    }
    delete[] tempDay;
}
