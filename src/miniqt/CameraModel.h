#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <QAbstractTableModel>
#include <CameraInfo.h>

class CameraModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColumnChannel,
        ColumnIP,
        ColumnProtocol,
        ColumnCodec,
        ColumnState,
        ColumnRTSP,
        ColumnDelete
    };

    explicit CameraModel(QObject *parent = nullptr);

    void resetModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant itemText(int row, int column) const;

public slots:
    void updateModel(int channel);

private:
    QMap<int, QString> m_horHeadLabels;
    QList<CameraInfo *> m_listData;
};

#endif // CAMERAMODEL_H
