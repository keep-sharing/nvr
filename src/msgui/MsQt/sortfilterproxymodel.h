#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

extern const int SortIntRole;

class SortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    enum SortType {
        SortInt,
        SortIP,
        SortPort
    };

    explicit SortFilterProxyModel(QObject *parent = 0);

    void setSortType(int column, SortType type);
    void setSortableForColumn(int column, int enable);

protected:
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

public:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

signals:

public slots:

private:
    QMap<int, SortType> m_mapSortType;
    QMap<int, bool> m_sortableMap;
};

#endif // SORTFILTERPROXYMODEL_H
