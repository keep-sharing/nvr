#ifndef ANPRBACKUPINFO_H
#define ANPRBACKUPINFO_H

#include <QDateTime>
#include <QString>

class MessageReceive;

class AnprBackupInfo {
public:
    int channel;
    int sid;
    int port;
    int index;
    QDateTime dateTime;
    QString plate;
    QString type;
    QString plateColor;
    QString vehicleType;
    QString vehicleBrand;
    QString vehicleColor;
    int vehicleSpeed;
    QString region;
    int direction;
    int roiId;

    explicit AnprBackupInfo();
    explicit AnprBackupInfo(MessageReceive *message);
    virtual ~AnprBackupInfo();

    QString typeString() const;

    static QString plateColorString(int value);
    static QString vehicleTypeString(int value);
    static QString vehicleBrandString(int value);
};

#endif // ANPRBACKUPINFO_H
