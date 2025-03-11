#include "abstractwizardpage.h"

#include "MsWaitting.h"
#include "mswizard.h"

#include <QtDebug>

AbstractWizardPage::AbstractWizardPage(QWidget *parent)
    : MsWidget(parent)
{
    m_waitting = new MsWaitting(this);
    m_waitting->setWindowModality(Qt::ApplicationModal);
    m_waitting->hide();

    setFocusPolicy(Qt::StrongFocus);
}

void AbstractWizardPage::previousPage()
{
}

void AbstractWizardPage::nextPage()
{
}

void AbstractWizardPage::skipWizard()
{
    MsWizard::instance()->skipWizard();
}

void AbstractWizardPage::setWizardMode(const WizardMode &mode)
{
    MsWizard::instance()->setWizardMode(mode);
}

WizardMode AbstractWizardPage::wizardMode() const
{
    return MsWizard::instance()->wizardMode();
}

void AbstractWizardPage::showWizardPage(const WizardType &type)
{
    MsWizard::instance()->showWizardPage(type);
}

void AbstractWizardPage::finishWizard()
{
    MsWizard::instance()->finishWizard();
}
