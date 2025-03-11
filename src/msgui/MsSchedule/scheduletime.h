#ifndef SCHEDULETIME_H
#define SCHEDULETIME_H

#include <QMap>
#include <QHash>
#include <array>

#define MAX_SCHEDULE_SEC (86400 + 1)

class ScheduleTime
{
public:
    enum TimeState
    {
        TimeNormal,
        TimeIntersectSameType,
        TimeIntersectDifferentType,
        TimeContains
    };

    ScheduleTime();
    ScheduleTime(const int &type, int beginMinute, int endMinute);

    static QList<ScheduleTime> fromSecondsHash(uint16_t* secArray);

    bool isValid() const;

    int type() const;
    int beginMinute() const;
    void setBeginMinute(int minute);
    int endMinute() const;
    void setEndMinute(int minute);

    bool operator <(const ScheduleTime &other) const;

private:

private:
    int m_type = 0;
    int m_beginMinute = 0;
    int m_endMinute = 0;
};

#endif // SCHEDULETIME_H
