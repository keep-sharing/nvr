#ifndef COMMONVIDEODATA_H
#define COMMONVIDEODATA_H

#include <QObject>

#define gCommonVideoData CommonVideoData::instance

class CommonVideoData : public QObject
{
    Q_OBJECT
public:
    explicit CommonVideoData(QObject *parent = nullptr);

    static CommonVideoData &instance();

signals:

};

#endif // COMMONVIDEODATA_H
