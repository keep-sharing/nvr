#ifndef DATETIMERANGE_H
#define DATETIMERANGE_H

#include <QDateTime>

class DateTimeRange
{
public:
    DateTimeRange();
    DateTimeRange(const QDateTime &dateTime);
    DateTimeRange(const QDateTime &start, const QDateTime &end);

    bool operator <(const DateTimeRange &other) const;

    QDateTime startDateTime;
    QDateTime endDateTime;
};

#endif // DATETIMERANGE_H
