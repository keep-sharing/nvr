#include "sortfilterproxymodel.h"
#include <QHostAddress>
#include <QStringList>
const int SortIntRole = Qt::UserRole + 10;

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void SortFilterProxyModel::setSortType(int column, SortFilterProxyModel::SortType type)
{
    m_mapSortType.insert(column, type);
}

void SortFilterProxyModel::setSortableForColumn(int column, int enable)
{
    m_sortableMap.insert(column, enable);
}

bool SortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int column = source_left.column();
    if (m_mapSortType.contains(column)) {
        SortType model = m_mapSortType.value(column);
        switch (model) {
        case SortInt: {
            qulonglong left = source_left.data(SortIntRole).toULongLong();
            qulonglong right = source_right.data(SortIntRole).toULongLong();
            return left < right;
        }
        case SortIP: {
            QString left = source_left.data().toString();
            QString right = source_right.data().toString();
            quint32 leftip = QHostAddress(left).toIPv4Address();
            quint32 rightip = QHostAddress(right).toIPv4Address();
            return leftip < rightip;
        }
        case SortPort: {
            QString left = source_left.data(Qt::DisplayRole).toString();
            QString right = source_right.data(Qt::DisplayRole).toString();
            int leftIsPoe = left.indexOf(",");
            int rightIsPoe = right.indexOf(",");
            if (leftIsPoe < 0 && rightIsPoe < 0) {
                return left.toInt() < right.toInt();
            } else if (leftIsPoe < 0) {
                return true;
            } else if (rightIsPoe < 0) {
                return false;
            } else {
                QStringList xs = left.split(", ", QString::SkipEmptyParts);
                QStringList ys = right.split(", ", QString::SkipEmptyParts);
                if (xs.size() == 2 && ys.size() == 2) {
                    if (xs.at(0).toInt() != ys.at(0).toInt()) {
                        return xs.at(0).toInt() < ys.at(0).toInt();
                    } else {
                        return xs.at(1).toInt() < ys.at(1).toInt();
                    }
                }
            }
            return left.toInt() < right.toInt();
        }
        default:
            return QSortFilterProxyModel::lessThan(source_left, source_right);
            break;
        }
    } else {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
}

void SortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    if (m_sortableMap.contains(column) && !m_sortableMap.value(column)) {
        return;
    }
    QSortFilterProxyModel::sort(column, order);
}
