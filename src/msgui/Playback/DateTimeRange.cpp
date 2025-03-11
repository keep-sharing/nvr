#include "DateTimeRange.h"

DateTimeRange::DateTimeRange()
{

}

DateTimeRange::DateTimeRange(const QDateTime &dateTime)
{
    startDateTime = dateTime;
    endDateTime = dateTime;
}

DateTimeRange::DateTimeRange(const QDateTime &start, const QDateTime &end)
{
    startDateTime = start;
    endDateTime = end;
}

bool DateTimeRange::operator <(const DateTimeRange &other) const
{
    return endDateTime <= other.startDateTime;
}
