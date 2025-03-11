#include "StopPlayback.h"
#include "MsMessage.h"
#include "BasePlayback.h"

StopPlayback::StopPlayback(QObject *parent)
    : MsObject(parent)
{

}

void StopPlayback::stop(int channel)
{
    m_channel = channel;

    char chnMaskl[MAX_LEN_64] = { 0 };
    BasePlayback::makeChannelMask(channel, chnMaskl, sizeof(chnMaskl));
    sendMessage(REQUEST_FLAG_STOP_ALL_PLAYBACK, &chnMaskl, sizeof(chnMaskl));
}

void StopPlayback::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_STOP_ALL_PLAYBACK:
        ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(message);
        break;
    }
}

void StopPlayback::ON_RESPONSE_FLAG_STOP_ALL_PLAYBACK(MessageReceive *message)
{
    Q_UNUSED(message)

    emit finished(m_channel);
}
