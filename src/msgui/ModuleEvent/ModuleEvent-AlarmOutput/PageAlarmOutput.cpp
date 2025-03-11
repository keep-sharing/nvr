#include "PageAlarmOutput.h"
#include "ui_PageAlarmOutput.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "TabCameraAlarmOutput.h"
#include "TabNvrAlarmOutput.h"
#include "channelcopydialog.h"

PageAlarmOutput::PageAlarmOutput(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PageAlarmOutput)
{
    ui->setupUi(this);

    if (qMsNvr->maxAlarmOutput() > 0) {
        ui->tabBar->addTab(GET_TEXT("ALARMOUT/53005", "NVR Alarm Output"), TAB_NVR_ALARM_OUTPUT);
    }
    ui->tabBar->addTab(GET_TEXT("ALARMOUT/53006", "Camera Alarm Output"), TAB_CAMERA_ALARM_OUTPUT);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

PageAlarmOutput::~PageAlarmOutput()
{
    delete ui;
}

void PageAlarmOutput::initializeData()
{
    if (qMsNvr->maxAlarmOutput() > 0) {
        m_currentTab = TAB_NVR_ALARM_OUTPUT;
    } else {
        m_currentTab = TAB_CAMERA_ALARM_OUTPUT;
    }
    ui->tabBar->setCurrentTab(m_currentTab);
}

void PageAlarmOutput::onLanguageChanged()
{
}

void PageAlarmOutput::onTabClicked(int index)
{
    m_currentTab = index;
    AbstractSettingTab *tabWidget = m_tabMap.value(m_currentTab);
    if (!tabWidget) {
        switch (m_currentTab) {
        case TAB_NVR_ALARM_OUTPUT:
            tabWidget = new TabNvrAlarmOutput(this);
            break;
        case TAB_CAMERA_ALARM_OUTPUT:
            tabWidget = new TabCameraAlarmOutput(this);
            break;
        }
        if (tabWidget) {
            connect(tabWidget, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
            m_tabMap.insert(m_currentTab, tabWidget);
        } else {
            return;
        }
    }
    //
    QLayoutItem *item = ui->gridLayout->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayout->removeItem(item);
        delete item;
    }
    ui->gridLayout->addWidget(tabWidget, 0, 0);
    tabWidget->show();
    tabWidget->initializeData();
}
