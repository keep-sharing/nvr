#ifndef FACEBACKUPINFO_H
#define FACEBACKUPINFO_H
#include <QDateTime>
#include <QString>
extern "C" {
#include "msg.h"
}
class MessageReceive;

class FaceBackupInfo {
public:
    explicit FaceBackupInfo();
    explicit FaceBackupInfo(MessageReceive *message);
    virtual ~FaceBackupInfo();
private:
    QString ageString(FACE_AGE value);
    QString genderString(int value);
    QString maskAndCapString(int value);
public:
    int channel;
    int sid;
    int port;
    int index;
    QDateTime dateTime;
    QString ageType;
    QString genderType;
    QString glassesType;
    QString maskType;
    QString capType;
};

#endif // FACEBACKUPINFO_H
