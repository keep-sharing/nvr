#include "GridPoint.h"

GridPoint::GridPoint()
{
}

GridPoint::GridPoint(int r, int c)
    : m_row(r)
    , m_column(c)
{
}

int GridPoint::row() const
{
    return m_row;
}

void GridPoint::setRow(int newRow)
{
    m_row = newRow;
}

int GridPoint::column() const
{
    return m_column;
}

void GridPoint::setColumn(int newColumn)
{
    m_column = newColumn;
}

bool GridPoint::operator<(const GridPoint &other) const
{
    if (row() != other.row()) {
        return row() < other.row();
    } else {
        return column() < other.column();
    }
}
