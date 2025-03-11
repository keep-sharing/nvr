#include "CameraInfoManagement.h"

CameraInfoManagement::CameraInfoManagement(QObject *parent)
    : QObject(parent)
{
}

CameraInfoManagement &CameraInfoManagement::instance()
{
    static CameraInfoManagement self;
    return self;
}

void CameraInfoManagement::updateData(int row)
{
    emit dataChanged(row);
}
