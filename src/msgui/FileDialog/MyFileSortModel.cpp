#include "MyFileSortModel.h"
#include "MyFileModel.h"

MyFileSortModel::MyFileSortModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{

}

void MyFileSortModel::reSort()
{
    sort(sortColumn(), sortOrder());
}

void MyFileSortModel::sort(int column, Qt::SortOrder order)
{
    return QSortFilterProxyModel::sort(column, order);
}

bool MyFileSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    switch (left.column())
    {
    case MyFileModel::ColumnName:
    {
        MyFileModel *fileModel = static_cast<MyFileModel *>(sourceModel());
        const QFileInfo &leftFileInfo = fileModel->fileInfo(left.row());
        const QFileInfo &rightFileInfo = fileModel->fileInfo(right.row());
        if (leftFileInfo.isDir() && rightFileInfo.isDir())
        {
            return QSortFilterProxyModel::lessThan(left, right);
        }
        else if (leftFileInfo.isDir() && !rightFileInfo.isDir())
        {
            return true;
        }
        else if (!leftFileInfo.isDir() && rightFileInfo.isDir())
        {
            return false;
        }
        else
        {
            return QSortFilterProxyModel::lessThan(left, right);
        }
    }
    case MyFileModel::ColumnSize:
    {
        MyFileModel *fileModel = static_cast<MyFileModel *>(sourceModel());
        const QFileInfo &leftFileInfo = fileModel->fileInfo(left.row());
        const QFileInfo &rightFileInfo = fileModel->fileInfo(right.row());
        if (leftFileInfo.isDir() && !rightFileInfo.isDir())
        {
            return true;
        }
        else if (!leftFileInfo.isDir() && rightFileInfo.isDir())
        {
            return false;
        }
        else
        {
            return leftFileInfo.size() < rightFileInfo.size();
        }
    }
    case MyFileModel::ColumnDate:
    {
        MyFileModel *fileModel = static_cast<MyFileModel *>(sourceModel());
        const QFileInfo &leftFileInfo = fileModel->fileInfo(left.row());
        const QFileInfo &rightFileInfo = fileModel->fileInfo(right.row());
        if (leftFileInfo.isDir() && !rightFileInfo.isDir())
        {
            return true;
        }
        else if (!leftFileInfo.isDir() && rightFileInfo.isDir())
        {
            return false;
        }
        else
        {
            return QSortFilterProxyModel::lessThan(left, right);
        }
    }
    default:
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

