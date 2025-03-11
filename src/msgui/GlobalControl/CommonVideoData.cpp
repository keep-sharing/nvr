#include "CommonVideoData.h"

CommonVideoData::CommonVideoData(QObject *parent)
    : QObject(parent)
{

}

CommonVideoData &CommonVideoData::instance()
{
    static CommonVideoData self;
    return self;
}
