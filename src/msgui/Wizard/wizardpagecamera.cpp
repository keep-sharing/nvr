#include "wizardpagecamera.h"
#include "ui_wizardpagecamera.h"

WizardPageCamera::WizardPageCamera(QWidget *parent) :
    AbstractWizardPage(parent),
    ui(new Ui::WizardPageCamera)
{
    ui->setupUi(this);

    ui->deviceSearch->setShowInWizard();
}

WizardPageCamera::~WizardPageCamera()
{
    delete ui;
}

void WizardPageCamera::initializeData()
{
    ui->deviceSearch->initializeData();
}

void WizardPageCamera::saveSetting()
{

}

void WizardPageCamera::previousPage()
{
    showWizardPage(Wizard_Disk);
}

void WizardPageCamera::nextPage()
{
    showWizardPage(Wizard_P2P);
}

void WizardPageCamera::skipWizard()
{
    AbstractWizardPage::skipWizard();
}

void WizardPageCamera::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}
