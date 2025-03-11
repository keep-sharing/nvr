#ifndef MYFILESORTMODEL_H
#define MYFILESORTMODEL_H

#include <QSortFilterProxyModel>

class MyFileSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MyFileSortModel(QObject *parent = 0);

    void reSort();

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

protected:
    virtual bool lessThan(const QModelIndex & left, const QModelIndex & right) const override;

signals:

public slots:
};

#endif // MYFILESORTMODEL_H
