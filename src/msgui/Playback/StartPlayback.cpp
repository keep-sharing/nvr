#include "StartPlayback.h"
#include "MsMessage.h"
#include "MyDebug.h"

StartPlayback::StartPlayback(QObject *parent)
    : MsObject(parent)
{

}

void StartPlayback::start(const rep_start_all_play &play)
{
    m_channel = -1;
    int size  = sizeof(play.chnMaskl);
    for (int i = 0; i < size; ++i) {
        if (play.chnMaskl[i] == '1') {
            m_channel = i;
            break;
        }
    }

    memcpy(&m_reqPlay, &play, sizeof(rep_start_all_play));
    sendMessage(REQUEST_FLAG_START_ALL_PLAYBACK, &play, sizeof(rep_start_all_play));
}

void StartPlayback::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_START_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_START_ALL_PLAYBACK(message);
        break;
    }
}

void StartPlayback::ON_RESPONSE_FLAG_START_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    qMsDebug() << QString("start playback finished, channel:%1").arg(m_channel);
    emit finished(m_channel);
}
