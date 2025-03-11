#include "AbstractSettingPage.h"
#include "AbstractSettingTab.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "MsGlobal.h"
#include "networkcommond.h"

AbstractSettingPage::AbstractSettingPage(QWidget *parent)
    : MsWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
}

void AbstractSettingPage::back()
{
    emit sig_back();
}

void AbstractSettingPage::showWait(QWidget *parent)
{
    Q_UNUSED(parent)
    //MsWaitting::showGlobalWait(parent);
}

void AbstractSettingPage::execWait(QWidget *parent)
{
    MsWaitting::execGlobalWait(parent);
}

void AbstractSettingPage::closeWait()
{
    //MsWaitting::closeGlobalWait();
}

void AbstractSettingPage::initializeData()
{
}

void AbstractSettingPage::dealMessage(MessageReceive *message)
{
    AbstractSettingTab *tab = m_tabMap.value(m_currentTab);
    if (tab) {
        tab->dealMessage(message);
    }
}

bool AbstractSettingPage::isCloseable()
{
    return true;
}

void AbstractSettingPage::closePage()
{
}

bool AbstractSettingPage::isChangeable()
{
    return true;
}

bool AbstractSettingPage::canAutoLogout()
{
    return true;
}

NetworkResult AbstractSettingPage::dealNetworkCommond(const QString &commond)
{
    Q_UNUSED(commond)

    return NetworkReject;
}

void AbstractSettingPage::onLanguageChanged()
{
}
