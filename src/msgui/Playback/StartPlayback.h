#ifndef STARTPLAYBACK_H
#define STARTPLAYBACK_H

#include "MsObject.h"

extern "C" {
#include "msg.h"
}

class StartPlayback : public MsObject {
    Q_OBJECT

public:
    explicit StartPlayback(QObject *parent = nullptr);

    void start(const rep_start_all_play & play);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message);

signals:
    void finished(int channel);

private:
    rep_start_all_play m_reqPlay;
    int m_channel = -1;
};

#endif // STARTPLAYBACK_H
