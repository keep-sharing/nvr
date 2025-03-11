#include "abstractcamerapage.h"
#include "MsWaitting.h"

AbstractCameraPage::AbstractCameraPage(QWidget *parent) :
    MsWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

void AbstractCameraPage::back()
{
    emit sig_back();
}

void AbstractCameraPage::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

bool AbstractCameraPage::isCloseable()
{
    return true;
}

bool AbstractCameraPage::isChangeable()
{
    return true;
}

bool AbstractCameraPage::canAutoLogout()
{
    return true;
}

void AbstractCameraPage::showWait()
{
    //MsWaitting::showGlobalWait();
}

void AbstractCameraPage::closeWait()
{
    //MsWaitting::closeGlobalWait();
}
