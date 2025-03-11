#include "AbstractAdvancedSettingsPage.h"
#include "AdvancedSettings.h"

AbstractAdvancedSettingsPage::AbstractAdvancedSettingsPage(QWidget *parent) :
    MsWidget(parent)
{
    m_advancedSettings = qobject_cast<AdvancedSettings *>(parent);
    connect(this, SIGNAL(showWait()), m_advancedSettings, SLOT(showWait()));
    connect(this, SIGNAL(closeWait()), m_advancedSettings, SLOT(closeWait()));
}

void AbstractAdvancedSettingsPage::setDrawWidget(QWidget *widget)
{
    if (m_advancedSettings)
        m_advancedSettings->setDrawWidget(widget);
}

void AbstractAdvancedSettingsPage::back()
{
    if (m_advancedSettings)
        m_advancedSettings->back();
}

void AbstractAdvancedSettingsPage::setChannelConnected(bool connected)
{
    m_isConnected = connected;
}

bool AbstractAdvancedSettingsPage::isChannelConnected()
{
    return m_isConnected;
}

void AbstractAdvancedSettingsPage::setChannel(int c)
{
    m_channel = c;
}

int AbstractAdvancedSettingsPage::channel()
{
    return m_channel;
}

void AbstractAdvancedSettingsPage::hideDrawWidget()
{

}
