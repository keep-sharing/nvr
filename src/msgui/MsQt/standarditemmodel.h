#ifndef STANDARDITEMMODEL_H
#define STANDARDITEMMODEL_H

#include <QStandardItemModel>

class StandardItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit StandardItemModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:

public slots:
};

#endif // STANDARDITEMMODEL_H
