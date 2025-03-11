#ifndef CAMERAKEY_H
#define CAMERAKEY_H

#include <QString>

class CameraKey
{
public:
    CameraKey();
    CameraKey(const QString &str_ip, const QString &str_mac);

    bool operator <(const CameraKey &other) const;
    bool operator ==(const CameraKey &other) const;

    QString ip;
    QString mac;

private:
};

#endif // CAMERAKEY_H
