#include "MyFileModel.h"
#include <QStandardItem>
#include <QLocale>
#include <QDateTime>
#include "MsLanguage.h"

Q_DECLARE_METATYPE(QFileInfo)

const int FileInfoRole = Qt::UserRole + 1;

MyFileModel::MyFileModel(QObject *parent) :
    QStandardItemModel(parent)
{
    setColumnCount(4);

    QStringList labels;
    labels << "Name";
    labels << "Size";
    labels << "Type";
    labels << "Date Modified";
    setHorizontalHeaderLabels(labels);
}

void MyFileModel::setFileInfo(int row, const QFileInfo &fileInfo)
{
    QStandardItem *item0 = new QStandardItem();
    item0->setData(QVariant::fromValue(fileInfo), FileInfoRole);
    item0->setText(fileInfo.fileName());

    QStandardItem *item1 = new QStandardItem();
    QStandardItem *item2 = new QStandardItem();
    QStandardItem *item3 = new QStandardItem();
    item3->setText(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));

    if (fileInfo.isDir())
    {
        item0->setIcon(QIcon(":/common/common/folder.png"));
        item1->setText("");
        item2->setText("Folder");
    }
    else
    {
        item0->setIcon(QIcon(":/common/common/file.png"));
        item1->setText(fileSize(fileInfo.size()));
        item2->setText("File");
    }

    setItem(row, 0, item0);
    setItem(row, 1, item1);
    setItem(row, 2, item2);
    setItem(row, 3, item3);
}

QFileInfo MyFileModel::fileInfo(int row)
{
    return data(index(row, 0), FileInfoRole).value<QFileInfo>();
}

QVariant MyFileModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole)
    {
        switch (index.column()) {
        case ColumnName:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignCenter);
        }
    }
    return QStandardItemModel::data(index, role);
}

QString MyFileModel::fileSize(qint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;
    if (bytes >= tb)
        return QString("%1 TB").arg(QLocale().toString(qreal(bytes) / tb - 0.0004, 'f', 3));
    if (bytes >= gb)
        return QString("%1 GB").arg(QLocale().toString(qreal(bytes) / gb - 0.004, 'f', 2));
    if (bytes >= mb)
        return QString("%1 MB").arg(QLocale().toString(qreal(bytes) / mb - 0.04, 'f', 1));
    if (bytes >= kb)
        return QString("%1 KB").arg(QLocale().toString(bytes / kb));
    return QString("%1 bytes").arg(QLocale().toString(bytes));
}

QString MyFileModel::fileSizeFromMb(qint64 mbs)
{
    const qint64 gb = 1024;
    const qint64 tb = 1024 * gb;
    if (mbs >= tb)
        return QString("%1 TB").arg(QLocale().toString(qreal(mbs) / tb - 0.0004, 'f', 3));
    if (mbs >= gb)
        return QString("%1 GB").arg(QLocale().toString(qreal(mbs) / gb - 0.004, 'f', 2));
    return QString("%1 MB").arg(QLocale().toString(mbs));
}

void MyFileModel::onLanguageChanged()
{
    QStringList labels;
    labels << GET_TEXT("COMMON/1051", "Name");
    labels << GET_TEXT("COMMON/1053", "Size");
    labels << GET_TEXT("COMMON/1052", "Type");
    labels << GET_TEXT("COMMON/1054", "Date Modified");
    setHorizontalHeaderLabels(labels);
}
