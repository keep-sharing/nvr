#include "ptzbasepage.h"
#include "PagePtzConfiguration.h"
#include "MessageBox.h"
#include "MsWaitting.h"

int PtzBasePage::s_currentChannel = 0;

PtzBasePage::PtzBasePage(QWidget *parent) :
    MsWidget(parent)
{
    m_ptzManager = static_cast<PagePtzConfiguration *>(parent);
}

void PtzBasePage::back()
{
    if (m_ptzManager)
    {
        m_ptzManager->back();
    }
}

int PtzBasePage::currentChannel()
{
    return s_currentChannel;
}

void PtzBasePage::setCurrentChannel(int channel)
{
    s_currentChannel = channel;
}

void PtzBasePage::showWait()
{
    if (!m_waitting)
    {
        m_waitting = new MsWaitting(this);
    }
    //m_waitting->//showWait();
}

void PtzBasePage::closeWait()
{
    if (!m_waitting)
    {
        return;
    }
    //m_waitting->//closeWait();
}
