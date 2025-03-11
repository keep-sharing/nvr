#include "exposurescheduletime.h"
#include <QTime>

ExposureScheduleTime::ExposureScheduleTime()
{
}

ExposureScheduleTime::ExposureScheduleTime(const exposure_schedule_item &item)
{
    m_actionType = item.action_type;
    m_beginMinute = getMinutesFromTimeString(item.start_time);
    m_endMinute = getMinutesFromTimeString(item.end_time);
    m_exposureTime = item.exposureTime;
    m_gainLevel = item.gainLevel;

    m_manualValue = ExposureManualValue(m_actionType, m_exposureTime, m_gainLevel);
}

ExposureScheduleTime::ExposureScheduleTime(const ExposureManualValue &type, int beginMinute, int endMinute)
{
    m_actionType = type.actionType;
    m_exposureTime = type.exposureTime;
    m_gainLevel = type.gainLevel;
    m_beginMinute = beginMinute;
    m_endMinute = endMinute;

    m_manualValue = type;
}

QList<ExposureScheduleTime> ExposureScheduleTime::fromSecondsHash(ExposureManualValue* secArray)
{
    QList<ExposureScheduleTime> times;
    int begin = -1;
    int end = -1;
    ExposureManualValue type;
    for (int i = 0; i <= 86400; ++i) {
        if (begin < 0) {
            begin = i;
            type = secArray[i];
        } else {
            ExposureManualValue temp = secArray[i];
            if (type != temp || i == 86400) {
                end = i;
                int beginMinute = qRound(begin / 60.0);
                int endMinute = qRound(end / 60.0);
                times.append(ExposureScheduleTime(type, beginMinute, endMinute));
                begin = -1;
            }
        }
    }
    return times;
}

bool ExposureScheduleTime::isValid() const
{
    return m_beginMinute >= 0 && m_beginMinute < m_endMinute;
}

ExposureManualValue ExposureScheduleTime::type() const
{
    return m_manualValue;
}

int ExposureScheduleTime::beginMinute() const
{
    return m_beginMinute;
}

void ExposureScheduleTime::setBeginMinute(int minute)
{
    m_beginMinute = minute;
}

int ExposureScheduleTime::endMinute() const
{
    return m_endMinute;
}

void ExposureScheduleTime::setEndMinute(int minute)
{
    m_endMinute = minute;
}

void ExposureScheduleTime::getExposureItem(exposure_schedule_item *item) const
{
    item->action_type = m_actionType;
    item->exposureTime = m_exposureTime;
    item->gainLevel = m_gainLevel;
    QString strBegin = QString("%1:%2").arg(m_beginMinute / 60, 2, 10, QLatin1Char('0')).arg(m_beginMinute % 60, 2, 10, QLatin1Char('0'));
    QString strEnd = QString("%1:%2").arg(m_endMinute / 60, 2, 10, QLatin1Char('0')).arg(m_endMinute % 60, 2, 10, QLatin1Char('0'));
    snprintf(item->start_time, sizeof(item->start_time), "%s", strBegin.toStdString().c_str());
    snprintf(item->end_time, sizeof(item->end_time), "%s", strEnd.toStdString().c_str());
}

QMap<ExposureScheduleTime, int> ExposureScheduleTime::merge(const ExposureScheduleTime &other) const
{
    QMap<ExposureScheduleTime, int> tempMap;
    //other包含this
    if (other.beginMinute() <= beginMinute() && other.endMinute() >= endMinute()) {
        tempMap.insert(other, 0);
    }
    //this包含other
    else if (beginMinute() <= other.beginMinute() && endMinute() >= other.endMinute()) {
        tempMap.insert(other, 0);

        ExposureScheduleTime tempTime = *this;
        tempTime.setBeginMinute(beginMinute());
        tempTime.setEndMinute(other.beginMinute() - 1);
        if (tempTime.isValid()) {
            tempMap.insert(tempTime, 0);
        }

        tempTime.setBeginMinute(other.endMinute() + 1);
        tempTime.setEndMinute(endMinute());
        if (tempTime.isValid()) {
            tempMap.insert(tempTime, 0);
        }
    }
    //this包含other.end
    else if (beginMinute() <= other.endMinute() && endMinute() >= other.endMinute()) {
        if (type() == other.type()) {
            ExposureScheduleTime tempTime = other;
            tempTime.setEndMinute(endMinute());
            if (tempTime.isValid()) {
                tempMap.insert(tempTime, 0);
            }
        } else {
            tempMap.insert(other, 0);

            ExposureScheduleTime tempTime = *this;
            tempTime.setBeginMinute(other.endMinute() + 1);
            if (tempTime.isValid()) {
                tempMap.insert(tempTime, 0);
            }
        }
    }
    //this包含other.begin
    else if (beginMinute() <= other.beginMinute() && endMinute() >= other.beginMinute()) {
        if (type() == other.type()) {
            ExposureScheduleTime tempTime = other;
            tempTime.setBeginMinute(beginMinute());
            if (tempTime.isValid()) {
                tempMap.insert(tempTime, 0);
            }
        } else {
            tempMap.insert(other, 0);

            ExposureScheduleTime tempTime = *this;
            tempTime.setEndMinute(other.beginMinute() - 1);
            if (tempTime.isValid()) {
                tempMap.insert(tempTime, 0);
            }
        }
    }
    return tempMap;
}

bool ExposureScheduleTime::operator<(const ExposureScheduleTime &other) const
{
    if (endMinute() < other.beginMinute()) {
        return true;
    } else {
        return false;
    }
}

int ExposureScheduleTime::getMinutesFromTimeString(const char *time)
{
    int minutes = 0;
    QString strTime(time);
    if (strTime == "24:0") {
        minutes = 1440;
    } else {
        QTime t = QTime::fromString(strTime, "H:m");
        minutes = t.hour() * 60 + t.minute();
    }
    return minutes;
}
