#ifndef CLOSECOMMONBACKUP_H
#define CLOSECOMMONBACKUP_H

#include "MsObject.h"

extern "C" {
#include "msg.h"
}


class CloseCommonBackup : public MsObject {
    Q_OBJECT

public:
    explicit CloseCommonBackup(QObject *parent = nullptr);

    void close(int channel, int sid);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE(MessageReceive *message);

signals:
    void finished(int sid);

private:
    int m_channel = -1;
    int m_sid = -1;
};

#endif // CLOSECOMMONBACKUP_H
