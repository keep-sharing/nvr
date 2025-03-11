#include "VideoPosition.h"
#include <QtDebug>

QDebug operator<<(QDebug debug, const VideoPosition &c)
{
    debug.nospace() << QString("VideoPosition(index: %1, channel: %2, row: %3, column: %4, rowSpan: %5, columnSpan: %6)")
                           .arg(c.index).arg(c.channel).arg(c.row).arg(c.column).arg(c.rowSpan).arg(c.columnSpan);
    return debug.space();
}

VideoPosition::VideoPosition()
{

}

VideoPosition::VideoPosition(int r, int c, int rSpan, int cSpan)
{
    row = r;
    column = c;
    rowSpan = rSpan;
    columnSpan = cSpan;
}

bool VideoPosition::operator<(const VideoPosition &other) const
{
    if (index < 0)
    {
        if (row == other.row)
        {
            return column < other.column;
        }
        else
        {
            return row < other.row;
        }
    }
    else
    {
        return index < other.index;
    }
}

bool VideoPosition::operator==(const VideoPosition &other) const
{
    if (index < 0)
    {
        return row == other.row && column == other.column;
    }
    else
    {
        return index == other.index;
    }
}
