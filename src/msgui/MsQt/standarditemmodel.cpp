#include "standarditemmodel.h"

StandardItemModel::StandardItemModel(QObject *parent) :
    QStandardItemModel(parent)
{

}

QVariant StandardItemModel::data(const QModelIndex &index, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    default:
        break;
    }
    return QStandardItemModel::data(index, role);
}

QVariant StandardItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QStandardItemModel::headerData(section, orientation, role);
}

