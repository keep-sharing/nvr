#include "MsColor.h"

MsColor::MsColor(QObject *parent)
    : QObject(parent)
{
    settings.text = QColor(74, 74, 74);
}

MsColor &MsColor::instance()
{
    static MsColor self;
    return self;
}

QColor MsColor::backgroundBlue() const
{
    return QColor(10, 169, 227);
}
