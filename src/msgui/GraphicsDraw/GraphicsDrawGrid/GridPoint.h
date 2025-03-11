#ifndef GRIDPOINT_H
#define GRIDPOINT_H

class GridPoint
{
public:
    explicit GridPoint();
    explicit GridPoint(int r, int c);

    int row() const;
    void setRow(int newRow);

    int column() const;
    void setColumn(int newColumn);

    bool operator< (const GridPoint &other) const;

private:
    int m_row = 0;
    int m_column = 0;
};

#endif // GRIDPOINT_H
