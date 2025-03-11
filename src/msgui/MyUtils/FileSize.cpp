#include "FileSize.h"
#include <QLocale>

FileSize::FileSize()
{
}

QString FileSize::sizeString(qint64 bytes)
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
