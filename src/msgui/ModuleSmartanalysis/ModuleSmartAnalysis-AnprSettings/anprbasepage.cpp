#include "anprbasepage.h"
#include "PageAnprSettings.h"
#include <QtDebug>

ms_lpr_support_info AnprBasePage::s_lpr_support_info = { 0, 0, 0 };
int AnprBasePage::s_currentChannel = -1;
bool AnprBasePage::s_isConnected = false;

AnprBasePage::AnprBasePage(QWidget *parent)
    : MsWidget(parent)
{
    m_anprSetting = static_cast<PageAnprSettings *>(parent);
}

void AnprBasePage::setAnprSupportInfo(ms_lpr_support_info *lpr_support_info)
{
    memset(&s_lpr_support_info, 0, sizeof(ms_lpr_support_info));
    if (lpr_support_info) {
        memcpy(&s_lpr_support_info, lpr_support_info, sizeof(ms_lpr_support_info));
    }
}

bool AnprBasePage::isAnprSupport()
{
    return s_lpr_support_info.vca_support;
}

int AnprBasePage::anprVersion()
{
    return s_lpr_support_info.vca_version;
}

void AnprBasePage::setCurrentChannel(int channel)
{
    s_currentChannel = channel;
}

int AnprBasePage::currentChannel()
{
    return s_currentChannel;
}

void AnprBasePage::setChannelConnected(bool connected)
{
    s_isConnected = connected;
}

bool AnprBasePage::isChannelConnected()
{
    return s_isConnected;
}

void AnprBasePage::setDrawWidget(QWidget *widget)
{
    if (m_anprSetting) {
        m_anprSetting->setDrawWidget(widget);
    }
}

void AnprBasePage::addGraphicsItem(QGraphicsItem *item)
{
    if (m_anprSetting) {
        m_anprSetting->addGraphicsItem(item);
    }
}

void AnprBasePage::back()
{
    if (m_anprSetting) {
        m_anprSetting->back();
    }
}

void AnprBasePage::showWait()
{
    if (m_anprSetting) {
        //m_anprSetting->//showWait();
    }
}

void AnprBasePage::closeWait()
{
    if (m_anprSetting) {
        //m_anprSetting->//closeWait();
    }
}
