#include "AbstractSettingTab.h"
#include "MsLanguage.h"
#include "MsWaitting.h"

int AbstractSettingTab::s_currentChannel = 0;

AbstractSettingTab::AbstractSettingTab(QWidget *parent) :
    MsWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
}

void AbstractSettingTab::setCurrentChannel(int channel)
{
    s_currentChannel = channel;
}

int AbstractSettingTab::currentChannel()
{
    return s_currentChannel;
}

void AbstractSettingTab::back()
{
    emit sig_back();
}

void AbstractSettingTab::showWait()
{
    //MsWaitting::showGlobalWait();
}

void AbstractSettingTab::showWait(QWidget *parent)
{
    Q_UNUSED(parent)
    //MsWaitting::showGlobalWait(parent);
}

void AbstractSettingTab::closeWait()
{
    //MsWaitting::closeGlobalWait();
}

void AbstractSettingTab::initializeData()
{

}

void AbstractSettingTab::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

bool AbstractSettingTab::isCloseable()
{
    return true;
}

bool AbstractSettingTab::isChangeable()
{
    return true;
}

bool AbstractSettingTab::canAutoLogout()
{
    return true;
}

void AbstractSettingTab::onLanguageChanged()
{

}
