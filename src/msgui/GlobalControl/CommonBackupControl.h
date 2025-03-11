#ifndef COMMONBACKUPCONTROL_H
#define COMMONBACKUPCONTROL_H

#include "MsObject.h"
#include <QDateTime>
#include <QEventLoop>
#include <QImage>
#include <QPixmap>

extern "C" {
#include "msg.h"

}

class CommonBackupControl : public MsObject {
    Q_OBJECT

public:
    explicit CommonBackupControl(QObject *parent = nullptr);

    int preSec() const;
    void setPreSec(int newPreSec);

    int postSec() const;
    void setPostSec(int newPostSec);

    int streamType() const;
    int fileSize() const;
    QPixmap pixmap() const;
    QImage image() const;

    bool hasBackup() const;
    bool isPlaying() const;

    const QDateTime &realStartDateTime() const;
    const QDateTime &realEndDateTime() const;

    int waitForSearchCommonBackup(int channel, const QDateTime &begin, const QDateTime &end, int streamType = -1);
    int waitForPlayCommonBackup();
    int waitForPauseCommonBackup();
    int waitForResumeCommonBackup();
    int waitForStopCommonBackup();
    int waitForCloseCommonBackup();
    int waitForSeekCommonBackup(const QDateTime &dateTime);

    int waitForSearchCommonBackupPicture();

    QDateTime getPlaybackRealTime();

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYPAUSE_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYRESTART_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYSTOP_COM_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_COM_BACKUP_CLOSE(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAYSEEK_BACKUP(MessageReceive *message);
    //
    void ON_RESPONSE_FLAG_PLAY_COM_PICTURE(MessageReceive *message);

signals:

private:
    QEventLoop m_eventLoop;

    int m_channel    = -1;
    int m_streamType = -1;
    int m_preSec     = 0;
    int m_postSec    = 0;
    int m_sid        = -1;
    bool m_isPlaying = false;
    QPixmap m_pixmap;
    QList<resp_search_common_backup> m_playBackupList;

    QDateTime m_realStartDateTime;
    QDateTime m_realEndDateTime;
};

#endif // COMMONBACKUPCONTROL_H
