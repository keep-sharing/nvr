#include "StreamKey.h"
#include <QtDebug>
extern "C" {
#include "msdb.h"
#include "vapi.h"
}

QDebug operator<<(QDebug debug, const StreamKey &c)
{
    debug.nospace() << QString("StreamKey(channel: %1, screen: %2, )")
                           .arg(c.currentChannel())
                           .arg(c.screen());
    return debug.space();
}

StreamKey::StreamKey()
{

}

StreamKey::StreamKey(int channel, int screen)
    : m_currentChannel(channel)
    , m_screen(screen)
{

}

int StreamKey::currentChannel() const
{
    return m_currentChannel;
}

void StreamKey::currentChannel(const int channel)
{
    m_currentChannel = channel;
}

int StreamKey::screen() const
{
    return m_screen;
}

void StreamKey::setScreen(int screen)
{
    m_screen = screen;
}

QString StreamKey::screenString() const
{
    QString text;
    switch (m_screen) {
    case SCREEN_MAIN:
        text = "SCREEN_MAIN";
        break;
    case SCREEN_SUB:
        text = "SCREEN_SUB";
        break;
    }
    return text;
}

bool StreamKey::operator <(const StreamKey &other) const
{
    if (currentChannel() != other.currentChannel()) {
        return currentChannel() < other.currentChannel();
    }
    if (screen() != other.screen()) {
        return screen() < other.screen();
    }
    return false;
}

bool StreamKey::operator ==(const StreamKey &other) const
{
    return currentChannel() == other.currentChannel() && screen() == other.screen() ;
}

bool StreamKey::operator !=(const StreamKey &other) const
{
    return currentChannel() != other.currentChannel() || screen() != other.screen() ;
}




