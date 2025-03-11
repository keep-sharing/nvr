#include "CloseCommonBackup.h"
#include "MsMessage.h"

CloseCommonBackup::CloseCommonBackup(QObject *parent)
    : MsObject(parent)
{

}

void CloseCommonBackup::close(int channel, int sid)
{
    m_channel = channel;
    m_sid = sid;
    sendMessage(REQUEST_FLAG_SEARCH_COM_PLAYBACK_CLOSE, &m_sid, sizeof(m_sid));
}

void CloseCommonBackup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE:
        ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE(message);
        break;
    }
}

void CloseCommonBackup::ON_RESPONSE_FLAG_SEARCH_COM_PLAYBACK_CLOSE(MessageReceive *message)
{
    Q_UNUSED(message)

    emit finished(m_channel);
}
