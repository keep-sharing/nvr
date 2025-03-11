#include "camerakey.h"

CameraKey::CameraKey()
{

}

CameraKey::CameraKey(const QString &str_ip, const QString &str_mac)
{
    ip = str_ip;
    mac = str_mac;
}

bool CameraKey::operator <(const CameraKey &other) const
{
    if (ip != other.ip)
    {
        return ip < other.ip;
    }
    else
    {
        return mac < other.mac;
    }
}

bool CameraKey::operator ==(const CameraKey &other) const
{
    return ip == other.ip && mac == other.mac;
}
