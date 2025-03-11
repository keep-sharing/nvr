#include "systeminfowidget.h"
#include "ui_systeminfowidget.h"
#include "MyDebug.h"
#include "MyDebugDialog.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "settingcontent.h"

SystemInfoWidget::SystemInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SystemInfoWidget)
{
    ui->setupUi(this);

    m_timerSystem = new QTimer(this);
    m_timerSystem->setInterval(5000);
    connect(m_timerSystem, SIGNAL(timeout()), this, SLOT(onTimerSystem()));
    m_timerSystem->start();
    onTimerSystem();

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

SystemInfoWidget::~SystemInfoWidget()
{
    delete ui;
}

void SystemInfoWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    MyDebugDialog::instance().show();
}

void SystemInfoWidget::onTimerSystem()
{
    
}

void SystemInfoWidget::onLanguageChanged()
{
    ui->label_cpu->setText(GET_TEXT("LIVEVIEW/20074", "CPU"));
    ui->label_memory->setText(GET_TEXT("LIVEVIEW/20075", "Memory"));
}
