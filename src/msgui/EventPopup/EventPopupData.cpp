#include "EventPopupData.h"
#include <QMetaType>

EventPopupData::EventPopupData(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<EventPopupInfo>("EventPopupInfo");
}

EventPopupData &EventPopupData::instance()
{
    static EventPopupData self;
    return self;
}
