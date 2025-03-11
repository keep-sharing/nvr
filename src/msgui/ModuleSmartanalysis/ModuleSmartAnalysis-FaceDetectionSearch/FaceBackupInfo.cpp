#include "FaceBackupInfo.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QDebug>
FaceBackupInfo::FaceBackupInfo()
{
}

FaceBackupInfo::FaceBackupInfo(MessageReceive *message)
{
    resp_search_face_backup *backup = static_cast<resp_search_face_backup *>(message->data);
    channel = backup->chnId;
    sid = backup->sid;
    port = backup->port;
    index = backup->index;
    dateTime = QDateTime::fromString(QString(backup->pTime), "yyyy-MM-dd HH:mm:ss");
    ageType = ageString(backup->sImgAtrb.age);
    genderType = genderString(backup->sImgAtrb.gender);
    glassesType = maskAndCapString(backup->sImgAtrb.glasses);
    maskType = maskAndCapString(backup->sImgAtrb.mask);
    capType = maskAndCapString(backup->sImgAtrb.cap);
}

FaceBackupInfo::~FaceBackupInfo()
{
}

QString FaceBackupInfo::ageString(FACE_AGE value)
{
    QString text;
    switch (value) {
    case FACE_AGE_NONE:
        text = GET_TEXT("FACE/141055", "N/A");
        break;
    case FACE_AGE_CHILD:
        text = GET_TEXT("FACE/141040", "Child");
        break;
    case FACE_AGE_ADULT:
        text = GET_TEXT("FACE/141041", "Adult");
        break;
    case FACE_AGE_ELDERLY:
        text = GET_TEXT("FACE/141042", "Elderly");
        break;
    case FACE_AGE_ALL:
        text = GET_TEXT("COMMON/1006", "All");
        break;
    }
    return text;
}

QString FaceBackupInfo::genderString(int value)
{
    QString text;
    switch (value) {
    case -1:
        text = GET_TEXT("FACE/141055", "N/A");
        break;
    case 0:
        text = GET_TEXT("FACE/141043", "Male");
        break;
    case 1:
        text = GET_TEXT("FACE/141044", "Female");
        break;
    case 2:
        text = GET_TEXT("COMMON/1006", "All");
        break;
    }
    return text;
}

QString FaceBackupInfo::maskAndCapString(int value)
{
    QString text;
    switch (value) {
    case -1:
        text = GET_TEXT("FACE/141055", "N/A");
        break;
    case 0:
        text = GET_TEXT("FACE/141045", "No");
        break;
    case 1:
        text = GET_TEXT("FACE/141048", "Yes");
        break;
    case 2:
        text = GET_TEXT("COMMON/1006", "All");
        break;
    }
    return text;
}
