#include "MsChannel.h"

MsChannel::MsChannel()
{
}

MsChannel::MsChannel(int screen, int channel)
    : m_screen(screen)
    , m_channel(channel)
{
}

bool MsChannel::valid() const
{
    if (m_screen < 0) {
        return false;
    }
    if (m_channel < 0) {
        return false;
    }
    return true;
}

int MsChannel::screen() const
{
    return m_screen;
}

int MsChannel::channel() const
{
    return m_channel;
}
