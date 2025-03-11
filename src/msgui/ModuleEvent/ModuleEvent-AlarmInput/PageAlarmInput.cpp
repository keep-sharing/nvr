#include "PageAlarmInput.h"
#include "ui_PageAlarmInput.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "TabCameraAlarmInput.h"
#include "TabNvrAlarmInput.h"

PageAlarmInput::PageAlarmInput(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PageAlarmInput)
{
    ui->setupUi(this);

    if (qMsNvr->maxAlarmInput() > 0) {
        ui->tabBar->addTab(GET_TEXT("ALARMIN/52017", "NVR Alarm Input"), TAB_NVR_ALARM_INPUT);
    }
    ui->tabBar->addTab(GET_TEXT("ALARMIN/52018", "Camera Alarm Input"), TAB_CAMERA_ALARM_INPUT);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

PageAlarmInput::~PageAlarmInput()
{
    delete ui;
}

void PageAlarmInput::initializeData()
{
    if (qMsNvr->maxAlarmInput() > 0) {
        m_currentTab = TAB_NVR_ALARM_INPUT;
    } else {
        m_currentTab = TAB_CAMERA_ALARM_INPUT;
    }
    ui->tabBar->setCurrentTab(m_currentTab);
}

void PageAlarmInput::onLanguageChanged()
{
}

void PageAlarmInput::onTabClicked(int index)
{
    m_currentTab = index;
    AbstractSettingTab *tabWidget = m_tabMap.value(m_currentTab);
    if (!tabWidget) {
        switch (m_currentTab) {
        case TAB_NVR_ALARM_INPUT:
            tabWidget = new TabNvrAlarmInput(this);
            break;
        case TAB_CAMERA_ALARM_INPUT:
            tabWidget = new TabCameraAlarmInput(this);
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
