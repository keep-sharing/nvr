#include "scheduletime.h"
#include "MyDebug.h"

ScheduleTime::ScheduleTime()
{
}

ScheduleTime::ScheduleTime(const int &type, int beginMinute, int endMinute)
    : m_type(type)
    , m_beginMinute(beginMinute)
    , m_endMinute(endMinute)
{
}

QList<ScheduleTime> ScheduleTime::fromSecondsHash(uint16_t* secArray)
{
    QList<ScheduleTime> times;
    int begin = -1;
    int end   = -1;
    int type  = 0;
    for (int i = 0; i <= 86400; ++i) {
        if (begin < 0) {
            begin = i;
            type = static_cast<int>(secArray[i]);
        } else {
            int temp = 0;
            temp = static_cast<int>(secArray[i]);
            if (type != temp || i == 86400) {
                end             = i;
                int beginMinute = qRound(begin / 60.0);
                int endMinute   = qRound(end / 60.0);
                if (type != 0) {
                    times.append(ScheduleTime(type, beginMinute, endMinute));
                }
                begin = -1;
            }
        }
    }
    return times;
}

bool ScheduleTime::isValid() const
{
    return m_beginMinute >= 0 && m_beginMinute < m_endMinute;
}

int ScheduleTime::type() const
{
    return m_type;
}

int ScheduleTime::beginMinute() const
{
    return m_beginMinute;
}

void ScheduleTime::setBeginMinute(int minute)
{
    m_beginMinute = minute;
}

int ScheduleTime::endMinute() const
{
    return m_endMinute;
}

void ScheduleTime::setEndMinute(int minute)
{
    m_endMinute = minute;
}

bool ScheduleTime::operator<(const ScheduleTime &other) const
{
    if (endMinute() <= other.beginMinute()) {
        return true;
    } else {
        return false;
    }
}
