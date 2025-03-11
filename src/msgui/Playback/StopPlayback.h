#ifndef STOPPLAYBACK_H
#define STOPPLAYBACK_H

#include "MsObject.h"

extern "C" {
#include "msg.h"
}

class StopPlayback : public MsObject {
    Q_OBJECT

public:
    explicit StopPlayback(QObject *parent = nullptr);

    void stop(int channel);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message);

signals:
    void finished(int channel);

private:
    int m_channel = -1;
};

#endif // STOPPLAYBACK_H
