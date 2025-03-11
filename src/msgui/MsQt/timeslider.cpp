#include "timeslider.h"
#include <QPainter>
#include <QtDebug>

TimeSlider::TimeSlider(QWidget *parent) :
    BaseSlider(parent)
{
    setShowValue(false);
}

void TimeSlider::setTimeRange(const QDateTime &begin, const QDateTime &end)
{
    qDebug() << QString("TimeSlider::setTimeRange, begin: %1, end: %2").arg(begin.toString("yyyy-MM-dd HH:mm:ss")).arg(end.toString("yyyy-MM-dd HH:mm:ss"));
    setRange(begin.toTime_t(), end.toTime_t());
}

void TimeSlider::setCurrentDateTime(const QDateTime &datetime)
{
    m_value = datetime.toTime_t();
    update();
}

QString TimeSlider::tipText()
{
    quint32 value = valueUnderMouse(m_mouseMovePos);
    QDateTime dateTime = QDateTime::fromTime_t(value);
    return dateTime.time().toString("HH:mm:ss");
}
