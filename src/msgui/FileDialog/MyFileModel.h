#ifndef MYFILEMODEL_H
#define MYFILEMODEL_H

#include <QStandardItemModel>
#include <QFileInfo>

class MyFileModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum FileColumn
    {
        ColumnName,
        ColumnSize,
        ColumnType,
        ColumnDate
    };

    explicit MyFileModel(QObject *parent = 0);

    void setFileInfo(int row, const QFileInfo &fileInfo);
    QFileInfo fileInfo(int row);

    virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const override;

    static QString fileSize(qint64 bytes);
    static QString fileSizeFromMb(qint64 mbs);

signals:

public slots:
    void onLanguageChanged();
};

#endif // MYFILEMODEL_H
