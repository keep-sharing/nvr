#ifndef FILESIZE_H
#define FILESIZE_H

#include <QString>

class FileSize
{
public:
    FileSize();

    static QString sizeString(qint64 bytes);
};

#endif // FILESIZE_H
